#include <16F876.h>
#device adc=10
#FUSES XT,NOWDT,NOLVP
#use delay(clock=4000000)
#define use_portb_lcd TRUE
#include <flex_lcd.c>
#bit ADFM=0x9F.7
#use RS232(baud=9600, xmit=pin_c6, rcv=pin_c7, bits=8, parity=N)
#include <math.h>
#use i2c (Master, sda=PIN_C4,scl=PIN_C3)
#define EEPROM_ADDRESS long int


#BYTE PORTB=0x06
#BYTE TRISB=0x86

EEPROM_ADDRESS address;

int16 cont,cont1;
int Tl,Th,Tl1,Th1,Ph,Pb,Hh,Hb,Vh,Vb,numero,valor,pulsos;
int b,j,byt,tiempo,borrar,lecturas,i,dato[8], datoviento[8];
char tot[80];

void write_ext_eeprom(long int address, BYTE data)
{
   short int status;
   i2c_start();
   i2c_write(0xa0);
   i2c_write(address>>7);
   i2c_write(address);
   i2c_write(data);
   i2c_stop();
}
void ESCRIBIR_EEPROM(long int address,int valor)
{
   if(address<80){
       WRITE_EXT_EEPROM(address, valor);
  }

}
BYTE read_ext_eeprom(long int address)
{
   BYTE data;
   i2c_start();
   i2c_write(0xa0);
   i2c_write(address>>7);
   i2c_write(address);
   i2c_start();
   i2c_write(0xa1);
   data=i2c_read(0);
   i2c_stop();
   return (data);
}
//***INT0************

#INT_EXT
ext_isr() {
pulsos++;
 output_toggle(PIN_C0);
}
//******************Temporizador********************
void temp1s(void)
{
    if(cont==2){
    //   output_toggle(PIN_C0);
       tiempo=1;
       cont=0;
    }
    else
    {
       cont++;
    }
       set_timer1(3036);

}
#int_rda
void serial_isr()
{
   valor=getc();
   if(valor==0x31)  //Pulso en VB6 leer eeprom en fichero
   {
      for(address=0;address<80;address++){
        numero=read_ext_eeprom(address);        //lee eeprom
        tot[address]=numero;
      }
      for(address=0;address<80;address++){
         putc(tot[address]);                  // envia buffer de datos eeprom
         }
   }
   if(valor==0x30) //Pulso en VB6 ON (leer datos normal)
   {
       for (i=0; i<=7; i++)
       {
          putc(dato[i]);         // Envia buffer de datos
       }
   }
    if(valor==0x32) //Pulso en VB6 velocidad
   {
       for (i=0; i<=7; i++)
       {
          putc(datoviento[i]);         // Envia buffer de datos
       }
   }
}
void main()
{

   unsigned int16 entrada;
   signed INT16 value,value1,vt,Vout,vel;
   float humedad2,salida,temp,volt,presion,volt2,velocidad;

   setup_adc_ports(ALL_ANALOG);
   setup_adc(ADC_CLOCK_INTERNAl);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);
   lcd_init();
   enable_interrupts(int_rda);
   enable_interrupts(global);

   bit_set(TRISB,0);
  port_b_pullups(TRUE);
   enable_interrupts(int_ext);
   ext_int_edge(L_TO_H);


   //inicialización variables
   
   j=0;
   byt=0;
   tiempo=0;
  
  do{
      temp1s();
      if(tiempo==1){
      //********************HUMEDAD*************;
      set_adc_channel(4);
      delay_us(10);
      entrada=Read_ADC();
         Hh=make8(entrada,1);
         Hb=make8(entrada,0);
      dato[4]=make8(entrada,1);
      dato[5]=make8(entrada,0);

      salida=(entrada/574.0)*2.8;
      humedad2=((salida-1)*48.54);

      //salida=(entrada/574.0)*2.8;
      //humedad2=((salida-1.3)*53.3+10.0);
      delay_ms(10);
      
      //********************PRESIÓN**************;
      set_adc_channel(2);
      delay_us(10);
      Vout=Read_ADC();
         Ph=make8(Vout,1);
         Pb=make8(Vout,0);
      dato[6]=make8(Vout,1);
      dato[7]=make8(Vout,0);

       volt2=5.0*Vout/1024.0;
      presion=((volt2+0.475)/(0.045));

      //volt2=5.0*Vout/1024.0;
     // presion=((volt2-(1.5*1*0.009*5))/(5*0.009))+(0.095/0.009);
     // presion=(presion*10)+16;//mBAR

     //**********************TEMPERATURA*********;
      set_adc_channel(0);
      delay_us(10);
      value=Read_ADC();
      set_adc_channel(1);
      delay_us(10);
      value1=Read_ADC();
      
          Th=make8(value,1);
      dato[0]=make8(value,1);
         Tl=make8(value,0);
      dato[1]=make8(value,0);
          Th1=make8(value1,1);
      dato[2]=make8(value1,1);
          Tl1=make8(value1,0);
       dato[3]=make8(value1,0);

         vt=value-value1;
      volt=5.0*vt/1024.0;
      temp=(volt*100.0); //-2.5


       //*******************VELOCIDAD VIENTO*********;

       vel=pulsos;
        velocidad =(pulsos*0.22)/1.0;  // [(D*Pi/2)*Pulsos]/temporización  =  0,07*Pi*Pulsos/1s= 0,22*pulsos/1
         pulsos=0;

     // //  Vh=make8(vel,1);
     //  Vb=make8(vel,0);


          datoviento[0]=make8(vel,1);

            datoviento[1]=make8(vel,0);




      //*********************EEPROM**************
      
   //delay_ms(200);

     ESCRIBIR_EEPROM(address,Th);
         delay_ms(100);
             address++;
     ESCRIBIR_EEPROM(address,Tl);
          delay_ms(100);
               address++;
     ESCRIBIR_EEPROM(address,Th1);
           delay_ms(100);
               address++;
     ESCRIBIR_EEPROM(address,Tl1);
           delay_ms(100);
               address++;
     ESCRIBIR_EEPROM(address,Hh);
           delay_ms(100);
               address++;
     ESCRIBIR_EEPROM(address,Hb);
           delay_ms(100);
               address++;
     ESCRIBIR_EEPROM(address,Ph);
           delay_ms(100);
               address++;
     ESCRIBIR_EEPROM(address,Pb);
           delay_ms(100);
                address++;
    // // ESCRIBIR_EEPROM(address,Vh);//
      //   delay_ms(100);
         //      address++;
    // ESCRIBIR_EEPROM(address,Vb);
        //   delay_ms(100);
           //     address++;
//
      }
      //***********************DISPLAY************
      printf(lcd_putc,"\fH=%04.1fRH T=%04.2fC",humedad2,temp);
      printf(lcd_putc,"\nP=%04.1fmB V=%04.1fm/s",presion,velocidad);

     // delay_ms(250);
      tiempo=0;

    }
      while(TRUE);
}

