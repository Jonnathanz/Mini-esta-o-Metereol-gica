#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

/*
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
          /*
          READ_PACKET_ENC28J60->data.IP.ip_data.TCP.length_data = ip_length - c + 14;
          READ_PACKET_ENC28J60->data.IP.ip_data.TCP.data = (unsigned char)malloc(READ_PACKET_ENC28J60->data.IP.ip_data.TCP.length_data);
          for(i = 0; c < ip_length+14; i++){ // ver depois essa variavel c
            write_SPI(0xFF);
            READ_PACKET_ENC28J60->data.IP.ip_data.TCP.data[i] = read_SPI();
            c++;
          }*/
