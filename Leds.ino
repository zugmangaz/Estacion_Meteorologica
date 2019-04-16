/*
; -------------------------------------------------------
;################################################################################
;# TÃ­tulo: Modulo Administración de los Leds                                   #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  09/03/2019                             #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                             #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n:  Funciones para administracion de colores de Leds RGB         #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 09/03/2019 | G.Zugman | Inicial                                     #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */

#include <Thread.h>


#define ACTIVO 1
#define PASIVO 0

/*--------------------------------            
-   Prototipos de funciones      -
----------------------------------*/

void Maquina_Leds1(void);

/* --------------------------------------
   -  Declaración de Prototipos   -
   -------------------------------------- */

void ColorRojo (void);
void ColorVerde (void);
void ColorAzul (void);
void ColorIndigo (void);
void ColorAmarillo (void);
void ColorVioleta (void);
void ColorBlanco (void);
void ColorNegro (void);


void Inicializar_Leds1 (void);
void InicializarTitilarLuces (void);
void InicializarColor(void);

void Maquina_Leds1(void);
void TitilarLuces(void);
void ColocarColor (unsigned char, unsigned int, unsigned int, unsigned char);
void EncenderColor(unsigned char);

void CambiarColor(void);
unsigned char NumeroColorActivo(void);
void AgregarColor(void);
void BorrarColores(void);

/*--------------------------
     Tiempos LED1   
 --------------------------*/

#define TIEMPO_TICKER_LEDS1             30000       // base de tiempo de maquina en segundos

#define TIEMPO_PARA_VERIFICAR_FALLAS    60       //  (5 segundos)

/*--------------------------------------
     Ticks para maquina de estados   
 ---------------------------------------*/

#define TICKS_VERIFICAR_FALLAS         TIEMPO_PARA_VERIFICAR_FALLAS     *1000/ TIEMPO_TICKER_LEDS1 



// ------------------------------------------------------
// -                      Estados LEDS                   -
// ------------------------------------------------------

typedef void(*Retorno_funcion)(void);

Retorno_funcion Rutina_Estado_LEDS_VERIFICAR_FALLA_Y_ALERTAS(void);

Retorno_funcion Puntero_Proximo_Estado_Leds1;


/*--------------------------
     Estados Titilar   
 --------------------------*/

#define DESACTIVAR            0
#define ENCENDIDO             1 
#define ENCENDIDO_TEMPORIZADO 2
#define ESPERAReNCENDIDO      3
#define APAGADO               4
#define ESPERARaPAGADO        5

#define ESTADOlUCESmAX  ESPERARaPAGADO + 1

#define ESTADOlUCESiNICIAL  DESACTIVAR


/* ------------------------
   -        Colores       -
   ------------------------ */

#define NEGRO    0
#define ROJO    1
#define VERDE   2
#define AZUL    3
#define INDIGO    4
#define AMARILLO  5
#define VIOLETA   6
#define BLANCO    7

#define COLORmAX  VIOLETA + 1

#define COLORiNICIAL NEGRO


/* --------------------------------------
   -  Declaración de Variables    -
   -------------------------------------- */

Thread Thread_Leds1 = Thread();


extern unsigned char Falla_Sensores;
extern bool Falla_Conexion;
extern bool Alerta_Mediciones;

unsigned char EstadoLuces;
unsigned char ColorActual;
unsigned int TiempoLedOn, TiempoLedOff;
unsigned char Repeticiones;
unsigned char Activar_Led;
unsigned int tickLuces;
unsigned int Tick_Leds1;

unsigned char NumeroColor, NumeroColorMax = 6;


