/*
 * persiana.c
 *	MIEEC - SBMI
 *	PROJETO FINAL - CONTROLO DE UMA PERSIANA
 *  Created on: 8 Nov 2018
 *      Author: In�s Soares(up201606615) e Silvia Faria(up201603368)
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
 *  	persiana abre e fica neste estado at� que o botao de abrir seja desativado -> state10
 *
 *  State10: VERIFICA BOTAO DE ABRIR
 *  	MOV=O -> persiana  continua a subir
 *  	DIR=1		enquanto verifica o tempo
 *  	se o tempo que se carregou no botao para abrir for:
 *  		- curto -> deve abrir at� ao fim -> state2
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
 *  	MOV=1 -> para motor quando est� completamente aberta (tempo superior ao tempo total de abertura)
 *  	se botao de fechar ativado -> state4
 *
 *  State4: FECHAR
 *  	MOV=0
 *  	DIR=0
 *  	persiana fecha e fica neste estado at� que o botao de fechar seja desativado -> state40
 *
 *  State40: VERIFICA BOTAO FECHAR
 *  	MOV=0
 *  	DIR=0
 *  	se o tempo que se carregou no botao para fechar for:
 *  		- curto -> deve fechar at� ao fim -> state5
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
 *  	MOV=1 -> para motor quando est� completamente fechada (tempo superior ao tempo total de fecho)
 *  	se botao de abrir ativado -> state1
 *
 * C�LCULO DO TIMER
 *	Fclk=16MHz  (frequ�ncia do cristal)
 *	Tintr=10 ms
 *	Escolheu-se o timer1, TC1
 *	CP*TP*CNT=10m*16M=160000
 *	Escolhendo CP=1, TP= {1, 8, 64, 256, 1024}
 *
 *	Tem-se as seguintes hip�teses:
 *		(1) TP=1  CNT=160000 (que n�o funciona pois n�o cabe em 16 bits)
 *		(2) TP=8  CNT=20000
 *		(3) TP=64  CNT=2500
 *		(4) TP=256  CNT=625
 *		(5) TP=1024  CNT=156,25 (tem erro associado)
 *
 *	Das hip�teses v�lidas (2-4), escolheu-se aquela que interrompe menos vezes a fun��o principal (main),
 *	ou seja, energia mais baixa que � a quarta hip�tese.
 *
 *	Como se usou o modo normal, a contagem foi feita de X at� 65535, em que X=65535-CNT, ou seja, X=65535-625.
 *	E o prescaler foi 4 (valor retirado na datasheet do micro) devido a TP=1024.
 *	A interrup��o era feita quando ocorresse overflow.
 *
 *	IMPLEMENTA��O DA PORTA S�RIE
 *
 *	Comunica��o s�rie no ATMEGA328p -> USART (Universal Synchronous Receiver Transmitter)
 *	Comunica��o s�rie ass�ncrona -> sinais: TxD e RxD
 *		- 1 start bit
 *		- 5 a 9 data bits e 1 de paridade
 *		- 1 ou 2 stop bits
 *	- C�lculo do UBRR0:
 *		- frequ�ncia CPU: fcpu=fosc/CP -> fcpu=16MHz (CP=1)
 *		- BAUD: 57600
 *		- k=16 (modo normal)
 *		- UBRR0= f/(k*BAUD)-1 = 16
 *
 *	Tecla do computador		Fun��o
 *		'a'					- abre persiana
 *		'f'					- fecha persiana
 *		' '	(espa�o)		- p�ra persiana
 *		'0'					- persiana abre completamente e a seguir fecha at� estarem apenas os furos abertos
 *		'1'					- persiana abre completamente e a seguir fecha at� estar apenas a um 1dm do ch�o
 *		'2'					- persiana abre completamente e a seguir fecha at� estar apenas a um 2dm do ch�o
 *		'3'					- persiana abre completamente e a seguir fecha at� estar apenas a um 3dm do ch�o
 *		'4'					- persiana abre completamente e a seguir fecha at� estar apenas a um 4dm do ch�o
 *		'5'					- persiana abre completamente e a seguir fecha at� estar apenas a um 5dm do ch�o
 *		'6'					- persiana abre completamente e a seguir fecha at� estar apenas a um 6dm do ch�o
 *		'7'					- persiana abre completamente e a seguir fecha at� estar apenas a um 7dm do ch�o
 *		'8'					- persiana abre completamente e a seguir fecha at� estar apenas a um 8dm do ch�o
 *
 *	 *	OBSERVA��ES
 *	- Compila��o condicional: utilizada nas fun��es de imprimir. Estas apenas s�o compiladas de a vari�vel
 *								DEBUG estiver definida. Ap�s o programa estar a funcionar, as suas funcionalidades
 *								j� n�o s�o necess�rias ent�o, n�o se define a vari�vel DEBUG e as fun��es n�o s�o
 *								compiladas, o que leva a que o programa ocupe menos espa�o na mem�ria.
 *	- Vari�veis vol�teis: as vari�veis foram declaradas como vol�teis, uma vez que se altera o seu valor dentro
 *							e fora da fun��o main, ou seja, conv�m que estas vari�veis n�o sejam otimizadas pelo sistema.
 *
 *	CONTROLO DA PERSIANA ATRAV�S DE TELECOMANDO (Protocolo RC5)
 *	Quando se carrega num bot�o � enviado um quadro de bits para o arduino,
 *	neste caso atrav�s do pino PD2 (correnspondente ao INT0, pino das interrup��es externas).
 *	Este quadro de bits � constitu�do por 14 bits (2 start bits, 1 toggle, 5 bits de endere�o
 *	e 6 bits de dados). Para o controlo da persiana basta saber o valor dos bits de dados, uma
 *	vez que s� interessa saber em que bot�o se carregou. Quando � pressionado um bot�o do comando,
 *	o sistema entra na interrup��o do INT0, uma vez que o sensor infravermelhos est� ligado ao
 *	pino PD2. Na interrup��o desliga-se as interrup��es externas. De seguida, configura-se o timer 2
 *	para o tempo de 3/4 de bit e come�a-se o timer com TP=1024. Posteriormente, l�-se o bit e de
 *	acordo com o seu valor e o bit que for guarda-se ou n�o o seu valor numa vari�vel.
 *	Resumidamente, o valor do bit � lido a meio da segunda metade do bit, ou seja, quando se l� o
 *	seu valor, � necess�rio consider�-lo negado para que o seu valor seja o correto.
 *	A interrup��o � ativa tanto no flanco ascendente como no flanco descendente, depois espera-se 3/4
 *	de bit para ler o valor e no pr�ximo flanco � feita outra interru��o. Deste modo, os erros temporais
 *	n�o s�o acumulativos como se se esperasse o tempo de 1 bit para ler o seguinte.
 *	Para a leitura da trama de dados construiu-se uma esp�cie de m�quina de estados sequencial, abaixo representada.
 *	No entanto, apenas interessam os estados 0 e 1 (startbits) para distinguir se o que estamos a ler � o quadro de dados
 *	ou � apenas ru�do, o estado 2 onde guardamos o valor do toggle que varia sempre que � pressionada um bot�o,
 *	e, por fim, os estados 8 a 13 que cont�m o valor do bot�o.
 *
 *	M�quina de estados de leitura dos bits:
 *
 *		-Estados 0 e 1: Interpreta valor de start bit
 *			-se !startbit=1 -> passa para estado 2 (para ler o resto dos dados)
 *			-se !startbit=0 -> passa para estado 14 (onde n�o faz nada, significa que o que
 *			foi lido era ru�do)
 *
 *		-Estado 2: L� o bit de toggle
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
 *		-Estado 12: Grava o quinto bit de dados e espera o tempo de 1 bit para ler o pr�ximo
 *			-> estado 13
 *
 *		-Estado 13: Grava o sexto e �ltimo bit de dados
 *
 *	Este timer utilizado para contar 3/4 de bit foi configurado com um CP=1, TP=1024 e CNT=21, uma vez que 3/4 de bit
 *	corresponde a 1360us. Com estes valores de prescalers, o valor de 3/4 de bit contabilizado � de 1344us, o que n�o � relevante,
 *	uma vez que a leitura continua a ser feira na segunda metade do bit (apenas um erro de 1.18%).
 *
 *  A tabela seguinte mostra as configura��es definidas para os bot�es do comando definidos para controlar a persiana.
 *
 *	FUN��O NO COMANDO		VALOR DOS DADOS		FUN��O NA PERSIANA		PR�XIMO ESTADO
 *	aumentar volume			16					abrir					10
 *	reduzir volume			17					fechar					40
 *	sem volume				13					parar					0
 *	tecla 5					5					fechar at� meio			10
 *
 *	Para cumprir esta fun��o de fechar at� meio, a persiana abre completamente (por raz�es de seguran�a) e, seguidamente,
 *	fecha at� meio.
 *
 */

