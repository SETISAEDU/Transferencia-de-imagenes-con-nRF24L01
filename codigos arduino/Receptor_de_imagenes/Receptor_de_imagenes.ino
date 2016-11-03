#include <UTFT.h>
#include <SPI.h>
#include "nRF24L01.h" //Librerias del modulos nRF24L01+
#include "RF24.h"

extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

 //Se declaran los objetos para el control la LCD y el nRF24L01    
UTFT myGLCD(ILI9341_16, 38, 39, 40, 41); //RS, WR, CS, RESET
RF24 radio(9, 10);//Declaracion de los pines de control CE y CSN para el modulo, se define "radio"

//Declaracion del buffer de recepcion de imagen
unsigned short buff[45000]={0};

// Definicion de las direcciones de comunicacion
const uint64_t direccion1 = 0x7878787878LL;
const uint64_t direccion2 = 0xB3B4B5B6F1LL;

//declaracion de variables globales
int j=0;
bool finish=0;

void setup()
{ 
  //Declaracion de pines de control del nRF
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  //Configuracion de la pantalla LCD
  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.clrScr();
  myGLCD.fillScr(VGA_WHITE);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRect(1, 1, 319, 50);
  myGLCD.setBackColor(VGA_TRANSPARENT);
  myGLCD.setColor(255, 0, 0);
  myGLCD.setFont(BigFont);
  //Mensaje inicial
  myGLCD.print("RECEPTOR DE IMAGENES", CENTER, 5);
  myGLCD.print("CON nRF24L01", CENTER, 25);
  delay(2000);

  //Configuracion del modulo RF
  radio.begin(); //Inicio del modulo nRF24L01+
  radio.setPALevel(RF24_PA_HIGH); //Configuracion en modo de maxima potencia
  radio.setDataRate(RF24_2MBPS); //Configuracion a maxima velocidad
  radio.setChannel(100); // Configuracion del canal de comunicacion
  //Apertura de los canales de comunicacion
  radio.openWritingPipe(direccion2);
  radio.openReadingPipe(1, direccion1);

  //Mensaje de espera despues de haber terminado las configuraciones
  myGLCD.fillScr(VGA_WHITE);
  //myGLCD.drawBitmap(10, 25, 300, 150, logo);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(BigFont);
  myGLCD.print("    ESPERA    ", CENTER, 5);

  radio.startListening(); //Iniciamos en modo escucha

}

void loop()
{
  if (radio.available())      // Comprobacion de datos recibidos
  {
    bool done = false;
    radio.read( &buff[j], sizeof(buff[j])); //Funcion de lectura de datos
    j++;  //incremento del contador de pixeles
    if(j==1)
    {
      //Mensaje de recepcion de nueva imagen
      myGLCD.clrScr();
      myGLCD.setBackColor(VGA_BLACK);
      myGLCD.setColor(0, 0, 255);
      myGLCD.setFont(BigFont);
      myGLCD.print("RECIBIENDO IMAGEN", CENTER, 5);
    }else if(j==45000)
    {
      j=0; //Reinicio del contador de pixeles
      finish=1; //Poner bandera de recepcion terminada
    }
    radio.startListening(); //Volvemos a la escucha
   
  }
  
  if(finish==1) //Verificacion de imagen recibida
  {
    myGLCD.clrScr();
    myGLCD.fillScr(VGA_WHITE);
    myGLCD.drawBitmap(10, 25, 300, 150, buff); //Impresion de la nueva imagen
    myGLCD.setBackColor(VGA_BLACK);
    myGLCD.setColor(0, 255, 0);
    myGLCD.setFont(BigFont);
    myGLCD.print("    IMAGEN RECIBIDA    ", CENTER, 5);
    finish=0;
    j=0;
  }

}


