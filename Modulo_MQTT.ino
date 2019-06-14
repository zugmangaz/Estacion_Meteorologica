/*
; -------------------------------------------------------
;################################################################################
;# TÃ­tulo: Modulo de conexion al servidor MQTT                                 #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  10/03/2019                             #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                             #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n: Administra la conexion con el servidor MQTT                   #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 10/03/2019 | G.Zugman | Inicial                                     #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */
#include <ESP8266WiFi.h>
#include <Thread.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <stdint.h>


/*--------------------------
     Definiciones AWS   
 --------------------------*/

#define AWS_MQTT_ENDPOINT "a14ifzhh6b83pg-ats.iot.us-east-1.amazonaws.com"
#define MQTT_TLS_PORT     8883
#define TOPIC_DATA        "iot/Dev"
#define CLIENT_ID         "raspy" 

/*--------------------------
     Tiempos Mediciones   
 --------------------------*/

#define TIEMPO_ESPERAR_PARA_CONECTAR     10       //  (5 segundos)
#define TIEMPO_ESPERAR_PARA_PUBLICAR     5       //  (5 segundos)


#define TIEMPO_TICKER_CLIENTE_MQTT  100             // 13 milisegundos



/*--------------------------------------
     Ticks para maquina de estados   
 ---------------------------------------*/

#define TICKS_CLIENTE_MQTT_POR_SEG  1000/(TIEMPO_TICKER_CLIENTE_MQTT)        // 10 Ticks/seg

#define TICKS_ESPERA_PARA_CONECTAR      TICKS_CLIENTE_MQTT_POR_SEG            * TIEMPO_ESPERAR_PARA_CONECTAR 
#define TICKS_ESPERA_PARA_PUBLICAR      TICKS_CLIENTE_MQTT_POR_SEG            * TIEMPO_ESPERAR_PARA_PUBLICAR 


// ------------------------------------------------------
// -               Estados MQTT Server                  -
// ------------------------------------------------------
typedef void(*Retorno_funcion)(void);

Retorno_funcion   Rutina_Estado_INICIALIZACION_BROKER_MQTT(void);
Retorno_funcion   Rutina_Estado_CONEXION_BROKER_MQTT(void);
Retorno_funcion   Rutina_Estado_CLIENTE_LOOP_MQTT(void);
Retorno_funcion   Rutina_Estado_PUBLICAR_MQTT(void);

Retorno_funcion Puntero_Proximo_Estado_Cliente_MQTT;


/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/

Thread Thread_MQTT =  Thread();
int Tick_Cliente_MQTT;

WiFiClientSecure espClient;
PubSubClient client_MQTT(espClient);



//-------------------VARIABLES GLOBALES--------------------------

char clientId_Char[30];
extern struct Informacion_Sensor Data_Sensor[ULTIMO_SENSOR];
extern bool Falla_Conexion;
unsigned char Conexiones_MQTT=0;
extern struct Tiempo Fecha_Hora_Actual;


/* ----------------------------------------------------------------------------------------------
  -                                                                                             -
  - FunciÃ³n:    void Inicializar_Cliente_MQTT(void);                                           -
  -                                                                                             -
  - AcciÃ³n:     Inicializa la conexion con el Broker MQTT                                      -
  - Recibe:     -                                                                               -
  - Devuelve:   -                                                                               -
  - Modifica:   -                                                                               -
  - Destruye:   -                                                                               -
  - Llama a:    -                                                                               -
  - Llamada por:  Conexion_WIFI                                                                 -
  - Macros usados:  -                                                                           -
  - Nivel de STACK:                                                                             -
  -                                                                                             -
  ---------------------------------------------------------------------------------------------- */


void Inicializar_Cliente_MQTT(void)
{

  
    Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_INICIALIZACION_BROKER_MQTT;
    Serial.println(F("Cliente MQTT Inicializado"));

    Thread_MQTT.onRun(Cliente_MQTT);
    Thread_MQTT.setInterval(TIEMPO_TICKER_CLIENTE_MQTT);
    controll.add(&Thread_MQTT);


}


