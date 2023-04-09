/* DESCRIÇÃO:
 *    
 *    Biblioteca para tratamento de pacotes Ethernet do módulo ENC28J60
 *        
 */

// DECLARAÇÕES DE TIPOS DE ESTRUTURAS__________________________________________________________________________________________________________________________________________________________
typedef struct SUDOSU_protocol{
  // struct do protocolo SUDOSU
  
} sudosu_protocol;

typedef struct TCP_protocol{
  // struct do protocolo TCP
  unsigned char source_port[2];
  unsigned char destination_port[2];
  unsigned char sequence_number[4];
  unsigned char acknowledgment_number[4];
  unsigned char dataOffset_control[2];
  unsigned char window_size[2];
  unsigned char checksum[2];
  unsigned char urgent_pointer[2];
  unsigned char options_padding[4];
  unsigned char data[100];
  unsigned int length_options_padding;  // atribui ele
  unsigned int length_data;             // atribui ele
} tcp_protocol;

typedef struct Icmp_protocol{
  // struct do protocolo ICMP
  unsigned char type;
  unsigned char code;
  unsigned char checksum[2];
  unsigned char unused[4];
  unsigned int length_data; // atribui ele
  unsigned char data[60];
} icmp_protocol;

typedef union IP_type{
  Icmp_protocol icmp;
  tcp_protocol  TCP;
} ip_type;

typedef struct IP_protocol{
  // struct do protocolo IP
  unsigned char version_and_hlength;
  unsigned char type_service;
  unsigned char total_length[2];
  unsigned char identification[2];
  unsigned char flag_displacement[2];
  unsigned char life_time;
  unsigned char protocol;
  unsigned char head_checksum[2];
  unsigned char source_address[4];
  unsigned char destination_address[4];
  unsigned char* options;
  ip_type ip_data;
} ip_protocol;

typedef struct Arp_protocol{
  // struct do protocolo ARP
  unsigned char hard_type[2];
  unsigned char prot_type[2];
  unsigned char hard_size;
  unsigned char prot_size;
  unsigned char op[2];
  unsigned char sender_Ethernet_addr[6];
  unsigned char sender_IP_addr[4];
  unsigned char target_Ethernet_addr[6];
  unsigned char target_IP_addr[4];
} arp_protocol;

typedef union Packet_payload{
  // union para cada estrutura de pacote
  arp_protocol arp;
  ip_protocol  IP;
} packet_payload;

typedef struct Packet{
  // Struct para fila de pacotes
  unsigned char _status[4];
  unsigned char destination_address[6];
  unsigned char source_address[6];
  unsigned char type[2];
  unsigned char CRC[4];       // MODIFIQUEI O LUGAR DO CRC
  packet_payload data;
} packet;

// PROTÓTIPOS DE SUBROTINA_____________________________________________________________________________________________________________________________________________________________________
void displayPacket(packet*);
void displayData(unsigned int, unsigned int, unsigned int);

// SUBROTINA PARA O DISPLAY DE ALGUNS DADOS SOBRE O PACOTE ATUAL_______________________________________________________________________________________________________________________________
void displayData(unsigned int length_packet, unsigned int _ERDPT, unsigned int next_packet_pointer){
  Serial.println("Ponteiro de Inicio: ");
  Serial.println(_ERDPT, HEX);
  Serial.println("Ponteiro de Fim: ");
  Serial.println(next_packet_pointer, HEX);
  Serial.println("Tamanho Pacote: ");
  Serial.println(length_packet);
  Serial.println("//////////////////////////////////////////");
}

