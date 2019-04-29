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
/*
[
  {"nroSensor": 1,"serial": "aa:bb:cc:dd:ee:ff-1","readTime":"01/01/19 00:00:00","metric": "C","value": 25.2,"lowest": 25,"low": 25,"high": 30,"highest": 35,"delta": 5,"status": "normal"},
  {"nroSensor": 2,"serial": "aa:bb:cc:dd:ee:ff-2","readTime":"01/01/19 00:00:00","metric": "C","value": 25.2,"lowest": 25,"low": 25,"high": 30,"highest": 35,"delta": 5,"status": "normal"},
  {"nroSensor": 3,"serial": "aa:bb:cc:dd:ee:ff-3","readTime":"01/01/19 00:00:00","metric": "C","value": 25.2,"lowest": 25,"low": 25,"high": 30,"highest": 35,"delta": 5,"status": "normal"},
  {"nroSensor": 4,"serial": "aa:bb:cc:dd:ee:ff-4","readTime":"01/01/19 00:00:00","metric": "C","value": 25.2,"lowest": 25,"low": 25,"high": 30,"highest": 35,"delta": 5,"status": "normal"},
  {"nroSensor": 5,"serial": "aa:bb:cc:dd:ee:ff-5","readTime":"01/01/19 00:00:00","metric": "C","value": 25.2,"lowest": 25,"low": 25,"high": 30,"highest": 35,"delta": 5,"status": "normal"}
]
*/

#include <WiFiUdp.h>
#include <Thread.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <sha256.h>
#include <Tiempo.h>
#include <stdlib.h>

#define NTP_SERVICE_PORT    123
#define NTP_MESSAGE_SIZE    48      // Tamaño del paquete NTP time stamp
#define NTP_Conection_PORT  1500    // Puerto de salida para conexion NTP
#define NTP_SERVER_NAME     "time.nist.gov"
#define UTC                 -3      //UTC -3:00 Argentina


#define LONGITUD_PATH_SERVIDOR_CONFIGURACION 150 // Longitud del Path para obtener la configuracion de los sensores
#define LONGITUD_MAC_ADDRESS    17

//#define MIEMBROS_JSON_CONFIGURACION_SENSORES 8
#define MIEMBROS_JSON_CONFIGURACION_SENSORES 11

#define NUMERO_INTENTOS_CONEXION_NTP 5

#define CANTIDAD_DE_SENSORES  5

/*--------------------------
     Tiempos Servicios   
 --------------------------*/

#define TIEMPO_TICKER_SERVICIOS               1000     // 1000 milisegundos

#define TIEMPO_ESPERAR_PARA_MEDIR_TIEMPO      5        //  (5 segundos)
#define TIEMPO_TIME_OUT_PEDIR_HORA            90       //  (90 segundos)
#define TIEMPO_INICIO_SERVICIOS               1        //  (1 segundo)


/*--------------------------------------
     Ticks para maquina de estados   
 ---------------------------------------*/


#define TICKS_SERVICIOS_POR_SEG  1000/(TIEMPO_TICKER_SERVICIOS)        // 10 Ticks/seg

#define TICKS_INICIO_SERVICIOS       TIEMPO_INICIO_SERVICIOS           *1000 / TIEMPO_TICKER_SERVICIOS
#define TIME_OUT_PEDIR_HORA          TIEMPO_TIME_OUT_PEDIR_HORA        *1000 / TIEMPO_TICKER_SERVICIOS

// ------------------------------------------------------
// -               Estados Servicios                   -
// ------------------------------------------------------
typedef void(*Retorno_funcion)(void);

Retorno_funcion   Rutina_Estado_PEDIR_HORA(void);
Retorno_funcion   Rutina_Estado_LEER_HORA(void);
Retorno_funcion   Rutina_Estado_CALCULAR_HORA(void);

