/*
;################################################################################
;# Titulo: Modulo de obtencion de la hora                                       #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  17/03/2019                             #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                             #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n:                                                               #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 17/03/2019 | G.Zugman | Inicial                                     #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */

#include <Thread.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Tiempo.h>

#define NTP_SERVICE_PORT    123
#define NTP_MESSAGE_SIZE    48      // Tamaño del paquete NTP time stamp
#define NTP_Conection_PORT  1500    // Puerto de salida para conexion NTP
#define NTP_SERVER_NAME     "time.nist.gov"
#define UTC                 -3      //UTC -3:00 Argentina



#define NUMERO_INTENTOS_CONEXION_NTP                10


/*--------------------------
     Tiempos Generacion_Hora   
 --------------------------*/

#define TIEMPO_TICKER_GENERACION_HORA               1000     // 1000 milisegundos

#define TIEMPO_INICIO_GENERACION_HORA               1        //  (1 segundo)
#define TIEMPO_TIME_OUT_PEDIR_HORA                  5       //  (90 segundos)
#define TIEMPO_ESPERA_RECEPCION_HORA                1        //  (2 segundos)


/*--------------------------------------
     Ticks para maquina de estados   
 ---------------------------------------*/


#define TICKS_INICIO_GENERACION_HORA      TIEMPO_INICIO_GENERACION_HORA     *1000 / TIEMPO_TICKER_GENERACION_HORA
#define TIME_OUT_PEDIR_HORA               TIEMPO_TIME_OUT_PEDIR_HORA        *1000 / TIEMPO_TICKER_GENERACION_HORA
#define TICKS_ESPERA_RECEPCION_HORA       TIEMPO_ESPERA_RECEPCION_HORA      *1000 / TIEMPO_TICKER_GENERACION_HORA

// ------------------------------------------------------
// -               Estados Generacion_Hora                   -
// ------------------------------------------------------
typedef void(*Retorno_funcion)(void);

Retorno_funcion   Rutina_Estado_PEDIR_HORA(void);
Retorno_funcion   Rutina_Estado_LEER_HORA(void);
//Retorno_funcion   Rutina_Estado_CALCULAR_HORA(void);

Retorno_funcion Puntero_Proximo_Estado_Generacion_Hora;


//void Inicializar_Get_Configuration(void);
void Generacion_Hora(void);

/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/


Thread Thread_Generacion_Hora = Thread();
unsigned char Tick_Generacion_Hora;

WiFiUDP NTP_UDP;                   // Create an instance of the WiFiUDP class to send and receive
IPAddress NTP_Server_IP(0,0,0,0);          // time.nist.gov NTP server address
//const char* NTP_Server_Name = "time.nist.gov";
byte NTP_Buffer[NTP_MESSAGE_SIZE]; // buffer to hold incoming and outgoing packets
unsigned char Intentos_conexion_NTP;

//int UTC = -3; //UTC -3:00 Argentina
#define LEAP_YEAR(Y) ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )

// NTPClient timeClient(NTP_UDP, "ar.pool.ntp.org", UTC*3600, 60000);

extern bool Falla_Conexion;

