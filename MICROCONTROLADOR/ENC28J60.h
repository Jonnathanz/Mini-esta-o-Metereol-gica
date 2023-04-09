/* DESCRIÇÃO:
 *    Biblioteca para a comunicação entre o Arduino UNO e ENC28J60 via SPI
 *    
 *        Modo =  Half-duplex
 *        fclk = fosc/16
 *        
 */
#include "SPI.h"
#include "Packets_enc28j60.h"

// DEFINIÇÕES__________________________________________________________________________________________________________________________________________________________________________________
#define __ERXSTL_value 0x1000
#define __ERXNDL_value 0x1FFF

// DFINIÇÕES DE VARIÁVEIS GLOBAIS______________________________________________________________________________________________________________________________________________________________
unsigned char __IP_EN28J60[4];
unsigned char __MAC_EN28J60[6];
//packet READ_PACKET_ENC28J60;

// PROTÓTIPOS DE SUBROTINA_____________________________________________________________________________________________________________________________________________________________________
void init_ENC28J60(unsigned char*, unsigned char*);                                   // FEITO        (CONFERIDO - FUNCIONANDO)
void systemReset_ENC28J60();                                                          // FEITO        (CONFERIDO - FUNCIONANDO)
unsigned char read_ETH_controlRegister_ENC28J60(unsigned char);                       // FEITO        (CONFERIDO - FUNCIONANDO)
unsigned char read_MAC_MII_controlRegister_ENC28J60(unsigned char);                   // FEITO        (CONFERIDO - FUNCIONANDO)
void read_bufferMemory_ENC28J60();                                                    // DEVO FAZER   
unsigned int read_PHY_ENC28J60(unsigned char);                                        // FEITO        (CONFERIDO - FUNCIONANDO)
void write_controlRegister_ENC28J60(unsigned char, unsigned char);                    // FEITO        (CONFERIDO - FUNCIONANDO)
void write_bufferMemory_ENC28J60(packet*);                                             // DEVO FAZER
void write_PHYRegister_ENC28J60(unsigned char, unsigned int);                         // FEITO        (CONFERIDO - FUNCIONANDO)
void bit_fieldSet_ENC28J60(unsigned char, unsigned char);                             // FUNCIONA SOMENTE EM ETH Control register  (NÃO FEITO)
void bit_fieldClear_ENC28J60(unsigned char, unsigned char);                           // FUNCIONA SOMENTE EM ETH Control register  (NÃO FEITO)
void switchBank_controlRegister_ENC28J60(unsigned char);                              // FEITO       
void write_controlRegisterBank_ENC28J60(unsigned char, unsigned char, unsigned char); // FEITO        (CONFERIDO - FUNCIONANDO) 
void build_read_packet(packet*);
void package_treatment(packet*);
unsigned char match_Array(unsigned char*, unsigned char*, unsigned int);
unsigned int array_checksum(unsigned char*, unsigned int);
void copy_array_bytes(unsigned char*, unsigned char*, unsigned int);
void delete_read_packet();

// PROTÓTIPOS DE SUBROTINA TCP______________________________________________________________________________________________________________________________________________________
void init_TCPserver(unsigned char*);
void treatment_TCP();
void handshake();
void ip_checksum(ip_protocol *ip_packet);
void tcp_checksum(ip_protocol *ip_packet);
void create_tcp_packet(packet *pacote_send, packet*);

// PROTÓTIPOS DE SUBROTINA PARA O SERVIDOR TCP_______________________________________________________________________________________________________________________
void init_TCPserver(unsigned char*);
void treatment_TCP(packet*);
void handshake();
void ip_checksum(ip_protocol *ip_packet);
void tcp_checksum(ip_protocol *ip_packet);
void create_tcp_packet(packet, packet*);