/* ----------------------------------------------------------------------------------------------
  -                                                                                             -
  - FunciÃ³n:    void Cliente_MQTT(void);                                                       -
  -                                                                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados que administra la conexion MQTT            -
  - Recibe:     -                                                                               -
  - Devuelve:   -                                                                               -
  - Modifica:   Tick_Cliente_MQTT                                                               -
  - Destruye:   -                                                                               -
  - Llama a:    -                                                                               - 
  - Llamada por:  Thread_MQTT                                                           -
  - Macros usados:  -                                                                           -
  - Nivel de STACK:                                                                             -
  -                                                                                             - 
  ---------------------------------------------------------------------------------------------- */



//------------------------MAQUINA DE ESTADOS-----------------------------

void Cliente_MQTT()
{
  if(Tick_Cliente_MQTT)
      Tick_Cliente_MQTT--;
  else
      Puntero_Proximo_Estado_Cliente_MQTT();
  return; 
}


/* -----------------------------------------------------
                  Estados Mediciones
  ------------------------------------------------------ */
//------------------------   1   ------------------------------
Retorno_funcion  Rutina_Estado_INICIALIZACION_BROKER_MQTT(void)
{

    if(Falla_Conexion)
        Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_INICIALIZACION_BROKER_MQTT;
    else
    {        
        String clientId = CLIENT_ID;
        clientId += String(random(0xffff), HEX);
        clientId.toCharArray(clientId_Char,30);
        client_MQTT.setClient(espClient);
        client_MQTT.setServer(AWS_MQTT_ENDPOINT, MQTT_TLS_PORT);
        
//        Serial.printf("Tamano heap al inicio de carga certificados: %u\n", ESP.getFreeHeap());
        Carga_de_Certificados();        
//        Serial.printf("Tamano heap al final de carga certificados: %u\n", ESP.getFreeHeap());
        
        Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
    }
    
    return Puntero_Proximo_Estado_Cliente_MQTT;
}

//------------------------   2   ------------------------------
Retorno_funcion  Rutina_Estado_CONEXION_BROKER_MQTT(void)
{
    if(Falla_Conexion)
        Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
    else
    {  
//        Serial.printf("Heap size para la conexion MQTT : %u\n", ESP.getFreeHeap());
//        espClient.setX509Time(Fecha_Hora_Actual.Reloj_UNIX);
        if(client_MQTT.connect(clientId_Char))//, Topic_LW, 1, true, LW_Msg, true))
        {
            Serial.println(F("Conexion MQTT exitosa"));
//            if(Conexiones_MQTT++ >= 20)
//                ESP.restart();
            Falla_Conexion = false;
            Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CLIENTE_LOOP_MQTT;
        }
        else
        {            
            Serial.println(F("Conexion MQTT fallida"));
            Serial.print("failed, rc="); 
            Serial.println(client_MQTT.state()); 
            Falla_Conexion = true;
//            Serial.printf("heap size luego de conexion MQTT fallida: %u\n", ESP.getFreeHeap());

            Tick_Cliente_MQTT = TICKS_ESPERA_PARA_CONECTAR;
            Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
        }    

    }
    if(ESP.getFreeHeap() > 33000)
    {
        espClient.stop();
        Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_INICIALIZACION_BROKER_MQTT;
    }

    return Puntero_Proximo_Estado_Cliente_MQTT;

}

//------------------------     3      ------------------------------
Retorno_funcion  Rutina_Estado_CLIENTE_LOOP_MQTT(void)
{      
    Serial.printf("Heap size previo a MQTT Loop: %u\n", ESP.getFreeHeap());
    if(!client_MQTT.loop())
    {
       Serial.println(F("Detecto desconexion en MQTT Loop"));
       Serial.print(F("MQTT No conectado, status rc=")); 
       Serial.println(client_MQTT.state()); 
       Falla_Conexion = true;
       Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
    }
    else        
    {
        Tick_Cliente_MQTT = TICKS_ESPERA_PARA_PUBLICAR;
        Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_PUBLICAR_MQTT;
    }
    return Puntero_Proximo_Estado_Cliente_MQTT;
}

