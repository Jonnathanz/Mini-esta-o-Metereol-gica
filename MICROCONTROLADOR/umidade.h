//DESENVOLVIMENTO DO CÓDIGO - S ENSOR DE UMIDADE/


//------------------------------------------------------------------------------------------/
//------------------------------------------------------------------------------------------/
//DEFINIÇÃO DOS LEDS GREEN, YELLOW E RED//
#define wet_soil_green      5
#define half_wet_dry_yellow 6
#define dry_soil_red        7
//------------------------------------------------------------------------------------------/
//------------------------------------------------------------------------------------------/



//------------------------------------------------------------------------------------------/
//------------------------------------------------------------------------------------------/
//DEFINIÇÃO ENTRADA ANALÓGICA
#define input_analog_sensor A3 // estava em A0
float analog_value;
//------------------------------------------------------------------------------------------/
//------------------------------------------------------------------------------------------/


void MOSTRA_AS_COISA(float NUMERO, char *VETOR_NOVO){
    /*
     * CONVERTE FLOAT EM STRING DE 5 BYTES
     * 
     */
    //char VETOR_NOVO[4];
    int CONTADOR = 10;
    int i = 0;
    //------->>>240.83/10  = 24.083
    //------->>>240.83/100 = 2.4083 

    while(NUMERO/CONTADOR >= 10)
        CONTADOR = CONTADOR*10;

    //////////////////////////////////////////////    
    //------->>>240.83/100  = 2
    //ADICIONA CHAR
    //------->>>NUMERO = 240.83 - 200 = 40.83
    // CONTADOR = 10
    //------->>>40.83/10 = 4
    //ADICIONA CHAR
    //------->>>NUMERO = 40.83 - 40 = 0.83
    // CONTADOR = 1
    //------->>>0.83/10 = 0
    //ADICIONA CHAR  
    //////////////////////////////////////////////  
    while(CONTADOR >= 1) {
        *(VETOR_NOVO+i) = ((int) NUMERO/CONTADOR) + '0';
        NUMERO = NUMERO - ((int) NUMERO/CONTADOR)*CONTADOR;
        CONTADOR = CONTADOR/10;
        i++;
    }

    //////////////////////////////////////////////
    int QUANTIDADE_DEPOIS_VIRGULA = 2;
    if(QUANTIDADE_DEPOIS_VIRGULA > 0){
        *(VETOR_NOVO+i) = '.';
        for(CONTADOR = i+1; CONTADOR < (QUANTIDADE_DEPOIS_VIRGULA + i + 1); CONTADOR++){
            NUMERO = 10*NUMERO;
            *(VETOR_NOVO + CONTADOR) = ((int)NUMERO) + '0';
            NUMERO = NUMERO - ((int)NUMERO);
        }
    }

    //Serial.print(VETOR_NOVO);
    //Serial.println('\n');
}



//------------------------------------------------------------------------------------------/
//------------------------------------------------------------------------------------------/
void TURN_OFF_LEDS(){
    PORTD &=~(1<<PORTD7);//RED LED
    PORTD &=~(1<<PORTD6);//YELLOW LED
    PORTD &=~(1<<PORTD5);//GREEN LED
}

void UMIDADE(char *VETOR_NOVO){
  /*
   * Retorna a porcentagem da umidades
   * 
   */
  pinMode (input_analog_sensor, INPUT);
  DDRD |= ((1 << DDD7) | (1 << DDD6) | (1 << DDD5));
  //Serial.print("PRÓXIMO DE 1 - MAIS SECO: ");
  analog_value = analogRead(input_analog_sensor);
  //Serial.print(analog_value);
  //Serial.println("abobrinha\n\n\n");
  //Serial.print("Porta analogica: ");
  analog_value = 100.00 - 100*(analog_value-440)/(1023.00-440);
  //Serial.print(analog_value);
  if(analog_value > 100){
    analog_value = 100.00;
  }
  if(analog_value < 0){
    analog_value = 0.00;
  }

  MOSTRA_AS_COISA(analog_value, VETOR_NOVO);
  if (analog_value  > 0.00 && analog_value < 40.00){
         // Serial.println ("---->> STATUS SOLO: SECO ----");
          PORTD |=(1<<PORTD5);
          TURN_OFF_LEDS();
          
    }    
   
   if (analog_value  >= 40.00 && analog_value < 83.00){
         // Serial.println ("---->> STATUS SOLO: ÚMIDO_SECO - MODERADO ----");
          PORTD |=(1<<PORTD6);
          TURN_OFF_LEDS(); 
          
    }
    if (analog_value  >= 83.00){
         // Serial.println ("---->> STATUS SOLO: SECO - ÚMIDO ----");
          PORTD |=(1<<PORTD7);
          TURN_OFF_LEDS(); 
    }
}


#define LDR_LUMINOSIDADE A1

void SENSOR_LUMINOSIDADE(char *VETOR_NOVO1){
  
   pinMode(LDR_LUMINOSIDADE, INPUT);
   float BRIGHTNESS_VALUE = analogRead(LDR_LUMINOSIDADE);
   BRIGHTNESS_VALUE = 100*BRIGHTNESS_VALUE/666.00;

   if(BRIGHTNESS_VALUE > 100){
      BRIGHTNESS_VALUE = 100.00;
    }
    if(BRIGHTNESS_VALUE < 0){
      BRIGHTNESS_VALUE = 0.00;
    }
   if (BRIGHTNESS_VALUE > 68.00 ){
      MOSTRA_AS_COISA(BRIGHTNESS_VALUE, VETOR_NOVO1);
    }

   else{
      MOSTRA_AS_COISA(BRIGHTNESS_VALUE, VETOR_NOVO1);
    }
}