/*BIBLIOTECAS*/
#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"
#include <avr/interrupt.h>


/*TEMPOS DE FECHO PARA CADA POSI��O DA PERSIANA*/
#define CF 1400 	/*Tempo total de fecho*/
#define TF0 1088	/*tempo de fechar at� estarem apenas os furos abertos*/
#define TF1 910		/*tempo de fechar at� estar apenas a 1 dm do ch�o*/
#define TF2 770		/*tempo de fechar at� estar apenas a 2 dm do ch�o*/
#define TF3 621		/*tempo de fechar at� estar apenas a 3 dm do ch�o*/
#define TF4 496		/*tempo de fechar at� estar apenas a 4 dm do ch�o*/
#define TF5 385		/*tempo de fechar at� estar apenas a 5 dm do ch�o*/
#define TF6 276		/*tempo de fechar at� estar apenas a 6 dm do ch�o*/
#define TF7 170		/*tempo de fechar at� estar apenas a 7 dm do ch�o*/
#define TF8 81		/*tempo de fechar at� estar apenas a 8 dm do ch�o*/
#define CA 1500 	/*Tempo total abertura*/

#define TB 50 /*Tempo que o bot�o foi pressionado | Impulso do bot�o*/

/*CONFIGURA��O DOS PINOS DE LIGA��O*/
/*Entradas*/
#define BA PD7	/*BOT�O DE ABERTURA| Ativo a 0*/
#define BF PD6	/*BOT�O DE FECHO | Ativo a 0*/
#define COM PD2 /*COMANDO TELEVIS�O*/
/*Sa�das*/
#define DIR PB2	/*DIRE��O DO MOTOR | DIR=0 -> Desce | DIR=1 -> Sobe*/
#define MOV PB3	/*MOTOR DA PERSIANA | Ativo a 0*/

