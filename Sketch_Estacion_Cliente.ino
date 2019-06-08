/*
; -------------------------------------------------------
;################################################################################
;# Título: Modulo esqueleto del servidor                                        #
;#                                                                              #
;#  Versión:    2.0             Fecha:  10/03/2019                              #
;#  Autor:      G. Zugman       Tab:  4                                         #
;#  Compilaciòn:  Usar Arduino IDE                                              #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  Descripción:                                                                #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 05/02/2019 | G.Zugman | Inicial                                     #
;#    2.0 | 10/03/2019 | G.Zugman | Final                                       #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */

#include <Arduino.h>
#include <Thread.h>
#include <ThreadController.h>


ThreadController controll = ThreadController();

extern void  Inicializar_Terminal();
extern void  Inicializar_Sensor_Humedad();
//extern void  Inicializar_TSL256x();
extern void  Inicializar_Wifi();
extern void  Inicializar_I2C_Bus();
extern void  Inicializar_Mediciones();
extern void  Inicializar_Tickers();
extern void  Inicializar_File_System();
extern void  Inicializar_Leds1();
extern void  Inicializar_EEPROM();
extern void  Inicializar_Cliente_MQTT();
extern void  Inicializar_Config_Sensores();
/* -----------------------------
    - Configuracion de Modulos -
    ---------------------------- */

void setup() {
 

  Inicializar_Terminal();
  Inicializar_File_System();
  Inicializar_Sensor_Humedad();
//  Inicializar_TSL256x();
  Inicializar_I2C_Bus();
  Inicializar_Wifi();
  Inicializar_EEPROM();
  Inicializar_Generacion_Hora();
  Inicializar_Config_Sensores();
  Inicializar_Leds1();
  Inicializar_Mediciones();
  Inicializar_Cliente_MQTT();
  Serial.printf("Tamano heap al inicio: %u\n", ESP.getFreeHeap());
 
}



void loop() {

    controll.run();

}
