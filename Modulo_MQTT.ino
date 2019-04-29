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


#define VARIACION_TOLERADA_MEDICION   5   // 5% de variacion tolerada para la medición

// ------------------------------------------------------
// -               Estados MQTT Server                  -
// ------------------------------------------------------
typedef void(*Retorno_funcion)(void);

Retorno_funcion   Rutina_Estado_INICIALIZACION_BROKER_MQTT(void);
Retorno_funcion   Rutina_Estado_CONEXION_BROKER_MQTT(void);
Retorno_funcion   Rutina_Estado_CLIENTE_LOOP_MQTT(void);

Retorno_funcion   Rutina_Estado_PUBLICAR_LUZ_MQTT(void);
Retorno_funcion   Rutina_Estado_PUBLICAR_RUIDO_MQTT(void);
Retorno_funcion   Rutina_Estado_PUBLICAR_GAS1_MQTT(void);
Retorno_funcion   Rutina_Estado_PUBLICAR_GAS2_MQTT(void);
Retorno_funcion   Rutina_Estado_PUBLICAR_TEMPERATURA_MQTT(void);
Retorno_funcion   Rutina_Estado_PUBLICAR_HUMEDAD_MQTT(void);

Retorno_funcion Puntero_Proximo_Estado_Cliente_MQTT;

void Mensaje_Broker_MQTT(char* , byte* , unsigned int );

/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/

Thread Thread_MQTT =  Thread();
int Tick_Cliente_MQTT;

WiFiClientSecure espClient;
PubSubClient client_MQTT(espClient);
//espClient = WiFiClientSecure();



//-------------------VARIABLES GLOBALES--------------------------

