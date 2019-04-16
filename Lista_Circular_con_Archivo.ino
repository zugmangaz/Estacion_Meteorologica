/*    -----------------------------------------------------------------------------------------
    -                                                                                         -
    -   Archivo:        Lista_Circular.C                                                            -
    -                                                                                         -
    -   Comentario:       Manejo de Lista Circular,                               -
    -                     de hasta 256 Datos de 1 Byte en la RAM                              -
    -                                                                                         -
    -   Contenido:        Funciones en C para uso de Listas de envio y recepcion               -
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
#include <FS.h>
/* --------------------------------------
   -    Defines de propósito general    -
   -------------------------------------- */
#define EXITO   0
#define ERROR   -1

#define COLAlLENA   ERROR         // SeÃ±alizaciÃ³n de Cola Llena
#define COLAvACIA   ERROR         // SeÃ±alizaciÃ³n de Cola VacÃ­a

#define LARGO_BUFFER            348
#define CANTIDAD_MAXIMA_DATOS   1000
struct Nodo
{
          struct Nodo *Nodo_Frontal;
          struct Nodo *Nodo_Trasero;
          char Nombre_Archivo[6];
          //File  *Contenido;
};


class Lista
{
      public:
              Lista();
              unsigned char Cantidad_Nodos  (void); // Cantidad de Nodos en la Lista
              bool Agregar_Dato_Lista (char*);// Agregar Dato y Nodo a la Lista
              bool Retirar_Dato_Lista (char*);// Retirar Dato y Nodo de la Lista  
              bool Reparar_Lista(void);       // Reparacion de la lista, mediante la busqueda de archivos en la memoria Flash
              ~Lista(void);
      private:
//              unsigned char   Largo_Lista;      // Largo de la lista      
//              class Nodo     * lista[];          // Comienzo de la lista en la que se Almacenan los Datos
              unsigned char  Cantidad_Nodos_Lista;    // Contador de Nodos Almacenados en la Lista
              struct Nodo   *  Entrada_Lista;  // Puntero para la Entrada de Datos a la Lista
              struct Nodo   *  Salida_Lista;   // Puntero para la Salida de Datos de la Lista
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
   -  Modifica:   Cantidad_Nodos_Lista / Entrada_Lista / Salida_Lista       -
   -  Destruye:   -                              -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

Lista::Lista()
{
//  lista = new unsigned char;
  Cantidad_Nodos_Lista = 0;
  Entrada_Lista = Salida_Lista;
  Salida_Lista = Entrada_Lista;
}

/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    void ~Lista (void);                      -
   -                                                -
   -  AcciÃ³n:  Elimina el espacio para la Lista.                                     -     
   -          Inicializa el Contador de Datos Almacenados en la Lista de Recepcion,  -
   -          Inicializa el Puntero para la Entrada de Datos a la Lista de Recepcion,  -
   -          Inicializa el Puntero para la Salida de Datos de la Lista de Recepcion.  -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   -                                   -
   -  Modifica:   cantidadListaRecibo / entradaListaRecibo / salidaListaRecibo       -
   -  Destruye:   ACC / PSW / R7                              -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

Lista::~Lista(void)
{

}



/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char CantidadEnLista ();                 -
   -                                                -
   -  AcciÃ³n:     Retorna el Contador de Datos Almacenados en la Lista.          -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   cantidadLista                             -
   -  Modifica:   -                                   -
   -  Destruye:   R7                                    -
   -  Llama a:    -                                   -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                 -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

unsigned char Lista::Cantidad_Nodos (void)
{
    return (Cantidad_Nodos_Lista);
} 



/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char Agregar_Nodo_Lista (char *dato);    -
   -                                              -
   -  AcciÃ³n:     Recibe un puntero de vector char,                             -
   -          Si la memoria esta llena retorna false.               -
   -          Si hay lugar en la memoria reserva memoria dinamicamente y copia el vector,        -
   -          Incrementa el Contador de Datos Almacenados en la Lista,         -
   -          Actualiza el Puntero de Ingreso de Datos a la Lista y ,           -
   -          SeÃ±aliza que ha podido y retorna                    -
   -                                              -
   -  Recibe:     Dato                                  -
   -  Devuelve:   true / false                           -
   -  Modifica:   Entrada_Lista / Cantidad_Nodos_Lista                   -
   -  Destruye:   -                        -
   -  Llama a:    -                                     -
   -  Llamada por:  Mediciones                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                   -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

bool Lista::Agregar_Dato_Lista (char *dato)
{
  struct Nodo *Copia_Dato;
  size_t largo;
  Copia_Dato = new struct Nodo;
  
  if(!Copia_Dato)
  {
      // Serial.printf("no hay mas lugar en la memoria");
      return false;
  }

  if(Cantidad_Nodos_Lista == 0)
  {
        Entrada_Lista = Copia_Dato;
        Salida_Lista  = Copia_Dato;
        Copia_Dato->Nodo_Frontal = Copia_Dato;
        Copia_Dato->Nodo_Trasero = Copia_Dato;
  }
  else
  {
        Copia_Dato->Nodo_Trasero = Entrada_Lista->Nodo_Trasero;
        Entrada_Lista->Nodo_Trasero = Copia_Dato;

        Salida_Lista->Nodo_Frontal = Copia_Dato;        
        Copia_Dato->Nodo_Frontal = Entrada_Lista;

        Entrada_Lista = Copia_Dato;
  }
  int i=0;
  do  
  {
    sprintf(Copia_Dato->Nombre_Archivo,"/M%d",(unsigned char)random(CANTIDAD_MAXIMA_DATOS));
  }while(SPIFFS.exists(Copia_Dato->Nombre_Archivo));

  Cantidad_Nodos_Lista++;
  File Archivo_Medicion = SPIFFS.open(Copia_Dato->Nombre_Archivo,"a+");
  if(!Archivo_Medicion)
      Serial.printf("No se pudo crear el archivo\n");
//  largo = strlen(dato);
  Archivo_Medicion.print(dato);
  Archivo_Medicion.close();
//  Copia_Dato->Contenido = new char[largo];
//  strcpy(Copia_Dato->Contenido, dato);
  return true;
}

/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char Retirar_Nodo_Lista ();                  -
   -                                              -
   -  AcciÃ³n:     Si la Lista estÃ¡ VacÃ­a seÃ±aliza y retorna.               -
   -          Si hay Datos en la Lista recupera un Dato de ella,           -
   -          Decrementa el Contador de Datos Almacenados en la Lista,         -
   -          Actualiza el Puntero de Egreso de Datos de la Lista,           -
   -          Retorna el Dato retirado de la Lista.                  -
   -                                              -
   -  Recibe:     -                                   -
   -  Devuelve:   COLAvACIA / dato                            -
   -  Modifica:   Cantidad_Nodos_Lista / Salida_Lista                       -
   -  Destruye:   ACC / PSW / R0 / R7                           -
   -  Llama a:    -                                     -
   -  Llamada por:  Main                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                   -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

bool Lista::Retirar_Dato_Lista (char *dato)
{
  struct Nodo *Aux;
  
  if (Cantidad_Nodos_Lista == 0)
    return false;


  File Archivo_Medicion = SPIFFS.open(Salida_Lista->Nombre_Archivo,"r");
  int j;
  for(j=0; j<Archivo_Medicion.size();j++)
        dato[j] = (char)Archivo_Medicion.read();
  dato[j] = '\0';
//  String s = Archivo_Medicion.readStringUntil('\0');
//  s.toCharArray(dato,LARGO_BUFFER);
  SPIFFS.remove(Salida_Lista->Nombre_Archivo);
//  strcpy(dato, Salida_Lista->Contenido);
  Cantidad_Nodos_Lista --;

  Aux = Salida_Lista->Nodo_Trasero;
  Entrada_Lista->Nodo_Trasero = Salida_Lista->Nodo_Trasero;
  Aux->Nodo_Frontal = Entrada_Lista;
  
  Aux = Salida_Lista;
  Salida_Lista = Salida_Lista->Nodo_Trasero;

  delete Aux;

  return true;
}



/* ----------------------------------------------------------------------------------------------
   -                                              -
   -  FunciÃ³n:    unsigned char Agregar_Nodo_Lista (char *dato);    -
   -                                              -
   -  AcciÃ³n:     Recibe un puntero de vector char,                             -
   -          Si la memoria esta llena retorna false.               -
   -          Si hay lugar en la memoria reserva memoria dinamicamente y copia el vector,        -
   -          Incrementa el Contador de Datos Almacenados en la Lista,         -
   -          Actualiza el Puntero de Ingreso de Datos a la Lista y ,           -
   -          SeÃ±aliza que ha podido y retorna                    -
   -                                              -
   -  Recibe:     Dato                                  -
   -  Devuelve:   true / false                           -
   -  Modifica:   Entrada_Lista / Cantidad_Nodos_Lista                   -
   -  Destruye:   -                        -
   -  Llama a:    -                                     -
   -  Llamada por:  Mediciones                                  -
   -  Macros usados:  -                                   -
   -  Nivel de STACK: 2 Bytes                                   -
   -                                              -
   ---------------------------------------------------------------------------------------------- */

bool Lista::Reparar_Lista (void)
{
  struct Nodo *Copia_Dato;
  char Nombre_Archivo_Perdido[6];
  
  for(int i=0; i<CANTIDAD_MAXIMA_DATOS;i++)
  {

          sprintf(Nombre_Archivo_Perdido,"/M%d",i);
        if(SPIFFS.exists(Nombre_Archivo_Perdido))
        {
              Serial.printf("Recupero Nodo /M%d\n",i);
              Copia_Dato = new struct Nodo;
                          
              if(Cantidad_Nodos_Lista == 0)
              {
                    Entrada_Lista = Copia_Dato;
                    Salida_Lista  = Copia_Dato;
                    Copia_Dato->Nodo_Frontal = Copia_Dato;
                    Copia_Dato->Nodo_Trasero = Copia_Dato;
              }
              else
              {
                    Copia_Dato->Nodo_Trasero = Entrada_Lista->Nodo_Trasero;
                    Entrada_Lista->Nodo_Trasero = Copia_Dato;
            
                    Salida_Lista->Nodo_Frontal = Copia_Dato;        
                    Copia_Dato->Nodo_Frontal = Entrada_Lista;
            
                    Entrada_Lista = Copia_Dato;
              }
              Cantidad_Nodos_Lista++;
              strcpy(Copia_Dato->Nombre_Archivo,Nombre_Archivo_Perdido);
        }
  }               
  Serial.printf("Termino la reparacion de la lista, se recuperaron %d Nodos\n",Cantidad_Nodos_Lista);
  return true;
}
