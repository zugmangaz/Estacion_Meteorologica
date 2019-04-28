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

#define POSICION_SEEK_ARCHIVO_ANTERIOR    0     
#define POSICION_SEEK_ARCHIVO_SIGUIENTE   6  
#define POSICION_SEEK_JSON                12

#define LARGO_NOMBRE_NODOS                6
typedef struct
{
          char Nombre_Archivo_Nodo_Frontal[LARGO_NOMBRE_NODOS];
          char Nombre_Archivo_Nodo_Trasero[LARGO_NOMBRE_NODOS];
          char JSON_Medicion[LARGO_BUFFER];
          //File  *Contenido;
}Nodo_Lista;


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
              char Nombre_Archivo_Entrada_Lista[LARGO_NOMBRE_NODOS];  // Puntero para la Entrada de Datos a la Lista
              char Nombre_Archivo_Salida_Lista[LARGO_NOMBRE_NODOS];   // Puntero para la Salida de Datos de la Lista
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
  strcpy(Nombre_Archivo_Entrada_Lista, Nombre_Archivo_Salida_Lista);
  strcpy(Nombre_Archivo_Salida_Lista, Nombre_Archivo_Entrada_Lista);
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
//  Nodo_Lista *P_Nodo_Entrada;
//  Nodo_Lista Nodo_Entrada, Nodo_Nuevo;
  char Nombre_Archivo_Nuevo[LARGO_NOMBRE_NODOS];
//  char Nombre_Archivo_Nodo_Frontal[LARGO_NOMBRE_NODOS];
//  char Nombre_Archivo_Nodo_Trasero[LARGO_NOMBRE_NODOS];
//  char JSON_Medicion[LARGO_BUFFER];
  
  do  
  {
      sprintf(Nombre_Archivo_Nuevo,"/M%003d",(unsigned int)random(CANTIDAD_MAXIMA_DATOS));
  }while(SPIFFS.exists(Nombre_Archivo_Nuevo));

  File Archivo_Medicion = SPIFFS.open(Nombre_Archivo_Nuevo,"a+");
  if(!Archivo_Medicion)
  {
      Serial.printf("No se pudo crear el archivo\n");
      return false;
  }

  if(Cantidad_Nodos_Lista == 0)
  {
        strcpy( Nombre_Archivo_Entrada_Lista,  Nombre_Archivo_Nuevo);
        strcpy( Nombre_Archivo_Salida_Lista,  Nombre_Archivo_Nuevo);
//        strcpy(Nombre_Archivo_Nodo_Frontal, Nombre_Archivo_Nuevo);
//        strcpy(Nombre_Archivo_Nodo_Trasero, Nombre_Archivo_Nuevo);

//        strcpy(Nodo_Aux.JSON_Medicion, dato);

        Archivo_Medicion.println(Nombre_Archivo_Nuevo);         // Guardo el nombre del archivo del nodo anterior
        Archivo_Medicion.println(Nombre_Archivo_Nuevo);         // Guardo el nombre del archivo del nodo siguiente
        Archivo_Medicion.println(dato);                         // Guardo el JSON de la medicion

        Archivo_Medicion.close();
  }
  else
  {
        File Archivo_Entrada_Lista  = SPIFFS.open(Nombre_Archivo_Entrada_Lista,"a+");
        File Archivo_Salida_Lista   = SPIFFS.open(Nombre_Archivo_Salida_Lista,"a+");
        File Archivo_Medicion       = SPIFFS.open(Nombre_Archivo_Nuevo,"a+");
        if(!Archivo_Medicion)
            Serial.printf("No se pudo crear el archivo\n");

// Guardo en el nodo nuevo
        Archivo_Medicion.println(Nombre_Archivo_Salida_Lista);  // Guardo el nombre del archivo del nodo anterior
        Archivo_Medicion.println(Nombre_Archivo_Entrada_Lista); // Guardo el nombre del archivo del nodo siguiente
        Archivo_Medicion.println(dato);                         // Guardo el JSON de la medicion


        Archivo_Entrada_Lista.println(Nombre_Archivo_Nuevo);    // Guardo el nodo nuevo en el siguiente nodo
        Archivo_Salida_Lista.seek(POSICION_SEEK_ARCHIVO_SIGUIENTE);
        Archivo_Salida_Lista.println(Nombre_Archivo_Nuevo);     // Guardo el nodo nuevo en el nodo anterior
        
        
/*        Copia_Dato->Nodo_Trasero = Entrada_Lista->Nodo_Trasero;   Archivo_Medicion.println(Nombre_Archivo_Nodo_Trasero);
        Entrada_Lista->Nodo_Trasero = Copia_Dato;

        Salida_Lista->Nodo_Frontal = Copia_Dato;        
        Copia_Dato->Nodo_Frontal = Entrada_Lista;                 Archivo_Medicion.println(Nombre_Archivo_Entrada_Lista);

        Entrada_Lista = Copia_Dato;
*/
  }

  Cantidad_Nodos_Lista++;
