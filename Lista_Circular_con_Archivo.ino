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
#include <FS.h>
#include <Lista_Circular_con_Archivo.h>    
/* --------------------------------------
   -    Defines de propósito general    -
   -------------------------------------- */
#define EXITO   0
#define ERROR   -1

#define COLAlLENA   ERROR         // SeÃ±alizaciÃ³n de Cola Llena
#define COLAvACIA   ERROR         // SeÃ±alizaciÃ³n de Cola VacÃ­a

#define CANTIDAD_MAXIMA_DATOS   2000
#define FORMATO_NOMBRE_ARCHIVO  "/M%0004d"

#define POSICION_SEEK_ARCHIVO_ANTERIOR    0     
#define POSICION_SEEK_ARCHIVO_SIGUIENTE   POSICION_SEEK_ARCHIVO_ANTERIOR  + LARGO_NOMBRE_NODOS + 1 
#define POSICION_SEEK_JSON                POSICION_SEEK_ARCHIVO_SIGUIENTE + LARGO_NOMBRE_NODOS + 1 


/*typedef struct
{
          char Nombre_Archivo_Nodo_Frontal[LARGO_NOMBRE_NODOS];
          char Nombre_Archivo_Nodo_Trasero[LARGO_NOMBRE_NODOS];
          char JSON_Medicion[LARGO_BUFFER];
          //File  *Contenido;
}Nodo_Lista;
*/



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

unsigned int Lista::Cantidad_Nodos (void)
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

  if(Cantidad_Nodos_Lista >= CANTIDAD_MAXIMA_DATOS)
      return false;
  
  char Nombre_Archivo_Nuevo[LARGO_NOMBRE_NODOS];
  do  
  {
      sprintf(Nombre_Archivo_Nuevo,FORMATO_NOMBRE_ARCHIVO,(unsigned int)random(CANTIDAD_MAXIMA_DATOS));
  }while(SPIFFS.exists(Nombre_Archivo_Nuevo));

  Serial.printf("Creo el nodo %s\n",Nombre_Archivo_Nuevo);
  
  File Archivo_Medicion = SPIFFS.open(Nombre_Archivo_Nuevo,"w+");
  if(!Archivo_Medicion)
  {
      Serial.printf("No se pudo crear el archivo\n");
      return false;
  }

  if(Cantidad_Nodos_Lista == 0)
  {
        strcpy( Nombre_Archivo_Entrada_Lista,  Nombre_Archivo_Nuevo);
        strcpy( Nombre_Archivo_Salida_Lista,  Nombre_Archivo_Nuevo);

        Archivo_Medicion.println(Nombre_Archivo_Nuevo);         // Guardo el nombre del archivo del nodo anterior
        Archivo_Medicion.println(Nombre_Archivo_Nuevo);         // Guardo el nombre del archivo del nodo siguiente
        Archivo_Medicion.println(dato);                         // Guardo el JSON de la medicion
  }
  else
  {

// Guardo en el nodo nuevo
        Archivo_Medicion.println(Nombre_Archivo_Salida_Lista);  // Guardo el nombre del archivo del nodo anterior
        Archivo_Medicion.println(Nombre_Archivo_Entrada_Lista); // Guardo el nombre del archivo del nodo siguiente
        Archivo_Medicion.println(dato);                         // Guardo el JSON de la medicion

        File Archivo_Entrada_Lista  = SPIFFS.open(Nombre_Archivo_Entrada_Lista,"r+");
        Archivo_Entrada_Lista.println(Nombre_Archivo_Nuevo);    // Apunto el nodo subsiguiente al nuevo nodo

        File Archivo_Salida_Lista   = SPIFFS.open(Nombre_Archivo_Salida_Lista,"r+");
        Archivo_Salida_Lista.seek(POSICION_SEEK_ARCHIVO_SIGUIENTE);
        Archivo_Salida_Lista.println(Nombre_Archivo_Nuevo);     // Guardo el nodo nuevo en el nodo anterior

        strcpy(Nombre_Archivo_Entrada_Lista, Nombre_Archivo_Nuevo);    // Guardo nueva entrada
        
        Archivo_Salida_Lista.close();
        Archivo_Entrada_Lista.close();
  }
  Archivo_Medicion.close();

  Cantidad_Nodos_Lista++;
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
  char Nombre_Archivo_Nodo_Aux[LARGO_NOMBRE_NODOS];
  int n;
  
  if (Cantidad_Nodos_Lista == 0)
    return false;
  
  File Archivo_Salida_Lista   = SPIFFS.open(Nombre_Archivo_Salida_Lista,"r+");

  Archivo_Salida_Lista.readBytesUntil('\n',Nombre_Archivo_Nodo_Aux,LARGO_NOMBRE_NODOS);
  Nombre_Archivo_Nodo_Aux[LARGO_NOMBRE_NODOS-1] = '\0';
  
  File Archivo_Nueva_Salida = SPIFFS.open(Nombre_Archivo_Nodo_Aux,"r+");  // Abro ante ultimo nodo
  if(!Archivo_Nueva_Salida)
      Serial.printf("No se pudo leer el archivo para nueva salida, intento nuevamente\n");
  
  Archivo_Nueva_Salida.seek(POSICION_SEEK_ARCHIVO_SIGUIENTE);    
  Archivo_Nueva_Salida.println(Nombre_Archivo_Entrada_Lista);    // Apunto al primer nodo de la lista
  
  File Archivo_Entrada_Lista  = SPIFFS.open( Nombre_Archivo_Entrada_Lista,"r+");
  if(!Archivo_Entrada_Lista)
      Serial.printf("No se pudo leer el archivo de entrada\n");

  Archivo_Entrada_Lista.println(Nombre_Archivo_Nodo_Aux);       // Apunto a la nueva salida 
 
  Archivo_Salida_Lista.seek(POSICION_SEEK_JSON);    

