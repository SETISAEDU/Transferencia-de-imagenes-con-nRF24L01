#include <UTFT.h>
#include <Touch_AS.h>
#include <SPI.h>

#include "nRF24L01.h" //Librerias del modulos nRF24L01+
#include "RF24.h"

extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

// Declaracion de pines a usar para el control del touch
#define DOUT 2  /* Data out pin */
#define DIN  3  /* Data in pin */
#define DCS  5  /* Chip select */
#define DCLK 6  /* Clock pin  */

// Declaracion de variables de valores maximos y minimos del panel touch
#define XMIN 0    
#define XMAX 4095 
#define YMIN 0    
#define YMAX 4095 

/* Se declaran los objetos para el control 
 * la LCD, Touch y el nRF24L01          */
Touch_AS Touch(DCS, DCLK, DIN, DOUT); //Iniciacion del touch con sus pines de control
UTFT myGLCD(ILI9341_16, 38, 39, 40, 41);//RS, WR, CS, RESET
RF24 radio(9, 10);//Declaracion de los pines de control CE y CSN para el modulo, se define "radio"

//Declaracion de las variables externas contenidas en los ficheros de las imagenes
extern unsigned short enviar[4096];
extern unsigned short siguiente[4096];
extern unsigned short keysight[45000] ;
extern unsigned short abi[45000];
extern unsigned short logo[45000];

// Definicion de las direcciones de comunicacion
const uint64_t direccion1 = 0x7878787878LL;
const uint64_t direccion2 = 0xB3B4B5B6F1LL;

//Declaracion de la variable
int imag = 1;

void setup()
{
//Calibracion del panel touch
  Touch.SetCal(XMIN, XMAX, YMIN, YMAX, 320, 240); // Raw xmin, xmax, ymin, ymax, width, height
  
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
  myGLCD.print("ENVIO DE IMAGENES", CENTER, 5);
  myGLCD.print("CON nRF24L01", CENTER, 25);
  delay(2000);

//Configuracion del modulo RF
  radio.begin(); //Inicio del modulo nRF24L01+
  radio.setPALevel(RF24_PA_HIGH);// Configuracion en modo de baja potencia
  radio.setDataRate(RF24_2MBPS); //Configuracion a maxima velocidad
  radio.setChannel(100); // Configuracion del canal de comunicacion
  //Apertura de los canales de comunicacion
  radio.openWritingPipe(direccion1);
  radio.openReadingPipe(1, direccion2);

  //Inicacion del menu
  myGLCD.fillScr(VGA_WHITE);
  myGLCD.drawBitmap(10, 25, 300, 150, logo);  //imagen 300x150
  myGLCD.drawBitmap(256, 176, 64, 64, siguiente); //imagen 64x64
  myGLCD.drawBitmap(120, 176, 64, 64, enviar); //imagen 64x64
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setFont(BigFont);
  myGLCD.print("    IMAGEN 1    ", CENTER, 5); 
}

