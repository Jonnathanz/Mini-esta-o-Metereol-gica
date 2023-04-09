#define pinoTemp A0

// recebe ponteiro do vetor ou struct onde será armazenado 4 bytes de 00.0 a 99.9
void temperature(char *Pdados){ // retorna a quantidade de caractere
 
    float temp;
    int num1, num2, num3;
    int tmp = 0;
    for( int i =0; i<10; i++){
      delay(1);
      tmp += analogRead(pinoTemp);
    }
      tmp = tmp/10;
    
    temp =(tmp*(5.0/1023.0))/0.01;
    num1 = (int(temp*10.0)%10);
    *(Pdados + 3) = char(num1+48);
    *(Pdados + 2) = '.';
    num2 = int(temp)%10;
    *(Pdados + 1) = char(num2+48);
    num3 = int(temp/10.0)%10;
    *(Pdados) = char(num3+48);
    
}