// SUBROTINA PARA A INICIALIZAÇÃO DO ENC28J60__________________________________________________________________________________________________________________________________________________
void init_ENC28J60(unsigned char *MAC_Address, unsigned char* IP_address){
  /* Descrição:
   *      Subrotina para a inicialização do ENC28J60
   * Entrada:
   *      : Vetor do endereço MAC para a configuração do ENC28J60
   */
  unsigned char _MACON1, _MACON3, _MACON4, _MAMXFLL, _MAMXFLH;
  unsigned int _PHCON2;
  unsigned char i;

  copy_array_bytes(__IP_EN28J60, IP_address, 4);
  copy_array_bytes(__MAC_EN28J60, MAC_Address, 6);
  
  init_master_SPI();                          // Configura o Arduino como mestre
  
  // 1º Etapa: Reset do ENC28J60
  systemReset_ENC28J60();
  
  // 2º Etapa: Waiting for ost
  delay(10);                                                                       // espera de 1 ms, conforme a ERRATAS do enc28j60
  while(!((read_ETH_controlRegister_ENC28J60(0x1D))&0x01));                        // Pooling no ESTAT.CLKRDY (Desnecessário)
  
  // 3º Etapa: Configuração do Ethernet Buffer
  write_controlRegister_ENC28J60(0x08, __ERXSTL_value&0xFF); // Atribuindo ERXSTL (Receive Buffer Start Nibble Low)
  write_controlRegister_ENC28J60(0x09, __ERXSTL_value >> 8); // Atribuindo ERXSTH (Receive Buffer Start Nibble High) (na metade)
  write_controlRegister_ENC28J60(0x0A, __ERXNDL_value&0xFF); // Atribuindo ERXNDL (Receive Buffer end Nibble Low)
  write_controlRegister_ENC28J60(0x0B, __ERXNDL_value >> 8); // Atribuindo ERXNDH (Receive Buffer end Nibble High)

  write_controlRegister_ENC28J60(0x00, __ERXSTL_value&0xFF);      // Atribuindo ERDPT LOW
  write_controlRegister_ENC28J60(0x01, __ERXSTL_value >> 8);      // Atribuindo ERDPT HIGH
  write_controlRegister_ENC28J60(0x0C ,__ERXSTL_value&0xFF);      // Atribuindo ERXRDPT LOW
  write_controlRegister_ENC28J60(0x0D ,__ERXSTL_value >> 8);      // Atribuindo ERXRDPT HIGH

  // 4º Etapa: Configuração dos filtros de recebimento de pacote
  switchBank_controlRegister_ENC28J60(1);                   // Mudança para o bank 2
  write_controlRegister_ENC28J60(0x18, 0b10100001);       // Comando de escrita para o ERXFCON
  /*
   * bit 7: UCEN  = 1:    With a destination address matching the local MAC address will be accepted
   * bit 6: ANDOR = 0:    OR: Packets will be accepted unless all enabled filters reject the packet
   * bit 5: CRCEN = 1:    All packets with an invalid CRC will be discarded
   * bit 4: PMEN  = 1:    Filtro Pattern Match habilitado ???
   * bit 3: MPEN  = 0:    Filtro Magic Packets desabilitado ???
   * bit 2: HTEN  = 0:    Filtro Hash Table desabilitado    ???
   * bit 1: MCEN  = 0:    Filtro multicast desabilitado
   * bit 0: BCEN  = 1:    Filtro broadcast habilitado
   */
    
  // 5º Etapa: Configuração dos registradores MAC
  switchBank_controlRegister_ENC28J60(2);                   // Mudança para o bank 2
  
  // 5.1: Habilitação do MAC para receber frames
  _MACON1 = read_MAC_MII_controlRegister_ENC28J60(0x00);    // Leitura do MACON1
  _MACON1 |= 1;                                             // Atribuindo MACON1.MARXEN = 1
  write_controlRegister_ENC28J60(0x00, _MACON1);            // Enable the MAC to receive frames

  // 5.2: Configuração do MACON3: PADCFG, TXCRCEN and FULDPX bits
  _MACON3 = read_MAC_MII_controlRegister_ENC28J60(0x02);    // Leitura do MACON3
  _MACON3 |= (0b111 << 5);                                  // PADCFG2:PADCFG0 = 0b111, All short frames will be zero padded to 64 bytes and a valid CRC will then be appended
  _MACON3 |= 1 << 4;                                        // MACON3.TXCRCEN = 1, MAC adicionará um CRC válido (?????)
  _MACON3 |= 1 << 1;                                        // MACON3.FRMLNEN = 1, The type/length field of transmitted and received frames will be checked
  write_controlRegister_ENC28J60(0x02, _MACON3);            // Enable the MAC to receive frames
  
  // 5.3: Configuração do MACON4: 
  _MACON4 = read_MAC_MII_controlRegister_ENC28J60(0x03);    // Leitura do MACON4
  _MACON4 |= (1 << 6);                                      // MACON4.DEFER = 1
  write_controlRegister_ENC28J60(0x03, _MACON4);            // Comando de escrita para o MACON4

  // 5.4: Configuração do MAMXFL: 
  write_controlRegister_ENC28J60(0x0A, 0xEE);               // Comando que escreve o nibble inferior no MAMXFLL
  write_controlRegister_ENC28J60(0x0B, 0x05);               // Comando que escreve o nibble inferior no MAMXFLH   0x05EE = 1518

  // 5.5: Configuração do MABBIPG:
  write_controlRegister_ENC28J60(0x04, 0x12);               // Comando que atribui 0x12 em MABBIG em half-duplex

  // 5.6: Configuração do MAIPGL:
  write_controlRegister_ENC28J60(0x06, 0x12);               // Comando que atribui 0x12 em MAIPGL

  // 5.7: Configuração do MAIPGH:  
  write_controlRegister_ENC28J60(0x07, 0x0C);               // Comando que atribui 0x0C em MAIPGL

  // 5.8: Obs: ver maclcon1 e maclcon2
  
  // 5.9: Programando o endereço MAC local MAADR1:MAADR6
  switchBank_controlRegister_ENC28J60(3);                   // Mudança para o bank 3
  write_controlRegister_ENC28J60(0x04, MAC_Address[0]);      // Comando que escreve o primeiro byte de endereço MAC em MAADRI1
  write_controlRegister_ENC28J60(0x05, MAC_Address[1]);      // Comando que escreve o segundo byte de endereço MAC em MAADRI2
  write_controlRegister_ENC28J60(0x02, MAC_Address[2]);      // Comando que escreve o terceiro byte de endereço MAC em MAADRI3
  write_controlRegister_ENC28J60(0x03, MAC_Address[3]);      // Comando que escreve o quarto byte de endereço MAC em MAADRI4
  write_controlRegister_ENC28J60(0x00, MAC_Address[4]);      // Comando que escreve o quinto byte de endereço MAC em MAADRI5
  write_controlRegister_ENC28J60(0x01, MAC_Address[5]);      // Comando que escreve o sexto byte de endereço MAC em MAADRI6

  // 6º Etapa: Configuração dos registradores PHY
  _PHCON2 = read_PHY_ENC28J60(0x10);
  _PHCON2 |= (1 << 8);                                      // set PHLCON2.HDLDIS = 1
  // obs; nao necessario ver PHCON1 pq está em default
  write_PHYRegister_ENC28J60(0x10, _PHCON2);                // Comando de escrita para o PHCON2

  // 7º Etapa: Configuração ECON1
  write_controlRegister_ENC28J60(0x1F, 0x00|(1 << 2));  // Atribuindo ECON1 como 0x00

  write_controlRegister_ENC28J60(0x0C ,__ERXNDL_value&0xFF);                    // LIBERAÇÃO DE MEMÓRIA DO BUFFER RECEIVER
  write_controlRegister_ENC28J60(0x0D ,(__ERXNDL_value&0xFF00) >> 8);           // LIBERAÇÃO DE MEMÓRIA DO BUFFER RECEIVER
}
// SUBROTINA PARA O RESET DO ENC28J60__________________________________________________________________________________________________________________________________________________________
void systemReset_ENC28J60(){
  /* Descrição:
   *    Subrotina para o soft reset do ENC28J60
   */
  init_comunication                   // Inicializa a comunicação
  write_SPI(0xFF);                    // Envio do comando de soft reset
  finish_comunication                 // Finaliza a comunicação
}
// SUBROTINA PARA A ESCRITA EM REGISTRADORES DE CONTROLE_______________________________________________________________________________________________________________________________________
void write_controlRegister_ENC28J60(unsigned char address, unsigned char data){
  /* Descrição:
   *      Subrotina para a escrita nos registradores de controle
   * Entradas:
   *      address:  Endereço do registrador
   *      data:     Dados a ser escrito
   */
  init_comunication                   // Inicializa a comunicação
  write_SPI((0b010 << 5)|address);    // Comando de Escrita para o address
  write_SPI(data);                    // Comando que escreve o NIBBLE superior
  finish_comunication                 // Finaliza a comunicação
}
// SUBROTINA PARA A ESCRITA EM REGISTRADORES DE CONTROLE COM A SELEÇÃO DE BANCO DE REGISTRADORES_______________________________________________________________________________________________
void write_controlRegisterBank_ENC28J60(unsigned char address, unsigned char data, unsigned char bank){
  /* Descrição:
   *      Subrotina para a escrita nos registradores de controle com a seleção de banco de registradores
   * Entradas:
   *      address:  Endereço do registrador
   *      data:     Dados a ser escrito
   *      bank:     Banco selecionado
   */
  switchBank_controlRegister_ENC28J60(bank);      // Comando para a mudança ao banco "bank" dos registradores de controle
  write_controlRegister_ENC28J60(address, data);  // Comando de escrita dos dados "data" no endereço "address"
}
// SUBROTINA PARA A ESCRITA EM REGISTRADORES PHY_______________________________________________________________________________________________________________________________________________
void write_PHYRegister_ENC28J60(unsigned char address, unsigned int data){
  /* Descrição:
   *      Subrotina para a escrita nos registradores PHY
   *      APÒS A EXECUÇÃO DESSE CÓDIGO O BANCO SELECIONADO É O BANCO 0
   * Entradas:
   *      address:  Endereço do registrador
   *      data:     Dados a ser escrito
   */
  unsigned char nibble_low  = data & (0xFF);            // Pega o NIBBLE inferior dos dados
  unsigned char nibble_high = data >> 8;                // Pega o NIBBLE superior dos dados
  write_controlRegisterBank_ENC28J60(0x14, address, 2); // Comando de Escrita para Endereço MIREGAPR escrevendo o endereço do address (banco 2)
  write_controlRegister_ENC28J60(0x16, nibble_low);     // Comando de Escrita para Endereço MIWRL escrevendo o NIBBLE inferior
  write_controlRegister_ENC28J60(0x17, nibble_high);    // Comando de Escrita para Endereço MIWRL escrevendo o NIBBLE superior
  delayMicroseconds(11);
  switchBank_controlRegister_ENC28J60(0);               // Mudança para o banco 0 de registradores de controle
}
// SUBROTINA PARA A LEITURA DE REGISTRADORES DE CONTROLE ETH___________________________________________________________________________________________________________________________________
unsigned char read_ETH_controlRegister_ENC28J60(unsigned char address){
  /* Descrição:
   *    Subrotina para a leitura de registradores de controle ETH
   * Entradas:
   *    address:  Endereço do registrador
   * Retorno
   *    data:     Dados lidos
   */
  unsigned char data;
  init_comunication                   // Inicializa a comunicação
  write_SPI((0b000 << 5)|address);    // Comando de Escrita para o address
  write_SPI(0xFF);                    // Comando de Escrita somente para intercâmbio de dados
  data = read_SPI();                  // Comando de leitura
  finish_comunication                 // Finaliza a comunicação
  return data;                        // Retorno dos dados
}
// SUBROTINA PARA A LEITURA DE REGISTRADORES DE CONTROLE MAC e MII_____________________________________________________________________________________________________________________________
unsigned char read_MAC_MII_controlRegister_ENC28J60(unsigned char address){
  /* Descrição:
   *    Subrotina para a leitura de registradores de controle MAC e MII
   * Entradas:
   *    address:  Endereço do registrador
   *    data:     Dados a ser escrito
   */
  unsigned char data;
  init_comunication                   // Inicializa a comunicação
  write_SPI((0b000 << 5)|address);    // Comando de Escrita para o address
  write_SPI(0xFF);                    // Comando de Escrita somente para intercâmbio de dados e deixar passar o dummy byte
  write_SPI(0xFF);                    // Comando de Escrita somente para intercâmbio de dados
  data = read_SPI();                  // Comando de leitura
  finish_comunication                 // Finaliza a comunicação
  return data;                        // Retorno dos dados
}
// SUBROTINA PARA A LEITURA DE REGISTRADORES PHY_______________________________________________________________________________________________________________________________________________
unsigned int read_PHY_ENC28J60(unsigned char address){
  /* Descrição:
   *    Subrotina para a leitura de registradores de controle MAC e MII
   *    APÒS A EXECUÇÃO DESSE CÓDIGO O BANCO SELECIONADO É O BANCO 0
   * Entradas:
   *    address:  Endereço do registrador
   * retorno:
   *    data:     Dados a ser escrito
   */
  unsigned int data;
  unsigned char _MICMD;
  unsigned char nibble_low;            
  unsigned char nibble_high; 
  switchBank_controlRegister_ENC28J60(2);                               // Mudança para o banco 2
  _MICMD = read_MAC_MII_controlRegister_ENC28J60(0x12);                 // LEITURA DO REGISTRADOR _MICMD
  write_controlRegister_ENC28J60(0x14, address);                        // Comando de Escrita para Endereço MIREGAPR escrevendo o endereço do address
  _MICMD |= (0x01);                                                     // Atribuição do valor 1 no MICMD.MIIRD para a inicialização da operação de leitura
  write_controlRegister_ENC28J60(0x12, _MICMD);                         // Comando de escrita para o registrador MICMD
  delayMicroseconds(11);
  switchBank_controlRegister_ENC28J60(3);                               // Mudança para o banco 3
  while((read_MAC_MII_controlRegister_ENC28J60(0x0A)&0x01));            // Pooling em MISTAT.BUSY para a verificação se a operação está completa
  switchBank_controlRegister_ENC28J60(2);                               // Mudança para o banco 2
  _MICMD = read_MAC_MII_controlRegister_ENC28J60(0x12);
  _MICMD &= ~(0x01);                                                    // Atribuição do valor 0 no MICMD.MIIRD para a finalização da operação de leitura
  write_controlRegister_ENC28J60(0x12, _MICMD);                         // Comando de escrita para o registrador MICMD
  delayMicroseconds(11);
  nibble_low = read_MAC_MII_controlRegister_ENC28J60(0x18);             // Comando de leitura do nibble low
  nibble_high = read_MAC_MII_controlRegister_ENC28J60(0x19);            // Comando de leitura do nibble high
  switchBank_controlRegister_ENC28J60(0);                               // Mudança para o banco 0
  return (nibble_high << 8) | nibble_low;
}
// SUBROTINA PARA A SELEÇÃO DE BANCO DE REGISTRADOR DE CONTROLE________________________________________________________________________________________________________________________________
void switchBank_controlRegister_ENC28J60(unsigned char bank){
  /* Descrição:
   *      Subrotina para a mudança de banco dos registradores de controle
   * Entrada:
   *      bank = Banco para ser mudado
   * 
   *        bank = 0: Banco 0
   *        bank = 1: Banco 1
   *        bank = 2: Banco 2
   *        bank = 3: Banco 3
   */
  unsigned char _ECON1 = read_ETH_controlRegister_ENC28J60(0x1F); // Leitura do registrador ECON1
  switch(bank){
    case 0: 
      _ECON1 = _ECON1&(0b11111100);   // Atribui os bits '0b00' aos bits 1 e 0
      break;
    case 1: // Atribuição dos bits 0b01
      _ECON1 |= 0x01;       // Atribui '1' ao bit 0  
      _ECON1 &= ~(1 << 1);  // Atribui '0' ao bit 1
      break;
    case 2: // Atribuição dos bits 0b10
      _ECON1 |= (1 << 1);   // Atribui '1' ao bit 1
      _ECON1 &= ~(0x01);    // Atribui '0' ao bit 0
      break;
    case 3: // Atribuição dos bits 0b11
      _ECON1 |= 0b11;       // Atribui '0b11' aos bits 1 e 0
      break;
    default:
      _ECON1 = _ECON1&(0b11111100);  // Atribui os bits '0b00' aos bits 1 e 0
  }
  write_controlRegister_ENC28J60(0x1F, _ECON1); // Comando de Escrita para Endereço ECON1 para mudar para o banco selecionado
}
// SUBROTINA PARA A LEITURA DO BUFFER DE RECEBIMENTO___________________________________________________________________________________________________________________________________________
void read_bufferMemory_ENC28J60(){
  /* Descrição:
   *      Subrotina para a leitura de somente um pacote;
   */
  packet READ_PACKET_ENC28J60;
  unsigned char _PKTIF = ((read_ETH_controlRegister_ENC28J60(0x1C)&(1 << 6)) >> 6);     // Bit de flag do recebimento de dados
  unsigned char next_packet_pointer_H, next_packet_pointer_L;                           // Variável do Ponteiros mais e menos significativo para o próximo pacote
  unsigned int  next_packet_pointer;                                                    // Variável do Ponteiro do próximo pacote                                               
  unsigned int  _ERDPT;                                                                 // Variável do ponteiro do pacote atual                                                                     
  unsigned char _ECON2;
  unsigned int _ERXRDPT;                                                                 // Liberação de espaço da memória do buffer de recebimento
  //packet pacote;
  if(!_PKTIF){
    //READ_PACKET_ENC28J60 = __PACKET_NONE;
  }
  if(_PKTIF){    
    switchBank_controlRegister_ENC28J60(0);
    _ERDPT = ((read_ETH_controlRegister_ENC28J60(0x01)&0b00011111) << 8)|read_ETH_controlRegister_ENC28J60(0x00); // leitura do ERDPT, ponteiro de leituras
    init_comunication                                                               // Inicialização da comunicação SPI
    write_SPI(0b00111010);                                                          // Escrita do opcode para a leitura
    write_SPI(0xFF);
    next_packet_pointer_L = read_SPI();                                             // Leitura do byte menos significativo do ponteiro do próximo pacote
    write_SPI(0xFF);
    next_packet_pointer_H = read_SPI();                                             // Leitura do byte mais significativo do ponteiro do próximo pacote
    next_packet_pointer   = (next_packet_pointer_H << 8)|next_packet_pointer_L;     // Valor do Ponteiro do próximo pacote
    build_read_packet(&READ_PACKET_ENC28J60);                                       // Leitura do pacote
    finish_comunication                                                             // Fim da comunicação SPI
    
    
    
    _ECON2 = read_ETH_controlRegister_ENC28J60(0x1E) | (1 << 6);                    // Decrementando _EPKTCNT após a leitura completa de um pacote
    write_controlRegister_ENC28J60(0x1E,_ECON2);                                    // Escrevendo no ECON2
    if(next_packet_pointer == __ERXSTL_value){
      write_controlRegister_ENC28J60(0x0C ,__ERXNDL_value&0xFF);                    // LIBERAÇÃO DE MEMÓRIA DO BUFFER RECEIVER
      write_controlRegister_ENC28J60(0x0D ,(__ERXNDL_value&0xFF00) >> 8);           // LIBERAÇÃO DE MEMÓRIA DO BUFFER RECEIVER
    } else {
      write_controlRegister_ENC28J60(0x0C ,next_packet_pointer_L);                    // LIBERAÇÃO DE MEMÓRIA DO BUFFER RECEIVER
      write_controlRegister_ENC28J60(0x0D ,next_packet_pointer_H);                    // LIBERAÇÃO DE MEMÓRIA DO BUFFER RECEIVER
    }
    
    package_treatment(&READ_PACKET_ENC28J60);
    treatment_TCP(&READ_PACKET_ENC28J60);
    //displayData((READ_PACKET_ENC28J60._status[1] << 8)|READ_PACKET_ENC28J60._status[0], _ERDPT, next_packet_pointer); // Display de alguns dados sobre o pacote
    //displayPacket(&READ_PACKET_ENC28J60);
    //Serial.println("Memória RAM Livre: ");
    //Serial.println(freeMemory());
  }
}
// SUBROTINA PARA A CRIAÇÃO DE PACOTES_________________________________________________________________________________________________________________________________________________________
void build_read_packet(packet *READ_PACKET_ENC28J60){
  /* Descrição:
   *      Subrotina para a criação de pacotes;
   */
  unsigned int i;
  unsigned int c = 0;
  unsigned int len_package; // Variável tamanho do pacote  
  // Atribuição do status
  for(i=0; i<4; i++){
    write_SPI(0xFF);
    READ_PACKET_ENC28J60->_status[i] = read_SPI();
  }
  len_package = (((unsigned int)(READ_PACKET_ENC28J60->_status[1])) << 8)|READ_PACKET_ENC28J60->_status[0];
  if(len_package%2 == 1){
    len_package++;
  }
  // Atribuição do endereço de destino
  for(i=0; i<6; i++){
    write_SPI(0xFF);
    READ_PACKET_ENC28J60->destination_address[i] = read_SPI();
    c++;
  }
  // Atribuição do endereço source_address
  for(i=0; i<6; i++){
    write_SPI(0xFF);
    READ_PACKET_ENC28J60->source_address[i] = read_SPI();
    c++;
  }
  // Atribuição do type/length
  for(i=0; i<2; i++){
    write_SPI(0xFF);
    READ_PACKET_ENC28J60->type[i] = read_SPI();
    c++;
  }
  unsigned int _type = (READ_PACKET_ENC28J60->type[0] << 8)|(READ_PACKET_ENC28J60->type[1]);
  switch(_type){
    case 0x806: // Protocolo ARP
      // Atribuição do hard_type
      for(i=0; i<2; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.arp.hard_type[i] = read_SPI();
        c++;
      }
      // Atribuição do prot_type
      for(i=0; i<2; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.arp.prot_type[i] = read_SPI();
        c++;
      }
      // Atribuição do hard_size e prot_size
      write_SPI(0xFF);
      READ_PACKET_ENC28J60->data.arp.hard_size = read_SPI();
      c++;
      write_SPI(0xFF);
      READ_PACKET_ENC28J60->data.arp.prot_size = read_SPI();
      c++;
      // Atribuição do op
      for(i=0; i<2; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.arp.op[i] = read_SPI();
        c++;
      }  
      // Atribuição do sender ethernet addr
      for(i=0; i<6; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.arp.sender_Ethernet_addr[i] = read_SPI();
        c++;
      }
      // Atribuição do sender sender_IP_addr
      for(i=0; i<4; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.arp.sender_IP_addr[i] = read_SPI();
        c++;
      }
      // Atribuição do sender target_Ethernet_addr
      for(i=0; i<6; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.arp.target_Ethernet_addr[i] = read_SPI();
        c++;
      }
      // Atribuição do sender target_IP_addr
      for(i=0; i<4; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.arp.target_IP_addr[i] = read_SPI();
        c++;
      }
      break;
    case 0x800: // PROTOCOLO IP
      // Atribuição da versão e tamanho do cabeçalho
      write_SPI(0xFF);
      READ_PACKET_ENC28J60->data.IP.version_and_hlength = read_SPI();
      c++;
      // Atribuição do tipo de serviço
      write_SPI(0xFF);
      READ_PACKET_ENC28J60->data.IP.type_service = read_SPI();
      c++;
      // Atribuição do tamanho total
      for(i=0; i<2; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.IP.total_length[i] = read_SPI();
        c++;
      }
      // Atribuição da identificação
      for(i=0; i<2; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.IP.identification[i] = read_SPI();
        c++;
      }
      // Atribuição do flag e deslocamento
      for(i=0; i<2; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.IP.flag_displacement[i] = read_SPI();
        c++;
      }
      // Atribuição do tempo de vida
      write_SPI(0xFF);
      READ_PACKET_ENC28J60->data.IP.life_time = read_SPI();
      c++;
      // Atribuição do protocolo
      write_SPI(0xFF);
      READ_PACKET_ENC28J60->data.IP.protocol = read_SPI();
      c++;
      // Atribuição do checksum do cabeçalho
      for(i=0; i<2; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.IP.head_checksum[i] = read_SPI();
        c++;
      }
      // Atribuição do endereço fonte
      for(i=0; i<4; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.IP.source_address[i] = read_SPI();
        c++;
      }
      // Atribuição do endereço destino
      for(i=0; i<4; i++){
        write_SPI(0xFF);
        READ_PACKET_ENC28J60->data.IP.destination_address[i] = read_SPI();
        c++;
      }
      unsigned int ip_length = (READ_PACKET_ENC28J60->data.IP.total_length[0] << 8)|READ_PACKET_ENC28J60->data.IP.total_length[1];
      switch(READ_PACKET_ENC28J60->data.IP.protocol){
        case 0x01:  // PROTOCOLO ICMP
          // Atribuição do tipo do ICMP
          write_SPI(0xFF);
          READ_PACKET_ENC28J60->data.IP.ip_data.icmp.type = read_SPI();
          c++;
          // Atribuição do código do ICMP
          write_SPI(0xFF);
          READ_PACKET_ENC28J60->data.IP.ip_data.icmp.code = read_SPI();
          c++;
          // Atribuição do checksum do ICMP
          for(i=0; i<2; i++){
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.icmp.checksum[i] = read_SPI();
            c++;
          }
          // Atribuição pra merda nenhuma do ICMP
          for(i=0; i<4; i++){
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.icmp.unused[i] = read_SPI();
            c++;
          }
          // Atribuição de dados do icmp
          //unsigned int ip_length = (READ_PACKET_ENC28J60->data.IP.total_length[0] << 8)|READ_PACKET_ENC28J60->data.IP.total_length[1];
          READ_PACKET_ENC28J60->data.IP.ip_data.icmp.length_data = ip_length - c + 14;
          //READ_PACKET_ENC28J60->data.IP.ip_data.icmp.data = malloc(ip_length - c + 14 + 5);
          //Serial.println("Tamanho dos dados ICMP");
          //Serial.println(READ_PACKET_ENC28J60->data.IP.ip_data.icmp.length_data);
          for(i = 0; i < READ_PACKET_ENC28J60->data.IP.ip_data.icmp.length_data; i++){ // ver depois essa variavel c
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.icmp.data[i] = read_SPI();
            //Serial.println(READ_PACKET_ENC28J60->data.IP.ip_data.icmp.data[i], HEX);
            c++;
          }
          break;
        case 0x06: // PROTOCOLO TCP
          for(i = 0; i < 2; i++){ // Atribuição da porta de origem
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.source_port[i] = read_SPI();
            c++;
          }
          for(i = 0; i < 2; i++){ // Atribuição da porta de destino
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.destination_port[i] = read_SPI();
            c++;
          }
          for(i = 0; i < 4; i++){ // Atribuição do número de sequência
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[i] = read_SPI();
            c++;
          }
          for(i = 0; i < 4; i++){ // Atribuição do número de reconhecimento
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.acknowledgment_number[i] = read_SPI();
            c++;
          }
          for(i = 0; i < 2; i++){ // Atribuição do deslocamento, reservado e controle
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.dataOffset_control[i] = read_SPI();
            c++;
          }
          for(i = 0; i < 2; i++){ // Atribuição do tamanho da janela
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.window_size[i] = read_SPI();
            c++;
          }
          for(i = 0; i < 2; i++){ // Atribuição do checksum
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.checksum[i] = read_SPI();
            c++;
          }
          for(i = 0; i < 2; i++){ // Atribuição do ponteiro de urgência
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.urgent_pointer[i] = read_SPI();
            c++;
          }
          // Atribuição de opções e preenchimento
          READ_PACKET_ENC28J60->data.IP.ip_data.TCP.length_options_padding = 4*(READ_PACKET_ENC28J60->data.IP.ip_data.TCP.dataOffset_control[0] >> 4) - 20;
          //READ_PACKET_ENC28J60->data.IP.ip_data.TCP.options_padding = malloc(READ_PACKET_ENC28J60->data.IP.ip_data.TCP.length_options_padding);
          for(i = 0; i < READ_PACKET_ENC28J60->data.IP.ip_data.TCP.length_options_padding; i++){ // Atribuição de opções e preenchimento
            write_SPI(0xFF);
            if(i < 4)
              READ_PACKET_ENC28J60->data.IP.ip_data.TCP.options_padding[i] = read_SPI();
            c++;
          }
          
          // Atribuição de dados
          READ_PACKET_ENC28J60->data.IP.ip_data.TCP.length_data = ip_length - c + 14;
          for(i = 0; c < ip_length+14; i++){ // ver depois essa variavel c
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.data[i] = read_SPI();
            c++;
          }

          //displayPacket(READ_PACKET_ENC28J60);
          
          break;
        default:
        
          break;
      }
      break;
    default:    // Caso seja outro tipo de protocolo

      break;   
  }  
  while(c < len_package){
    write_SPI(0xFF);
    c++;
  }

  //package_treatment(READ_PACKET_ENC28J60);
}
// SUBROTINA PARA O TRATAMENTO DE UM PACOTE LIDO_______________________________________________________________________________________________________________________________________________
void package_treatment(packet *READ_PACKET_ENC28J60){
  /* Descrição:
   *      Subrotina para o tratamento de pacote;
   * Entrada:
   *      packet pacote: pacote do buffer de recebimento
   */
  unsigned int _type = (READ_PACKET_ENC28J60->type[0] << 8)|(READ_PACKET_ENC28J60->type[1]);
  unsigned int i=0;
  unsigned char *IP_target_pacote;
  packet pacote_send;
  if(_type == 0x806){
    // Protocolo ARP
    unsigned int _op = (READ_PACKET_ENC28J60->data.arp.op[0] << 8)|(READ_PACKET_ENC28J60->data.arp.op[1]);
    IP_target_pacote = READ_PACKET_ENC28J60->data.arp.target_IP_addr; 
    //Serial.println("OPÇÔES");
    //Serial.println(_op, HEX);
    if(_op == 0x01) {
      // Protocolo ARP - Requisição
      //Serial.println(IP_EN28J60[2]);
      if(match_Array(IP_target_pacote, __IP_EN28J60, 4)){
        // CASO O IP DA MATCH, IRÁ RETORNAR UM PACOTE ARP DE REPLY
        //packet pacote_send;
        for(i=0;i<6;i++)
          pacote_send.destination_address[i] = READ_PACKET_ENC28J60->source_address[i];
        for(i=0;i<6;i++)
          pacote_send.source_address[i] = __MAC_EN28J60[i];
        pacote_send.type[0] = 0x08;
        pacote_send.type[1] = 0x06;
        pacote_send.data.arp.hard_type[0] = 0x00;
        pacote_send.data.arp.hard_type[1] = 0x01;
        pacote_send.data.arp.prot_type[0] = 0x08;
        pacote_send.data.arp.prot_type[1] = 0x00;
        pacote_send.data.arp.hard_size = 0x06;
        pacote_send.data.arp.prot_size = 0x04;
        pacote_send.data.arp.op[0] = 0x00;
        pacote_send.data.arp.op[1] = 0x02;
        for(i=0;i<6;i++)
          pacote_send.data.arp.sender_Ethernet_addr[i] = __MAC_EN28J60[i];
        for(i=0;i<4;i++)
          pacote_send.data.arp.sender_IP_addr[i] = __IP_EN28J60[i];
        for(i=0;i<6;i++)
          pacote_send.data.arp.target_Ethernet_addr[i] = READ_PACKET_ENC28J60->data.arp.sender_Ethernet_addr[i];
        for(i=0;i<4;i++)
          pacote_send.data.arp.target_IP_addr[i] = READ_PACKET_ENC28J60->data.arp.sender_IP_addr[i];
        write_bufferMemory_ENC28J60(&pacote_send);
      }  
    } 
  } else if(_type == 0x800){
    // Protocolo IPV4
    unsigned char ip_version = READ_PACKET_ENC28J60->data.IP.protocol;
    IP_target_pacote = READ_PACKET_ENC28J60->data.IP.destination_address;
    if(match_Array(IP_target_pacote, __IP_EN28J60, 4)){ 
      // Verificação do endereço IP de destino
      switch(ip_version){
          case 0x01:
            // Protocolo ICMP
            unsigned char type_icmp = READ_PACKET_ENC28J60->data.IP.ip_data.icmp.type;
            unsigned char code_icmp = READ_PACKET_ENC28J60->data.IP.ip_data.icmp.code;
            if((type_icmp == 0x08)&(code_icmp == 0x00)){
              // Tipo do pacote recebido de solicitação de ECO de ping
              
              //-----CONSTRUÇÃO DADOS IP--------
              //unsigned char *array_bytes_headerIP = new unsigned char [20]; // observação: verificar o tamanho do cabeçalho
              unsigned char array_bytes_headerIP[20];
              unsigned int j = 0;
              unsigned int check;
              // Protocolo ICMP de solicitação de eco de ping
              for(i=0;i<6;i++){
                pacote_send.destination_address[i] = READ_PACKET_ENC28J60->source_address[i]; // Endereço MAC do PC
                pacote_send.source_address[i] = __MAC_EN28J60[i];              // Endereço MAC do ENC28j60 + arduino
              }
              pacote_send.type[0] = 0x08;
              pacote_send.type[1] = 0x00;   // PACOTE DO TIPO IPV4
              pacote_send.data.IP.version_and_hlength = 0x45; // 4: Versão IPV4; 5: Tamanho 5 WORDs do cabeçalho do IP
              array_bytes_headerIP[j++] = pacote_send.data.IP.version_and_hlength;
              
              pacote_send.data.IP.type_service = 0x00;
              array_bytes_headerIP[j++] = pacote_send.data.IP.type_service;

              pacote_send.data.IP.total_length[0] = 0x00;  // MUDAR DEPOIS O TAMANHO DO
              pacote_send.data.IP.total_length[1] = 0x54;  // TAMANHO DE 84 BYTES
              array_bytes_headerIP[j++] = pacote_send.data.IP.total_length[0];
              array_bytes_headerIP[j++] = pacote_send.data.IP.total_length[1];

              for(i=0;i<2;i++){
                pacote_send.data.IP.identification[i] = READ_PACKET_ENC28J60->data.IP.identification[i];
                array_bytes_headerIP[j++] = pacote_send.data.IP.identification[i];
              }
              for(i=0;i<2;i++){
                pacote_send.data.IP.flag_displacement[i] = READ_PACKET_ENC28J60->data.IP.flag_displacement[i]; // MARCARRRR
                array_bytes_headerIP[j++] = pacote_send.data.IP.flag_displacement[i];
              }
              
              pacote_send.data.IP.life_time = 0x40;
              array_bytes_headerIP[j++] = pacote_send.data.IP.life_time;
              pacote_send.data.IP.protocol = 0x01;
              array_bytes_headerIP[j++] = pacote_send.data.IP.protocol;

              //pacote_send.data.IP.head_checksum = 0x01;// CRIAR FUNÇÃO DO CHECKSUM
              for(i=0;i<4;i++){
                pacote_send.data.IP.source_address[i] = READ_PACKET_ENC28J60->data.IP.destination_address[i]; // Endereço IP do ARDUINO + ENC28J60   
                array_bytes_headerIP[j++] = pacote_send.data.IP.source_address[i];
              }

              //DOIS ULTIMOS FOR TROCADOSSSS
              for(i=0;i<4;i++){
                pacote_send.data.IP.destination_address[i] = READ_PACKET_ENC28J60->data.IP.source_address[i]; // Endereço IP do PC
                pacote_send.data.IP.ip_data.icmp.unused[i] = READ_PACKET_ENC28J60->data.IP.ip_data.icmp.unused[i]; // RESTO DO CABEÇALHO DO ICMP
                array_bytes_headerIP[j++] = pacote_send.data.IP.destination_address[i];
              }
              check = array_checksum(array_bytes_headerIP, 18);
              pacote_send.data.IP.head_checksum[1] = check&0xFF;
              pacote_send.data.IP.head_checksum[0] = check >> 8;
              
              //-----CONSTRUÇÃO DADOS ICMP--------s
              pacote_send.data.IP.ip_data.icmp.length_data = READ_PACKET_ENC28J60->data.IP.ip_data.icmp.length_data; // TAMANHO DO PACOTE ICMP
              //array_bytes_headerIP = new unsigned char [pacote_send.data.IP.ip_data.icmp.length_data + 8];
              unsigned char array_bytes_headerICMP[10+pacote_send.data.IP.ip_data.icmp.length_data];
              //unsigned char dataICMP[pacote_send.data.IP.ip_data.icmp.length_data];
              //pacote_send.data.IP.ip_data.icmp.data = dataICMP;
              array_bytes_headerIP[pacote_send.data.IP.ip_data.icmp.length_data + 8];
              j = 0;
              
              pacote_send.data.IP.ip_data.icmp.type = 0x00; // MUDAR PARA 0X00
              array_bytes_headerICMP[j++] = pacote_send.data.IP.ip_data.icmp.type;
              
              pacote_send.data.IP.ip_data.icmp.code = 0x00; // Resposta do ECO
              array_bytes_headerICMP[j++] = pacote_send.data.IP.ip_data.icmp.code;
              
              for(i=0; i<4; i++){
                pacote_send.data.IP.ip_data.icmp.unused[i] = READ_PACKET_ENC28J60->data.IP.ip_data.icmp.unused[i];
                array_bytes_headerICMP[j++] = pacote_send.data.IP.ip_data.icmp.unused[i];
              }
              
              for(i = 0; i < pacote_send.data.IP.ip_data.icmp.length_data; i++){
                pacote_send.data.IP.ip_data.icmp.data[i] = READ_PACKET_ENC28J60->data.IP.ip_data.icmp.data[i];    // DADOS
                array_bytes_headerICMP[j++] = pacote_send.data.IP.ip_data.icmp.data[i];
              }
              check = array_checksum(array_bytes_headerICMP, pacote_send.data.IP.ip_data.icmp.length_data + 6);
              pacote_send.data.IP.ip_data.icmp.checksum[1] = check&0xFF;
              pacote_send.data.IP.ip_data.icmp.checksum[0] = check >> 8;
              
              
              write_bufferMemory_ENC28J60(&pacote_send);
              //displayPacket(&pacote_send);
            }
            break;
          default:

            break;
        }  
    }     
  }
  /*
  switch(_type){
    case 0x806: 
      
      //break;
    case 0x800:
        
      break;
    default:
      break;
  }
  */
}
// SUBROTINA PARA UM MATCH DE DOIS ARRAYS______________________________________________________________________________________________________________________________________________________
unsigned char match_Array(unsigned char* array1, unsigned char array2[], unsigned int _length){
  /* Descrição:
   *      Subrotina para a verificação da igualdade entre dois arrays;
   * Entrada:
   *      unsigned char *array1
   *      unsigned char *array2
   *      unsigned int _length
   * Saída:
   *      unsigned char: 1 se forem iguais, 0 se forem diferentes
   */
  unsigned int i=0;
  for(i=0;i<_length;i++){
    if(array1[i] != array2[i])
      return 0;
  }
  return 1;
}
// SUBROTINA PARA A TRANSMISSÃO DE UM PACOTE___________________________________________________________________________________________________________________________________________________
void write_bufferMemory_ENC28J60(packet* pacote){
  /* Descrição:
   *      Subrotina para o envio de pacotes
   * Entrada:
   *      packet pacote
   */
  unsigned char _ECON1;
  unsigned char _EIR;
  unsigned int i;
  unsigned int _length=0;
  switchBank_controlRegister_ENC28J60(0);
  write_controlRegister_ENC28J60(0x02, 0x00);  // Atribuição do EWRPTH
  write_controlRegister_ENC28J60(0x03, 0x00);  // Atribuição do EWRPTL
  write_controlRegister_ENC28J60(0x04, 0x00);  // Atribuição do ETXSTL
  write_controlRegister_ENC28J60(0x05, 0x00);  // Atribuição do ETXSTH
  //______________________ENVIO DOS DADOS NO BUFFER DE TRANSMISSÃO - CAMADA DE ENLACE_____________________________
  init_comunication
  write_SPI(0b01111010);    // OP CODE PARA A ESCRITA NO BUFFER DE TRANSMISSÃO
  write_SPI(0x00);          // Envio do per packet control byte
  for(i=0;i<6;i++){
    write_SPI(pacote->destination_address[i]);
    _length++;
  } 
  for(i=0;i<6;i++){
    _length++;
    write_SPI(pacote->source_address[i]);
  }
  write_SPI(pacote->type[0]);
  _length++;
  write_SPI(pacote->type[1]);
  _length++;
  if(((pacote->type[0] << 8)|(pacote->type[1])) == 0x0806){
    //______________________ENVIO DOS DADOS NO BUFFER DE TRANSMISSÃO - PACOTES TIPO ARP_____________________________
    // Envio de pacotes do tipo ARP
    unsigned int _op = (pacote->data.arp.op[0] << 8)|(pacote->data.arp.op[1]);
    if(_op == 0x02){
      write_SPI(pacote->data.arp.hard_type[0]);
      _length++;
      write_SPI(pacote->data.arp.hard_type[1]);
      _length++;
      write_SPI(pacote->data.arp.prot_type[0]);
      _length++;
      write_SPI(pacote->data.arp.prot_type[1]);
      _length++;
      write_SPI(pacote->data.arp.hard_size);
      _length++;
      write_SPI(pacote->data.arp.prot_size);
      _length++;
      write_SPI(pacote->data.arp.op[0]);
      _length++;
      write_SPI(pacote->data.arp.op[1]);
      _length++;
      for(i=0;i<6;i++){
        _length++;
        write_SPI(pacote->data.arp.sender_Ethernet_addr[i]);
      }
      for(i=0;i<4;i++){
        _length++;
        write_SPI(pacote->data.arp.sender_IP_addr[i]);
      }
      for(i=0;i<6;i++){
        _length++;
        write_SPI(pacote->data.arp.target_Ethernet_addr[i]);
      }
      for(i=0;i<4;i++){
        _length++;
        write_SPI(pacote->data.arp.target_IP_addr[i]);
      } 
    } 
  } else if(((pacote->type[0] << 8)|(pacote->type[1])) == 0x800){
    //______________________ENVIO DOS DADOS NO BUFFER DE TRANSMISSÃO - PACOTES TIPO IP_____________________________
    _length++;
    write_SPI(pacote->data.IP.version_and_hlength);
    _length++;
    write_SPI(pacote->data.IP.type_service);
    for(i=0;i<2;i++){
      _length++;
      write_SPI(pacote->data.IP.total_length[i]);
    }
    for(i=0;i<2;i++){
      _length++;
      write_SPI(pacote->data.IP.identification[i]);
    }
    for(i=0;i<2;i++){
      _length++;
      write_SPI(pacote->data.IP.flag_displacement[i]);
    }
    _length++;
    write_SPI(pacote->data.IP.life_time);
    _length++;
    write_SPI(pacote->data.IP.protocol);
    for(i=0;i<2;i++){
      _length++;
      write_SPI(pacote->data.IP.head_checksum[i]);
    }
    for(i=0;i<4;i++){
      _length++;
      write_SPI(pacote->data.IP.source_address[i]);
    }
    for(i=0;i<4;i++){
      _length++;
      write_SPI(pacote->data.IP.destination_address[i]);
    }
    if(pacote->data.IP.protocol == 0x01){
        //__________________________ENVIO DOS DADOS NO BUFFER DE TRANSMISSÃO - PACOTES TIPO ICMP__________________________________
         _length++;
         write_SPI(pacote->data.IP.ip_data.icmp.type);
        _length++;
         write_SPI(pacote->data.IP.ip_data.icmp.code);
        for(i=0;i<2;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.icmp.checksum[i]);
        }
        for(i=0;i<4;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.icmp.unused[i]);
        }
        for(i=0; i < pacote->data.IP.ip_data.icmp.length_data; i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.icmp.data[i]);
        }
      } else if(pacote->data.IP.protocol == 0x06){
        
        //__________________________ENVIO DOS DADOS NO BUFFER DE TRANSMISSÃO - PACOTES TIPO TCP_________________________________________
        for(i=0;i<2;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.source_port[i]);
        }
        for(i=0;i<2;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.destination_port[i]);
        }
        for(i=0;i<4;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.sequence_number[i]);
        }
        for(i=0;i<4;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.acknowledgment_number[i]);
        }
        for(i=0;i<2;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.dataOffset_control[i]);
        }
        for(i=0;i<2;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.window_size[i]);
        }
        for(i=0;i<2;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.checksum[i]);
        }
        for(i=0;i<2;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.urgent_pointer[i]);
        }
        
        for(i = 0; i < pacote->data.IP.ip_data.TCP.length_options_padding ; i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.options_padding[i]);
        }
        
        for(i=0;i<pacote->data.IP.ip_data.TCP.length_data;i++){
          _length++;
          write_SPI(pacote->data.IP.ip_data.TCP.data[i]);
        }
        
      }
  }
  finish_comunication
  
  write_controlRegister_ENC28J60(0x06, _length&0xFF);    // Atribuição do byte menos significatvo do tamanho do pacote de transmissão para ETXNDL
  write_controlRegister_ENC28J60(0x07, (_length >> 8));  // Atribuição do byte mais significativo do tamanho do pacote de transmissão para ETXNDH

  // Atribuição do ECON1 para envio do pacote;
  _ECON1 = read_ETH_controlRegister_ENC28J60(0x1F);
  _ECON1 |= (1 << 3);
  write_controlRegister_ENC28J60(0x1F, _ECON1);

  // loop para o envio de todo o pacote
  do{
    _ECON1 = read_ETH_controlRegister_ENC28J60(0x1F);
  }while(_ECON1&(1 << 3));
}