/* ------------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_Leds1(void);                        -
  -                                                                                               -
  - AcciÃ³n:     Inicializa el la maquina de estados del led 1. Apaga los Led Rojo, Verde y Azul, -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Estado_Leds1                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_Leds1(void)
{
//  pinMode(LED_ROJO, OUTPUT);
//  digitalWrite(LED_ROJO, HIGH);
//  pinMode(LED_VERDE,OUTPUT);
//  digitalWrite(LED_VERDE,HIGH);
//  pinMode(LED_AZUL, OUTPUT);
//  digitalWrite(LED_AZUL,HIGH);
  
//  InicializarTitilarLuces();
  
  Tick_Leds1 = TICKS_VERIFICAR_FALLAS;
  Puntero_Proximo_Estado_Leds1=(Retorno_funcion)&Rutina_Estado_LEDS_VERIFICAR_FALLA_Y_ALERTAS;

  Thread_Leds1.onRun(Maquina_Leds1);
  Thread_Leds1.setInterval(TIEMPO_TICKER_LEDS1);
  controll.add(&Thread_Leds1);
}



//-------------------------1-----------------------------

Retorno_funcion  Rutina_Estado_LEDS_VERIFICAR_FALLA_Y_ALERTAS(void)
{

    if(Falla_Sensores)
//-------> Descomentar con Led funcionando       ColocarColor(ROJO,NULL,NULL,0); 
        Serial.printf("Led Rojo\n");
    else if(Falla_Conexion) 
//-------> Descomentar con Led funcionando        ColocarColor(ROJO,NULL,NULL,0); 
        Serial.printf("Led Rojo\n");
    else if(Alerta_Mediciones)
//-------> Descomentar con Led funcionando        ColocarColor(AMARILLO,NULL,NULL,0); 
        Serial.printf("Led Amarillo\n");
    else
//-------> Descomentar con Led funcionando        ColocarColor(VERDE,NULL,NULL,0);     
        Serial.printf("Led Verde\n");  

    Tick_Leds1 = TICKS_VERIFICAR_FALLAS;

    Puntero_Proximo_Estado_Leds1=(Retorno_funcion)&Rutina_Estado_LEDS_VERIFICAR_FALLA_Y_ALERTAS;
    return Puntero_Proximo_Estado_Leds1;
}



/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Maquina_Leds1(void);                        -
  -                                             -
  - AcciÃ³n:     Ejecuta la maquina de control del led 1                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Estado_Leds1                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void Maquina_Leds1()
{
//  TitilarLuces();

  if(Tick_Leds1)
      Tick_Leds1--;
  else
      Puntero_Proximo_Estado_Leds1();
  return;    
}


/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void InicializarTitilarLuces(void);                     -
  -                                             -
  - Acción:     Inicializa el estado del parpadeo de luces                -
  -         Inicializa el color y el tiempo del parpadeo              -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   EstadoDigito                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Main                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void InicializarTitilarLuces (void)
{
  EstadoLuces = ESTADOlUCESiNICIAL;
  ColorActual = ROJO;
  TiempoLedOn = 500;
  TiempoLedOff = 100;
  Repeticiones = 2;
  Activar_Led = ACTIVO;
}
/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void InicializarColor(void);                        -
  -                                             -
  - Acción:     Inicializa el numero de color inicial                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   NumeroColor                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Main                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void InicializarColor(void)
{
  NumeroColor = 0;
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void TitilarLuces (void);                         -
  -                                               -
  - Acción:     Si no es tiempo de hacer un cambio decrementa el "tick" de Botonera y -
  -         retorna.                                -
  -                                               -
  -         Si el Estado esta fuera de rango reinicializa la Botonera y retorna.  -
  -                                             -
  -         En función del Estado Actual y de la Excitación Actualiza las Variables -
  -         y las Salidas de Control y retorna.                   -
  -                                             -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   estado / tick / columna                         -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    InicializarTitilarLuces / EncenderColor                 -
  - Llamada por:  IntTimer0                               -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void TitilarLuces(void)
{
   
    if (EstadoLuces >= ESTADOlUCESmAX) 
    {                     // Si el Estado de Control de Teclado es Mayor o
        InicializarTitilarLuces ();         // igual que el Estado Máximo Inicializo el Control                     // de Teclado y retorno
        return;
    }
    
    switch (EstadoLuces)
    {
    
        case DESACTIVAR:
                  if(Activar_Led)
                      EstadoLuces = ENCENDIDO;
                  break;
        case ENCENDIDO:
                  EncenderColor(ColorActual);         // Enciendo el color
                  if(Repeticiones)
                      EstadoLuces = ENCENDIDO_TEMPORIZADO;
                  break;
    
        case ENCENDIDO_TEMPORIZADO:
                  EncenderColor(ColorActual);         // Enciendo el color
                  tickLuces = TiempoLedOn;
                  EstadoLuces = ESPERAReNCENDIDO;
                  break;
    
        case ESPERAReNCENDIDO:
                  if (--tickLuces == 0)         // Decremento Tick de Control de Teclado
                      EstadoLuces = APAGADO;
                  break;                  // y Si aún no es tiempo de procesar retorno
        
        case APAGADO:
                  EncenderColor(NEGRO);
                  tickLuces = TiempoLedOff;
                  EstadoLuces = ESPERARaPAGADO;
                  break;
        
        case ESPERARaPAGADO:
                  if (tickLuces)          // Decremento Tick de Control de Teclado
                      tickLuces--;
                  else
                      if(--Repeticiones == 0)
                      { 
                          EstadoLuces = DESACTIVAR;
                          Activar_Led = PASIVO;
                      }
                      else
                          EstadoLuces = ENCENDIDO_TEMPORIZADO;
            
                  break;                  // y Si aún no es tiempo de procesar retorno
    }

}
      

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColocarColor (void);                       -
  -                                               -
  - Acción:     Recibe datos para encender un color, el tiempo y las veces de repeticion-                                   -
  -                                             -
  - Recibe:     Color / TiempoOn / TiempoOff / Cantidad                 -
  - Devuelve:   -                                   -
  - Modifica:   ColorActual / TiempoLedOn / TiempoLedOff / Repeticiones / Titilar   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    InicializarTitilarLuces                         -
  - Llamada por:  IntTimer0                               -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void ColocarColor(unsigned char Color, unsigned int TiempoOn, unsigned int TiempoOff, unsigned char Cantidad) 
{
  InicializarTitilarLuces();
  ColorActual = Color;
  TiempoLedOn = TiempoOn;
  TiempoLedOff = TiempoOff;
  Repeticiones = Cantidad;
  Activar_Led = ACTIVO;
}
/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void EncenderColor (unsigned char);                       -
  -                                               -
  - Acción:     Recibe el color para encender                       -                                   -
  -                                             -
  - Recibe:     Color                                   -
  - Devuelve:   -                                   -
  - Modifica:   Color                                 -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    ColorRojo / ColorVerde / ColorAzul / ColorIndigo / ColorAmarillo    -
  -         ColorVioleta / ColorBlanco / ColorNegro                 -
  - Llamada por:  IntTimer0                               -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void EncenderColor (unsigned char Color)
{

  switch(Color)
  {
    case ROJO:
      ColorRojo();
      break;
    case VERDE:
      ColorVerde();
      break;
    case AZUL:
      ColorAzul();
      break;
    case INDIGO:
      ColorIndigo();
      break;
    case AMARILLO:
      ColorAmarillo();
      break;
    case VIOLETA:
      ColorVioleta();
      break;
    case BLANCO:
      ColorBlanco();
      break;
    case NEGRO:
      ColorNegro();
      break;
    }
} 

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void CambiarColor (void);                       -
  -                                               -
  - Acción:     Cambia el color para encender                       -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   NumeroColor                               -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void CambiarColor(void)
{
    NumeroColor++;                // Cambio el color a encender
    if (NumeroColor > NumeroColorMax)     // Verifico si llego al ultimo color
      NumeroColor = 1;            // Vuelvo a color Rojo
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void NumeroColorActivo (void);                      -
  -                                               -
  - Acción:     Devuelve el color activo                        -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   NumeroColor                               -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                   -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

unsigned char NumeroColorActivo(void)
{
  return NumeroColor;
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void AgregarColor (void);                       -
  -                                               -
  - Acción:     Agrega un color para encender                       -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   NumeroColorMax                                -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void AgregarColor(void)
{
  NumeroColorMax++;
  if(NumeroColorMax >= COLORmAX)
    NumeroColorMax = COLORmAX - 1;

}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void BorrarColores (void);                        -
  -                                               -
  - Acción:     Borra secuencia de colores actual                   -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   NumeroColorMax                                -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void BorrarColores(void)
{
  NumeroColorMax = 0;
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColorRojo (void);                          -
  -                                               -
  - Acción:     Enciende Color Rojo                           -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void ColorRojo (void)
{
  digitalWrite(LED_ROJO,LOW); 
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AZUL,HIGH);
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColorVerde (void);                         -
  -                                               -
  - Acción:     Enciende Color Verde                            -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void ColorVerde (void)
{
  digitalWrite(LED_ROJO, HIGH); 
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AZUL,HIGH);
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColorAzul (void);                          -
  -                                               -
  - Acción:     Enciende Color Azul                           -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void ColorAzul (void)
{
  digitalWrite(LED_ROJO, HIGH); 
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AZUL, LOW);
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColorIndigo (void);                          -
  -                                               -
  - Acción:     Enciende Color Indigo                           -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void ColorIndigo (void)
{
  digitalWrite(LED_ROJO, HIGH); 
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AZUL, LOW);
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColorAmarillo (void);                          -
  -                                               -
  - Acción:     Enciende Color Amarillo                           -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void ColorAmarillo (void)
{
  digitalWrite(LED_ROJO,LOW); 
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AZUL,HIGH);
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColorVioleta (void);                         -
  -                                               -
  - Acción:     Enciende Color Violeta                            -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void ColorVioleta (void)
{
  digitalWrite(LED_ROJO,LOW); 
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AZUL, LOW);
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColorBlanco (void);                          -
  -                                               -
  - Acción:     Enciende Color Blanco                           -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void ColorBlanco (void)
{
  digitalWrite(LED_ROJO,LOW); 
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AZUL, LOW);
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - Función:    void ColorNegro (void);                         -
  -                                               -
  - Acción:     Apaga Luces                               -                                   -
  -                                             -
  - Recibe:     -                                     -
  - Devuelve:   -                                   -
  - Modifica:   -                                   -
  - Destruye:   ACC / PSW / R5 / R7                           -
  - Llama a:    -                                     -
  - Llamada por:  -                                   -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void ColorNegro (void)
{
  digitalWrite(LED_ROJO, HIGH); 
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AZUL,HIGH);
}