// SUBROTINA PARA O DISPLAY DOS PACOTES________________________________________________________________________________________________________________________________________________________
void displayPacket(packet* pacote){
  unsigned int i = 0;
  unsigned int _type;
  //Serial.println("________________________________________");
  Serial.print("Status[0]:");
  Serial.print(pacote->_status[0], HEX);
  Serial.print(", ");
  Serial.print("Status[1]:");
  Serial.print(pacote->_status[1], HEX);
  Serial.print(", ");
  Serial.print("Status[2]:");
  Serial.print(pacote->_status[2], HEX);
  Serial.print(", ");
  Serial.print("Status[3]:");
  Serial.print(pacote->_status[3], HEX);
  Serial.print(", ");
  Serial.print("ENDEREÇO DESTINO:");
  Serial.print(pacote->destination_address[0], HEX);
  Serial.print(", ");
  Serial.print(pacote->destination_address[1], HEX);
  Serial.print(", ");
  Serial.print(pacote->destination_address[2], HEX);
  Serial.print(", ");
  Serial.print(pacote->destination_address[3], HEX);
  Serial.print(", ");
  Serial.print(pacote->destination_address[4], HEX);
  Serial.print(", ");
  Serial.println(pacote->destination_address[5], HEX);
  Serial.print("ENDEREÇO FONTE:");
  Serial.print(pacote->source_address[0], HEX);
  Serial.print(", ");
  Serial.print(pacote->source_address[1], HEX);
  Serial.print(", ");
  Serial.print(pacote->source_address[2], HEX);
  Serial.print(", ");
  Serial.print(pacote->source_address[3], HEX);
  Serial.print(", ");
  Serial.print(pacote->source_address[4], HEX);
  Serial.print(", ");
  Serial.println(pacote->source_address[5], HEX);
  Serial.print("TIPO:");
  Serial.print(pacote->type[0], HEX);
  Serial.print(", ");
  Serial.println(pacote->type[1], HEX);
  _type = (pacote->type[0] << 8)|(pacote->type[1]);
  switch(_type){
    case 0x806: // Protocolo ARP
      Serial.print("HARD TYPE: ");
      Serial.print(pacote->data.arp.hard_type[0], HEX);
      Serial.print(", ");
      Serial.println(pacote->data.arp.hard_type[1], HEX);
      Serial.print("PROT TYPE: ");
      Serial.print(pacote->data.arp.prot_type[0], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.prot_type[1], HEX);
      Serial.println(", ");
      Serial.print("HARD SIZE: ");
      Serial.println(pacote->data.arp.hard_size, HEX);
      Serial.print("PROT SIZE: ");
      Serial.println(pacote->data.arp.prot_size, HEX);
      Serial.print("OP: ");
      Serial.print(pacote->data.arp.op[0], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.op[1], HEX);
      Serial.println(", ");
      Serial.print("SENDER ETHERNET ADDR: ");
      Serial.print(pacote->data.arp.sender_Ethernet_addr[0], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.sender_Ethernet_addr[1], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.sender_Ethernet_addr[2], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.sender_Ethernet_addr[3], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.sender_Ethernet_addr[4], HEX);
      Serial.print(", ");
      Serial.println(pacote->data.arp.sender_Ethernet_addr[5], HEX);
      Serial.print("SENDER IP ADDR: ");
      Serial.print(pacote->data.arp.sender_IP_addr[0]);
      Serial.print(", ");
      Serial.print(pacote->data.arp.sender_IP_addr[1]);
      Serial.print(", ");
      Serial.print(pacote->data.arp.sender_IP_addr[2]);
      Serial.print(", ");
      Serial.println(pacote->data.arp.sender_IP_addr[3]);
      Serial.print("TARGET ETHERNET ADDR:");
      Serial.print(pacote->data.arp.target_Ethernet_addr[0], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.target_Ethernet_addr[1], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.target_Ethernet_addr[2], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.target_Ethernet_addr[3], HEX);
      Serial.print(", ");
      Serial.print(pacote->data.arp.target_Ethernet_addr[4], HEX);
      Serial.print(", ");
      Serial.println(pacote->data.arp.target_Ethernet_addr[5], HEX);
      Serial.print("TARGET IP ADDR:");
      Serial.print(pacote->data.arp.target_IP_addr[0]);
      Serial.print(", ");
      Serial.print(pacote->data.arp.target_IP_addr[1]);
      Serial.print(", ");
      Serial.print(pacote->data.arp.target_IP_addr[2]);
      Serial.print(", ");
      Serial.println(pacote->data.arp.target_IP_addr[3]);
      break;
    case 0x800: // Protocolo IP
      Serial.print("VERSION AND HEADER LENGTH:");
      Serial.println(pacote->data.IP.version_and_hlength, HEX);
      Serial.print("TYPE SERVICE:");
      Serial.println(pacote->data.IP.type_service, HEX);
      Serial.print("TOTAL LENGTH:");
      for(i=0; i<2; i++){
        Serial.print(pacote->data.IP.total_length[i], HEX);
        Serial.print(", ");
      }
      Serial.println("");
      Serial.print("IDENTIFICATION:");
      for(i=0; i<2; i++){
        Serial.print(pacote->data.IP.identification[i], HEX);
        Serial.print(", ");
      }
      Serial.println("");
      Serial.println("FLAG AND DISPLACEMENT:");
      for(i=0; i<2; i++){
        Serial.print(pacote->data.IP.flag_displacement[i], HEX);
        Serial.print(", ");
      }
      Serial.println("");
      Serial.print("LIFE TIME:");
      Serial.println(pacote->data.IP.life_time, HEX);
      Serial.print("PROTOCOL:");
      Serial.println(pacote->data.IP.protocol, HEX);
      Serial.print("CHECKSUM HEADER IP:");
      for(i=0; i<2; i++){
        Serial.print(pacote->data.IP.head_checksum[i], HEX);
        Serial.print(", ");
      }
      Serial.println("");
      Serial.println("IP SOURCE ADDRESS:");
      for(i=0; i<4; i++){
        Serial.print(pacote->data.IP.source_address[i]);
        Serial.print(", ");
      }
      Serial.println("");
      Serial.print("IP DEST ADDRESS:");
      for(i=0; i<4; i++){
        Serial.print(pacote->data.IP.destination_address[i]);
        Serial.print(", ");
      }
      Serial.println("");
      switch(pacote->data.IP.protocol){
        case 0x01:  // PROTOCOLO ICMP
          Serial.print("TYPE ICMP:");
          Serial.println(pacote->data.IP.ip_data.icmp.type, HEX);
          Serial.print("CODE ICMP:");
          Serial.println(pacote->data.IP.ip_data.icmp.code, HEX);
          Serial.print("CHECKSUM ICMP:");
          for(i=0; i<2; i++){
            Serial.print(pacote->data.IP.ip_data.icmp.checksum[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("MERDA NENHUMA ICMP:");
          for(i=0; i<4; i++){
            Serial.print(pacote->data.IP.ip_data.icmp.unused[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          // PRINTAR OS DADOS
          Serial.print("Dados ICMP:");
          for(i = 0; i < pacote->data.IP.ip_data.icmp.length_data; i++){
            Serial.print(pacote->data.IP.ip_data.icmp.data[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          break;
        case 0x06:  // PROTOCOLO TCP
          Serial.print("SOURCE PORT:");
          for(i = 0; i < 2; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.source_port[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("DESTINATION PORT:");
          for(i = 0; i < 2; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.destination_port[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("SEQUENCE NUMBER:");
          for(i = 0; i < 4; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.sequence_number[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("ACKNOWLEDGMENT NUMBER:");
          for(i = 0; i < 4; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.acknowledgment_number[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("DATA OFFSET, RESERVED AND CONTROL:");
          for(i = 0; i < 2; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.dataOffset_control[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("WINDOW SIZE:");
          for(i = 0; i < 2; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.window_size[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("CHECKSUM TCP:");
          for(i = 0; i < 2; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.checksum[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("URGENT POINTER:");
          for(i = 0; i < 2; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.urgent_pointer[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("OPÇÕES E PREENCHIMENTO:");
          for(i = 0; i < pacote->data.IP.ip_data.TCP.length_options_padding; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.options_padding[i], HEX);
            Serial.print(", ");
          }
          Serial.println("");
          Serial.print("DADOS:");
          //Serial.print(pacote->data.IP.ip_data.TCP.data);
          
          for(i = 0; i < pacote->data.IP.ip_data.TCP.length_data; i++){
            Serial.print(pacote->data.IP.ip_data.TCP.data[i], HEX);
            Serial.print(", ");
          }
          
          Serial.println("");
      }
      break;
    default:
      break;
  }
  Serial.println("________________________________________");
}
