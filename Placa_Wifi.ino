/*
; -------------------------------------------------------
;################################################################################
;# TÃ­tulo: Modulo Conexion WiFi                                                #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  11/02/2019                             #
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

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Thread.h>

/*--------------------------
     Estados WiFi   
 --------------------------*/


#define DESCONECTADO_WIFI       0
#define DESCONECTAR_WIFI        1 
#define ACTIVAR_SOFT_AP         2
#define CONEXION_SOFT_AP        3
#define CONFIGURAR_SSID         4
#define CONECTAR_WIFI           5
#define ESPERAR_CONEXION_WIFI   6
#define VERIFICAR_CONEXION_WIFI 7
#define ESTADO_WIFI_MAX  VERIFICAR_CONEXION_WIFI + 1

#define ESTADO_WIFI_INICIAL  ACTIVAR_SOFT_AP


/*--------------------------
        Redes WiFi   
 --------------------------*/


#define CANTIDAD_DE_SSID_A_GUARDAR   3
#define LONGITUD_SSID               50
#define LONGITUD_MAC_ADDRESS        17

#define SSID_CONFIG           0
#define SSID_DEFAULT          1
#define SSID_CUSTOM           2

/*--------------------------
     Tiempos WiFi   
 --------------------------*/

#define TIEMPO_ESPERA_DESCONECTADO                      3000       // (3 milisegundos)
#define TIEMPO_CONEXION_SOFT_AP                         1000       // (1 milisegundos)
#define TIEMPO_CONEXION_SOFT_AP_TIMEOUT                 180000      // (60 milisegundos)   ---> Pendiente dejar al valor correspondiente
#define TIEMPO_CONFIGURAR_SSID                          200        // (200 milisegundos)
#define TIEMPO_CONFIGURAR_SSID_TIMEOUT                  120000      // (120 milisegundos)
#define TIEMPO_ESPERAR_CONEXION_WIFI                    1000       // (1000 milisegundo)
#define TIEMPO_CONECTANDO_TIMEOUT                       45000      // (10000 milisegundos)
#define TIEMPO_VERIFICAR_CONEXION_WIFI                  60000      // (60000 milisegundos)


#define TIEMPO_TICKER_WIFI  100                // tiempo en milisegundos


/*--------------------------------------
     Ticks para maquina de estados   
 ---------------------------------------*/

#define TICKS_WIFI_POR_SEG  1000/(TIEMPO_TICKER_WIFI)        // 10 Ticks/seg

#define TICKS_ESPERA_DESCONECTADO                       TICKS_WIFI_POR_SEG            * TIEMPO_ESPERA_DESCONECTADO      /1000
#define TICKS_CONEXION_SOFT_AP                          TICKS_WIFI_POR_SEG            * TIEMPO_CONEXION_SOFT_AP         /1000
#define TICKS_CONEXION_SOFT_AP_TIMEOUT                  TIEMPO_CONEXION_SOFT_AP_TIMEOUT / TIEMPO_CONEXION_SOFT_AP
#define TICKS_CONFIGURAR_SSID                           TICKS_WIFI_POR_SEG            * TIEMPO_CONFIGURAR_SSID          /1000
#define TICKS_CONFIGURAR_SSID_TIMEOUT                   TIEMPO_CONFIGURAR_SSID_TIMEOUT / TIEMPO_CONFIGURAR_SSID
#define TICKS_ESPERAR_CONEXION_WIFI                     TICKS_WIFI_POR_SEG            * TIEMPO_ESPERAR_CONEXION_WIFI    /1000
#define TICKS_CONECTANDO_TIMEOUT                                                        TIEMPO_CONECTANDO_TIMEOUT / TIEMPO_ESPERAR_CONEXION_WIFI
#define TICKS_VERIFICAR_CONEXION_WIFI                   TICKS_WIFI_POR_SEG            * TIEMPO_VERIFICAR_CONEXION_WIFI  /1000

/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/


Thread Thread_WIFI = Thread();

IPAddress Direccion_IP;

WiFiClient Cliente_SSID;
ESP8266WebServer  SSID_Server(80);

char Tabla_SSID[CANTIDAD_DE_SSID_A_GUARDAR][LONGITUD_SSID]     = {"TP-LINK_493C42"   , "TP-Link_Extender"} ;
char Tabla_Password[CANTIDAD_DE_SSID_A_GUARDAR][LONGITUD_SSID] = {"notemetasconmigo" , "aa11zz44x.x55"};
String aux;

const char SSID_Config[LONGITUD_SSID]     = "DiWaIT";
const char Password_Config[LONGITUD_SSID] = "excelente";

unsigned char numero_de_SSID;

byte Mac_Address[LONGITUD_MAC_ADDRESS+1];

int Estado_Wifi;
int time_out;
static int tick_wifi;

bool Falla_Conexion = false;