// Leo del dato del archivo
  for(n=0; dato[n-1] != '\n';n++)
      dato[n] = (char)Archivo_Salida_Lista.read();
  dato[n-1] = '\0';

  Archivo_Entrada_Lista.close();
  Archivo_Nueva_Salida.close();
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
  char Nombre_Archivo_Perdido[LARGO_NOMBRE_NODOS];
  unsigned int N_Nodos=0;
  
  for(int i=0; i<CANTIDAD_MAXIMA_DATOS;i++)
  {
      sprintf(Nombre_Archivo_Perdido,FORMATO_NOMBRE_ARCHIVO,i);
      if(SPIFFS.exists(Nombre_Archivo_Perdido))
          N_Nodos++;
  }      
  Serial.printf("Hay %d Nodos\n",N_Nodos);
  if(!N_Nodos)
  {
      Serial.printf("Termino la reparacion de la lista, se recuperaron %d Nodos\n",N_Nodos);
      return true;
  }

  for(int i=0; i<CANTIDAD_MAXIMA_DATOS;i++)
  {
        sprintf(Nombre_Archivo_Perdido,FORMATO_NOMBRE_ARCHIVO,i);
        if(SPIFFS.exists(Nombre_Archivo_Perdido))
        {
              strcpy(Nombre_Archivo_Salida_Lista,  Nombre_Archivo_Perdido);    // Guardo nueva salida
              do
              {
                  Serial.printf("Encontre el Nodo %s\n",Nombre_Archivo_Perdido);
                  File Archivo_Perdido = SPIFFS.open(Nombre_Archivo_Perdido,"a+");
                  Cantidad_Nodos_Lista++;
                  Archivo_Perdido.readBytesUntil('\n',Nombre_Archivo_Perdido,LARGO_NOMBRE_NODOS);
                  Nombre_Archivo_Perdido[LARGO_NOMBRE_NODOS-1] = '\0';
                  if(strcmp( Nombre_Archivo_Perdido,  Nombre_Archivo_Salida_Lista) == 0)
                      sprintf(Nombre_Archivo_Entrada_Lista,"%s",Archivo_Perdido.name());  // Guardo nueva entrada
                      
                  Archivo_Perdido.close();
              }while(strcmp( Nombre_Archivo_Perdido, Nombre_Archivo_Salida_Lista) != 0);
              Serial.printf("Termino la reparacion de la lista, se recuperaron %d Nodos\n",Cantidad_Nodos_Lista);
              break;
        }
  }
  if(N_Nodos != Cantidad_Nodos_Lista)
  {
      Serial.println(F("La lista esta corrupta. Se procede a eliminarla"));
      for(int i=0; i<CANTIDAD_MAXIMA_DATOS;i++)
      {
          sprintf(Nombre_Archivo_Perdido,FORMATO_NOMBRE_ARCHIVO,i);
          if(SPIFFS.exists(Nombre_Archivo_Perdido))
              SPIFFS.remove(Nombre_Archivo_Perdido);              
      }
      return false;      
  }
  return true;
}