// CHECKSUM_ARRAY___________________________________________________________________________________________________________________________________________________________________________
unsigned int array_checksum(unsigned char *array_bytes, unsigned int length_array){
  unsigned int i, retorno;
  unsigned long _buffer = 0;
  for(i=0; i < length_array; i += 2){
    if(i + 1 < length_array){
      _buffer += (((unsigned int)array_bytes[i]) << 8)|(array_bytes[i+1]);
    } else {
      _buffer += (((unsigned int)array_bytes[i]) << 8)|(0x00);
    }
    _buffer = (_buffer&0xFFFF) + (_buffer >> 16);
  }

  retorno = 0xFFFF&(~_buffer);
  return retorno;
}

// SUBROTINA PARA A CÓPIA DE UM ARRAY PARA OUTRO ARRAY______________________________________________________________________________________________________________________________________________
void copy_array_bytes(unsigned char *array1, unsigned char *array2, unsigned int length_array){
  /* Descrição:
   *      Copia o que está no array2 para o array1
   * Entrada:
   *      unsigned char *array1
   *      unsigned char *array2
   *      unsigned int length_array
   */
  unsigned int i = 0;
  for(i = 0; i < length_array; i++)
    array1[i] = array2[i];
}

// SUBROTINA PARA DELETAR OS DADOS DO PACOTE DE LEITURA GLOBAL______________________________________________________________________________________________________________________________________
void delete_read_packet(){
  /* Descrição:
   *      Subrotina para o delete do pacote de recebimento
   */

   /*
  // Delete da alocação dinamica do ICMP:
  if(READ_PACKET_ENC28J60->data.IP.ip_data.icmp.data != NULL){
    free(READ_PACKET_ENC28J60->data.IP.ip_data.icmp.data);
    READ_PACKET_ENC28J60->data.IP.ip_data.icmp.data = NULL;
  }
  
  
  // Delete da alocação dinamica do TCP:
  if(READ_PACKET_ENC28J60->data.IP.ip_data.TCP.data != NULL){
    free(READ_PACKET_ENC28J60->data.IP.ip_data.TCP.data);
    READ_PACKET_ENC28J60->data.IP.ip_data.TCP.data = NULL;
  }

  READ_PACKET_ENC28J60->data.IP.ip_data.none = 0x00;
  READ_PACKET_ENC28J60->data.none = 0x00;
  
  // Delete total:
  //READ_PACKET_ENC28J60 = __PACKET_NONE;
  */
}