//unsigned char clave_esp8266_bin_key[] = { 0x30,0x82,0x04,0xA2,0x02,0x01,0x00,0x02,0x82,0x01,0x01,0x00,0xAE,0xE6,0xC6,0x8B,0x89,0x78,0xA6,0xF0,0xF5,0xDA,0x97,0x1F,0x11,0xF7,0x15,0x0F,0x60,0x56,0x5A,0x08,0xDF,0xFF,0x28,0x54,0x1E,0x2E,0xA5,0xDD,0x16,0xF1,0x93,0x51,0xC8,0x3F,0x2D,0x37,0xB1,0x38,0x8F,0x7E,0x1F,0x60,0xFA,0xF3,0xB3,0x29,0xB2,0x75,0x55,0x3F,0x5A,0x61,0x63,0x14,0xE1,0xCA,0x70,0xCB,0x00,0x17,0xF4,0x65,0x74,0x78,0x20,0xEB,0x15,0xF8,0xA5,0x81,0x93,0x2E,0x37,0x93,0xBC,0x96,0x0E,0x6A,0xD6,0xC9,0x24,0xC7,0xF6,0xAA,0xCE,0x52,0x6C,0x42,0x8D,0x5F,0x3E,0x8F,0x54,0x6F,0xD4,0x13,0xE2,0x93,0xE2,0x5D,0x3A,0x1A,0x0E,0x76,0x9F,0xDB,0xE2,0xE3,0xE7,0x21,0x8B,0x76,0xA4,0x17,0x06,0x07,0xEB,0xD4,0x07,0x9D,0xB6,0xC3,0x66,0xAD,0x21,0x81,0x65,0x93,0xBC,0x13,0x3A,0x90,0x68,0xD7,0xAE,0x77,0xE1,0x51,0xC4,0x44,0x3E,0x95,0xA8,0xD0,0xFB,0xA9,0xA1,0x43,0x05,0x2C,0x71,0x53,0xA8,0x7E,0xD3,0x7D,0xE5,0x03,0x85,0xFE,0xBF,0x5A,0xB8,0x13,0x21,0x6E,0x48,0x91,0x63,0xC4,0x8A,0x14,0x2B,0x1F,0xB9,0x59,0xC2,0x7A,0x97,0xFB,0x5A,0x30,0x94,0xB4,0xCC,0x41,0x05,0x88,0x28,0xF9,0x21,0x0B,0x6D,0x31,0xB0,0x4B,0x42,0x2D,0x94,0x39,0x77,0x26,0xAB,0xE9,0xD8,0x0D,0x2F,0x99,0x34,0xB5,0x85,0x2B,0x38,0x74,0xFD,0x49,0x01,0xC4,0x48,0x8B,0x3F,0xA6,0x45,0xBA,0xD5,0xCC,0x14,0x50,0xD9,0xCF,0xFA,0xA6,0x86,0xC9,0x0F,0x5D,0xD0,0x7B,0x58,0x7B,0x5F,0xE9,0xD1,0x35,0xDA,0x7A,0x01,0xC0,0x0B,0x17,0x84,0x99,0xCE,0x49,0xF9,0xE9,0x02,0x03,0x01,0x00,0x01,0x02,0x82,0x01,0x00,0x26,0x20,0xDB,0x21,0xAA,0x3B,0x6A,0x31,0x57,0x77,0x6A,0xC0,0x74,0x3B,0x28,0x15,0x2D,0x0C,0x46,0x1E,0x6F,0x2E,0x81,0xCD,0x2C,0xB3,0x40,0xA7,0x5D,0x0C,0x16,0xF0,0x9A,0x35,0xE3,0x96,0xA3,0x84,0xD0,0x0B,0x4F,0xAD,0xE5,0xA7,0x60,0xE4,0x73,0x2A,0x0F,0x06,0x7B,0x9E,0x85,0x6A,0x14,0xFA,0xFB,0xF1,0xD5,0x38,0x63,0xA8,0x06,0x66,0xF5,0xCE,0x2A,0x01,0x05,0x9B,0xA4,0xC9,0x4B,0x86,0x62,0x73,0x57,0xFB,0x44,0x8D,0x9E,0xFE,0x14,0xB1,0x11,0xBF,0x78,0x28,0x5E,0x07,0x68,0x6C,0xF8,0x9C,0x54,0x60,0x6D,0x61,0x8D,0xD4,0x83,0x09,0x11,0x24,0x1D,0x1F,0x63,0x6C,0x09,0x84,0x0F,0x92,0x32,0x9D,0x6E,0x4C,0x25,0xC6,0x18,0x4E,0xF2,0x7B,0xD0,0x93,0x67,0x9D,0x7C,0xD6,0x06,0x94,0x3F,0xB4,0x0A,0x84,0xF6,0x04,0x6C,0xAE,0xA0,0xDC,0x34,0xE0,0x36,0x84,0xB6,0x68,0x46,0xD4,0x37,0x12,0xC9,0x3A,0x3D,0x9D,0x0D,0xB3,0xFF,0xB4,0xBC,0x48,0x95,0xCD,0x84,0x2E,0x5C,0x67,0x8F,0x6F,0xD7,0x61,0xAB,0x21,0xF1,0x8C,0x27,0xEA,0xB1,0x3C,0x17,0x81,0x11,0x78,0x57,0xBB,0xE6,0xA4,0xED,0x58,0x1E,0x0B,0xF5,0xE4,0x8D,0xBB,0x9C,0x7E,0x99,0x89,0x51,0x3D,0x03,0x27,0xE5,0x28,0xAB,0x95,0xC7,0x5F,0x8D,0x15,0x1C,0x55,0x40,0xDB,0xA0,0x72,0x37,0x9C,0x99,0x62,0x14,0x84,0x9E,0x33,0x68,0x26,0x1F,0x67,0xFD,0x8A,0xF0,0x00,0xBC,0x13,0xBC,0x90,0x1D,0xAB,0x52,0xAC,0x28,0x05,0x67,0x13,0x9A,0x4C,0x24,0xD3,0x2A,0x2A,0x12,0x9C,0x7E,0x76,0x9E,0xB1,0x02,0x81,0x81,0x00,0xE1,0xA1,0x17,0xA1,0x36,0x9A,0xB1,0x28,0xC2,0xF8,0xA2,0x9A,0x3B,0x7D,0xD4,0x7C,0x80,0xA7,0x51,0xB0,0x5B,0xB3,0xDE,0x9E,0x0E,0x77,0x5C,0xD6,0xA0,0x36,0x86,0xE1,0xFE,0x57,0xAB,0xA8,0x24,0x4D,0x9E,0x25,0xFB,0x2B,0xCA,0xD3,0x07,0x45,0xDB,0x52,0x6E,0x6B,0xBA,0x8B,0xDD,0x3D,0xEA,0xFD,0x37,0x3D,0x2B,0x02,0xE5,0x5D,0x1E,0xED,0x03,0xF6,0x07,0xB6,0x64,0xD5,0x3F,0x05,0x3E,0x08,0xFB,0xDF,0xA8,0x9E,0x64,0xC7,0x27,0xDB,0x97,0xC9,0xA3,0x23,0x8F,0x0C,0x9D,0x6A,0x14,0x19,0x07,0x89,0xEB,0xF3,0x4A,0xCB,0xBD,0x03,0x37,0xF6,0x78,0xC1,0x82,0x90,0xF5,0x57,0x4D,0x9D,0x56,0x93,0xEA,0xE3,0x89,0xEE,0x7D,0xA4,0xB6,0xE1,0x4D,0xCD,0xEB,0x10,0x35,0xC8,0x3F,0x55,0x02,0x81,0x81,0x00,0xC6,0x71,0xAA,0x5F,0xCF,0x2F,0x73,0x94,0xD8,0x29,0x30,0xAA,0x2D,0x96,0x85,0xF4,0xD0,0xC8,0xD7,0xA2,0xE5,0xD6,0x88,0x1E,0x7F,0xAF,0x11,0x00,0x5E,0x9B,0xA6,0x44,0x67,0xE2,0x4F,0xD8,0xD5,0xFE,0xC8,0xED,0x68,0xE4,0x63,0xA9,0xA0,0xBD,0x8C,0x45,0x0D,0xF2,0x81,0x3E,0x16,0x69,0x59,0xC7,0x61,0x1B,0xCA,0xF2,0xDD,0x26,0x92,0x7E,0xF6,0x48,0x9C,0x04,0x55,0x98,0xCA,0x0F,0xD2,0xE4,0xAA,0x6B,0xD3,0xC1,0x17,0x98,0x4A,0x83,0xB6,0xE7,0x1B,0x90,0x96,0x64,0xD0,0x99,0xC2,0xF0,0x5D,0xCC,0x6B,0xFB,0xEE,0x19,0x33,0xA9,0x5B,0x7C,0x44,0xAC,0x60,0x01,0xB0,0xA1,0xD2,0xD8,0xD7,0x3F,0xF1,0x90,0xB2,0x3B,0xC7,0xD8,0xA8,0xAF,0x6A,0xC9,0x78,0x29,0xCD,0x7D,0x48,0x45,0x02,0x81,0x80,0x14,0xDE,0xBF,0xBE,0xCB,0xBC,0x37,0x52,0xDB,0xE5,0xFF,0xB4,0x60,0x78,0x5D,0x25,0x3F,0x09,0x3B,0x07,0x23,0x14,0x5A,0x2D,0x28,0x8D,0x65,0xEA,0xFC,0x5A,0xCE,0xD3,0x2C,0xA1,0x93,0x61,0x34,0x2A,0x71,0xCC,0x75,0x95,0x31,0x67,0x9F,0xAE,0x98,0x80,0xE1,0x71,0xD9,0x89,0x82,0x67,0x6D,0xCB,0x0A,0x47,0x00,0x29,0x7E,0x69,0xC4,0xA8,0x8B,0xCB,0x40,0x1F,0x60,0xE8,0x50,0xB1,0xFD,0x70,0xFB,0x4B,0x4E,0xDF,0x87,0xD6,0xDC,0x1A,0xF5,0xC9,0x52,0xEB,0xC1,0x19,0x9B,0x9D,0xEA,0x49,0xFE,0x66,0x85,0x8A,0xB5,0x8A,0x6E,0xEB,0x53,0x4B,0xDB,0x13,0x9B,0x2E,0x29,0x63,0x6F,0xEC,0x73,0xCB,0xAE,0xD8,0x69,0x33,0xC4,0x2F,0x6E,0x33,0x1C,0xED,0x45,0x26,0xF4,0xB2,0x97,0x19,0x02,0x81,0x80,0x37,0x37,0xD1,0xAC,0xAF,0x3E,0x08,0xEF,0x55,0xE6,0xC0,0x3F,0xDA,0x47,0x94,0x26,0xB7,0x21,0xEB,0xBE,0xCC,0x2D,0x91,0xDA,0xCA,0xB4,0xB0,0x0A,0x66,0x48,0xF1,0x4E,0xFF,0x89,0x1D,0xA6,0xBC,0x85,0x14,0xEB,0xDA,0xD0,0x12,0x5B,0x54,0xFA,0xE9,0x59,0x1D,0xB4,0x68,0xFA,0x19,0xCA,0x5E,0xCC,0xF7,0xDC,0x7F,0xF0,0x6C,0x51,0x1B,0x5A,0x70,0xB5,0x6B,0x94,0x59,0x2D,0x16,0xAC,0xA8,0x86,0x30,0x55,0xA8,0xAA,0x86,0x22,0x3E,0x39,0xC0,0x5F,0x36,0x70,0x59,0x28,0xDC,0x52,0xAC,0xC7,0xAD,0x92,0x00,0x4A,0x2D,0xCA,0x9D,0x71,0x76,0x9F,0x71,0x8C,0x75,0x7A,0x82,0xE7,0xC4,0x7F,0xCA,0x05,0x64,0xF6,0x5A,0x62,0x00,0x0D,0xED,0x82,0x42,0x38,0xA5,0x53,0x1C,0x76,0xCB,0xF1,0x02,0x81,0x80,0x2E,0xFF,0xFD,0xF0,0x3C,0x90,0x27,0xFF,0xEF,0x8C,0x17,0xFB,0x78,0x39,0xEF,0x41,0x77,0xD8,0x79,0xD5,0x7D,0x4F,0xFD,0xD8,0x11,0xAC,0x79,0xF7,0x04,0xE2,0x15,0xA2,0x94,0x27,0x21,0xEF,0x52,0xE3,0xCB,0x62,0xE9,0x32,0xF0,0x82,0x0D,0x30,0xFA,0x23,0x2F,0x54,0x84,0x71,0x4F,0x48,0x13,0x04,0xA3,0x7D,0x31,0xFF,0xFF,0xD2,0x00,0xF1,0x1D,0x39,0xBD,0xB4,0x51,0x2B,0xDA,0x7C,0x2A,0x09,0x51,0xD0,0xF5,0x00,0x88,0x11,0xDB,0x95,0x9B,0x93,0x36,0xFF,0xBE,0x5D,0x21,0x1E,0x91,0xDD,0x80,0x98,0x61,0x93,0xE1,0x1A,0x0B,0x19,0xA2,0x94,0x0C,0x24,0x64,0xB5,0xAE,0x06,0x45,0xA9,0x45,0xC5,0x20,0x29,0x60,0x42,0x6E,0x92,0xE4,0x59,0x50,0x51,0xBB,0x7F,0x70,0x01,0xF3,0xB3};
//unsigned int clave_esp8266_bin_key_len = 1190;

