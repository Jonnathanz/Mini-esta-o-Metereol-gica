	ARQUIVO		|	FUNÇÃO					|
________________________|_______________________________________________|
REDES_FINAL_2.ino	| Arquivo principal				|						|
			| 						|
________________________|_______________________________________________|
	ENC28J60.h	| Inicialização do ENC28J60, LEITURA		|
			| Tratamento e escrita de pacotes: 		|
			| (ARP, ICMP, TCP, SUDOSU)			|
________________________|_______________________________________________|
	SPI.h		| Comunicação SPI entre o Arduino e o ENC28J60	|
			| 						|
________________________|_______________________________________________|
Packets_enc28j60.h	| Declaração de structs para os pacotes, 	|
			| print por comunicação serial de pacotes e 	|
			| dados						|
________________________|_______________________________________________|
SENSOR_TEMPERATURA.h	| Leitura de temperatura do sensor LM 35 	|
________________________|_______________________________________________|
umidade.h		| Leitura dos sensores de umidade e iluminação	|
________________________|_______________________________________________|