/*
 * persiana.c
 *	MIEEC - SBMI
 *	PROJETO FINAL - CONTROLO DE UMA PERSIANA
 *  Created on: 8 Nov 2018
 *      Author: Inês Soares(up201606615) e Silvia Faria(up201603368)
 *
 *  FUNCIONAMENTO NORMAL
 * 		Motor parado: MOV=1
 * 		Persiana abre: MOV=0, DIR=1
 * 		Persiana fecha: MOV=0, DIR=0
 *
 *  State0: PARADO
 *  	MOV=1
 *  	se botao de abrir ativado -> state1
 *  	se botao de fechar ativado -> state4
 *
 *  State1: ABRIR
 *  	MOV=0
 *  	DIR=1
 *  	persiana abre e fica neste estado até que o botao de abrir seja desativado -> state10
 *
 *  State10: VERIFICA BOTAO DE ABRIR
 *  	MOV=O -> persiana  continua a subir
 *  	DIR=1		enquanto verifica o tempo
 *  	se o tempo que se carregou no botao para abrir for:
 *  		- curto -> deve abrir até ao fim -> state2
 *  		- longo -> deve parar de abrir -> state11
 *
 *  State11: VERIFICA TEMPO DE ABRIR
 *  	MOV=1 -> para motor
 *  	se o tempo que esteve a abrir for:
 *  		- superior ao tempo que a persiana demora a abrir completamente -> state3
 *  		- inferior ao tempo que a persiana demora a abrir completamente -> state0
 *
 *  State2: CONTINUA A ABRIR
 *  	MOV=0
 *  	DIR=1
 *  	persiana abre completamente -> state3
 *
 *  State3: COMPLETAMENTE ABERTO
 *  	MOV=1 -> para motor quando está completamente aberta (tempo superior ao tempo total de abertura)
 *  	se botao de fechar ativado -> state4
 *
 *  State4: FECHAR
 *  	MOV=0
 *  	DIR=0
 *  	persiana fecha e fica neste estado até que o botao de fechar seja desativado -> state40
 *
 *  State40: VERIFICA BOTAO FECHAR
 *  	MOV=0
 *  	DIR=0
 *  	se o tempo que se carregou no botao para fechar for:
 *  		- curto -> deve fechar até ao fim -> state5
 *  		- longo -> deve parar de fechar -> state41
 *
 *  State41: VERIFICA TEMPO ABRIR
 *  	MOV=1
 *  	se o tempo que esteve a fechar for:
 *  		- superior ao tempo que a persiana demora a fechar completamente -> state6
 *  		- inferior ao tempo que a persiana demora a fechar completamente -> state0
 *
 *  State5: CONTINUA A FECHAR
 *  	MOV=0
 *  	DIR=0
 *  	persiana fecha completamente -> state6
 *
 *  State6: COMPLETAMENTE FECHADO
 *  	MOV=1 -> para motor quando está completamente fechada (tempo superior ao tempo total de fecho)
 *  	se botao de abrir ativado -> state1
 *
 * CÁLCULO DO TIMER
 *	Fclk=16MHz  (frequência do cristal)
 *	Tintr=10 ms
 *	Escolheu-se o timer1, TC1
 *	CP*TP*CNT=10m*16M=160000
 *	Escolhendo CP=1, TP= {1, 8, 64, 256, 1024}
 *
 *	Tem-se as seguintes hipóteses:
 *		(1) TP=1  CNT=160000 (que não funciona pois não cabe em 16 bits)
 *		(2) TP=8  CNT=20000
 *		(3) TP=64  CNT=2500
 *		(4) TP=256  CNT=625
 *		(5) TP=1024  CNT=156,25 (tem erro associado)
 *
 *	Das hipóteses válidas (2-4), escolheu-se aquela que interrompe menos vezes a função principal (main),
 *	ou seja, energia mais baixa que é a quarta hipótese.
 *
 *	Como se usou o modo normal, a contagem foi feita de X até 65535, em que X=65535-CNT, ou seja, X=65535-625.
 *	E o prescaler foi 4 (valor retirado na datasheet do micro) devido a TP=1024.
 *	A interrupção era feita quando ocorresse overflow.
 *
 *	IMPLEMENTAÇÃO DA PORTA SÉRIE
 *
 *	Comunicação série no ATMEGA328p -> USART (Universal Synchronous Receiver Transmitter)
 *	Comunicação série assíncrona -> sinais: TxD e RxD
 *		- 1 start bit
 *		- 5 a 9 data bits e 1 de paridade
 *		- 1 ou 2 stop bits
 *	- Cálculo do UBRR0:
 *		- frequência CPU: fcpu=fosc/CP -> fcpu=16MHz (CP=1)
 *		- BAUD: 57600
 *		- k=16 (modo normal)
 *		- UBRR0= f/(k*BAUD)-1 = 16
 *
 *	Tecla do computador		Função
 *		'a'					- abre persiana
 *		'f'					- fecha persiana
 *		' '	(espaço)		- pára persiana
 *		'0'					- persiana abre completamente e a seguir fecha até estarem apenas os furos abertos
 *		'1'					- persiana abre completamente e a seguir fecha até estar apenas a um 1dm do chão
 *		'2'					- persiana abre completamente e a seguir fecha até estar apenas a um 2dm do chão
 *		'3'					- persiana abre completamente e a seguir fecha até estar apenas a um 3dm do chão
 *		'4'					- persiana abre completamente e a seguir fecha até estar apenas a um 4dm do chão
 *		'5'					- persiana abre completamente e a seguir fecha até estar apenas a um 5dm do chão
 *		'6'					- persiana abre completamente e a seguir fecha até estar apenas a um 6dm do chão
 *		'7'					- persiana abre completamente e a seguir fecha até estar apenas a um 7dm do chão
 *		'8'					- persiana abre completamente e a seguir fecha até estar apenas a um 8dm do chão
 *
 *	 *	OBSERVAÇÕES
 *	- Compilação condicional: utilizada nas funções de imprimir. Estas apenas são compiladas de a variável
 *								DEBUG estiver definida. Após o programa estar a funcionar, as suas funcionalidades
 *								já não são necessárias então, não se define a variável DEBUG e as funções não são
 *								compiladas, o que leva a que o programa ocupe menos espaço na memória.
 *	- Variáveis voláteis: as variáveis foram declaradas como voláteis, uma vez que se altera o seu valor dentro
 *							e fora da função main, ou seja, convém que estas variáveis não sejam otimizadas pelo sistema.
 *
 *	CONTROLO DA PERSIANA ATRAVÉS DE TELECOMANDO (Protocolo RC5)
 *	Quando se carrega num botão é enviado um quadro de bits para o arduino,
 *	neste caso através do pino PD2 (correnspondente ao INT0, pino das interrupções externas).
 *	Este quadro de bits é constituído por 14 bits (2 start bits, 1 toggle, 5 bits de endereço
 *	e 6 bits de dados). Para o controlo da persiana basta saber o valor dos bits de dados, uma
 *	vez que só interessa saber em que botão se carregou. Quando é pressionado um botão do comando,
 *	o sistema entra na interrupção do INT0, uma vez que o sensor infravermelhos está ligado ao
 *	pino PD2. Na interrupção desliga-se as interrupções externas. De seguida, configura-se o timer 2
 *	para o tempo de 3/4 de bit e começa-se o timer com TP=1024. Posteriormente, lê-se o bit e de
 *	acordo com o seu valor e o bit que for guarda-se ou não o seu valor numa variável.
 *	Resumidamente, o valor do bit é lido a meio da segunda metade do bit, ou seja, quando se lê o
 *	seu valor, é necessário considerá-lo negado para que o seu valor seja o correto.
 *	A interrupção é ativa tanto no flanco ascendente como no flanco descendente, depois espera-se 3/4
 *	de bit para ler o valor e no próximo flanco é feita outra interrução. Deste modo, os erros temporais
 *	não são acumulativos como se se esperasse o tempo de 1 bit para ler o seguinte.
 *	Para a leitura da trama de dados construiu-se uma espécie de máquina de estados sequencial, abaixo representada.
 *	No entanto, apenas interessam os estados 0 e 1 (startbits) para distinguir se o que estamos a ler é o quadro de dados
 *	ou é apenas ruído, o estado 2 onde guardamos o valor do toggle que varia sempre que é pressionada um botão,
 *	e, por fim, os estados 8 a 13 que contém o valor do botão.
 *
 *	Máquina de estados de leitura dos bits:
 *
 *		-Estados 0 e 1: Interpreta valor de start bit
 *			-se !startbit=1 -> passa para estado 2 (para ler o resto dos dados)
 *			-se !startbit=0 -> passa para estado 14 (onde não faz nada, significa que o que
 *			foi lido era ruído)
 *
 *		-Estado 2: Lê o bit de toggle
 *			-> estado 3
 *
 *		-Estado 8: Grava o primeiro bit de dados
 *			-> estado 9
 *
 *		-Estado 9: Grava o segundo bit de dados
 *			-> estado 10
 *
 *		-Estado 10: Grava o terceiro bit de dados
 *			-> estado 11
 *
 *		-Estado 11: Grava o quarto bit de dados
 *			-> estado 12
 *
 *		-Estado 12: Grava o quinto bit de dados e espera o tempo de 1 bit para ler o próximo
 *			-> estado 13
 *
 *		-Estado 13: Grava o sexto e último bit de dados
 *
 *	Este timer utilizado para contar 3/4 de bit foi configurado com um CP=1, TP=1024 e CNT=21, uma vez que 3/4 de bit
 *	corresponde a 1360us. Com estes valores de prescalers, o valor de 3/4 de bit contabilizado é de 1344us, o que não é relevante,
 *	uma vez que a leitura continua a ser feira na segunda metade do bit (apenas um erro de 1.18%).
 *
 *  A tabela seguinte mostra as configurações definidas para os botões do comando definidos para controlar a persiana.
 *
 *	FUNÇÃO NO COMANDO		VALOR DOS DADOS		FUNÇÃO NA PERSIANA		PRÓXIMO ESTADO
 *	aumentar volume			16					abrir					10
 *	reduzir volume			17					fechar					40
 *	sem volume				13					parar					0
 *	tecla 5					5					fechar até meio			10
 *
 *	Para cumprir esta função de fechar até meio, a persiana abre completamente (por razões de segurança) e, seguidamente,
 *	fecha até meio.
 *
 */

