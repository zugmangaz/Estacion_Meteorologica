/*
;################################################################################
;# Titulo: Modulo de obtencion de la configuracion de los sensores              #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  30/04/2019                             #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                             #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n:                                                               #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 30/04/2019 | G.Zugman | Inicial                                     #
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

#include <Thread.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <sha256.h>
#include <stdlib.h>
#include <Tiempo.h>
#include <Mediciones.h>


#define LONGITUD_PATH_SERVIDOR_CONFIGURACION 150 // Longitud del Path para obtener la configuracion de los sensores
#define LONGITUD_MAC_ADDRESS    17

#define MIEMBROS_JSON_CONFIGURACION_SENSORES 8
//#define MIEMBROS_JSON_CONFIGURACION_SENSORES 11

#define CANTIDAD_DE_SENSORES  5


/*--------------------------
     Tiempos Config_Sensores   
 --------------------------*/

#define TIEMPO_TICKER_CONFIGURACION             1000     // 1000 milisegundos

#define TIEMPO_INICIO_CONFIGURACION               1      //  (1 segundo)
#define TIEMPO_RECARGA_CONFIGURACION            60       //  (3600 segundo)
#define TIEMPO_RECARGA_FALLIDA_CONFIGURACION    60       //  (3600 segundo)


/*--------------------------------------
     Ticks para maquina de estados   
 ---------------------------------------*/


#define TICKS_INICIO_CONFIGURACION              TIEMPO_INICIO_CONFIGURACION           *1000 / TIEMPO_TICKER_CONFIGURACION
#define TICKS_RECARGA_CONFIGURACION             TIEMPO_RECARGA_CONFIGURACION          *1000 / TIEMPO_TICKER_CONFIGURACION
#define TICKS_RECARGA_FALLIDA_CONFIGURACION     TIEMPO_RECARGA_FALLIDA_CONFIGURACION  *1000 / TIEMPO_TICKER_CONFIGURACION

// -----------------------------------------------------------
// -               Estados Config_Sensores                   -
// -----------------------------------------------------------
typedef void(*Retorno_funcion)(void);

Retorno_funcion   Rutina_Estado_PEDIR_CONFIGURACION_SENSORES(void);
//Retorno_funcion   Rutina_Estado_OBTENER_CONFIGURACION_SENSORES(void);

Retorno_funcion Puntero_Proximo_Estado_Config_Sensores;


void Config_Sensores(void);



/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/

extern unsigned char Num_Sensor;
//extern enum Numeracion_Sensor {SENSOR_TEMPERATURA, SENSOR_HUMEDAD, SENSOR_ILUMINACION, SENSOR_SONIDO, SENSOR_GAS1, SENSOR_GAS2} Num_Sensor;
extern struct Informacion_Sensor Data_Sensor[CANTIDAD_SENSORES];
extern struct Tiempo Fecha_Hora_Actual;

Thread Thread_Config_Sensores = Thread();
unsigned char Tick_Config_Sensores;

extern bool Falla_Conexion;


/*---------------------------------------            
-   Variable para generacion del hash   -
-----------------------------------------*/

 /* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_Config_Sensores(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados para obtener la configuracion de los sensores -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Puntero_Proximo_Estado_Config_Sensores / Tick_Config_Sensores                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_Config_Sensores(void)
{

  Puntero_Proximo_Estado_Config_Sensores=(Retorno_funcion)&Rutina_Estado_PEDIR_CONFIGURACION_SENSORES;
  Tick_Config_Sensores = TICKS_INICIO_CONFIGURACION;
  Thread_Config_Sensores.onRun(Config_Sensores);
  Thread_Config_Sensores.setInterval(TIEMPO_TICKER_CONFIGURACION);
  controll.add(&Thread_Config_Sensores);
}



/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Config_Sensores(void);                        -
  -                                             -
  - AcciÃ³n:     Ejecuta la maquina de estados que realiza las conexion para el pedido de configuraciones -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Tick_Config_Sensores                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Thread_Config_Sensores                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */



void Config_Sensores()
{
  if(Tick_Config_Sensores)
      Tick_Config_Sensores--;
  else
      Puntero_Proximo_Estado_Config_Sensores();
  return;
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

    if(Falla_Conexion == true  || Fecha_Hora_Actual.Ano == 0)
    {
        Tick_Config_Sensores = TICKS_RECARGA_FALLIDA_CONFIGURACION;
        Puntero_Proximo_Estado_Config_Sensores=(Retorno_funcion)&Rutina_Estado_PEDIR_CONFIGURACION_SENSORES;
        return Puntero_Proximo_Estado_Config_Sensores;        
    }
    http.setReuse(true);
//    Serial.printf("MAC ADDRESS_HoRA_ACTUAL: %s%s\n",Mac_Address,Fecha_Hora_Actual.Char_Fecha_Hora_Actual);
    sprintf(Fecha,"%02d-%02d-%04d-",Fecha_Hora_Actual.Dia, Fecha_Hora_Actual.Mes, Fecha_Hora_Actual.Ano);
//    Serial.printf("Tamano heap antes de crear el hash: %u\n", ESP.getFreeHeap());
    hasher.update((byte*)Fecha, strlen(Fecha));
    hasher.update(Mac_Address, strlen((const char *)Mac_Address));
    byte hash[SHA256_BLOCK_SIZE];
    hasher.final(hash);
    int pos=0;

    
    pos+=sprintf(Servidor_Configuracion+pos, "http://iotdev.expertaart.com.ar:8080/iot/raspy/getConfiguration/");
    pos+=sprintf(Servidor_Configuracion+pos,"%s/",Mac_Address);
    for(int i=0; i<SHA256_BLOCK_SIZE;i++)
       pos+=sprintf(Servidor_Configuracion+pos,"%02x",hash[i]);
//    Serial.printf("Dir: %s \n",Servidor_Configuracion);
    
//    Serial.printf("Tamano heap antes de iniciar cliente HTTP: %u\n", ESP.getFreeHeap());
    http.begin(Servidor_Configuracion);
    httpCode = http.GET();            //Enviar pedido
    Serial.printf("HTTP Code: %d \n",httpCode); 
    Respuesta_HTTP = http.getString();    //Guardar la respuesta del servidor
//    httpCode = 200;  // ************** Eliminar junto con la linea de abajo
//    Respuesta_HTTP = String(F("[{\"nroSensor\": 1,\"serial\": \"aa:bb:cc:dd:ee:ff-1\",\"readTime\":\"01/01/19 00:00:00\",\"metric\":\"C\",\"value\": 25.2,\"lowest\": 20,\"low\": 25,\"high\": 30,\"highest\": 50,\"delta\": 5,\"status\": \"normal\"}, {\"nroSensor\": 2,\"serial\": \"aa:bb:cc:dd:ee:ff-2\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"%\",\"value\": 25.2,\"lowest\": 5,\"low\": 15,\"high\": 70,\"highest\": 80,\"delta\": 5,\"status\": \"normal\"},{\"nroSensor\": 3,\"serial\": \"aa:bb:cc:dd:ee:ff-3\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"Lux\",\"value\": 25.2,\"lowest\": 5,\"low\": 5,\"high\": 1000,\"highest\": 10000,\"delta\": 100,\"status\": \"normal\"},{\"nroSensor\": 4,\"serial\": \"aa:bb:cc:dd:ee:ff-4\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"dBR\",\"value\":-25,\"lowest\": -40,\"low\": -20,\"high\": -5,\"highest\": -3,\"delta\": 5,\"status\": \"normal\"},{\"nroSensor\": 5,\"serial\": \"aa:bb:cc:dd:ee:ff-5\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"PPM\",\"value\": 25.2,\"lowest\": 10,\"low\": 25,\"high\": 80,\"highest\": 100,\"delta\": 10,\"status\": \"normal\"}]"));

//    Serial.printf("Tamano heap luego de obtener configuraciones: %u\n", ESP.getFreeHeap());
    
    Serial.print(F("Codigo respuesta del servidor:")); //200 is OK
    Serial.println(httpCode);   //Print HTTP return code
//    Serial.print(F("Respuesta del Get_Configuration:"));
//    Serial.println(Respuesta_HTTP);    //Print request response payload

    const char *String_JSON_Buffer;
    
    if(httpCode == 200)
    {
          // Use arduinojson.org/assistant to compute the capacity.
//          const size_t capacity = JSON_ARRAY_SIZE(CANTIDAD_DE_SENSORES) + CANTIDAD_DE_SENSORES*JSON_OBJECT_SIZE(MIEMBROS_JSON_CONFIGURACION_SENSORES) + 720;
          const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(CANTIDAD_DE_SENSORES) + CANTIDAD_DE_SENSORES*JSON_OBJECT_SIZE(MIEMBROS_JSON_CONFIGURACION_SENSORES) + 353;
          Serial.printf("Tamano heap antes de crear objeto JSON: %u\n", ESP.getFreeHeap());

          StaticJsonDocument<capacity> Configuracion_Sensores;
          // Deserialize JSON object
          DeserializationError error = deserializeJson(Configuracion_Sensores, Respuesta_HTTP);
          
          if (error)
          {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.c_str()); 
          }
              
          const JsonArray& Array_Configuracion_Sensores = Configuracion_Sensores["sensors"];
          Num_Sensor = 0;
          for(JsonArray::iterator it=Array_Configuracion_Sensores.begin(); it!=Array_Configuracion_Sensores.end(); ++it) 
          {
            const JsonObject& Objeto_Configuracion_Sensores = *it;
//          for(Num_Sensor = 0 ; Num_Sensor <CANTIDAD_DE_SENSORES ; Num_Sensor++)
//          { 
/*              const JsonObject& root = Configuracion_Sensores[Num_Sensor];
              Data_Sensor[Num_Sensor].Numero_Sensor = root["nroSensor"];
              String_JSON_Buffer                    =root["metric"];
              sprintf(Data_Sensor[Num_Sensor].Unidad,"%s",String_JSON_Buffer);
              Data_Sensor[Num_Sensor].Lowest        = root["lowest"];
              Data_Sensor[Num_Sensor].Low           = root["low"];
              Data_Sensor[Num_Sensor].High          = root["high"];
              Data_Sensor[Num_Sensor].Highest       = root["highest"];
              Data_Sensor[Num_Sensor].Delta         = root["delta"];
*/
//              while(Objeto_Configuracion_Sensores[Num_Sensor]["nroSensor"]
              
              Data_Sensor[Num_Sensor].Numero_Sensor = Objeto_Configuracion_Sensores["nroSensor"];
              String_JSON_Buffer                    = Objeto_Configuracion_Sensores["metric"];
              sprintf(Data_Sensor[Num_Sensor].Unidad,"%s",String_JSON_Buffer);
              Data_Sensor[Num_Sensor].Lowest        = Objeto_Configuracion_Sensores["lowest"];
              Data_Sensor[Num_Sensor].Low           = Objeto_Configuracion_Sensores["low"];
              Data_Sensor[Num_Sensor].High          = Objeto_Configuracion_Sensores["high"];
              Data_Sensor[Num_Sensor].Highest       = Objeto_Configuracion_Sensores["highest"];
              Data_Sensor[Num_Sensor].Delta         = Objeto_Configuracion_Sensores["delta"];

              Num_Sensor++;

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
          Configuracion_Sensores.clear();
          Serial.printf("Tamano heap luego de destruir objeto JSON: %u\n", ESP.getFreeHeap());
          Tick_Config_Sensores = TICKS_RECARGA_CONFIGURACION;
    }
    else
    {
          Serial.println(F("El servicio de configuraciones no respondio adecuadamente"));
          Tick_Config_Sensores = TICKS_RECARGA_FALLIDA_CONFIGURACION;
    }
    
    http.end();  //Cerrar conexion
//    Serial.printf("Tamano heap luego de cerrar cliente HTTP: %u\n", ESP.getFreeHeap());
    //     Puntero_Proximo_Estado_Config_Sensores=(Retorno_funcion)&Rutina_Estado_OBTENER_CONFIGURACION_SENSORES;
    Puntero_Proximo_Estado_Config_Sensores=(Retorno_funcion)&Rutina_Estado_PEDIR_CONFIGURACION_SENSORES;
    return Puntero_Proximo_Estado_Config_Sensores;

}

//------------------------   5   ------------------------------
/*
Retorno_funcion  Rutina_Estado_OBTENER_CONFIGURACION_SENSORES(void)
{

     
     return Puntero_Proximo_Estado_Config_Sensores;

}
*/