/*DEFINI��O DE TEMPOS*/
#define T1BOTTOM 65536-625 	/*CNT=625*/
#define T2BOTTOM34 256-21	/*CNT=21*/	/*Tempo de leitura de 3/4 de bit*/

#define DEBUG	/*Compila��o condicional*/
#define LEGENDA

/*DEFINI��O DO BAUD RATE*/
#ifndef F_CPU
#define F_CPU 16000000ul
#endif
#define BAUD 57600
#define UBBR_VAL ((F_CPU/(BAUD<<4))-1)

/*DEFINI��O DOS BOT�ES DOS COMANDOS DA PORTA S�RIE*/
#define ABRIR 'a'	/*Persiana abre*/
#define FECHAR 'f'	/*Persiana fecha*/
#define STOP ' '	/*Persiana p�ra movimento*/
#define FUROS '0'	/*Persiana fecha at� estarem apenas os furos abertos*/
#define DM1 '1'		/*DM1: persiana fecha at� ficar a 1dm do ch�o*/
#define DM2 '2'		/*DM2: persiana fecha at� ficar a 2dm do ch�o*/
#define DM3 '3'		/*DM3: persiana fecha at� ficar a 3dm do ch�o*/
#define DM4 '4'		/*DM4: persiana fecha at� ficar a 4dm do ch�o*/
#define DM5 '5'		/*DM5: persiana fecha at� ficar a 5dm do ch�o*/
#define DM6 '6'		/*DM6: persiana fecha at� ficar a 6dm do ch�o*/
#define DM7 '7'		/*DM7: persiana fecha at� ficar a 7dm do ch�o*/
#define DM8 '8'		/*DM8: persiana fecha at� ficar a 8dm do ch�o*/