/*BIBLIOTECAS*/
#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"
#include <avr/interrupt.h>


/*TEMPOS DE FECHO PARA CADA POSIÇÃO DA PERSIANA*/
#define CF 1400 	/*Tempo total de fecho*/
#define TF0 1088	/*tempo de fechar até estarem apenas os furos abertos*/
#define TF1 910		/*tempo de fechar até estar apenas a 1 dm do chão*/
#define TF2 770		/*tempo de fechar até estar apenas a 2 dm do chão*/
#define TF3 621		/*tempo de fechar até estar apenas a 3 dm do chão*/
#define TF4 496		/*tempo de fechar até estar apenas a 4 dm do chão*/
#define TF5 385		/*tempo de fechar até estar apenas a 5 dm do chão*/
#define TF6 276		/*tempo de fechar até estar apenas a 6 dm do chão*/
#define TF7 170		/*tempo de fechar até estar apenas a 7 dm do chão*/
#define TF8 81		/*tempo de fechar até estar apenas a 8 dm do chão*/
#define CA 1500 	/*Tempo total abertura*/

#define TB 50 /*Tempo que o botão foi pressionado | Impulso do botão*/

/*CONFIGURAÇÃO DOS PINOS DE LIGAÇÃO*/
/*Entradas*/
#define BA PD7	/*BOTÃO DE ABERTURA| Ativo a 0*/
#define BF PD6	/*BOTÃO DE FECHO | Ativo a 0*/
#define COM PD2 /*COMANDO TELEVISÃO*/
/*Saídas*/
#define DIR PB2	/*DIREÇÃO DO MOTOR | DIR=0 -> Desce | DIR=1 -> Sobe*/
#define MOV PB3	/*MOTOR DA PERSIANA | Ativo a 0*/

