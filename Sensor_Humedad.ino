 /*
; -------------------------------------------------------
;################################################################################
;# Título: Modulo de medicion sensor de humedad                                 #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  10/02/2019                              #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                              #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n:    #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 05/02/2019 | G.Zugman | Inicial                                     #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */

#include <DHT_U.h>
#include <Conexiones_Dispositivos.h>



//DHT_Unified Sensor_De_Humedad(HUMIDITY_DATA, DHT_TYPE);

//DHT_Unified Sensor_De_Humedad(HUMIDITY_DATA, DHT_TYPE);
class DHT_Unified Sensor_De_Humedad(HUMIDITY_DATA, DHT_TYPE);


 /* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_Sensor_Humedad(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados del bus I2C                 -
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

void Inicializar_Sensor_Humedad(void)
{
  Sensor_De_Humedad.begin();

}