/*VARI�VEIS DO PROGRAMA*/
volatile unsigned char  state=0, nstate=0, aux=1, bit=0, togg=0;
volatile uint16_t t_botao=0, t_per=0 , dados=0, dados_prev=50;
volatile uint8_t cont;
/*
 * state: vari�vel da m�quina de estados
 * nstate: pr�ximo estado (next state)
 * aux: vari�vel que verifica se o estado da m�quina j� foi escrito para n�o repetir
 * t_botao: tempo de impulso do botao
 * t_per: tempo que a presiana est� ligada
 * t_com: tempo para leitura de bit
 * dados: vari�vel que guarda o valor do bot�o do telecomando que foi pressionado
 * dados_prev (dados previous): guarda o valor da vari�vel dados para ser utilizado posteriormente
 * bit: valor do bit lido do quadro de bits
 * togg: valor do bit toggle
 * cont: vari�vel da m�quina de estados na interrup��o pertencente ao controlo por telecomando
 */

/*INICIALIZA��O DO TIMER 1 - para contabilizar tempo no funcionamento normal*/
void tc1_init(void){
	TCCR1B=0;							/*PARA TC1*/
	TIFR1=(7<<TOV1) | (1<<ICF1);		/*LIMPA INTERRUPCOES PENDENTES*/
	TCCR1A=0;							/*MODO NORMAL*/
	TCNT1=T1BOTTOM;						/*CARREGA VALOR DE BOTTOM*/
	TIMSK1=(1<<TOIE1);					/*ATIVA INTERRUPCAO DE OVERFLOW*/
	TCCR1B=4;							/*START TC1 (TP=256)*/
}

/*INICIALIZA��O DO TIMER 2 - para leitura de bits*/
void tc2_init(void){
	TCCR2B=0;							/*PARA TC2*/
	TIFR2=(7<<TOV2);					/*LIMPA INTERRUPCOES PENDENTES*/
	TCCR2A=0;							/*MODO NORMAL*/
	TIMSK2=(1<<TOIE2);					/*ATIVA INTERRUPCAO DE OVERFLOW*/
}

