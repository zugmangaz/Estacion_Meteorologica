/*    -----------------------------------------------------------------------------------------
    -                                                                                         -
    -   Archivo:        Cola_I2C.C                                                            -
    -                                                                                         -
    -   Comentario:       Manejo de Cola Circular para I2C_Bus,                               -
    -                     de hasta 256 Datos de 1 Byte en la RAM                              -
    -                                                                                         -
    -   Contenido:        Funciones en C para uso de colas de envio y recepcion               -
    -                                                                                         -
    -   Autor:          Gabriel Andres Zugman                                                 -
    -                                                                                         -
    -                                                                                         -
    -   Compiler / Assembler /                                                                -
    -   Linker / Debugger:    Arduino IDE v 1.8.8                                             -
    -                                                                                         -
    -   VersiÃ³n:        001                                                                  - 
    -                                                                                         -
    -   Ultima ModificaciÃ³n:  12/02/19                                                       -
    -                                                                                         -
    ----------------------------------------------------------------------------------------- */


/* ------------------------------
   -  InclusiÃ³n de Archivos -
   ------------------------------ */
#include <Cola_I2C.h>

/* ------------------------------------------
   -  Defines correspondientes a la Cola  -
   ------------------------------------------ */

#define LONGITUDcOLArECIBOI2C 6     // Longitud de la Cola de Recepcion
#define LONGITUDcOLAeNVIOI2C  5     // Longitud de la Cola de Envio


/* --------------------------------------------------
   -  Reserva de Variables de Cola de Recepcion   -
   -------------------------------------------------- */
// Reservas en RAM Interna de Acceso Directo

unsigned char  colaReciboI2C [LONGITUDcOLArECIBOI2C];// Cola en la que se Almacenan los Datos
unsigned char  cantidadColaReciboI2C;   // Contador de Datos Almacenados en la Cola de recepcion
unsigned char  *  entradaColaReciboI2C; // Puntero para la Entrada de Datos a la Cola de recepcion
unsigned char  *  salidaColaReciboI2C;  // Puntero para la Salida de Datos de la Cola de recepcion




/* --------------------------------------------------
   -  Reserva de Variables de Cola de Envio   -
   -------------------------------------------------- */
// Reservas en RAM Interna de Acceso Directo

unsigned char  colaEnvioI2C [LONGITUDcOLAeNVIOI2C];// Cola en la que se Almacenan los Datos
unsigned char  cantidadColaEnvioI2C;    // Contador de Datos Almacenados en la Cola de envio
unsigned char  *  entradaColaEnvioI2C;  // Puntero para la Entrada de Datos a la Cola de envio
unsigned char  *  salidaColaEnvioI2C; // Puntero para la Salida de Datos de la Cola de envio





/* ------------------
   -  Programa  -
   ------------------ */


/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    void InicializarColaRecibo (void);                      -
   -                                                -
   -  AcciÃ³n:     Inicializa el Contador de Datos Almacenados en la Cola de Recepcion,  -
   -          Inicializa el Puntero para la Entrada de Datos a la Cola de Recepcion,  -
   -          Inicializa el Puntero para la Salida de Datos de la Cola de Recepcion.  -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   -                                   -
   -  Modifica:   cantidadColaRecibo / entradaColaRecibo / salidaColaRecibo       -
   -  Destruye:   ACC / PSW / R7                              -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

void InicializarColaReciboI2C (void)
{
  cantidadColaReciboI2C = 0;
  entradaColaReciboI2C = colaReciboI2C;
  salidaColaReciboI2C = colaReciboI2C;

  return;
}



/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    void InicializarColaEnvio (void);                         -
   -                                                -
   -  AcciÃ³n:     Inicializa el Contador de Datos Almacenados en la Cola de Envio,          -
   -          Inicializa el Puntero para la Entrada de Datos a la Cola de Envio,        -
   -          Inicializa el Puntero para la Salida de Datos de la Cola de Envio.        -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   -                                   -
   -  Modifica:   cantidadColaEnvio / entradaColaEnvio / salidaColaEnvio                  -
   -  Destruye:   ACC / PSW / R7                              -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

void InicializarColaEnvioI2C (void)
{
  cantidadColaEnvioI2C = 0;
  entradaColaEnvioI2C = colaEnvioI2C;
  salidaColaEnvioI2C = colaEnvioI2C;

  return; 
}




/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char CantidadEnColaEnvioI2C ();                  -
   -                                                -
   -  AcciÃ³n:     Retorna el Contador de Datos Almacenados en la Cola.          -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   cantidadColaEnvioI2C                              -
   -  Modifica:   -                                   -
   -  Destruye:   R7                                    -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

unsigned char CantidadEnColaEnvioI2C (void)
{
    return (cantidadColaEnvioI2C);
} 