/* DESCRIÇÃO:
 *    Biblioteca para que o Arduino UNO e ENC28J60 atuem como servidor TCP
 *    
 *        Modo =  Half-duplex
 *        fclk = fosc/16
 */

 /* DESCRIÇÃO:
 *    Biblioteca para que o Arduino UNO e ENC28J60 atuem como servidor TCP
 *    
 *        Modo =  Half-duplex
 *        fclk = fosc/16
 */

// DFINIÇÕES DE VARIÁVEIS GLOBAIS______________________________________________________________________________________________________________________________________________________________
unsigned char __PORT[2];
unsigned char __status_handshake; 
//unsigned char __seq_number[4] = {0x77, 0x72, 0x2F, 0xDE};
unsigned long int __seq_number = 0x00000000;
/*
 * 0 - Não houve Handshake
 * 1 - Primeiro pacote do handshake recebido do cliente
 * 2 - Segundo pacote do handshake enviado ao cliente
 * 3 - Terceiro pacote do hanshake recebido do cliente
 * 4 - Estado para o recebimento da mensagem do cliente
 * 5 - Envio do pacote de confirmação ao cliente
 * 6 - Envio do pacote mensagem ao cliente
 * 7 - Estado para o recebimento da mensagem 
 */

///////////////////////////////////////////PARTE REFERENTE AO SERVIDOR TCP//////////////////////////////////////////////////////////////////////////////
// SUBROTINA PARA A INICIALIZAÇÃO DO SERVIDOR TCP______________________________________________________________________________________________________________________________________________
void init_TCPserver(unsigned char* Port){
  copy_array_bytes(__PORT, Port, 2);
  __status_handshake = 0;
  //__seq_number = 0;
}