/*DEFINIÇÃO DE TEMPOS*/
#define T1BOTTOM 65536-625 	/*CNT=625*/
#define T2BOTTOM34 256-21	/*CNT=21*/	/*Tempo de leitura de 3/4 de bit*/

#define DEBUG	/*Compilação condicional*/
#define LEGENDA

/*DEFINIÇÃO DO BAUD RATE*/
#ifndef F_CPU
#define F_CPU 16000000ul
#endif
#define BAUD 57600
#define UBBR_VAL ((F_CPU/(BAUD<<4))-1)

/*DEFINIÇÃO DOS BOTÕES DOS COMANDOS DA PORTA SÉRIE*/
#define ABRIR 'a'	/*Persiana abre*/
#define FECHAR 'f'	/*Persiana fecha*/
#define STOP ' '	/*Persiana pára movimento*/
#define FUROS '0'	/*Persiana fecha até estarem apenas os furos abertos*/
#define DM1 '1'		/*DM1: persiana fecha até ficar a 1dm do chão*/
#define DM2 '2'		/*DM2: persiana fecha até ficar a 2dm do chão*/
#define DM3 '3'		/*DM3: persiana fecha até ficar a 3dm do chão*/
#define DM4 '4'		/*DM4: persiana fecha até ficar a 4dm do chão*/
#define DM5 '5'		/*DM5: persiana fecha até ficar a 5dm do chão*/
#define DM6 '6'		/*DM6: persiana fecha até ficar a 6dm do chão*/
#define DM7 '7'		/*DM7: persiana fecha até ficar a 7dm do chão*/
#define DM8 '8'		/*DM8: persiana fecha até ficar a 8dm do chão*/