/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char AgregarDatoColaEnvioI2C (unsigned char dato);     -
   -                                              -
   -  AcciÃ³n:     Recibe un Dato,                             -
   -          Si la Cola estÃ¡ Llena seÃ±aliza y retorna.               -
   -          Si hay lugar en la Cola Almacena el Dato recibido en ella,        -
   -          Incrementa el Contador de Datos Almacenados en la Cola,         -
   -          Actualiza el Puntero de Ingreso de Datos a la Cola,           -
   -          SeÃ±aliza que ha podido y retorna                    -
   -                                              -
   -  Recibe:     Dato                                  -
   -  Devuelve:   COLAlLENA / EXITO                           -
   -  Modifica:   cola / cantidadCola / entradaCola                   -
   -  Destruye:   ACC / PSW / R0 / R6 / R7                        -
   -  Llama a:    -                                     -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                   -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

unsigned char AgregarDatoColaEnvioI2C (unsigned char  dato)
{
  if (cantidadColaEnvioI2C >= LONGITUDcOLAeNVIOI2C)
    return (COLAlLENA);

  *entradaColaEnvioI2C = dato;
  cantidadColaEnvioI2C ++;

  if (++entradaColaEnvioI2C >= (colaEnvioI2C + LONGITUDcOLAeNVIOI2C))
    entradaColaEnvioI2C = colaEnvioI2C;

  return (EXITO);
}





/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char RetirarDatoColaEnvioI2C ();             -
   -                                              -
   -  AcciÃ³n:     Si la Cola estÃ¡ VacÃ­a seÃ±aliza y retorna.               -
   -          Si hay Datos en la Cola recupera un Dato de ella,           -
   -          Decrementa el Contador de Datos Almacenados en la Cola,         -
   -          Actualiza el Puntero de Egreso de Datos de la Cola,           -
   -          Retorna el Dato retirado de la Cola.                  -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   COLAvACIA / dato                            -
   -  Modifica:   cantidadCola / salidaCola                       -
   -  Destruye:   ACC / PSW / R0 / R7                           -
   -  Llama a:    -                                     -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                   -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

unsigned char RetirarDatoColaEnvioI2C (void)
{
  char  dato;
  
  if (cantidadColaEnvioI2C == 0)
    return (COLAvACIA);

  dato = *salidaColaEnvioI2C;
  cantidadColaEnvioI2C --;

  if (++salidaColaEnvioI2C >= (colaEnvioI2C + LONGITUDcOLAeNVIOI2C))
    salidaColaEnvioI2C = colaEnvioI2C;

  return (dato);
} 





/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char CantidadEnColaReciboI2C ();                 -
   -                                                -
   -  AcciÃ³n:     Retorna el Contador de Datos Almacenados en la Cola.          -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   cantidadColaReciboI2C                             -
   -  Modifica:   -                                   -
   -  Destruye:   R7                                    -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

unsigned char CantidadEnColaReciboI2C (void)
{
    return (cantidadColaReciboI2C);
} 





/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char AgregarDatoColaReciboI2C (unsigned char dato);    -
   -                                              -
   -  AcciÃ³n:     Recibe un Dato,                             -
   -          Si la Cola estÃ¡ Llena seÃ±aliza y retorna.               -
   -          Si hay lugar en la Cola Almacena el Dato recibido en ella,        -
   -          Incrementa el Contador de Datos Almacenados en la Cola,         -
   -          Actualiza el Puntero de Ingreso de Datos a la Cola,           -
   -          SeÃ±aliza que ha podido y retorna                    -
   -                                              -
   -  Recibe:     Dato                                  -
   -  Devuelve:   COLAlLENA / EXITO                           -
   -  Modifica:   cola / cantidadCola / entradaCola                   -
   -  Destruye:   ACC / PSW / R0 / R6 / R7                        -
   -  Llama a:    -                                     -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                   -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

unsigned char AgregarDatoColaReciboI2C (unsigned char  dato)
{
  if (cantidadColaReciboI2C >= LONGITUDcOLArECIBOI2C)
    return (COLAlLENA);

  *entradaColaReciboI2C = dato;
  cantidadColaReciboI2C ++;

  if (++entradaColaReciboI2C >= (colaReciboI2C + LONGITUDcOLArECIBOI2C))
    entradaColaReciboI2C = colaReciboI2C;

  return (EXITO);
}






/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char RetirarDatoColaReciboI2C ();                  -
   -                                              -
   -  AcciÃ³n:     Si la Cola estÃ¡ VacÃ­a seÃ±aliza y retorna.               -
   -          Si hay Datos en la Cola recupera un Dato de ella,           -
   -          Decrementa el Contador de Datos Almacenados en la Cola,         -
   -          Actualiza el Puntero de Egreso de Datos de la Cola,           -
   -          Retorna el Dato retirado de la Cola.                  -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   COLAvACIA / dato                            -
   -  Modifica:   cantidadCola / salidaCola                       -
   -  Destruye:   ACC / PSW / R0 / R7                           -
   -  Llama a:    -                                     -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                   -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

unsigned char RetirarDatoColaReciboI2C (void)
{
  char  dato;
  if (cantidadColaReciboI2C == 0)
    return (COLAvACIA);

  dato = *salidaColaReciboI2C;
  cantidadColaReciboI2C --;

  if (++salidaColaReciboI2C >= (colaReciboI2C + LONGITUDcOLArECIBOI2C))
    salidaColaReciboI2C = colaReciboI2C;

  return (dato);
}