//IPAddress Servidor_MQTT(192,168,1,111);
//IPAddress AWS_MQTT_endpoint(192,168,2,4);
//const char  AWS_MQTT_endpoint[]   = {"m16.cloudmqtt.com"}; //"34.230.23.213"; //
//const char* AWS_MQTT_endpoint = "a14ifzhh6b83pg.iot.us-east-1.amazonaws.com"; //MQTT broker ip
//const char* AWS_MQTT_endpoint = "a14ifzhh6b83pg-ats.iot.us-east-1.amazonaws.com";
//int         MQTT_Port     = 15233;
//int         MQTT_Port     = 8883;
//int         MQTT_SSL_Port = 25233;
//int         MQTT_TLS_Port = 8883;

//char        *USERNAME = "Gabriel";   
//char        *PASSWORD = "12345678";     
//String      clientId = "raspy";
//char        clientId_Char[]= "raspy";


//extern unsigned char Num_Sensor;
extern struct Informacion_Sensor Data_Sensor[CANTIDAD_SENSORES];
extern bool Falla_Conexion;
unsigned char Conexiones_MQTT=0;

//String Topic_Data = "iot/Dev";

//char *Topic_LW = "";
//char *LW_Msg = "Me desconecte";


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
          
        Serial.printf("Tamano heap al inicio de carga certificados: %u\n", ESP.getFreeHeap());
        
        unsigned char char_private_key[1190];
        
        // Load private key file
        File Private_key = SPIFFS.open("/cf055fd78b-private.der.key", "r");
      //  File Private_key = SPIFFS.open("/clave-privada-dispo.key", "r"); 
      
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
    //    File Certificate = SPIFFS.open("/dispo.crt", "r");
    
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
    //    File CA = SPIFFS.open("/cert-trucho-de-CA-trucho.crt", "r");
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
        
        Serial.printf("Tamano heap al final de carga certificados: %u\n", ESP.getFreeHeap());
    
    
    //    clientId += String(random(0xffff), HEX);
    //    clientId.toCharArray(clientId_Char,30);
        client_MQTT.setServer(AWS_MQTT_ENDPOINT, MQTT_TLS_PORT);
