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

#define CANTIDAD_DE_SENSORES  5


/*--------------------------
     Tiempos Config_Sensores   
 --------------------------*/

#define TIEMPO_TICKER_CONFIGURACION             1000     // 1000 milisegundos

#define TIEMPO_INICIO_CONFIGURACION               1      //  (1 segundo)
#define TIEMPO_RECARGA_CONFIGURACION            600       //  (3600 segundo)
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

Retorno_funcion Puntero_Proximo_Estado_Config_Sensores;


void Config_Sensores(void);



/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/

extern unsigned char Num_Sensor;
extern struct Informacion_Sensor Data_Sensor[ULTIMO_SENSOR];
extern struct Tiempo Fecha_Hora_Actual;

Thread Thread_Config_Sensores = Thread();
unsigned int Tick_Config_Sensores;

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



//------------------------   1   ------------------------------
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
    
    http.begin(Servidor_Configuracion);
    httpCode = http.GET();            //Enviar pedido
    Serial.printf("HTTP Code: %d \n",httpCode); 
    Respuesta_HTTP = http.getString();    //Guardar la respuesta del servidor
//    httpCode = 200;  // ************** Eliminar junto con la linea de abajo
//    Respuesta_HTTP = String(F("[{\"nroSensor\": 1,\"serial\": \"aa:bb:cc:dd:ee:ff-1\",\"readTime\":\"01/01/19 00:00:00\",\"metric\":\"C\",\"value\": 25.2,\"lowest\": 20,\"low\": 25,\"high\": 30,\"highest\": 50,\"delta\": 5,\"status\": \"normal\"}, {\"nroSensor\": 2,\"serial\": \"aa:bb:cc:dd:ee:ff-2\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"%\",\"value\": 25.2,\"lowest\": 5,\"low\": 15,\"high\": 70,\"highest\": 80,\"delta\": 5,\"status\": \"normal\"},{\"nroSensor\": 3,\"serial\": \"aa:bb:cc:dd:ee:ff-3\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"Lux\",\"value\": 25.2,\"lowest\": 5,\"low\": 5,\"high\": 1000,\"highest\": 10000,\"delta\": 100,\"status\": \"normal\"},{\"nroSensor\": 4,\"serial\": \"aa:bb:cc:dd:ee:ff-4\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"dBR\",\"value\":-25,\"lowest\": -40,\"low\": -20,\"high\": -5,\"highest\": -3,\"delta\": 5,\"status\": \"normal\"},{\"nroSensor\": 5,\"serial\": \"aa:bb:cc:dd:ee:ff-5\",\"readTime\":\"01/01/19 00:00:00\",\"metric\": \"PPM\",\"value\": 25.2,\"lowest\": 10,\"low\": 25,\"high\": 80,\"highest\": 100,\"delta\": 10,\"status\": \"normal\"}]"));

    
    Serial.print(F("Codigo respuesta del servidor:")); //200 is OK
    Serial.println(httpCode);   //Print HTTP return code

    const char *String_JSON_Buffer;
    
    if(httpCode == 200)
    {
          // Use arduinojson.org/assistant to compute the capacity.
          const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(CANTIDAD_DE_SENSORES) + CANTIDAD_DE_SENSORES*JSON_OBJECT_SIZE(MIEMBROS_JSON_CONFIGURACION_SENSORES) + 353;

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
              
              Data_Sensor[Num_Sensor].Numero_Sensor = Objeto_Configuracion_Sensores["nroSensor"];
              String_JSON_Buffer                    = Objeto_Configuracion_Sensores["metric"];
              sprintf(Data_Sensor[Num_Sensor].Unidad,"%s",String_JSON_Buffer);
              Data_Sensor[Num_Sensor].Lowest        = Objeto_Configuracion_Sensores["lowest"];
              Data_Sensor[Num_Sensor].Low           = Objeto_Configuracion_Sensores["low"];
              Data_Sensor[Num_Sensor].High          = Objeto_Configuracion_Sensores["high"];
              Data_Sensor[Num_Sensor].Highest       = Objeto_Configuracion_Sensores["highest"];
              Data_Sensor[Num_Sensor].Delta         = Objeto_Configuracion_Sensores["delta"];

              Num_Sensor++;

          }        
          Configuracion_Sensores.clear();
          Tick_Config_Sensores = TICKS_RECARGA_CONFIGURACION;
    }
    else
    {
          Serial.println(F("El servicio de configuraciones no respondio adecuadamente"));
          Tick_Config_Sensores = TICKS_RECARGA_FALLIDA_CONFIGURACION;
    }
    
    http.end();  //Cerrar conexion
    Puntero_Proximo_Estado_Config_Sensores=(Retorno_funcion)&Rutina_Estado_PEDIR_CONFIGURACION_SENSORES;
    return Puntero_Proximo_Estado_Config_Sensores;

}