/*VARIÁVEIS DO PROGRAMA*/
volatile unsigned char  state=0, nstate=0, aux=1, bit=0, togg=0;
volatile uint16_t t_botao=0, t_per=0 , dados=0, dados_prev=50;
volatile uint8_t cont;
/*
 * state: variável da máquina de estados
 * nstate: próximo estado (next state)
 * aux: variável que verifica se o estado da máquina já foi escrito para não repetir
 * t_botao: tempo de impulso do botao
 * t_per: tempo que a presiana está ligada
 * t_com: tempo para leitura de bit
 * dados: variável que guarda o valor do botão do telecomando que foi pressionado
 * dados_prev (dados previous): guarda o valor da variável dados para ser utilizado posteriormente
 * bit: valor do bit lido do quadro de bits
 * togg: valor do bit toggle
 * cont: variável da máquina de estados na interrupção pertencente ao controlo por telecomando
 */

/*INICIALIZAÇÃO DO TIMER 1 - para contabilizar tempo no funcionamento normal*/
void tc1_init(void){
	TCCR1B=0;							/*PARA TC1*/
	TIFR1=(7<<TOV1) | (1<<ICF1);		/*LIMPA INTERRUPCOES PENDENTES*/
	TCCR1A=0;							/*MODO NORMAL*/
	TCNT1=T1BOTTOM;						/*CARREGA VALOR DE BOTTOM*/
	TIMSK1=(1<<TOIE1);					/*ATIVA INTERRUPCAO DE OVERFLOW*/
	TCCR1B=4;							/*START TC1 (TP=256)*/
}

/*INICIALIZAÇÃO DO TIMER 2 - para leitura de bits*/
void tc2_init(void){
	TCCR2B=0;							/*PARA TC2*/
	TIFR2=(7<<TOV2);					/*LIMPA INTERRUPCOES PENDENTES*/
	TCCR2A=0;							/*MODO NORMAL*/
	TIMSK2=(1<<TOIE2);					/*ATIVA INTERRUPCAO DE OVERFLOW*/
}