/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Conexion_Wifi (void);                         -
  -                                               -
  - AcciÃ³n:     Administra la conexion del wifi.                                -
  -                                               -
  -         Si el Estado esta fuera de rango reinicializa la Botonera y retorna.  -
  -                                             -
  -         En funciÃ³n del Estado Actual y de la ExcitaciÃ³n Actualiza las Variables -
  -         y las Salidas de Control y retorna.                   -
  -                                             -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   estado / tick / columna                         -
  - Destruye:   -                           -
  - Llama a:    InicializarTitilarLuces / EncenderColor                 -
  - Llamada por:  IntTimer0                               -
  - Macros usados:  -                                   -
  - Nivel de STACK: 2 Bytes                                   -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Conexion_WiFi(void)
{
   
  if (Estado_Wifi >= ESTADO_WIFI_MAX)
  {                     // Si el Estado de Control de Wifi es Mayor o
    Inicializar_Wifi();         // igual que el Estado MÃ¡ximo Inicializo el Control de Wifi y retorna
    return;
  }
      if (!--tick_wifi)          // Decremento Tick de Control de wifi
      {
   
          switch (Estado_Wifi)
          {
          
              case DESCONECTADO_WIFI:
                             
                              Estado_Wifi = CONECTAR_WIFI;
                              tick_wifi = TICKS_ESPERA_DESCONECTADO;
                              break;
                             
              case ACTIVAR_SOFT_AP:

                              WiFi.softAP(SSID_Config, Password_Config);
                              Direccion_IP = WiFi.softAPIP();
                              Serial.println(F("Access Point activado \n"));
                              Serial.print(F("IP del Access Point: "));
                              Serial.println(Direccion_IP);
                              Inicializar_DNS();
                                                            
                              time_out = TICKS_CONEXION_SOFT_AP_TIMEOUT;
                              tick_wifi = TICKS_CONEXION_SOFT_AP;
                              Estado_Wifi = CONEXION_SOFT_AP;

                              break;
                              
              case CONEXION_SOFT_AP:
                             if(--time_out)
                             {
                                  Serial.printf("Time out %d \n", time_out);       
                                  if (WiFi.softAPgetStationNum())
                                  { 
                                        Serial.println(F("WebServer iniciado..."));
                                        SSID_Server.on("/", WebSite_SSID_Conf); //esta es la pagina de configuracion          
                                        SSID_Server.on("/Configuracion_Guardada.html", Guardar_SSID); //Graba en la eeprom la configuracion
                                        SSID_Server.on("/Salir_de_Configuracion.html", Salir_de_Config); //Escanean las redes wifi disponibles
                                        SSID_Server.begin();

                                        time_out = TICKS_CONFIGURAR_SSID_TIMEOUT;
                                        tick_wifi = TICKS_CONFIGURAR_SSID; 
                                        Estado_Wifi = CONFIGURAR_SSID;                                      
                                  }
                                  else
                                  {
                                        tick_wifi = TICKS_CONEXION_SOFT_AP; 
                                        Estado_Wifi = CONEXION_SOFT_AP;
                                  }
                             }
                             else
                             {
                                        tick_wifi = TICKS_ESPERA_DESCONECTADO; 
                                        Estado_Wifi = CONECTAR_WIFI;
                             }
                             break;
                             
              case CONFIGURAR_SSID:
                             if(--time_out)
                             {
                                  Serial.printf("Time out %d \n", time_out);       
                                  SSID_Server.handleClient();
                                  tick_wifi = TICKS_CONFIGURAR_SSID;                             
                             }
                             else
                             {
                                  if(WiFi.softAPdisconnect())
                                      Serial.println(F("Se elimino el Acces Point \n"));
                                  SSID_Server.close();
                                  Estado_Wifi = CONECTAR_WIFI;
                                  tick_wifi = TICKS_ESPERA_DESCONECTADO;                             
                             }
                             break;

              case CONECTAR_WIFI:
                            
                             aux = Leer_EEPROM(0);
                             aux.toCharArray(Tabla_SSID[CANTIDAD_DE_SSID_A_GUARDAR-1],LONGITUD_SSID);
                             aux = Leer_EEPROM(LONGITUD_SSID);
                             aux.toCharArray(Tabla_Password[CANTIDAD_DE_SSID_A_GUARDAR-1], LONGITUD_SSID); 
                             
                             Serial.print(F("Conectando a \n"));
                             Serial.println(Tabla_SSID[numero_de_SSID]);
                             Serial.println();
                             WiFi.mode(WIFI_OFF);
                             WiFi.mode(WIFI_STA);
                             WiFi.macAddress(Mac_Address);
                             sprintf((char*)Mac_Address,"%02x:%02x:%02x:%02x:%02x:%02x",Mac_Address[0],Mac_Address[1],Mac_Address[2],Mac_Address[3],Mac_Address[4],Mac_Address[5]);
//                             Serial.printf("MAC ADDRESS: %s\n",Mac_Address);
                             WiFi.begin(Tabla_SSID[numero_de_SSID], Tabla_Password[numero_de_SSID]);

                             time_out = TICKS_CONECTANDO_TIMEOUT;
                             tick_wifi = TICKS_ESPERAR_CONEXION_WIFI;
                             Estado_Wifi = ESPERAR_CONEXION_WIFI;
                             
                             break;
          
              case ESPERAR_CONEXION_WIFI:

                             if(--time_out)
                             {
                                   if(WiFi.status() != WL_CONNECTED) 
                                   {
                                       Serial.print(".");
                                       tick_wifi = TICKS_ESPERAR_CONEXION_WIFI;
                                       Estado_Wifi = ESPERAR_CONEXION_WIFI;
                                   }
                                   else
                                   {
                                      Direccion_IP = WiFi.localIP();
                                      Serial.println("");
                                      Serial.println(F("WiFi conectado \n"));
                                      Serial.print(F("Direccion IP: "));
                                      Serial.println(Direccion_IP);
                                      Falla_Conexion = false;

//                                      Inicializar_Servicios();
                                      
//                                      time_out = TICKS_ESPERAR_CONEXION_CLIENTE_TIMEOUT;
                                      tick_wifi = TICKS_VERIFICAR_CONEXION_WIFI;
                                      Estado_Wifi = VERIFICAR_CONEXION_WIFI;
                                      
                                   }
                             }
                             else
                             {
/*                                   if(!numero_de_SSID)
                                        numero_de_SSID = CANTIDAD_DE_SSID_A_GUARDAR-1;
                                   else
                                        numero_de_SSID--;
*/                                   
                                   tick_wifi = TICKS_ESPERA_DESCONECTADO;
                                   Estado_Wifi = DESCONECTAR_WIFI;  
                             }
                             break;
 
              
              case VERIFICAR_CONEXION_WIFI:
                            if(WiFi.status() != WL_CONNECTED || Falla_Conexion) 
                            {
                               tick_wifi = TICKS_ESPERA_DESCONECTADO;
                               Estado_Wifi = DESCONECTAR_WIFI;  
                            }
                            else
                            {
                              tick_wifi = TICKS_VERIFICAR_CONEXION_WIFI;
                              Estado_Wifi = VERIFICAR_CONEXION_WIFI;
                            }
                            break;


              case DESCONECTAR_WIFI:
              
                            Serial.println("");
                            Serial.println(F("Wifi se desconectó "));
                            Serial.println("");
                            WiFi.disconnect();
                            Falla_Conexion = true;

                            tick_wifi = TICKS_ESPERA_DESCONECTADO;
                            Estado_Wifi = DESCONECTADO_WIFI;
                            break;

          }
      }  
}         

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_Wifi(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados Wifi                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Estado_wifi                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_Wifi(void)
{

  tick_wifi = TICKS_ESPERA_DESCONECTADO;
//  Estado_Wifi = ESTADO_WIFI_INICIAL;
  Estado_Wifi = CONECTAR_WIFI;
  Thread_WIFI.onRun(Conexion_WiFi);
  Thread_WIFI.setInterval(TIEMPO_TICKER_WIFI);
  controll.add(&Thread_WIFI);

}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_DNS(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el DNS                       -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   -                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_DNS(void)
{
    // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("configurar-SSID")) 
  {
    Serial.println(F("Error setting up MDNS responder!"));
    return;
  }

  Serial.println(F("mDNS responder started"));

  // Start TCP (HTTP) server
//  server.begin();
//  Serial.println("TCP server started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_SSID_Server(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el servicio para cambio de SSID                       -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   -                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_SSID_Server(void)
{

  // Start TCP (HTTP) server
  SSID_Server.begin();

}


void WebSite_SSID_Conf(void)
{
    File Portal_Configuracion = SPIFFS.open("/Config-SSID.html", "r");
      if (!Portal_Configuracion) 
        Serial.println(F("Portal_Configuracion no se pudo abrir "));
      else
        Serial.println(F("Portal_Configuracion se abrio con exito "));
    Serial.println(F("Envio Pagina de Inicio \n"));
    SSID_Server.send(200, "text/html", Portal_Configuracion.readString());
    Portal_Configuracion.close();
//    Armar_Pagina();

}


void Guardar_SSID(void)
{
    File Portal_Configuracion = SPIFFS.open("/Configuracion_Guardada.html", "r");
    Serial.println(SSID_Server.arg("SSID"));//Recibimos los valores que envia por GET el formulario web
//--> Usar otro metodo realizado por mi    
    Guardar_EEPROM(0,SSID_Server.arg("SSID"));
    Serial.println(SSID_Server.arg("Password"));
//--> Usar otro metodo realizado por mi        
    Guardar_EEPROM(LONGITUD_SSID,SSID_Server.arg(F("Password")));
  
//    mensaje = "Configuracion Guardada...";
    SSID_Server.send(200, "text/html",  Portal_Configuracion.readString());
    Portal_Configuracion.close();

    time_out = 5;                             

}


void Salir_de_Config()
{

        File Portal_Configuracion = SPIFFS.open(F("/Salir_de_Configuracion.html"), "r");
//      Serial.printf("Envio Pagina de Inicio \n");
//      mensaje = "Configuracion Finalizada...";
        SSID_Server.send(200, "text/html",  Portal_Configuracion.readString());
//      Armar_Pagina();

        Portal_Configuracion.close();
  
//      Estado_Wifi = CONECTAR_WIFI;
      time_out = 5;                             
}