/*INICIALIZA��O DA CONFIGURA��O DA PORTA S�RIE*/
void init_usart(void){
	/*Definir baudrate*/
	UBRR0=UBBR_VAL;

	/*Definir formato da trama*/
	UCSR0C=(3<<UCSZ00)		/*8 data bits*/
			| (0<<UPM00)	/*N�o h� paridade*/
			| (0<<USBS0);	/*1 stop bit*/

	/*Ativar Rx, Tx e interrupt Rx*/
	UCSR0B=(1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
}

/*INICIALIZA��O DO SISTEMA (ENTRADAS E SA�DAS)*/
void hw_init(void){
	DDRB= DDRB | (1<<MOV) | (1<<DIR) | (1<<PB1); 		/*Define vari�vel de motor e dire��o como sa�das*/
	DDRD = DDRD & ~((1<<BA) | (1<<BF) | (1<<COM)); 		/*Define como entradas o bot�o de abrir e fechar e o pino de reca��o de bits do telecomando*/
	PORTD = PORTD | (1<<BA) | (1<<BF) | (1<<COM); 		/*Ativa pull-ups internos das entradas*/
	EICRA = EICRA | (1<<ISC00); 						/*Pedido de interrup��o em both edges*/
	EIMSK = EIMSK | (1<<INT0);							/*Ativa interrup��es no INT0*/

	tc1_init();		/*Inicializa timer 1*/
	tc2_init();		/*Inicializa timer 2*/
	init_usart();	/*Inicializa porta s�rie*/
	printf_init();	/*Inicializa impress�o na consola*/
}

/*INTERRUP��O DO TIMER1*/	/*base de tempo 10ms*/
ISR(TIMER1_OVF_vect){
	TCNT1=T1BOTTOM; /*RECARREGA TC1*/
	if(t_botao) t_botao--;		/*Decrementa vari�vel de contagem do tempo de press�o do bot�o*/
	if(t_per) t_per--;			/*Decrementa vari�vel de contagem do tempo da persiana*/

}

/*INTERRUP��O DO TIMER2*/	/*base de tempo 64 us*/
ISR(TIMER2_OVF_vect){
	EIFR=EIFR|(1<<INTF0);		/*Limpa pedidos pendentes*/
	EIMSK = EIMSK | (1<<INT0);	/*Ativa interrup��es no INT0*/
	TCCR2B=0;					/*P�ra timer*/
}

/*INTERRUP��O DA PORTA S�RIE*/
ISR(USART_RX_vect){
	unsigned char RecByte;
	RecByte=UDR0;	/*Guarda bit recebido pela porta s�rie*/
	UDR0=RecByte;	/*Devolve bit*/
	if(RecByte==ABRIR){
		nstate=10;			/*ABRE PERSIANA*/
		t_botao=TB;			/*Para que a persiana abra at� ao fim e n�o fique � espera do tempo do bot�o que neste caso � irrelevante*/
		t_per=CA;			/*Para que a persiana abra completamente*/
	}
	else if(RecByte==FECHAR){
		nstate=40;			/*FECHA PERSIANA*/
		t_botao=TB;			/*Para que a persiana feche at� ao fim e n�o fique � espera do tempo do bot�o que neste caso � irrelevante*/
		t_per=CF;			/*Para que a persiana feche completamente*/
	}
	else if(RecByte==STOP){
		nstate=0;			/*PERSIANA P�RA*/
		t_botao=TB;
		t_per=CA;
		dados_prev=50;	/*limpa vari�vel*/
	}
	else if(RecByte==FUROS){
		nstate=10;			/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAREM APENAS OS FUROS ABERTOS*/
		t_botao=TB;		/*Para que a persiana abra at� ao fim e n�o fique � espera do tempo do bot�o que neste caso � irrelevante*/
		t_per=CA;		/*Persiana abre completamente*/
		dados_prev=0;	/*Grava a vari�vel lida para quando a persiana abrir completamente, o sistema saber o que tem de fazer a seguir*/
	}
	else if(RecByte==DM1){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAR APENAS A 1 DM DO CH�O*/
		t_botao=TB;
		t_per=CA;
		dados_prev=1;
	}
	else if(RecByte==DM2){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAR APENAS A 2 DM DO CH�O*/
		t_botao=TB;
		t_per=CA;
		dados_prev=2;
	}
	else if(RecByte==DM3){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAR APENAS A 3 DM DO CH�O*/
		t_botao=TB;
		t_per=CA;
		dados_prev=3;
	}
	else if(RecByte==DM4){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAR APENAS A 4 DM DO CH�O*/
		t_botao=TB;
		t_per=CA;
		dados_prev=4;
	}
	else if(RecByte==DM5){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAR APENAS A 5 DM DO CH�O*/
		t_botao=TB;
		t_per=CA;
		dados_prev=5;
	}
	else if(RecByte==DM6){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAR APENAS A 6 DM DO CH�O*/
		t_botao=TB;
		t_per=CA;
		dados_prev=6;
	}
	else if(RecByte==DM7){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAR APENAS A 7 DM DO CH�O*/
		t_botao=TB;
		t_per=CA;
		dados_prev=7;
	}
	else if(RecByte==DM8){
		nstate=10;		/*PERSIANA ABRE COMPLETAMENTE PARA DEPOIS FECHAR AT� FICAR APENAS A 8 DM DO CH�O*/
		t_botao=TB;
		t_per=CA;
		dados_prev=8;
	}
}

/*INTERRUP��O PARA LEITURA DE BITS ENVIADOS PELO TELECOMANDO*/
ISR(INT0_vect){
	EIMSK = 0;				/*Desativa as interrup��es*/
	TCNT2=T2BOTTOM34;		/*Configura timer para tempo de 3/4 de bit*/
	TCCR2B=7;				/*Inicia timer com TP=1024*/
	bit=(PIND&(1<<COM));	/*L� valor do bit*/
		if(cont<2){		/*Se valor de startbit (cont<2) for um, interpreta os dados como ru�do e ignora leitura*/
			if(bit) cont=14;
		}
		else{			/*Se valor de startbit for zero, prossegue com a leitura dos dados*/
			if(cont==2){
				togg=bit;	/*Guarda valor do toggle bit na vari�vel togg*/
				dados=0;	/*Limpa a vari�vel dados para que de seguida a leitura seja correta*/
			}
			else if(cont>7 && cont<14){	/*L� valor dos bits de dados (�ltimos 6 bits da trama)*/
				dados=(dados<<1);	/*Faz shift da vari�vel para guardar o bit menos significativo e n�o perder o anterior*/
				if(!bit) dados++;	/*Se o bit for 0 (como o sinal est� negado, significa que o bit � 1), p�e um 1 no bit mais � direita da vari�vel*/
			}
		}
		if(cont<13) cont++;	/*Incrementa vari�vel da m�quina de estados para ler bit seguinte*/
		else{
			cont=0;	/*Quando acaba de ler os bits da trama, p�e a vari�vel da m�quina de estados a 0*/
		}
}

/*FUN��O PRINCIPAL*/
int main(void){

	hw_init();  /*Inicializa��o do sistema*/
	sei();		/*Ativa interrup��es externas*/

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
			t_botao=TB;	/*Para que a persiana abra at� ao fim e n�o fique � espera do tempo do bot�o que neste caso � irrelevante*/
			t_per=CA;	/*Abre completamente*/
			dados=0;	/*Limpa vari�vel de dados*/
		}break;
		case 17:{		/*Se carregar em reduzir o volume do comando (dados=17)*/
			nstate=40;	/*Persiana fecha*/
			t_botao=TB;	/*Para que a persiana abra at� ao fim e n�o fique � espera do tempo do bot�o que neste caso � irrelevante*/
			t_per=CF;	/*Fecha completamente*/
			dados=0;	/*Limpa vari�vel de dados*/
		}break;
		case 13:{		/*Se carregar em retirar o som do comando (dados=13)*/
			nstate=0;	/*Persiana p�ra*/
			t_botao=0;
			t_per=0;
			dados=0;	/*Limpa vari�vel de dados*/
			dados_prev=50;	/*limpa vari�vel*/
		}break;
		case 5:{		/*Se carregar na tecla 5 do comando (dados=5)*/
			nstate=10;	/*Persiana abre completamente para depois fechar apenas at� metade*/
			t_botao=TB;	/*Para que a persiana abra at� ao fim e n�o fique � espera do tempo do bot�o que neste caso � irrelevante*/
			t_per=CA;	/*Abre completamente*/
			dados_prev=dados;	/*Grava valor de dados para quando a persiana abrir completamente, o sistema saber o que tem de fazer de seguida*/
			dados=0;	/*Limpa vari�vel de dados*/
		}break;
		}
		state=nstate; /*Atualiza valor da m�quina de estados*/