/*INICIALIZAÇÃO DA CONFIGURAÇÃO DA PORTA SÉRIE*/
void init_usart(void){
	/*Definir baudrate*/
	UBRR0=UBBR_VAL;

	/*Definir formato da trama*/
	UCSR0C=(3<<UCSZ00)		/*8 data bits*/
			| (0<<UPM00)	/*Não há paridade*/
			| (0<<USBS0);	/*1 stop bit*/

	/*Ativar Rx, Tx e interrupt Rx*/
	UCSR0B=(1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
}

/*INICIALIZAÇÃO DO SISTEMA (ENTRADAS E SAÍDAS)*/
void hw_init(void){
	DDRB= DDRB | (1<<MOV) | (1<<DIR) | (1<<PB1); 		/*Define variável de motor e direção como saídas*/
	DDRD = DDRD & ~((1<<BA) | (1<<BF) | (1<<COM)); 		/*Define como entradas o botão de abrir e fechar e o pino de recação de bits do telecomando*/
	PORTD = PORTD | (1<<BA) | (1<<BF) | (1<<COM); 		/*Ativa pull-ups internos das entradas*/
	EICRA = EICRA | (1<<ISC00); 						/*Pedido de interrupção em both edges*/
	EIMSK = EIMSK | (1<<INT0);							/*Ativa interrupções no INT0*/

	tc1_init();		/*Inicializa timer 1*/
	tc2_init();		/*Inicializa timer 2*/
	init_usart();	/*Inicializa porta série*/
	printf_init();	/*Inicializa impressão na consola*/
}

/*INTERRUPÇÃO DO TIMER1*/	/*base de tempo 10ms*/
ISR(TIMER1_OVF_vect){
	TCNT1=T1BOTTOM; /*RECARREGA TC1*/
	if(t_botao) t_botao--;		/*Decrementa variável de contagem do tempo de pressão do botão*/
	if(t_per) t_per--;			/*Decrementa variável de contagem do tempo da persiana*/

}

/*INTERRUPÇÃO DO TIMER2*/	/*base de tempo 64 us*/
ISR(TIMER2_OVF_vect){
	EIFR=EIFR|(1<<INTF0);		/*Limpa pedidos pendentes*/
	EIMSK = EIMSK | (1<<INT0);	/*Ativa interrupções no INT0*/
	TCCR2B=0;					/*Pára timer*/
}

/*INTERRUPÇÃO DA PORTA SÉRIE*/
ISR(USART_RX_vect){
	unsigned char RecByte;
	RecByte=UDR0;	/*Guarda bit recebido pela porta série*/
	UDR0=RecByte;	/*Devolve bit*/
	if(RecByte==ABRIR){
		nstate=10;			/*ABRE PERSIANA*/
		t_botao=TB;			/*Para que a persiana abra até ao fim e não fique à espera do tempo do botão que neste caso é irrelevante*/
		t_per=CA;			/*Para que a persiana abra completamente*/
	}
	else if(RecByte==FECHAR){
		nstate=40;			/*FECHA PERSIANA*/
		t_botao=TB;			/*Para que a persiana feche até ao fim e não fique à espera do tempo do botão que neste caso é irrelevante*/
		t_per=CF;			/*Para que a persiana feche completamente*/
	}
	else if(RecByte==STOP){
		nstate=0;			/*PERSIANA PÁRA*/
		t_botao=TB;
		t_per=CA;
		dados_prev=50;	/*limpa variável*/
	}
	else if(RecByte==FUROS){
		nstate=10;			/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAREM APENAS OS FUROS ABERTOS*/
		t_botao=TB;		/*Para que a persiana abra até ao fim e não fique à espera do tempo do botão que neste caso é irrelevante*/
		t_per=CA;		/*Persiana abre completamente*/
		dados_prev=0;	/*Grava a variável lida para quando a persiana abrir completamente, o sistema saber o que tem de fazer a seguir*/
	}
	else if(RecByte==DM1){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAR APENAS A 1 DM DO CHÃO*/
		t_botao=TB;
		t_per=CA;
		dados_prev=1;
	}
	else if(RecByte==DM2){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAR APENAS A 2 DM DO CHÃO*/
		t_botao=TB;
		t_per=CA;
		dados_prev=2;
	}
	else if(RecByte==DM3){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAR APENAS A 3 DM DO CHÃO*/
		t_botao=TB;
		t_per=CA;
		dados_prev=3;
	}
	else if(RecByte==DM4){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAR APENAS A 4 DM DO CHÃO*/
		t_botao=TB;
		t_per=CA;
		dados_prev=4;
	}
	else if(RecByte==DM5){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAR APENAS A 5 DM DO CHÃO*/
		t_botao=TB;
		t_per=CA;
		dados_prev=5;
	}
	else if(RecByte==DM6){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAR APENAS A 6 DM DO CHÃO*/
		t_botao=TB;
		t_per=CA;
		dados_prev=6;
	}
	else if(RecByte==DM7){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAR APENAS A 7 DM DO CHÃO*/
		t_botao=TB;
		t_per=CA;
		dados_prev=7;
	}
	else if(RecByte==DM8){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR ATÉ FICAR APENAS A 8 DM DO CHÃO*/
		t_botao=TB;
		t_per=CA;
		dados_prev=8;
	}
}

/*INTERRUPÇÃO PARA LEITURA DE BITS ENVIADOS PELO TELECOMANDO*/
ISR(INT0_vect){
	EIMSK = 0;				/*Desativa as interrupções*/
	TCNT2=T2BOTTOM34;		/*Configura timer para tempo de 3/4 de bit*/
	TCCR2B=7;				/*Inicia timer com TP=1024*/
	bit=(PIND&(1<<COM));	/*Lê valor do bit*/
		if(cont<2){		/*Se valor de startbit (cont<2) for um, interpreta os dados como ruído e ignora leitura*/
			if(bit) cont=14;
		}
		else{			/*Se valor de startbit for zero, prossegue com a leitura dos dados*/
			if(cont==2){
				togg=bit;	/*Guarda valor do toggle bit na variável togg*/
				dados=0;	/*Limpa a variável dados para que de seguida a leitura seja correta*/
			}
			else if(cont>7 && cont<14){	/*Lê valor dos bits de dados (últimos 6 bits da trama)*/
				dados=(dados<<1);	/*Faz shift da variável para guardar o bit menos significativo e não perder o anterior*/
				if(!bit) dados++;	/*Se o bit for 0 (como o sinal está negado, significa que o bit é 1), põe um 1 no bit mais à direita da variável*/
			}
		}
		if(cont<13) cont++;	/*Incrementa variável da máquina de estados para ler bit seguinte*/
		else{
			cont=0;	/*Quando acaba de ler os bits da trama, põe a variável da máquina de estados a 0*/
		}
}

/*FUNÇÃO PRINCIPAL*/
int main(void){

	hw_init();  /*Inicialização do sistema*/
	sei();		/*Ativa interrupções externas*/

	while(1){

/*IMPRIME NA CONSOLA*/
#ifdef DEBUG
			if(aux!=state) {
				printf(" Estado: %u \n",state);
				aux=state;
			}
#endif
/*COMANDOS RECEBIDOS PELO TELECOMANDO*/
		switch(dados){
		case 16:{		/*Se carregar em aumentar o volume do comando (dados=16)*/
			nstate=10;	/*Persiana abre*/
			t_botao=TB;	/*Para que a persiana abra até ao fim e não fique à espera do tempo do botão que neste caso é irrelevante*/
			t_per=CA;	/*Abre completamente*/
			dados=0;	/*Limpa variável de dados*/
		}break;
		case 17:{		/*Se carregar em reduzir o volume do comando (dados=17)*/
			nstate=40;	/*Persiana fecha*/
			t_botao=TB;	/*Para que a persiana abra até ao fim e não fique à espera do tempo do botão que neste caso é irrelevante*/
			t_per=CF;	/*Fecha completamente*/
			dados=0;	/*Limpa variável de dados*/
		}break;
		case 13:{		/*Se carregar em retirar o som do comando (dados=13)*/
			nstate=0;	/*Persiana pára*/
			t_botao=0;
			t_per=0;
			dados=0;	/*Limpa variável de dados*/
			dados_prev=50;	/*limpa variável*/
		}break;
		case 5:{		/*Se carregar na tecla 5 do comando (dados=5)*/
			nstate=10;	/*Persiana abre completamente para depois fechar apenas até metade*/
			t_botao=TB;	/*Para que a persiana abra até ao fim e não fique à espera do tempo do botão que neste caso é irrelevante*/
			t_per=CA;	/*Abre completamente*/
			dados_prev=dados;	/*Grava valor de dados para quando a persiana abrir completamente, o sistema saber o que tem de fazer de seguida*/
			dados=0;	/*Limpa variável de dados*/
		}break;
		}
		state=nstate; /*Atualiza valor da máquina de estados*/

/*MÁQUINA DE ESTADOS - MOVIMENTO DA PERSIANA*/
		switch(state){
		/* ESTADO 0*/
		case 0:{ 						/*PARADO*/
			PORTB=PORTB | (1<<MOV);			/*MOV=1*/
			if(!(PIND&(1<<BA))){	/*Se carregar no botão de abrir*/
				nstate=1;			/*Persiana abre*/
				t_botao=TB;			/*Começa a contar o tempo que o botão é pressionado*/
				t_per=CA;			/*Começa a contar o tempo que a persiana está a abrir*/
			}
			else if(!(PIND&(1<<BF))){	/*Se carregar no botão de fechar*/
				nstate=4;				/*Persiana fecha*/
				t_botao=TB;				/*Começa a contar o tempo que o botão é pressionado*/
				t_per=CF;				/*Começa a contar o tempo que a persiana está a fechar*/
			}
		}break;
		/* ESTADO 1*/
		case 1:{						/*ABRIR*/
			PORTB=PORTB & ~(1<<MOV); 		/*MOV=0*/
			PORTB=PORTB | (1<<DIR); 		/*DIR=1*/
			if(PIND&(1<<BA)){		/*Se largar o botão de abrir*/
				nstate=10;
			}
		}break;
		/* ESTADO 10*/
		case 10:{						/*VERIFICA BOTAO ABRIR - mas continua a abrir*/
			PORTB=PORTB & ~(1<<MOV); 		/*MOV=0*/
			PORTB=PORTB | (1<<DIR); 		/*DIR=1*/
#ifdef LEGENDA
	printf(" \n                                        ABRIR \n");
#endif
			if(t_botao==0){
				nstate=11;	/*longo impulso -> pára de abrir*/
			}
			else{
				nstate=2;	/*curto impulso -> abre ate ao fim*/
			}
		}break;
		/* ESTADO 11*/
		case 11:{						/*VERIFICA TEMPO ABRIR*/
			PORTB=PORTB | (1<<MOV);			/*MOV=1*/
			if(t_per==0){
				nstate=3; /*nao abriu completamente*/
			}
			else{
				nstate=0; /*abriu completamente*/
			}
		}break;
		/* ESTADO 2*/
		case 2:{						/*CONTINUAR ABRIR*/
			PORTB=PORTB & ~(1<<MOV); 		/*MOV=0*/
			PORTB=PORTB | (1<<DIR); 		/*DIR=1*/
			if(t_per==0){	/*Quando abre completamente verifica qual a etapa a cumprir a seguir*/
			/*COMANDOS DA PORTA SÉRIE E DO TELECOMANDO*/
				switch(dados_prev){	/*Se estiver a fechar até um determinado local (comando enviado pela porta série ou pelo telecomando), lê qual o valor em dados_prev*/
				case 0:{
					t_per=TF0;	/*Fecha até estarem apenas os furos abertos*/
					t_botao=TB;	/*Para que a persiana abra até ao fim e não fique à espera do tempo do botão que neste caso é irrelevante*/
					nstate=40;	/*Começa a fechar*/
				}break;
								/*Este procedimento repete-se para as restantes teclas, variando apenas o tempo de fecho*/
				case 1:{
					t_per=TF1;	/*Fecha até estar apenas a 1dm do chão*/
					t_botao=TB;
					nstate=40;
				}break;
				case 2:{
					t_per=TF2;	/*Fecha até estar apenas a 2dm do chão*/
					t_botao=TB;
					nstate=40;
				}break;
				case 3:{
					t_per=TF3;	/*Fecha até estar apenas a 3dm do chão*/
					t_botao=TB;
					nstate=40;
				}break;
				case 4:{
					t_per=TF4;	/*Fecha até estar apenas a 4dm do chão*/
					t_botao=TB;
					nstate=40;
				}break;
				case 5:{
					t_per=TF5;	/*Fecha até estar apenas a 5dm do chão*/
					t_botao=TB;
					nstate=40;
				}break;
				case 6:{
					t_per=TF6;	/*Fecha até estar apenas a 6dm do chão*/
					t_botao=TB;
					nstate=40;
				}break;
				case 7:{
					t_per=TF7;	/*Fecha até estar apenas a 7dm do chão*/
					t_botao=TB;
					nstate=40;
				}break;
				case 8:{
					t_per=TF8;	/*Fecha até estar apenas a 8dm do chão*/
					t_botao=TB;
					nstate=40;
				}break;
				default:{	/*Se estiver a cumprir o seu funcionamento normal*/
					nstate=3;	/*abre completamente*/
				}
				}
				dados_prev=50;	/*Limpa variável dados_prev*/
			}
		}break;
		/* ESTADO 3*/
		case 3:{						/*COMPLETAMENTE ABERTO*/
			PORTB=PORTB | (1<<MOV);			/*MOV=1*/
			if(!(PIND&(1<<BF))){	/*Se carregar no botão de fechar*/
				nstate=4;			/*Persiana fecha*/
				t_per=CF;			/*Começa a contar o tempo que a persiana está a fechar*/
				t_botao=TB;			/*Começa a contar o tempo que o botão é pressionado*/
			}
		}break;
		/* ESTADO 4*/
		case 4:{						/*FECHAR*/
			PORTB=PORTB & ~(1<<MOV); 		/*MOV=0*/
			PORTB=PORTB & ~(1<<DIR); 		/*DIR=0*/
			if(PIND&(1<<BF)){	/*Se largar o botão de fechar*/
				nstate=40;
			}
		}break;
		/* ESTADO 40*/
		case 40:{						/*VERIFICA BOTAO FECHAR - mas continua a fechar*/
			PORTB=PORTB & ~(1<<MOV); 		/*MOV=0*/
			PORTB=PORTB & ~(1<<DIR); 		/*DIR=0*/
#ifdef LEGENDA
	printf(" \n                                   FECHAR \n");
#endif
			if(t_botao==0){
				nstate=41;	/*longo impulso -> para de abrir*/
			}
			else{
				nstate=5;	/*curto impulso -> abre ate ao fim*/
			}
		}break;
		/* ESTADO 41*/
		case 41:{						/*VERIFICA TEMPO FECHAR*/
			PORTB=PORTB | (1<<MOV);			/*MOV=1*/
			if(t_per==0){
				nstate=6;	/*fechou completamente*/
			}
			else{
				nstate=0;	/*nao fechou completamente*/
			}
		}break;
		/* ESTADO 5*/
		case 5:{						/*CONTINUAR FECHAR*/
			PORTB=PORTB & ~(1<<MOV); 		/*MOV=0*/
			PORTB=PORTB & ~(1<<DIR); 		/*DIR=0*/
			if(t_per==0){
				nstate=6;	/*fechou completamente*/
			}
		}break;
		/* ESTADO 6*/
		case 6:{						/*COMPLETAMENTE FECHADO*/
			PORTB=PORTB | (1<<MOV);			/*MOV=1*/
			if(!(PIND&(1<<BA))){	/*Se carregar no botão de abrir*/
				nstate=1;			/*Persiana abre*/
				t_per=CA;			/*Começa a contar o tempo que a persiana está a abrir*/
				t_botao=TB;			/*Começa a contar o tempo que o botão é pressionado*/
			}
		}break;
		default:{		/*ESTADO ILEGAL - em caso de anomalia pára a persiana (state=0)*/
			nstate=0;
		}break;
		}
	}
}
