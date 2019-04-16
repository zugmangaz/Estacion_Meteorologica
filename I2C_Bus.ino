/*
; -------------------------------------------------------
;################################################################################
;# TÃ­tulo: Modulo de control de la interfaz I2C                                #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  10/02/2019                             #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                             #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n:                                                               #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 05/02/2019 | G.Zugman | Inicial                                     #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */
#include <Wire.h>           // I2C
#include <Cola_I2C.h>       
#include <Conexiones_Dispositivos.h>
#include <MAX11603.h>
#include <Thread.h>


/*--------------------------------            
-   Prototipos de funciones      -
----------------------------------*/

void Maquina_I2C_Bus();

/*--------------------------
     Tiempos I2C   
 --------------------------*/

#define TIEMPO_TICKER_I2C  100                // tiempo en milisegundos


/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/

Thread Thread_I2C_Bus = Thread();
//Ticker  I2C_Ticker;

int Tick_I2C_Bus;

typedef void(*Retorno_funcion)(void);


static int Dir_Escritura_Lectura;
static int datos_restantes;
static char datos_transmitidos_por_I2C;

// ------------------------------------------------------
// -                      Estados I2C                   -
// ------------------------------------------------------

Retorno_funcion   Rutina_Estado_START_I2C(void);
Retorno_funcion   Rutina_Estado_ESCRIBIR_I2C(void);
Retorno_funcion   Rutina_Estado_LEER_I2C(void);
Retorno_funcion   Rutina_Estado_IDLE_I2C(void);


Retorno_funcion Puntero_Proximo_Estado_I2C;


/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_I2C_Bus(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados del bus I2C                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Estado_I2C_Bus                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_I2C_Bus(void)
{

  Wire.begin(MASTER_SDA,MASTER_SCL);             // Define los pines del bus I2C, primero SDA y segundo SCL
  Puntero_Proximo_Estado_I2C=(Retorno_funcion)&Rutina_Estado_IDLE_I2C;
  InicializarColaEnvioI2C();
  InicializarColaReciboI2C();

  Thread_I2C_Bus.onRun(Maquina_I2C_Bus);
  Thread_I2C_Bus.setInterval(TIEMPO_TICKER_I2C);
  controll.add(&Thread_I2C_Bus);
//  Serial.println("I2C Inicializado");
}

//------------------------------------------------------

Retorno_funcion  Rutina_Estado_IDLE_I2C(void)
{

  if(!CantidadEnColaEnvioI2C())
    {
      Puntero_Proximo_Estado_I2C=(Retorno_funcion)&Rutina_Estado_IDLE_I2C;
      return Puntero_Proximo_Estado_I2C;
    }
  
    
  Puntero_Proximo_Estado_I2C=(Retorno_funcion)&Rutina_Estado_START_I2C;
  return Puntero_Proximo_Estado_I2C;
}


//------------------------------------------------------
Retorno_funcion  Rutina_Estado_START_I2C(void)
{


  Dir_Escritura_Lectura = RetirarDatoColaEnvioI2C();  
  if(Dir_Escritura_Lectura&0x80)
  {
    Puntero_Proximo_Estado_I2C=(Retorno_funcion)&Rutina_Estado_LEER_I2C;
  }
  else
  {
    Puntero_Proximo_Estado_I2C=(Retorno_funcion)&Rutina_Estado_ESCRIBIR_I2C;
  }

  
  return Puntero_Proximo_Estado_I2C;
}

//-----------------------------------------------------
Retorno_funcion  Rutina_Estado_ESCRIBIR_I2C(void)
{

//    Serial.printf("Enviando por I2C a %#02x \n",Dir_Escritura_Lectura);

    Wire.beginTransmission(Dir_Escritura_Lectura);
  
    datos_restantes = RetirarDatoColaEnvioI2C();                  // Me informo de cuantos byte va a ser el envio
    do{
        Wire.write(RetirarDatoColaEnvioI2C());      // envia palabra de control
    }while(--datos_restantes);                                        // Continuo mientras haya datos que enviar
  
    Wire.endTransmission();

  Puntero_Proximo_Estado_I2C=(Retorno_funcion)&Rutina_Estado_IDLE_I2C;
  return Puntero_Proximo_Estado_I2C;
}

//-----------------------------------------------------
Retorno_funcion  Rutina_Estado_LEER_I2C(void)
{

    datos_transmitidos_por_I2C = Wire.requestFrom(Dir_Escritura_Lectura,(char) RetirarDatoColaEnvioI2C());
    while(Wire.available())
    {
      char dato_leido = Wire.read();
      AgregarDatoColaReciboI2C(dato_leido);
    }

    Puntero_Proximo_Estado_I2C=(Retorno_funcion)&Rutina_Estado_IDLE_I2C;
    return Puntero_Proximo_Estado_I2C;
}

//------------------------------------------------------

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Maquina_I2C_Bus(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados del bus I2C                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Estado_I2C_Bus                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void Maquina_I2C_Bus()
{


  if(Tick_I2C_Bus)
  {
      Tick_I2C_Bus--;
      return;
  }
  else
  {
      Puntero_Proximo_Estado_I2C();
      return;    
  }
}