/*M�QUINA DE ESTADOS - MOVIMENTO DA PERSIANA*/
		switch(state){
		/* ESTADO 0*/
		case 0:{ 						/*PARADO*/
			PORTB=PORTB | (1<<MOV);			/*MOV=1*/
			if(!(PIND&(1<<BA))){	/*Se carregar no bot�o de abrir*/
				nstate=1;			/*Persiana abre*/
				t_botao=TB;			/*Come�a a contar o tempo que o bot�o � pressionado*/
				t_per=CA;			/*Come�a a contar o tempo que a persiana est� a abrir*/
			}
			else if(!(PIND&(1<<BF))){	/*Se carregar no bot�o de fechar*/
				nstate=4;				/*Persiana fecha*/
				t_botao=TB;				/*Come�a a contar o tempo que o bot�o � pressionado*/
				t_per=CF;				/*Come�a a contar o tempo que a persiana est� a fechar*/
			}
		}break;
		/* ESTADO 1*/
		case 1:{						/*ABRIR*/
			PORTB=PORTB & ~(1<<MOV); 		/*MOV=0*/
			PORTB=PORTB | (1<<DIR); 		/*DIR=1*/
			if(PIND&(1<<BA)){		/*Se largar o bot�o de abrir*/
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
				nstate=11;	/*longo impulso -> p�ra de abrir*/
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
			/*COMANDOS DA PORTA S�RIE E DO TELECOMANDO*/
				switch(dados_prev){	/*Se estiver a fechar at� um determinado local (comando enviado pela porta s�rie ou pelo telecomando), l� qual o valor em dados_prev*/
				case 0:{
					t_per=TF0;	/*Fecha at� estarem apenas os furos abertos*/
					t_botao=TB;	/*Para que a persiana abra at� ao fim e n�o fique � espera do tempo do bot�o que neste caso � irrelevante*/
					nstate=40;	/*Come�a a fechar*/
				}break;
								/*Este procedimento repete-se para as restantes teclas, variando apenas o tempo de fecho*/
				case 1:{
					t_per=TF1;	/*Fecha at� estar apenas a 1dm do ch�o*/
					t_botao=TB;
					nstate=40;
				}break;
				case 2:{
					t_per=TF2;	/*Fecha at� estar apenas a 2dm do ch�o*/
					t_botao=TB;
					nstate=40;
				}break;
				case 3:{
					t_per=TF3;	/*Fecha at� estar apenas a 3dm do ch�o*/
					t_botao=TB;
					nstate=40;
				}break;
				case 4:{
					t_per=TF4;	/*Fecha at� estar apenas a 4dm do ch�o*/
					t_botao=TB;
					nstate=40;
				}break;
				case 5:{
					t_per=TF5;	/*Fecha at� estar apenas a 5dm do ch�o*/
					t_botao=TB;
					nstate=40;
				}break;
				case 6:{
					t_per=TF6;	/*Fecha at� estar apenas a 6dm do ch�o*/
					t_botao=TB;
					nstate=40;
				}break;
				case 7:{
					t_per=TF7;	/*Fecha at� estar apenas a 7dm do ch�o*/
					t_botao=TB;
					nstate=40;
				}break;
				case 8:{
					t_per=TF8;	/*Fecha at� estar apenas a 8dm do ch�o*/
					t_botao=TB;
					nstate=40;
				}break;
				default:{	/*Se estiver a cumprir o seu funcionamento normal*/
					nstate=3;	/*abre completamente*/
				}
				}
				dados_prev=50;	/*Limpa vari�vel dados_prev*/
			}
		}break;
		/* ESTADO 3*/
		case 3:{						/*COMPLETAMENTE ABERTO*/
			PORTB=PORTB | (1<<MOV);			/*MOV=1*/
			if(!(PIND&(1<<BF))){	/*Se carregar no bot�o de fechar*/
				nstate=4;			/*Persiana fecha*/
				t_per=CF;			/*Come�a a contar o tempo que a persiana est� a fechar*/
				t_botao=TB;			/*Come�a a contar o tempo que o bot�o � pressionado*/
			}
		}break;
		/* ESTADO 4*/
		case 4:{						/*FECHAR*/
			PORTB=PORTB & ~(1<<MOV); 		/*MOV=0*/
			PORTB=PORTB & ~(1<<DIR); 		/*DIR=0*/
			if(PIND&(1<<BF)){	/*Se largar o bot�o de fechar*/
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
			if(!(PIND&(1<<BA))){	/*Se carregar no bot�o de abrir*/
				nstate=1;			/*Persiana abre*/
				t_per=CA;			/*Come�a a contar o tempo que a persiana est� a abrir*/
				t_botao=TB;			/*Come�a a contar o tempo que o bot�o � pressionado*/
			}
		}break;
		default:{		/*ESTADO ILEGAL - em caso de anomalia p�ra a persiana (state=0)*/
			nstate=0;
		}break;
		}
	}
}