/*  File Archivo_Medicion = SPIFFS.open(Copia_Dato->Nombre_Archivo,"a+");
  if(!Archivo_Medicion)
      Serial.printf("No se pudo crear el archivo\n");
//  largo = strlen(dato);
*/
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
  char Nombre_Archivo_Nodo_Aux[LARGO_NOMBRE_NODOS];
  char Nombre_Archivo_Nodo_Trasero[LARGO_NOMBRE_NODOS];
  
  if (Cantidad_Nodos_Lista == 0)
    return false;

  File Archivo_Salida_Lista   = SPIFFS.open(Nombre_Archivo_Salida_Lista,"a+");

  int n;
//      Leo el nombre del archivo del ante ultimo nodo        
  for(n=0; Nombre_Archivo_Nodo_Aux[n-1] != '\n';n++)
      Nombre_Archivo_Nodo_Aux[n] = (char)Archivo_Salida_Lista.read();
  Nombre_Archivo_Nodo_Aux[n-1] = '\0';
 
  File Archivo_Nueva_Salida       = SPIFFS.open(Nombre_Archivo_Nodo_Aux,"a+");  // Abro ante ultimo nodo
  Archivo_Nueva_Salida.seek(POSICION_SEEK_ARCHIVO_SIGUIENTE);    
  Archivo_Nueva_Salida.println(Nombre_Archivo_Entrada_Lista);    // Apunto al primer nodo de la lista
  Archivo_Nueva_Salida.close();
  
  File Archivo_Entrada_Lista  = SPIFFS.open( Nombre_Archivo_Entrada_Lista,"a+");

  Archivo_Entrada_Lista.println(Nombre_Archivo_Nodo_Aux);       // Apunto a la nueva salida 
  Archivo_Entrada_Lista.close();
 
  Archivo_Salida_Lista.seek(POSICION_SEEK_JSON);    

// Leo del dato del archivo
  for(n=0; dato[n-1] != '\n';n++)
      dato[n] = (char)Archivo_Salida_Lista.read();
  dato[n-1] = '\0';


  Archivo_Salida_Lista.close();                 // Cierro el archivo que retiro de la lista
  SPIFFS.remove( Nombre_Archivo_Salida_Lista);   // Remuevo el nodo de la lista
  
  strcpy(Nombre_Archivo_Salida_Lista, Nombre_Archivo_Nodo_Aux);    // Guardo nueva salida

  Cantidad_Nodos_Lista --;

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
//  struct Nodo *Copia_Dato;
  char Nombre_Archivo_Perdido[LARGO_NOMBRE_NODOS];
  

  for(int i=0; i<CANTIDAD_MAXIMA_DATOS;i++)
  {
        sprintf(Nombre_Archivo_Perdido,"/M%003d",i);
        if(SPIFFS.exists(Nombre_Archivo_Perdido))
        {
              Serial.printf("Encontre el Nodo /M%003d\n",i);
              strcpy(Nombre_Archivo_Salida_Lista,  Nombre_Archivo_Perdido);    // Guardo nueva salida
              do
              {
                  File Archivo_Perdido = SPIFFS.open(Nombre_Archivo_Perdido,"a+");
                  Cantidad_Nodos_Lista++;
                    int n;
                  for(n=0; Nombre_Archivo_Perdido[n-1] != '\n';n++)
                      Nombre_Archivo_Perdido[n] = (char)Archivo_Perdido.read();
                  Nombre_Archivo_Perdido[n-1] = '\0';
                  if(strcmp( Nombre_Archivo_Perdido,  Nombre_Archivo_Salida_Lista) == 0)
                      sprintf(Nombre_Archivo_Entrada_Lista,"%s",Archivo_Perdido.name());  // Guardo nueva entrada
                      
                  Archivo_Perdido.close();
              }while(strcmp( Nombre_Archivo_Perdido, Nombre_Archivo_Salida_Lista) != 0);
              Serial.printf("Termino la reparacion de la lista, se recuperaron %d Nodos\n",Cantidad_Nodos_Lista);
              return true;
        }
  }
  Serial.printf("Termino la reparacion de la lista, se recuperaron %d Nodos\n",Cantidad_Nodos_Lista);
  return true;
}
