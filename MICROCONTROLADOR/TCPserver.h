
// SUBROTINA PARA O TRATAMENTO DE PACOTES TCP________________________________________________________________________________________________________________________________________________
void treatment_TCP(packet* READ_PACKET_ENC28J60){
  unsigned int _type = (READ_PACKET_ENC28J60.type[0] << 8)|(READ_PACKET_ENC28J60.type[1]);
  unsigned int i=0;
  unsigned char *IP_target_pacote;
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
          unsigned int length_ip;
          unsigned long int ack_number;
          //Serial.println("PACOTE RECEBIDO:");
          //displayPacket(&READ_PACKET_ENC28J60);                   // Display na tela do Pacote Recebido
          //Serial.println("*****************************************************************");
          //if(__status_handshake != 3){
          if(__status_handshake < 3){
            //*********************************************COMUNICAÇÃO NÃO ESTABELECIDA PELO HANDSHAKE***********************************
            //if((__status_handshake == 0) &&  SYN == 1){
            if(SYN == 1 && ACK == 0){
              //*******************************************TRATAMENTO DO PRIMEIRO PACOTE DO HANDSHAKE RECEBIDO***************************
              // 1º Pacote handshake recebido: SYN == 1 e __status_handshake == 0
              // Montagem do segundo pacote
              Serial.println("1º apos o handshake");
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
              delete_read_packet();
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[0] = ack_number >> 24;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[1] = (ack_number >> 16)&0xFF;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[2] = (ack_number >> 8)&0xFF;
              pacote_send.data.IP.ip_data.TCP.acknowledgment_number[3] = (ack_number)&0xFF;
              
              //---------------ATRIBUIÇÃO DO OFFSET----------------------------------------------------------------------------
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[0] = 0x60; // Atribuição do Offset
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[1] = 0x12;
              //---------------CÁLCULO CHECKSUM TCP----------------------------------------------------------------------------
              tcp_checksum(&(pacote_send.data.IP));  // Cálculo do checksum TCP
              displayPacket(&(pacote_send));
              
              write_bufferMemory_ENC28J60(&(pacote_send));
              Serial.println("2º apos o handshake");
              if(__seq_number == 0xFFFFFFFF){
                __seq_number = 0;
              }else{
                __seq_number++;;
              }
              __status_handshake = 2;
              
            } else if(SYN == 0 && ACK == 1){
              //*******************************************TRATAMENTO DO TERCEIRO PACOTE DO HANDSHAKE***********************************
               __status_handshake = 3;
               Serial.println("3º handshake");
            }
          } else{
            //*********************************************COMUNICAÇÃO JÁ ESTABELECIDA PELO HANDSHAKE***********************************
            if(PSH == 1 && ACK == 1){
              Serial.println("4º apos o handshake");
              //displayPacket(&(pacote_send));
              /*
              //*******************************************TRATAMENTO DO PACOTE DO RECEBIMENTO DE DADOS*********************************
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
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[0] = 0x80; // Atribuição do Offset
              pacote_send.data.IP.ip_data.TCP.dataOffset_control[1] = 0x10; // 0x00
              //---------------CÁLCULO CHECKSUM TCP----------------------------------------------------------------------------
              tcp_checksum(&(pacote_send.data.IP));  // Cálculo do checksum TCP
              //displayPacket(&(pacote_send));
              //displayPacket(&(READ_PACKET_ENC28J60));
              write_bufferMemory_ENC28J60(&(pacote_send)); // ENVIO DO PACOTE DE CONFIRMAÇÃO
              
              //*****************TRATAMENTO DO ENVIO DA MENSAGEM DO SERVIDOR PARA O CLIENTE*****
              
              */
            } 
              //*****************TRATAMENTO DO COMANDO RESET DO CLIENTE PARA O SERVIDOR*********(ANALISAR DEPOIS)
              //*****************TRATAMENTO DO COMANDO CLOSE DO CLIENTE PARA O SERVIDOR*********
          }
        }
      }
    }
  }
  Serial.println(freeMemory());
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
  }*/
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

  //pacote_send.data.IP.ip_data.TCP.dataOffset_control[0] = 0xA0;
  //pacote_send.data.IP.ip_data.TCP.dataOffset_control[1] = 0x12;

  //pacote_send->data.IP.ip_data.TCP.window_size[0] = 0x00;
  //pacote_send->data.IP.ip_data.TCP.window_size[1] = 0x10;
  pacote_send->data.IP.ip_data.TCP.window_size[0] = READ_PACKET_ENC28J60->data.IP.ip_data.TCP.window_size[0];
  pacote_send->data.IP.ip_data.TCP.window_size[1] = READ_PACKET_ENC28J60->data.IP.ip_data.TCP.window_size[1];
  // CHECKSUM TCP: FAZER

  pacote_send->data.IP.ip_data.TCP.urgent_pointer[0] = 0x00;
  pacote_send->data.IP.ip_data.TCP.urgent_pointer[1] = 0x00;

  pacote_send->data.IP.ip_data.TCP.length_options_padding = 4;
  //pacote_send->data.IP.ip_data.TCP.length_options_padding = READ_PACKET_ENC28J60->data.IP.ip_data.TCP.length_options_padding; // 0x01F4
  //pacote_send->data.IP.ip_data.TCP.options_padding = (unsigned char) malloc(4);
  //pacote_send->data.IP.ip_data.TCP.options_padding[0] = 0x04;
  //pacote_send->data.IP.ip_data.TCP.options_padding[1] = 0x02;
  //pacote_send->data.IP.ip_data.TCP.options_padding[2] = 0x01;
  //pacote_send->data.IP.ip_data.TCP.options_padding[3] = 0xF4;
  
  copy_array_bytes(  
                      pacote_send->data.IP.ip_data.TCP.options_padding, 
                      READ_PACKET_ENC28J60->data.IP.ip_data.TCP.options_padding, 
                      4
                  );
  
  //pacote_send->data.IP.ip_data.TCP.options_padding[2] = 0x01;  // LIMITANDO O TAMANHO DO PACOTE PARA 500 BYTES
  //pacote_send->data.IP.ip_data.TCP.options_padding[3] = 0xF4;
  pacote_send->data.IP.ip_data.TCP.length_data = 0;
}
