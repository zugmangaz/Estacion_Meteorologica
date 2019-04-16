
/*----------------------------------------------------------------------------
  -                                                                          -  
  - Archivo:            Sensor_Luz.ino                                            -
  -                                                                          -
  - Comentario:         -
  -                                                                          -
  - Contenido:         Genera los tags del pin out del sensor TSL256x.          -
  -                                                                          -
  - Autor:             Gabriel Andres Zugman                                 - 
  -                                                                           -
  - Compiler/Linker                                                         -
  - Debugger:            Arduino IDE V   1.8.8                               -
  -                                                      -
  - VersiÃ³n:             001                         -
  -                                                      -
  - Ultima ModificaciÃ³n: 07/02/19                                    -
  ----------------------------------------------------------------------------*/

#include <TSL256x.h>
#include <Adafruit_TSL2561_U.h>

Adafruit_TSL2561_Unified Sensor_De_Luz_TSL = Adafruit_TSL2561_Unified(DIRECCION_TSL2561_SENSOR1, ID_TSL2561_SENSOR1);

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_TSL256x(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el sensor de luz TSL256x                                       -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Estado_I2C_Bus                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_TSL256x(void)
{

    Sensor_De_Luz_TSL.enableAutoRange(true);
    Sensor_De_Luz_TSL.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); 
    
    Sensor_De_Luz_TSL.begin();
//    Serial.println("Exito -> Sensor de Luz TSL Inicializado");
}