Retorno_funcion   Rutina_Estado_PEDIR_CONFIGURACION_SENSORES(void);
//Retorno_funcion   Rutina_Estado_OBTENER_CONFIGURACION_SENSORES(void);

Retorno_funcion Puntero_Proximo_Estado_Servicios;


//void Inicializar_Get_Configuration(void);
void Servicios(void);

/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/

extern unsigned char Num_Sensor;
//extern enum Numeracion_Sensor {SENSOR_TEMPERATURA, SENSOR_HUMEDAD, SENSOR_ILUMINACION, SENSOR_SONIDO, SENSOR_GAS1, SENSOR_GAS2} Num_Sensor;
extern struct Informacion_Sensor Data_Sensor[CANTIDAD_SENSORES];

Thread Thread_Servicios = Thread();
unsigned char Tick_Servicios;

WiFiUDP NTP_UDP;                   // Create an instance of the WiFiUDP class to send and receive
IPAddress NTP_Server_IP;          // time.nist.gov NTP server address
//const char* NTP_Server_Name = "time.nist.gov";
byte NTP_Buffer[NTP_MESSAGE_SIZE]; // buffer to hold incoming and outgoing packets
unsigned char Intentos_conexion_NTP;

//int UTC = -3; //UTC -3:00 Argentina
#define LEAP_YEAR(Y) ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )

// NTPClient timeClient(NTP_UDP, "ar.pool.ntp.org", UTC*3600, 60000);

extern bool Falla_Conexion;

bool Hora_Inicial_Establecida=false;
//struct Tiempo Fecha_Hora_Inicial;
struct Tiempo Fecha_Hora_Actual;
int Time_Out_Pedir_Hora;
/*---------------------------------------            
-   Variable para generacion del hash   -
-----------------------------------------*/




 /* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_Servicios(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados para medir                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Puntero_Proximo_Estado_Servicios / Tick_Servicios                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_Servicios(void)
{

  Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;
  Tick_Servicios = TICKS_INICIO_SERVICIOS;
  Inicializar_Calculo_Hora();
  Thread_Servicios.onRun(Servicios);
  Thread_Servicios.setInterval(TIEMPO_TICKER_SERVICIOS);
  controll.add(&Thread_Servicios);
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
  - FunciÃ³n:    void Inicializar_Get_Configuration(void);                        -
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

/*void Inicializar_Get_Configuration(void)
{
          


}
*/
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


void Servicios()
{
  if(Tick_Servicios)
      Tick_Servicios--;
  else
      Puntero_Proximo_Estado_Servicios();
  return;
}



/* -----------------------------------------------------
                  Estados Servicios
  ------------------------------------------------------ */



//------------------------   1   ------------------------------
Retorno_funcion  Rutina_Estado_PEDIR_HORA(void)
{

    memset(NTP_Buffer, 0, NTP_MESSAGE_SIZE);  // Vaciar el buffer de NTP
    NTP_Buffer[0] = 0b11100011;       // Initialize values needed to form NTP request LI, Version, Mode
    if(!NTP_UDP.beginPacket(NTP_Server_IP, NTP_SERVICE_PORT)) // NTP requests are to port 123
      Serial.printf("Fallo la conexion UDP al servidor NTP\n");
    Serial.printf("Bytes enviados por UDP %d \n",NTP_UDP.write(NTP_Buffer, NTP_MESSAGE_SIZE));
    if(!NTP_UDP.endPacket())
      Serial.printf("Fallo el envio de datos UDP al servidor NTP\n");
    Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_LEER_HORA;
    return Puntero_Proximo_Estado_Servicios;

}

