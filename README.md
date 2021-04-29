# Controlador de uma persiana

Este trabalho foi desenvolvido no âmbito da unidade curricular Sistemas Baseados em Microprocessadores da Faculdade de Engenharia da Universidade do Porto. O seu objetivo foi desenvolver um controlador de uma persiana. Esse controlo foi feito através da utilização do microcontrolador ATmega328p.

Inicialmente o programa foi desenvolvido de maneira a que o controlo da persiana fosse feito com recurso a dois botões de pressão, um para abrir e outro para fechar. Se o tempo que se pressionasse o botão fosse inferior a meio segundo, a persiana devia abrir ou fechar completamente consoante o botão que era carregado. Por outro lado, se o botão estivesse pressionado durante um período de tempo mais longo (superior a 0.5 segundos), a persiana devia parar de se mover quando se largasse o botão.

De seguida adicionou-se a funcionalidade de utilizar o teclado do computador para o controlo da persiana. Foram definidas teclas específicas, uma para abrir, uma para fechar e uma para parar. Além disso, os dígitos de 0 a 8 foram utilizados para que a persiana fechasse até uma determinada distância do chão, consoante a tecla pressionada. Para tal, foi necessária a implementação de uma comunicação através da porta série.

Por fim, foi adicionado mais um método de controlo da persiana, sendo este feito por um comando de televisão. O comando utilizado operava segundo o protocolo RC5 desenvolvido pela Philips. Os botões definidos para controlar a persiana foram os botões de controlo do volume e tecla 5. Mais especificamente, o botão de aumentar o volume fazia com que a persiana subisse, o de reduzir o volume fazia a persiana descer e o de silenciar parava o movimento. Tal como no controlo por teclado, a tecla 5 fazia com que a persiana fechasse até estar a 5 decímetros do chão.

Com isto, foi possível desenvolver um sistema de controlo da persiana. Controlo feito através de botões de pressão, do teclado do computador e de um comando televisivo.

<img src="https://user-images.githubusercontent.com/76999213/107848251-d0092580-6de9-11eb-8a91-dc223a2b5ebe.JPG" width="400">

# Autores
- Inês Soares
- Sílvia Faria

