/* DESCRIÇÃO:
 *    Biblioteca para a comunicação SPI
 *    
 *    fclk = fosc/16
 */
 
#define init_comunication     PORTB &= ~(1 << PB2); // SS = 0   // Inicializa a comunicação SPI
#define finish_comunication   PORTB |= (1 << PB2);  // SS = 1   // Finaliza a comunicação SPI

// PROTÓTIPOS DE SUBROTINAS____________________________________________________________________________________________________________________________________________________________________
void init_master_SPI(void);
void init_slave_SPI(void);
void write_SPI(unsigned char);
unsigned char read_SPI(void);

// SUBROTINA PARA A INICIALIZAÇÃO DA CONFIGURAÇÃO DO ATMEGA328P COMO MESTRE_____________________________________________________________________________________________________________________
void init_master_SPI(){
  DDRB |= (1 << DDB5)|(1 << DDB3)|(1 << DDB2);
  DDRB &= ~(1 << DDB4);
  /*
   * PB5 (SCK):   OUTPUT
   * PB4 (MISO):  INPUT
   * PB3 (MOSI):  OUTPUT
   * PB2 (~SS):   OUTPUT
   */
  // Manter SS=1 para que o dispositivo slave não comece a ler antes da hora
  finish_comunication  // SS = 1 
  SPCR  |= (1 << SPE)|(1 << MSTR)|(0 << SPR0);// SPR0 É 1S
  /*
   * bit 7: SPIE = 0: Não é tratado como interrupção
   * bit 6: SPE  = 1: Protocolo SPI habilitado
   * bit 5: DORD = 0: Bit menos significativo é transmitido primeiro (MSB)
   * bit 4: MSTR = 1: Definindo o ATmega328 como mestre
   * bit 3: CPOL = 0: SCK é nível baixo quando ocioso
   * bit 2: CPHA = 0: Amostragem na primeira borda do clock
   * bit 1: SPR1 = 0
   * bit 0: SPR0 = 1
   *      SCK frequência = fosc/16
   */
}
// SUBROTINA PARA A INICIALIZAÇÃO DA CONFIGURAÇÃO DO ATMEGA328P COMO ESCRAVO_____________________________________________________________________________________________________________________
void init_slave_SPI(void){
  DDRB |= (1 << PB4); // MISO (PB4) como output
  SPCR |= (1 << SPE); // Habilitação do modo escravo
}
// SUBROTINA PARA O ENVIO DO BYTE PELO ATMEGA328P________________________________________________________________________________________________________________________________________________
void write_SPI(unsigned char _byte){
  SPDR = _byte;                   // Escreve o byte no registrador de dados
  while(!((SPSR >> SPIF)&0x01));  // Espera o ATmega328 enviar todo o byte 
  SPSR &= ~(1 << SPIF);
}
// SUBROTINA PARA O LEITURA DO BYTE PELO ATMEGA328P______________________________________________________________________________________________________________________________________________
unsigned char read_SPI(){
  //SPSR &= ~(1 << SPIF);
  while(!((SPSR >> SPIF)&0x01));  // Espera o ATmega328 receber todo o byte
  //SPSR &= ~(1 << SPIF);
  return SPDR;                    // Retorna o byte
}