//------------------------   2   ------------------------------
Retorno_funcion  Rutina_Estado_LEER_HORA(void)
{


      if (NTP_UDP.parsePacket() < NTP_MESSAGE_SIZE) 
      {   
          Serial.printf("Faltan recibir datos del NTP, se recibieron %d bytes vuelvo a pedir hora\n",NTP_UDP.parsePacket());
          if(!--Intentos_conexion_NTP)
          {
              NTP_UDP.stop();  
              Inicializar_Calculo_Hora();           
          }
          if(Fecha_Hora_Actual.Reloj_UNIX)
          {
              Time_Out_Pedir_Hora =  TIME_OUT_PEDIR_HORA;   
              Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_CALCULAR_HORA;
          }
          else
              Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;    
      }
      else
      {
          const unsigned long Segundo_en_Setenta_Anos = 2208988800UL;  // Unix time comienza el Jan 1 1970. Son 2208988800 segundos para el reloj NTP:
          Falla_Conexion = false;             
          NTP_UDP.read(NTP_Buffer, NTP_MESSAGE_SIZE); // read the packet into the buffer    
          unsigned long Segundos_Desde_1900 = (NTP_Buffer[40] << 24) | (NTP_Buffer[41] << 16) | (NTP_Buffer[42] << 8) | NTP_Buffer[43];
          // Convertir hora NTP a UNIX timestamp:
          Fecha_Hora_Actual.Reloj_UNIX = Segundos_Desde_1900 - Segundo_en_Setenta_Anos + UTC * 3600; // Restamos los 70 años:
          Fecha_Hora_Actual.Millis_Ultimo_Sinc = millis();

/*          if(!Hora_Inicial_Establecida)
          {
 //             Fecha_Hora_Inicial.Reloj_UNIX = Segundos_Desde_1900 - Segundo_en_Setenta_Anos + UTC * 3600; // Restamos los 70 años:
              Calcular_Fecha_Hora(Fecha_Hora_Actual.Reloj_UNIX);
              Fecha_Hora_Inicial=Fecha_Hora_Actual;
              Hora_Inicial_Establecida = true;
          }
          sprintf(Fecha_Hora_Inicial.Char_Fecha_Hora_Actual,"%04d%02d%02d%02d%02d%02d",Fecha_Hora_Inicial.Ano, Fecha_Hora_Inicial.Mes, Fecha_Hora_Inicial.Dia, Fecha_Hora_Inicial.Hora, Fecha_Hora_Inicial.Minuto, Fecha_Hora_Inicial.Segundo);
          Serial.printf("Fecha y Hora Inicial: %s\n",Fecha_Hora_Inicial.Char_Fecha_Hora_Actual);
*/          Time_Out_Pedir_Hora =  TIME_OUT_PEDIR_HORA;   
          Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_CALCULAR_HORA;
          
      } 
      return Puntero_Proximo_Estado_Servicios;

}

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
          Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_CALCULAR_HORA;
    }
    else
          Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_PEDIR_CONFIGURACION_SENSORES;
    return Puntero_Proximo_Estado_Servicios;

}