//------------------------     4     ------------------------------
Retorno_funcion  Rutina_Estado_PUBLICAR_MQTT(void)
{

  
      char Medicion_Data_Char[LARGO_BUFFER];
      char Topic_Data_Char[20]= TOPIC_DATA;

      for(int i=0; i<ULTIMO_SENSOR; i++)
      {
             if(strlen(Data_Sensor[i].JSON_Serializado) > 10)
             {
                  if(client_MQTT.publish(Topic_Data_Char, Data_Sensor[i].JSON_Serializado))
                      Serial.printf("JSON se envio desde RAM satisfatoriamente: %s\n",Data_Sensor[i].JSON_Serializado); 
                  else
                  {
                      Serial.printf("Fallo el envio del JSON\n");
                      Lista_Mediciones.Agregar_Dato_Lista(Medicion_Data_Char);
                  }
                  sprintf(Data_Sensor[i].JSON_Serializado,"\0");
             }
//             else
//                  Serial.printf("No hay mediciones en RAM para enviar del sensor %d\n",i);
      }
//      Serial.printf("Mediciones Guardadas en memoria Flash: %d\n",Lista_Mediciones.Cantidad_Nodos());
        
      if(Lista_Mediciones.Cantidad_Nodos())
      {  
//              Topic_Data.toCharArray(Topic_Data_Char, 20);
            if(Lista_Mediciones.Retirar_Dato_Lista(Medicion_Data_Char))
            {
                Serial.printf("Enviando JSON desde Flash: %s\n",Medicion_Data_Char);
                if(client_MQTT.publish(Topic_Data_Char, Medicion_Data_Char))
                    Serial.printf("El JSON se envio satisfatoriamente\n"); 
                else
                {
                    Serial.printf("Fallo el envio del JSON\n");
                    Lista_Mediciones.Agregar_Dato_Lista(Medicion_Data_Char);
                }
            }
            else
                Serial.printf("Error retirando de la lista\n");
       
       }
         
      Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CLIENTE_LOOP_MQTT;
      return Puntero_Proximo_Estado_Cliente_MQTT;
  
}



/* ----------------------------------------------------------------------------------------------
  -                                                                                             -
  - FunciÃ³n:    void Carga_de_Certificados(void);                                              -
  -                                                                                             -
  - AcciÃ³n:    Carga las credenciales de la conexion TLS 1.2                                   -
  - Recibe:     -                                                                               -
  - Devuelve:   -                                                                               -
  - Llama a:    -                                                                               - 
  - Llamada por:  Rutina_Estado_INICIALIZACION_BROKER_MQTT                                      -
  - Macros usados:  -                                                                           -
  -                                                                                             - 
  ---------------------------------------------------------------------------------------------- */

void Carga_de_Certificados(void)
{
            // Load private key file
        File Private_key = SPIFFS.open("/cf055fd78b-private.der.key", "r");
      
        if (!Private_key) 
           Serial.println(F("La llave privada no se pudo abrir "));
        else
            Serial.println(F("La llave privada se abrio con exito "));
    
        if (espClient.loadPrivateKey(Private_key))
            Serial.println(F("Llave privada cargada "));
        else
            Serial.println(F("Llave privada no cargada "));     
            
      // Load certificate file
      File Certificate = SPIFFS.open("/cf055fd78b-certificate.der.crt", "r");
    
        if (!Certificate) 
            Serial.println(F("El certificado no se pudo abrir "));
        else
            Serial.println(F("El certificado se abrio con exito "));
      
        if (espClient.loadCertificate(Certificate))
            Serial.println(F("Certificado cargado"));
        else
            Serial.println(F("Certificado no cargado"));
     
        // Load CA file
        File CA = SPIFFS.open("/Verisign-CA.der.crt", "r");
        if (!CA) 
            Serial.println(F("No se pudo abrir el certificado de la CA "));
        else
            Serial.println(F("Se abrio con exito el certificado de la CA "));
    
        if(espClient.loadCACert(CA))
            Serial.println(F("certificado de CA cargada"));
        else
           Serial.println(F("certificado de CA no cargada"));

        Private_key.close();
        Certificate.close();
        CA.close();
}
