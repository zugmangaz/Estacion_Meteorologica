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
//#include <Cola_Mediciones.h>

/* --------------------------------------
   -    Defines de propósito general    -
   -------------------------------------- */
#define EXITO   0
#define ERROR   -1

#define COLAlLENA   ERROR         // SeÃ±alizaciÃ³n de Cola Llena
#define COLAvACIA   ERROR         // SeÃ±alizaciÃ³n de Cola VacÃ­a

/* ------------------------------------------
   -  Defines correspondientes a la Cola  -
   ------------------------------------------ */

class Cola
{
      public:
              Cola(unsigned char);
              unsigned char CantidadEnCola  (void); // Cantidad de Datos en la Cola
              unsigned char AgregarDatoCola (unsigned char);// Agregar Dato a la Cola
              unsigned char RetirarDatoCola (void);// Retirar Dato de la Cola  
              ~Cola(void);
      private:
              unsigned char   largo_cola;      // Largo de la cola      
              unsigned char  * cola;          // Cola en la que se Almacenan los Datos
              unsigned char  cantidadCola;    // Contador de Datos Almacenados en la Cola de recepcion
              unsigned char  *  entradaCola;  // Puntero para la Entrada de Datos a la Cola de recepcion
              unsigned char  *  salidaCola;   // Puntero para la Salida de Datos de la Cola de recepcion
};


/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    void Cola (void);                      -
   -                                                -
   -  AcciÃ³n:  Reserva dinamicamente espacio para la cola.                                     -     
   -          Inicializa el Contador de Datos Almacenados en la Cola de Recepcion,  -
   -          Inicializa el Puntero para la Entrada de Datos a la Cola de Recepcion,  -
   -          Inicializa el Puntero para la Salida de Datos de la Cola de Recepcion.  -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   -                                   -
   -  Modifica:   cantidadCola / entradaCola / salidaCola       -
   -  Destruye:   -                              -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

Cola::Cola(unsigned char largo)
{
  cola = new unsigned char [largo];
  largo_cola = largo;
  cantidadCola = 0;
  entradaCola = cola;
  salidaCola = cola;
}

/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    void ~Cola (void);                      -
   -                                                -
   -  AcciÃ³n:  Elimina el espacio para la cola.                                     -     
   -          Inicializa el Contador de Datos Almacenados en la Cola de Recepcion,  -
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

Cola::~Cola(void)
{
  delete[] cola;
}



/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char CantidadEnCola ();                 -
   -                                                -
   -  AcciÃ³n:     Retorna el Contador de Datos Almacenados en la Cola.          -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   cantidadCola                             -
   -  Modifica:   -                                   -
   -  Destruye:   R7                                    -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

unsigned char Cola::CantidadEnCola (void)
{
    return (cantidadCola);
} 





/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char AgregarDatoCola (unsigned char dato);    -
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

unsigned char Cola::AgregarDatoCola (unsigned char  dato)
{
  if (cantidadCola >= largo_cola)
    return (COLAlLENA);

  *entradaCola = dato;
  cantidadCola ++;

  if (++entradaCola >= (cola + largo_cola))
    entradaCola = cola;

  return (EXITO);
}






/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char RetirarDatoCola ();                  -
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

unsigned char Cola::RetirarDatoCola (void)
{
  char  dato;
  if (cantidadCola == 0)
    return (COLAvACIA);

  dato = *salidaCola;
  cantidadCola --;

  if (++salidaCola >= (cola + largo_cola))
    salidaCola = cola;

  return (dato);
}