//bool Hora_Inicial_Establecida=false;
struct Tiempo Fecha_Hora_Actual;
//int Time_Out_Pedir_Hora;


 /* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_Generacion_Hora(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados para medir                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Puntero_Proximo_Estado_Generacion_Hora / Tick_Generacion_Hora                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_Generacion_Hora(void)
{

  Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;
  Tick_Generacion_Hora = TICKS_INICIO_GENERACION_HORA;
  Inicializar_Calculo_Hora();
  Thread_Generacion_Hora.onRun(Generacion_Hora);
  Thread_Generacion_Hora.setInterval(TIEMPO_TICKER_GENERACION_HORA);
  controll.add(&Thread_Generacion_Hora);
}

 /* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_Calculo_Hora(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados para medir                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Puntero_Proximo_Estado_Mediciones / Tick_Mediciones                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_Calculo_Hora(void)
{

    if(!WiFi.hostByName(NTP_SERVER_NAME, NTP_Server_IP))  // Get the IP address of the NTP server
        Serial.println("Falla busqueda servidor DNS.");
    else
    {
        Serial.println("Servidor NTP encontrado.");
        NTP_UDP.begin(NTP_Conection_PORT); 
        Serial.print("NTP server IP:\t");
        Serial.println(NTP_Server_IP);
        Serial.println("Calculo de Hora inicializado");
    }
    Fecha_Hora_Actual.Ano = 0;
    Intentos_conexion_NTP = NUMERO_INTENTOS_CONEXION_NTP;
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Calculo_Hora(void);                        -
  -                                             -
  - AcciÃ³n:     Ejecuta la maquina de estados que realiza las mediciones                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Tick_Hora                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Thread_Hora                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void Generacion_Hora()
{
  if(Tick_Generacion_Hora)
      Tick_Generacion_Hora--;
  else
      Puntero_Proximo_Estado_Generacion_Hora();
  return;
}



/* -----------------------------------------------------
                  Estados Generacion_Hora
  ------------------------------------------------------ */



//------------------------   1   ------------------------------
Retorno_funcion  Rutina_Estado_PEDIR_HORA(void)
{

    if(NTP_Server_IP != (0,0,0,0))
    {   
        memset(NTP_Buffer, 0, NTP_MESSAGE_SIZE);  // Vaciar el buffer de NTP
        NTP_Buffer[0] = 0b11100011;       // Initialize values needed to form NTP request LI, Version, Mode
        if(!NTP_UDP.beginPacket(NTP_Server_IP, NTP_SERVICE_PORT)) // NTP requests are to port 123
          Serial.printf("Fallo la conexion UDP al servidor NTP\n");
        Serial.printf("Bytes enviados por UDP %d \n",NTP_UDP.write(NTP_Buffer, NTP_MESSAGE_SIZE));
        Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_LEER_HORA;
        if(!NTP_UDP.endPacket())
        {
            Serial.printf("Fallo el envio de datos UDP al servidor NTP\n");
            Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;    
        }
        Tick_Generacion_Hora = TICKS_ESPERA_RECEPCION_HORA;
    }
    else
    {
        Inicializar_Calculo_Hora();  
        Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;    
    }
    return Puntero_Proximo_Estado_Generacion_Hora;

}