//        client_MQTT.setCallback(Mensaje_Broker_MQTT);
        Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
    }
    
    return Puntero_Proximo_Estado_Cliente_MQTT;
}

//------------------------   1   ------------------------------
Retorno_funcion  Rutina_Estado_CONEXION_BROKER_MQTT(void)
{
    if(Falla_Conexion)
        Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
    else
    {  
        if(!client_MQTT.connected())
        {
    //          Serial.printf("MQTT No esta conectado\n");
    //          Serial.printf("Conexion MQTT heap size: %u\n", ESP.getFreeHeap());
            if(client_MQTT.connect(CLIENT_ID))//, Topic_LW, 1, true, LW_Msg, true))
            {
                Serial.println(F("Conexion MQTT exitosa"));
                if(Conexiones_MQTT++ >= 2)
                    ESP.restart();
                Falla_Conexion = false;
                Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CLIENTE_LOOP_MQTT;
            }
            else
            {            
                Serial.println(F("Conexion MQTT fallida"));
                Serial.print("failed, rc="); 
                Serial.println(client_MQTT.state()); 
                Falla_Conexion = true;
    //              client_MQTT.disconnect();
                Serial.printf("heap size despues de desconexion MQTT: %u\n", ESP.getFreeHeap());
                Tick_Cliente_MQTT = TICKS_ESPERA_PARA_CONECTAR;
                Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
            }    
        }
        else
          Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CLIENTE_LOOP_MQTT;
    }
    return Puntero_Proximo_Estado_Cliente_MQTT;

}