// SUBROTINA PARA O TRATAMENTO DE PACOTES TCP________________________________________________________________________________________________________________________________________________
void treatment_TCP(packet *READ_PACKET_ENC28J60){
  unsigned int _type = (READ_PACKET_ENC28J60->type[0] << 8)|(READ_PACKET_ENC28J60->type[1]);
  unsigned int i=0;
  unsigned char *IP_target_pacote;
  unsigned char data_send[] = "DATA: {\"TEMP\": \"00.0   \", \"ILUM\": \"80    \", \"UMID\": \"30    \"}         ";
  if(_type == 0x800){
    // **************************************************PROTOCOLO IPV4*****************************************************************
    unsigned char ip_version = READ_PACKET_ENC28J60->data.IP.protocol;
    IP_target_pacote = READ_PACKET_ENC28J60->data.IP.destination_address;
    if(match_Array(IP_target_pacote, __IP_EN28J60, 4)){ 
      // Verificação do endereço IP de destino
      if(ip_version == 0x06){
        packet pacote_send;
        // ***********************************************PROTOCOLO TCP*****************************************************************
        unsigned int porta_pacote = (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.destination_port[0] << 8)|READ_PACKET_ENC28J60->data.IP.ip_data.TCP.destination_port[1];
        if(porta_pacote == ((__PORT[0] << 8)|(__PORT[1]))){
          //**********************************************VERIFICANDO A PORTA***********************************************************
          //unsigned char SYN = ((READ_PACKET_ENC28J60->data.IP.ip_data.TCP.dataOffset_control[1])&1) << 1;
          unsigned char SYN = (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.dataOffset_control[1] >> 1)&1;
          unsigned char ACK = (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.dataOffset_control[1] >> 4)&1;
          unsigned char RST = (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.dataOffset_control[1] >> 2)&1;
          unsigned char PSH = (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.dataOffset_control[1] >> 3)&1;
          unsigned char FIN = (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.dataOffset_control[1])&1;
          unsigned int length_ip;
          unsigned long int ack_number;
          //Serial.println("PACOTE RECEBIDO:");
          //displayPacket(&READ_PACKET_ENC28J60);                   // Display na tela do Pacote Recebido
          //Serial.println("*****************************************************************");
          //if(__status_handshake != 3){
          //Serial.println("VERIFICOU A PORTA DO PACOTE");
          if(__status_handshake < 3){
            //*********************************************COMUNICAÇÃO NÃO ESTABELECIDA PELO HANDSHAKE***********************************
            //if((__status_handshake == 0) &&  SYN == 1){
            if(SYN == 1 && ACK == 0 && __status_handshake == 0){
              //Serial.println("1º Mensagem Handshake recebida");
              //*******************************************TRATAMENTO DO PRIMEIRO PACOTE DO HANDSHAKE RECEBIDO***************************
              // 1º Pacote handshake recebido: SYN == 1 e __status_handshake == 0
              // Montagem do segundo pacote
              
              //Serial.println("PACOTE PARA ENVIO:");
              __status_handshake = 1;
              //---------------CRIAÇÃO DO PACOTE TCP GENÉRICO-----------------------------------------------------------------
              create_tcp_packet(&(pacote_send), READ_PACKET_ENC28J60);  
              //---------------CASO TIVESSE DADOS, DEVERIA SER PREENCHIDO NESSA LINHA-----------------------------------------
              //---------------CÁLCULO DO TAMANHO DO PACOTE IP----------------------------------------------------------------
              length_ip = 40 +  pacote_send.data.IP.ip_data.TCP.length_options_padding +  pacote_send.data.IP.ip_data.TCP.length_data; // Calculo do tamanho pacote IP
              pacote_send.data.IP.total_length[0] = length_ip >> 8;
              pacote_send.data.IP.total_length[1] = length_ip&0xFF;
              //---------------CÁLCULO DO CHECKSUM IP-------------------------------------------------------------------------
              ip_checksum(&(pacote_send.data.IP));  // Calculo do checksum IP
              //---------------CÁLCULO DA SEQUÊNCIA DE RECONHECIMENTO DO TCP--------------------------------------------------
              
              ack_number= ((((unsigned long int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[0]) << 24)|
               (((unsigned long int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[1]) << 16)|
               (((unsigned int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[2]) << 8)|
               (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[3])) + 1;

              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[0] = ack_number >> 24;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[1] = (ack_number >> 16)&0xFF;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[2] = (ack_number >> 8)&0xFF;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[3] = (ack_number)&0xFF;
              
              //---------------ATRIBUIÇÃO DO OFFSET----------------------------------------------------------------------------
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[0] = 0x60; // Atribuição do Offset
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[1] = 0x12;
              //---------------CÁLCULO CHECKSUM TCP----------------------------------------------------------------------------
              tcp_checksum(&(pacote_send.data.IP));  // Cálculo do checksum TCP
              //displayPacket(&(pacote_send));              
              write_bufferMemory_ENC28J60(&(pacote_send));
              __status_handshake = 2;
              //Serial.println("2º Mensagem Handshake enviada");
            } else if(SYN == 0 && ACK == 1 && __status_handshake == 2){
              //*******************************************TRATAMENTO DO TERCEIRO PACOTE DO HANDSHAKE***********************************
              //Serial.println("3º Mensagem Handshake recebida");
              //displayPacket(READ_PACKET_ENC28J60);
              if(__seq_number == 0xFFFFFFFF){
                __seq_number = 0;
              }else{
                __seq_number++;;
              }
               __status_handshake = 3;
            }
          } else{
            //*********************************************COMUNICAÇÃO JÁ ESTABELECIDA PELO HANDSHAKE***********************************
            if(PSH == 1 && ACK == 1 && __status_handshake == 3){
              //*******************************************TRATAMENTO DO PACOTE DO RECEBIMENTO DE DADOS*********************************
              //Serial.println("4º Mensagem recebida");
              //    ENVIO DO PACOTE DE CONFIRMAÇÃO PARA O CLIENTE
              //---------------CRIAÇÃO DO PACOTE TCP GENÉRICO-----------------------------------------------------------------
              create_tcp_packet(&(pacote_send), READ_PACKET_ENC28J60);  

              //---------------CASO TIVESSE DADOS, DEVERIA SER PREENCHIDO NESSA LINHA-----------------------------------------
              //---------------CÁLCULO DO TAMANHO DO PACOTE IP----------------------------------------------------------------
              length_ip = 40 +  pacote_send.data.IP.ip_data.TCP.length_options_padding +  pacote_send.data.IP.ip_data.TCP.length_data; // Calculo do tamanho pacote IP
              pacote_send.data.IP.total_length[0] = length_ip >> 8;
              pacote_send.data.IP.total_length[1] = length_ip&0xFF;
              //---------------CÁLCULO DO CHECKSUM IP-------------------------------------------------------------------------
              ip_checksum(&(pacote_send.data.IP));  // Calculo do checksum IP
              //---------------CÁLCULO DA SEQUÊNCIA DE RECONHECIMENTO DO TCP--------------------------------------------------
              
              ack_number= ((((unsigned long int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[0]) << 24)|
               (((unsigned long int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[1]) << 16)|
               (((unsigned int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[2]) << 8)|
               (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[3])) + READ_PACKET_ENC28J60->data.IP.ip_data.TCP.length_data;

              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[0] = ack_number >> 24;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[1] = (ack_number >> 16)&0xFF;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[2] = (ack_number >> 8)&0xFF;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[3] = (ack_number)&0xFF;
              
              //---------------ATRIBUIÇÃO DO OFFSET----------------------------------------------------------------------------
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[0] = 0x60;; // Atribuição do Offset
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[1] = 0x10; // 0x00
              //---------------CÁLCULO CHECKSUM TCP----------------------------------------------------------------------------
              tcp_checksum(&(pacote_send.data.IP));  // Cálculo do checksum TCP
              //displayPacket(&(pacote_send));

              write_bufferMemory_ENC28J60(&(pacote_send)); // ENVIO DO PACOTE DE CONFIRMAÇÃO
              //Serial.println("5º Envio do pacote de confirmação");
              //displayPacket((READ_PACKET_ENC28J60));
              //*****************TRATAMENTO DO ENVIO DA MENSAGEM DO SERVIDOR PARA O CLIENTE*****
              //--------------------LEITURA DOS DADOS--------------------------------------------------//
              copy_array_bytes(data_send+53,__umid ,4);
              copy_array_bytes(data_send+35,__ilum ,4);
              Serial.println(__temp);
              copy_array_bytes(data_send+16,__temp ,4);
              Serial.println(__temp);
              copy_array_bytes(pacote_send.data.IP.ip_data.TCP.data, data_send, sizeof(data_send)/sizeof(data_send[0]));  // MUDAR TAMANHO DE BYTES DEPOIS
              pacote_send.data.IP.ip_data.TCP.length_data = sizeof(data_send)/sizeof(data_send[0]);                       // MUDAR TAMANHO DE BYTES DEPOIS 
              //---------------CÁLCULO DO TAMANHO DO PACOTE IP----------------------------------------------------------------
              length_ip = 40 +  pacote_send.data.IP.ip_data.TCP.length_options_padding +  pacote_send.data.IP.ip_data.TCP.length_data; // Calculo do tamanho pacote IP
              pacote_send.data.IP.total_length[0] = length_ip >> 8;
              pacote_send.data.IP.total_length[1] = length_ip&0xFF;

              ip_checksum(&(pacote_send.data.IP));  // Calculo do checksum IP

              pacote_send.data.IP.ip_data.TCP.dataOffset_control[0] = 0x60;; // Atribuição do Offset
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[1] = 0x18; // 0x00

              tcp_checksum(&(pacote_send.data.IP));  // Cálculo do checksum TCP
              
              /*
              UMIDADE(data_send+53);
              SENSOR_LUMINOSIDADE(data_send+35);
              temperature(data_send+16);     
              */       
              //---------------------------------------------------------------------------------------//S
              write_bufferMemory_ENC28J60(&(pacote_send)); // ENVIO DO PACOTE DE CONFIRMAÇÃO
              //Serial.println("6º Envio de mensagem");
              __status_handshake = 4;
            } else if(ACK == 1 & FIN == 0 && __status_handshake == 4) {
              //*****************RECEBIMENTO DO PACOTE DE CONFIRMAÇÃO DO CLIENTE********************************************************************************
              __seq_number += sizeof(data_send)/sizeof(data_send[0]);
              //Serial.println("7º Recebimento do pacote de confirmação");
              __status_handshake = 5;
            } else if(ACK == 1 & FIN == 1 && __status_handshake == 5){
              //Serial.println("8º Recebimento do primeiro pacote closes");
              //*****************TRATAMENTO DO COMANDO CLOSE DO CLIENTE PARA O SERVIDOR************************************************************************
              __status_handshake = 6; // status para fechamento
              //displayPacket((READ_PACKET_ENC28J60));
               //---------------CRIAÇÃO DO PACOTE TCP GENÉRICO-----------------------------------------------------------------
              create_tcp_packet(&(pacote_send), READ_PACKET_ENC28J60);  

              //copy_array_bytes(pacote_send.data.IP.ip_data.TCP.sequence_number, pacote_send.data.IP.ip_data.TCP.acknowledgment_number, 4);
              //---------------CÁLCULO DO TAMANHO DO PACOTE IP----------------------------------------------------------------
              length_ip = 40 +  pacote_send.data.IP.ip_data.TCP.length_options_padding +  pacote_send.data.IP.ip_data.TCP.length_data; // Calculo do tamanho pacote IP
              pacote_send.data.IP.total_length[0] = length_ip >> 8;
              pacote_send.data.IP.total_length[1] = length_ip&0xFF;
              //---------------CÁLCULO DO CHECKSUM IP-------------------------------------------------------------------------
              ip_checksum(&(pacote_send.data.IP));  // Calculo do checksum IP
              //---------------CÁLCULO DA SEQUÊNCIA DE RECONHECIMENTO DO TCP--------------------------------------------------
              
              ack_number= ((((unsigned long int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[0]) << 24)|
               (((unsigned long int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[1]) << 16)|
               (((unsigned int) READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[2]) << 8)|
               (READ_PACKET_ENC28J60->data.IP.ip_data.TCP.sequence_number[3])) + 1;

              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[0] = ack_number >> 24;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[1] = (ack_number >> 16)&0xFF;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[2] = (ack_number >> 8)&0xFF;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[3] = (ack_number)&0xFF;
              
              //---------------ATRIBUIÇÃO DO OFFSET----------------------------------------------------------------------------
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[0] = 0x60;; // Atribuição do Offset
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[1] = 0x10; // 0x00
              //---------------CÁLCULO CHECKSUM TCP----------------------------------------------------------------------------
              tcp_checksum(&(pacote_send.data.IP));  // Cálculo do checksum TCP
              write_bufferMemory_ENC28J60(&(pacote_send)); // ENVIO DO PACOTE DE CONFIRMAÇÃO
              //Serial.println("9º Envio do pacote de confirmação para fechar");
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[0] = 0x60;; // Atribuição do Offset
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[1] = 0x11;

              tcp_checksum(&(pacote_send.data.IP));  // Cálculo do checksum TCP
              write_bufferMemory_ENC28J60(&(pacote_send)); // ENVIO DO PACOTE DE CONFIRMAÇÃO
              //Serial.println("10º Envio do pacote FIN");
              __status_handshake = 6;
            } else if(ACK == 1 & FIN == 0 && __status_handshake == 6){
              //*****************TRATAMENTO PARA O PACOTE DE CONFIRMAÇÃO DO CLIENTE PARA FECHAR A CONEXÃO TCP*********
              __status_handshake=0;
              __seq_number=0;
              //Serial.println("11º Recebimento do pacote de confirmação apra fechar a cnexao");
              //displayPacket(READ_PACKET_ENC28J60);
            }
              //*****************TRATAMENTO DO COMANDO RESET DO CLIENTE PARA O SERVIDOR*********(ANALISAR DEPOIS)
              //*****************TRATAMENTO DO COMANDO CLOSE DO CLIENTE PARA O SERVIDOR*********
          }
        }
      }
    }
  }
}

// SUBROTINA PARA O CÁLCULO DO CABEÇALHO DO CHECKSUM IP____________________________________________________________________________________________________________________________________
void ip_checksum(ip_protocol* ip_packet){
  unsigned int ipchecksum, i;
  unsigned char header_ip[20];
  header_ip[0] = ip_packet->version_and_hlength;
  header_ip[1] = ip_packet->type_service;
  copy_array_bytes(header_ip+2, ip_packet->total_length, 2);
  copy_array_bytes(header_ip+4, ip_packet->identification, 2);
  copy_array_bytes(header_ip+6, ip_packet->flag_displacement, 2);
  header_ip[8] = ip_packet->life_time;
  header_ip[9] = ip_packet->protocol;
  copy_array_bytes(header_ip+10, ip_packet->source_address, 4);
  copy_array_bytes(header_ip+14, ip_packet->destination_address, 4);
  ipchecksum = array_checksum(header_ip, 18);
  ip_packet->head_checksum[0] = ipchecksum >> 8;
  ip_packet->head_checksum[1] = ipchecksum&0xFF;
}

// SUBROTINA PARA O CÁLCULO DO CABEÇALHO DO CHECKSUM TCP___________________________________________________________________________________________________________________________________
void tcp_checksum(ip_protocol* ip_packet){
  unsigned int TCPchecksum;
  unsigned int length_vetor_tcp = 30 + ip_packet->ip_data.TCP.length_options_padding + ip_packet->ip_data.TCP.length_data;
  unsigned int length_tcp = 20 + ip_packet->ip_data.TCP.length_options_padding + ip_packet->ip_data.TCP.length_data; 
  unsigned char checksumArray[length_vetor_tcp];
  copy_array_bytes(checksumArray, ip_packet->source_address, 4);
  copy_array_bytes(checksumArray+4, ip_packet->destination_address, 4);
  checksumArray[8] = 0x00;
  checksumArray[9] = ip_packet->protocol;
  checksumArray[10] = length_tcp >> 8;
  checksumArray[11] = length_tcp&0xFF;
  copy_array_bytes(checksumArray+12, ip_packet->ip_data.TCP.source_port, 2);
  copy_array_bytes(checksumArray+14, ip_packet->ip_data.TCP.destination_port, 2);
  copy_array_bytes(checksumArray+16, ip_packet->ip_data.TCP.sequence_number, 4);
  copy_array_bytes(checksumArray+20, ip_packet->ip_data.TCP.acknowledgment_number, 4);
  copy_array_bytes(checksumArray+24, ip_packet->ip_data.TCP.dataOffset_control, 2);
  copy_array_bytes(checksumArray+26, ip_packet->ip_data.TCP.window_size, 2);
  //copy_array_bytes(checksumArray+27, ip_packet->ip_data.TCP.checksum, 2);
  copy_array_bytes(checksumArray+28, ip_packet->ip_data.TCP.urgent_pointer, 2);
  copy_array_bytes(
                      checksumArray+30, 
                      ip_packet->ip_data.TCP.options_padding,
                      ip_packet->ip_data.TCP.length_options_padding
                  );
  copy_array_bytes(
                      checksumArray+30+ip_packet->ip_data.TCP.length_options_padding, // POSIÇÃO/ENDEREÇO
                      ip_packet->ip_data.TCP.data,          // DADOS
                      ip_packet->ip_data.TCP.length_data    // TAMANHO
                  );
  TCPchecksum = array_checksum(checksumArray, length_vetor_tcp);
  ip_packet->ip_data.TCP.checksum[0] = (TCPchecksum >> 8); // apagar o +1
  ip_packet->ip_data.TCP.checksum[1] = TCPchecksum&0xFF;
  /*
  unsigned int i=0;
  for(i=0;i<length_vetor_tcp;i++){
    Serial.println(checksumArray[i], HEX);
  }
  */
}

// SUBROTINA PARA A MONTAGEM GENERALIZADA DO PACOTE TCP/IP_________________________________________________________________________________________________________________________________
void create_tcp_packet(packet *pacote_send, packet *READ_PACKET_ENC28J60){
  /*
   * 
   * NÃO FAZ TOTAL LENGTH DO IP
   * NÃO FAZ CHECKSUM DO IP
   * NÃO FAZ O DATA OFFSET E CONTROL DO TCP
   * NÃO FAZ A SEQUÊNCIA ACK
   * NÃO FAZ CHECKSUM DO TCP
   * NÃO PREENCHE O VETOR DE DADOS DO TCP
   */
  //Serial.println(__seq_number, HEX);
  unsigned char seq_number[4] = {
                                  (__seq_number >> 24)&0xFF,
                                  (__seq_number >> 16)&0xFF,
                                  (__seq_number >> 8)&0xFF,
                                  (__seq_number)&0xFF,
                               };
  
  //******************************************************MONTAGEM CAMADA ENLACE***********************************************************
  
  copy_array_bytes(pacote_send->destination_address, READ_PACKET_ENC28J60->source_address, 6);   // ATRIBUIÇÃO DO ENDEREÇO MAC DE DESTINO
  
  copy_array_bytes(pacote_send->source_address, __MAC_EN28J60, 6);                             // ATRIBUIÇÃO DO ENDEREÇO MAC DE ORIGEM
  
  copy_array_bytes(pacote_send->source_address, __MAC_EN28J60, 6);                             
  pacote_send->type[0] = 0x08;                                                                 // ATRIBUIÇÃO DO 1º BYTE DO TIPO DE PACOTE
  pacote_send->type[1] = 0x00;                                                                 // ATRIBUIÇÃO DO 2º BYTE DO TIPO DE PACOTE

  //*******************************************************MONTAGEM PROTOCOLO IP***********************************************************
  
  pacote_send->data.IP.version_and_hlength = 0x45;                                             // ATRIBUIÇÃO DA VERSÃO E TAMANHO
  pacote_send->data.IP.type_service = 0x00;                                                    // ATRIBUIÇÃO DO TIPO DE SERVIÇO
  //pacote_send.data.IP.total_length[0] = 0x00;  // MUDAR DEPOIS O TAMANHO DO
  //pacote_send.data.IP.total_length[1] = 0x54;  // TAMANHO DE 84 BYTES
  
  copy_array_bytes(pacote_send->data.IP.identification, READ_PACKET_ENC28J60->data.IP.identification, 2);   
  //pacote_send->data.IP.identification[0] = 0x00;
  //pacote_send->data.IP.identification[1] = 0x00;   
  copy_array_bytes(pacote_send->data.IP.flag_displacement, READ_PACKET_ENC28J60->data.IP.flag_displacement, 2);
  
  pacote_send->data.IP.life_time = 0x40;
  pacote_send->data.IP.protocol = 0x06;
  
  copy_array_bytes(pacote_send->data.IP.source_address, READ_PACKET_ENC28J60->data.IP.destination_address, 4);
  copy_array_bytes(pacote_send->data.IP.destination_address, READ_PACKET_ENC28J60->data.IP.source_address, 4);

  // CHECKSUM IP: FAZER DEPOIS
  //ip_checksum(&(pacote_send.data.IP));

  //*****************************************************MONTAGEM PROTOCOLO TCP***********************************************************
  
  copy_array_bytes(pacote_send->data.IP.ip_data.TCP.source_port, __PORT, 2);
  copy_array_bytes(
                    pacote_send->data.IP.ip_data.TCP.destination_port, 
                    READ_PACKET_ENC28J60->data.IP.ip_data.TCP.source_port,
                    2
                   );

  copy_array_bytes(pacote_send->data.IP.ip_data.TCP.sequence_number, seq_number, 4);
  
  pacote_send->data.IP.ip_data.TCP.window_size[0] = READ_PACKET_ENC28J60->data.IP.ip_data.TCP.window_size[0];
  pacote_send->data.IP.ip_data.TCP.window_size[1] = READ_PACKET_ENC28J60->data.IP.ip_data.TCP.window_size[1];
  
  // CHECKSUM TCP: FAZER
  pacote_send->data.IP.ip_data.TCP.urgent_pointer[0] = 0x00;
  pacote_send->data.IP.ip_data.TCP.urgent_pointer[1] = 0x00;

  pacote_send->data.IP.ip_data.TCP.length_options_padding = 4; // 0x01F4
  copy_array_bytes(  
                      pacote_send->data.IP.ip_data.TCP.options_padding, 
                      READ_PACKET_ENC28J60->data.IP.ip_data.TCP.options_padding, 
                      pacote_send->data.IP.ip_data.TCP.length_options_padding
                  );
  
  //pacote_send->data.IP.ip_data.TCP.options_padding[2] = 0x01;  // LIMITANDO O TAMANHO DO PACOTE PARA 500 BYTES
  //pacote_send->data.IP.ip_data.TCP.options_padding[3] = 0xF4;
  pacote_send->data.IP.ip_data.TCP.length_data = 0;
}
 