void loop()
{
  if (Touch.Pressed()) // Verificacion si se ha hecho presion en el panel touch
  {
    /* Se lee el valor actual presionado en el panel touch */
    int X_Coord = Touch.X(); //Lectura del valor en X
    int Y_Coord = Touch.Y(); //Lectura del valor en Y
    delay(10);
//Comparacion para saber que boton se presiono
    if (((X_Coord > 256) && (X_Coord < 320)) && ((Y_Coord > 176) && (Y_Coord < 240))) //Coordenadas boton siguiente
    {
      /*Imag, contador de imagenes 
       * para seleccionar la que se debe imprimir  */
      if (imag == 0) 
      {
        //myGLCD.fillScr(VGA_WHITE);
        myGLCD.drawBitmap(10, 25, 300, 150, logo);
        myGLCD.setBackColor(VGA_BLACK);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setFont(BigFont);
        myGLCD.print("    IMAGEN 1    ", CENTER, 5);
      } else if (imag == 1)
      {
        //myGLCD.fillScr(VGA_WHITE);
        myGLCD.drawBitmap(10, 25, 300, 150, abi);
        myGLCD.setBackColor(VGA_BLACK);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setFont(BigFont);
        myGLCD.print("    IMAGEN 2    ", CENTER, 5);
      } else if (imag == 2)
      {
        //myGLCD.fillScr(VGA_WHITE);
        myGLCD.drawBitmap(10, 25, 300, 150, keysight);
        myGLCD.setBackColor(VGA_BLACK);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setFont(BigFont);
        myGLCD.print("    IMAGEN 3    ", CENTER, 5);
      } 
      imag++; //incremento del contador de imagenes
      if (imag > 2)imag = 0; //reinicio del contador al llegar a la ultima imagen
    } else if (((X_Coord > 120) && (X_Coord < 176)) && ((Y_Coord > 176) && (Y_Coord < 240))) //Coordenadas boton enviar
    {
      //myGLCD.fillScr(VGA_BLACK);
      myGLCD.setColor(0, 0, 0);
      myGLCD.fillRect(0, 176, 319, 239);
      myGLCD.setBackColor(VGA_RED);
      myGLCD.setColor(255, 255, 255);
      myGLCD.setFont(BigFont);
      myGLCD.print("    ENVIANDO..    ", CENTER, 200);
      //Envio de imagen por el nRF
      if(imag==1)
      {
         radio.stopListening(); // Paramos la escucha para poder escribir
         for (unsigned int i = 0; i < 45000; i++)
         {
          bool ok = radio.write(&logo[i], sizeof(logo[i])); //Envio de pixeles
          //Impresion de porcentage de envio
          if(i==0)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 0%   ", CENTER, 200);
          }else if(i==4500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 10%   ", CENTER, 200);
          }else if(i==9000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 20%   ", CENTER, 200);
          }else if(i==13500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 30%   ", CENTER, 200);
          }else if(i==18000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 40%   ", CENTER, 200);
          }else if(i==22500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 50%   ", CENTER, 200);
          }else if(i==27000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 60%   ", CENTER, 200);
          }else if(i==31500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 70%   ", CENTER, 200);
          }else if(i==36000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 80%   ", CENTER, 200);
          }else if(i==41500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 90%   ", CENTER, 200);
          }else if(i==44999)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 100%   ", CENTER, 200);
          }
          //if (!ok){} 
         }
      }else if(imag==2)
      {
          radio.stopListening(); // Paramos la escucha para poder escribir
         for (unsigned int i = 0; i < 45000; i++)
         {
          bool ok = radio.write(&abi[i], sizeof(abi[i])); //Envio de pixeles
          //Impresion de porcentage de envio
          if(i==0)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 0%   ", CENTER, 200);
          }else if(i==4500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 10%   ", CENTER, 200);
          }else if(i==9000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 20%   ", CENTER, 200);
          }else if(i==13500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 30%   ", CENTER, 200);
          }else if(i==18000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 40%   ", CENTER, 200);
          }else if(i==22500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 50%   ", CENTER, 200);
          }else if(i==27000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 60%   ", CENTER, 200);
          }else if(i==31500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 70%   ", CENTER, 200);
          }else if(i==36000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 80%   ", CENTER, 200);
          }else if(i==41500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 90%   ", CENTER, 200);
          }else if(i==44999)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 100%   ", CENTER, 200);
          }
         }
      }else if(imag==0)
      {
         radio.stopListening(); // Paramos la escucha para poder escribir
         for (unsigned int i = 0; i < 45000; i++)
         {
          bool ok = radio.write(&keysight[i], sizeof(keysight[i])); //Envio de pixeles
          //Impresion de porcentage de envio
          if(i==0)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 0%   ", CENTER, 200);
          }else if(i==4500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 10%   ", CENTER, 200);
          }else if(i==9000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 20%   ", CENTER, 200);
          }else if(i==13500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 30%   ", CENTER, 200);
          }else if(i==18000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 40%   ", CENTER, 200);
          }else if(i==22500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 50%   ", CENTER, 200);
          }else if(i==27000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 60%   ", CENTER, 200);
          }else if(i==31500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 70%   ", CENTER, 200);
          }else if(i==36000)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 80%   ", CENTER, 200);
          }else if(i==41500)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 90%   ", CENTER, 200);
          }else if(i==44999)
          {
            myGLCD.setBackColor(VGA_RED);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setFont(BigFont);
            myGLCD.print("    ENVIANDO.. 100%   ", CENTER, 200);
          }
         }
      }
      
      //Mensaje de finalizacion
      myGLCD.clrScr();
      myGLCD.fillScr(VGA_WHITE);
      myGLCD.setColor(0, 0, 255);
      myGLCD.fillRect(1, 1, 319, 50);
      myGLCD.setBackColor(VGA_TRANSPARENT);
      myGLCD.setColor(255, 0, 0);
      myGLCD.setFont(BigFont);
      myGLCD.print("ENVIO DE IMAGEN", CENTER, 5);
      myGLCD.print("FINALIZADO", CENTER, 25);
      delay(1000);
      myGLCD.clrScr();
      myGLCD.fillScr(VGA_WHITE);
      myGLCD.drawBitmap(10, 25, 300, 150, logo);
      myGLCD.drawBitmap(256, 176, 64, 64, siguiente);
      myGLCD.drawBitmap(120, 176, 64, 64, enviar);
      myGLCD.setBackColor(VGA_BLACK);
      myGLCD.setColor(0, 255, 0);
      myGLCD.setFont(BigFont);
      myGLCD.print("    IMAGEN 1    ", CENTER, 5);
      imag=1;

    }
  }
}