//------------------------   2   ------------------------------
Retorno_funcion  Rutina_Estado_LEER_HORA(void)
{


      if (NTP_UDP.parsePacket() >= NTP_MESSAGE_SIZE) 
      {   
          const unsigned long Segundo_en_Setenta_Anos = 2208988800UL;  // Unix time comienza el Jan 1 1970. Son 2208988800 segundos para el reloj NTP:
          Falla_Conexion = false;             
          NTP_UDP.read(NTP_Buffer, NTP_MESSAGE_SIZE); // read the packet into the buffer    
          unsigned long Segundos_Desde_1900 = (NTP_Buffer[40] << 24) | (NTP_Buffer[41] << 16) | (NTP_Buffer[42] << 8) | NTP_Buffer[43];
          // Convertir hora NTP a UNIX timestamp:
          Fecha_Hora_Actual.Reloj_UNIX = Segundos_Desde_1900 - Segundo_en_Setenta_Anos + UTC * 3600; // Restamos los 70 años:
 //         Fecha_Hora_Actual.Millis_Ultimo_Sinc = millis();

//          Calcular_Fecha_Hora(Fecha_Hora_Actual.Reloj_UNIX);
          Serial.println(F("Recibo hora"));
//          Time_Out_Pedir_Hora =  TIME_OUT_PEDIR_HORA;   
//          Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_CALCULAR_HORA;
      }
      else
      {
          Serial.printf("Faltan recibir datos del NTP, se recibieron %d bytes vuelvo a pedir hora\n",NTP_UDP.parsePacket());
          if(!--Intentos_conexion_NTP)
//                Falla_Conexion = true;  
              ESP.restart();
          if(!Fecha_Hora_Actual.Reloj_UNIX)
          {
              Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;    
              return Puntero_Proximo_Estado_Generacion_Hora;
          }
          
      } 

      Fecha_Hora_Actual.Reloj_UNIX += (millis() - Fecha_Hora_Actual.Millis_Ultimo_Sinc)/1000;
      Fecha_Hora_Actual.Millis_Ultimo_Sinc = millis();    
      Calcular_Fecha_Hora(Fecha_Hora_Actual.Reloj_UNIX);

      sprintf(Fecha_Hora_Actual.Char_Fecha_Hora_Actual,"%04d%02d%02d%02d%02d%02d",Fecha_Hora_Actual.Ano, Fecha_Hora_Actual.Mes, Fecha_Hora_Actual.Dia, Fecha_Hora_Actual.Hora, Fecha_Hora_Actual.Minuto, Fecha_Hora_Actual.Segundo);
      Serial.printf("%s\n",Fecha_Hora_Actual.Char_Fecha_Hora_Actual);

      Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;    
      return Puntero_Proximo_Estado_Generacion_Hora;

}
/*
//------------------------   3   ------------------------------
Retorno_funcion  Rutina_Estado_CALCULAR_HORA(void)
{   
    if(--Time_Out_Pedir_Hora)
    { 
          Fecha_Hora_Actual.Reloj_UNIX += (millis() - Fecha_Hora_Actual.Millis_Ultimo_Sinc)/1000;
          Fecha_Hora_Actual.Millis_Ultimo_Sinc = millis();    
          Calcular_Fecha_Hora(Fecha_Hora_Actual.Reloj_UNIX);
      
          sprintf(Fecha_Hora_Actual.Char_Fecha_Hora_Actual,"%04d%02d%02d%02d%02d%02d",Fecha_Hora_Actual.Ano, Fecha_Hora_Actual.Mes, Fecha_Hora_Actual.Dia, Fecha_Hora_Actual.Hora, Fecha_Hora_Actual.Minuto, Fecha_Hora_Actual.Segundo);
//          Serial.printf("%s\n",Fecha_Hora_Actual.Char_Fecha_Hora_Actual);
      
      //     str_Reloj_UNIX = String(Reloj_UNIX, DEC)
          Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_CALCULAR_HORA;
    }
    else
          Puntero_Proximo_Estado_Generacion_Hora=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;
    return Puntero_Proximo_Estado_Generacion_Hora;

}



*/


void Calcular_Fecha_Hora(unsigned long Tiempo_Epoch)
{
    Fecha_Hora_Actual.Hora    = ((Tiempo_Epoch % 86400L) / 3600);
    Fecha_Hora_Actual.Minuto  = ((Tiempo_Epoch % 3600) / 60);
    Fecha_Hora_Actual.Segundo = (Tiempo_Epoch % 60);
    Fecha_Hora_Actual.Wday    = (((Tiempo_Epoch / 86400L) + 4 ) % 7);

    // Now set to be in-days.
    Tiempo_Epoch /= 86400L;

    // Setup.
    unsigned long days = 0, year = 1970;
    uint8_t month;
    static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};

    // Walk forward until we've found the number of years.
    while((days += (LEAP_YEAR(year) ? 366 : 365)) <= Tiempo_Epoch)
        year++;

    // now it is days in this year, starting at 0
    Tiempo_Epoch -= days - (LEAP_YEAR(year) ? 366 : 365);
    days=0;

    // Count forward until we've run out of days.
    for (month=0; month<12; month++)
    {
        uint8_t monthLength;
        if (month==1)       // february 
            monthLength = LEAP_YEAR(year) ? 29 : 28;  
        else 
            monthLength = monthDays[month];
        if (Tiempo_Epoch < monthLength)
            break;
        Tiempo_Epoch -= monthLength;
    }

    Fecha_Hora_Actual.Mes = month + 1;
    Fecha_Hora_Actual.Ano = year;
    Fecha_Hora_Actual.Dia  = Tiempo_Epoch + 1;
}