//------------------------      2      ------------------------------
Retorno_funcion  Rutina_Estado_CLIENTE_LOOP_MQTT(void)
{      
      Serial.printf("Heap size previo a MQTT Loop: %u\n", ESP.getFreeHeap());
      if(!client_MQTT.loop())
      {
         Serial.println("Detecto desconexion en MQTT Loop");
         Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
      }
      else        
      {
          Tick_Cliente_MQTT = TICKS_ESPERA_PARA_PUBLICAR;
          Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_PUBLICAR_LUZ_MQTT;
      }
      return Puntero_Proximo_Estado_Cliente_MQTT;
}

//------------------------     3     ------------------------------
Retorno_funcion  Rutina_Estado_PUBLICAR_LUZ_MQTT(void)
{

  
      char Medicion_Data_Char[LARGO_BUFFER];
      char Topic_Data_Char[20]= TOPIC_DATA;
      Serial.printf("Publicacion MQTT heap size: %u\n", ESP.getFreeHeap());

      for(int i=0; i<CANTIDAD_SENSORES; i++)
      {
             if(strlen(Data_Sensor[i].JSON_Serializado) > 10)
             {
/*                  if(client_MQTT.publish(Topic_Data_Char, Medicion_Data_Char))
                      Serial.printf("El JSON se envio satisfatoriamente\n"); 
                  else
                      Serial.printf("Fallo el envio del JSON\n");
                  Serial.printf("JSON Enviado desde RAM: %s\n",Data_Sensor[i].JSON_Serializado);
*/                  sprintf(Data_Sensor[i].JSON_Serializado,"\0");
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
/*                if(client_MQTT.publish(Topic_Data_Char, Medicion_Data_Char))
                    Serial.printf("El JSON se envio satisfatoriamente\n"); 
                else
                {
                    Serial.printf("Fallo el envio del JSON\n");
                    Lista_Mediciones.Agregar_Dato_Lista(Medicion_Data_Char);
                }
*/            }
            else
                Serial.printf("Error retirando de la lista\n");
       
       }
         
      Puntero_Proximo_Estado_Cliente_MQTT=(Retorno_funcion)&Rutina_Estado_CONEXION_BROKER_MQTT;
      return Puntero_Proximo_Estado_Cliente_MQTT;
  
}


/*
//------------------------CALLBACK-----------------------------
void Mensaje_Broker_MQTT(char* topic, byte* payload, unsigned int length) 
{

  char PAYLOAD[5] = "    ";
  
  Serial.print("Mensaje Recibido: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    PAYLOAD[i] = (char)payload[i];
  }
  Serial.println(PAYLOAD);

  if (String(topic) ==  String(SALIDADIGITAL)) {
    if (payload[1] == 'N'){
     digitalWrite(12, HIGH);
    }
    if (payload[1] == 'F'){
      digitalWrite(12, LOW);
    }
  }

  if (String(topic) ==  String(SALIDAANALOGICA)) {
    analogWrite(13, String(PAYLOAD).toInt());
  }


}
*/