//------------------------   4   ------------------------------
Retorno_funcion  Rutina_Estado_PEDIR_CONFIGURACION_SENSORES(void)
{

/*----------------------------------------------------            
-   Variable para la obtencion de la configuracion   -
------------------------------------------------------*/

    HTTPClient http;    //Declare object of class HTTPClient
    char Servidor_Configuracion[LONGITUD_PATH_SERVIDOR_CONFIGURACION];
    String Respuesta_HTTP;
    char Fecha[9];
    int httpCode;
    Sha256 hasher;
    extern byte Mac_Address[LONGITUD_MAC_ADDRESS+1];

    Serial.printf("Tamano heap antes de solicitar configuraciones: %u\n", ESP.getFreeHeap());
    Serial.printf("MAC ADDRESS_HoRA_ACTUAL: %s%s\n",Mac_Address,Fecha_Hora_Actual.Char_Fecha_Hora_Actual);
    sprintf(Fecha,"%02d-%02d-%04d-",Fecha_Hora_Actual.Dia, Fecha_Hora_Actual.Mes, Fecha_Hora_Actual.Ano);
    hasher.update((byte*)Fecha, strlen(Fecha));
    hasher.update(Mac_Address, strlen((const char *)Mac_Address));
    byte hash[SHA256_BLOCK_SIZE];
    hasher.final(hash);
    int pos=0;
        
    pos+=sprintf(Servidor_Configuracion+pos, "http://iotdev.expertaart.com.ar:8080/iot/raspy/getConfiguration/");
    pos+=sprintf(Servidor_Configuracion+pos,"%s/",Mac_Address);
    for(int i=0; i<SHA256_BLOCK_SIZE;i++)
       pos+=sprintf(Servidor_Configuracion+pos,"%02x",hash[i]);
    Serial.printf("Dir: %s \n",Servidor_Configuracion);
    
    http.begin(Servidor_Configuracion);
    httpCode = http.GET();            //Enviar pedido
    Serial.printf("HTTP Code: %d \n",httpCode); 
//    Respuesta_HTTP = http.getString();    //Guardar la respuesta del servidor
    httpCode = 200;  // ************** Eliminar junto con la linea de abajo
    Respuesta_HTTP = String(F("[{\"nroSensor\": 1,\"serial\": \"aa:bb:cc:dd:ee:ff-1\",\"readTime\":\"01/01/19 00:00:00\",\"metric\":\"C\",\"value\": 25.2,\"lowest\": 20,\"low\": 25,\"high\": 30,\"highest\": 50,\"delta\": 5,\"status\": \"normal\"}, {\"nroSensor\": 2,\"serial\": \"aa:bb:cc:dd:ee:ff-2\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"%\",\"value\": 25.2,\"lowest\": 5,\"low\": 15,\"high\": 70,\"highest\": 80,\"delta\": 5,\"status\": \"normal\"},{\"nroSensor\": 3,\"serial\": \"aa:bb:cc:dd:ee:ff-3\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"Lux\",\"value\": 25.2,\"lowest\": 5,\"low\": 5,\"high\": 1000,\"highest\": 10000,\"delta\": 100,\"status\": \"normal\"},{\"nroSensor\": 4,\"serial\": \"aa:bb:cc:dd:ee:ff-4\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"dBR\",\"value\":-25,\"lowest\": -40,\"low\": -20,\"high\": -5,\"highest\": -3,\"delta\": 5,\"status\": \"normal\"},{\"nroSensor\": 5,\"serial\": \"aa:bb:cc:dd:ee:ff-5\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"PPM\",\"value\": 25.2,\"lowest\": 10,\"low\": 25,\"high\": 80,\"highest\": 100,\"delta\": 10,\"status\": \"normal\"}]"));
    
    Serial.print(F("Codigo respuesta del servidor:")); //200 is OK
    Serial.println(httpCode);   //Print HTTP return code
    Serial.print(F("Respuesta del Get_Configuration:"));
    Serial.println(Respuesta_HTTP);    //Print request response payload

    const char *String_JSON_Buffer;
    
    if(httpCode == 200)
    {
          // Use arduinojson.org/assistant to compute the capacity.
          const size_t capacity = JSON_ARRAY_SIZE(CANTIDAD_DE_SENSORES) + CANTIDAD_DE_SENSORES*JSON_OBJECT_SIZE(MIEMBROS_JSON_CONFIGURACION_SENSORES) + 720;
//          const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(CANTIDAD_DE_SENSORES) + CANTIDAD_DE_SENSORES*JSON_OBJECT_SIZE(MIEMBROS_JSON_CONFIGURACION_SENSORES) + 353;
          StaticJsonDocument<capacity> Configuracion_Sensores;
          // Deserialize JSON object
          DeserializationError error = deserializeJson(Configuracion_Sensores, Respuesta_HTTP);
          
          if (error)
          {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.c_str()); 
          }
              
//          const JsonObject& Objeto_Configuracion_Sensores = Configuracion_Sensores["sensors"];
 
          for(Num_Sensor = 0 ; Num_Sensor <CANTIDAD_DE_SENSORES ; Num_Sensor++)
          { 
              const JsonObject& root = Configuracion_Sensores[Num_Sensor];
              Data_Sensor[Num_Sensor].Numero_Sensor = root["nroSensor"];
              String_JSON_Buffer                    =root["metric"];
              sprintf(Data_Sensor[Num_Sensor].Unidad,"%s",String_JSON_Buffer);
              Data_Sensor[Num_Sensor].Lowest        = root["lowest"];
              Data_Sensor[Num_Sensor].Low           = root["low"];
              Data_Sensor[Num_Sensor].High          = root["high"];
              Data_Sensor[Num_Sensor].Highest       = root["highest"];
              Data_Sensor[Num_Sensor].Delta         = root["delta"];



/*              while(Objeto_Configuracion_Sensores[Num_Sensor]["nroSensor"]
              Data_Sensor[Num_Sensor].Numero_Sensor = Objeto_Configuracion_Sensores[Num_Sensor]["nroSensor"];
              String_JSON_Buffer                    = Objeto_Configuracion_Sensores[Num_Sensor]["metric"];
              sprintf(Data_Sensor[Num_Sensor].Unidad,"%s",String_JSON_Buffer);
              Data_Sensor[Num_Sensor].Lowest        = Objeto_Configuracion_Sensores[Num_Sensor]["lowest"];
              Data_Sensor[Num_Sensor].Low           = Objeto_Configuracion_Sensores[Num_Sensor]["low"];
              Data_Sensor[Num_Sensor].High          = Objeto_Configuracion_Sensores[Num_Sensor]["high"];
              Data_Sensor[Num_Sensor].Highest       = Objeto_Configuracion_Sensores[Num_Sensor]["highest"];
              Data_Sensor[Num_Sensor].Delta         = Objeto_Configuracion_Sensores[Num_Sensor]["delta"];
*/
/*              Serial.println(F("Respuesta:"));
              Serial.printf("Sensor:  %d, Sensor   %d \n",          Num_Sensor, Data_Sensor[Num_Sensor].Numero_Sensor);
              Serial.printf("Sensor:  %d, Unidad   %s \n",           Num_Sensor, Data_Sensor[Num_Sensor].Unidad);
              Serial.printf("Sensor:  %d, Lowest   %8.2f \n",        Num_Sensor, Data_Sensor[Num_Sensor].Lowest);
              Serial.printf("Sensor:  %d, Low      %8.2f \n",        Num_Sensor, Data_Sensor[Num_Sensor].Low);   
              Serial.printf("Sensor:  %d, High     %8.2f \n",        Num_Sensor, Data_Sensor[Num_Sensor].High);  
              Serial.printf("Sensor:  %d, Highest  %8.2f \n",        Num_Sensor, Data_Sensor[Num_Sensor].Highest);
              Serial.printf("Sensor:  %d, Delta    %8.2f \n",        Num_Sensor, Data_Sensor[Num_Sensor].Delta); 
              Serial.printf("Sensor:  %d, Time_Out_Sin_Publicaciones: %d \n",Num_Sensor , Data_Sensor[Num_Sensor].Time_Out_Sin_Publicaciones);
*/
          }        
          Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_PEDIR_HORA;
          Configuracion_Sensores.clear();

    }
    else
    {
          Serial.println(F("El servicio de configuraciones no respondio adecuadamente"));
          Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_PEDIR_CONFIGURACION_SENSORES;
    }
    
    http.end();  //Cerrar conexion
    //     Puntero_Proximo_Estado_Servicios=(Retorno_funcion)&Rutina_Estado_OBTENER_CONFIGURACION_SENSORES;
    return Puntero_Proximo_Estado_Servicios;

}

//------------------------   5   ------------------------------
/*
Retorno_funcion  Rutina_Estado_OBTENER_CONFIGURACION_SENSORES(void)
{

     
     return Puntero_Proximo_Estado_Servicios;

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
