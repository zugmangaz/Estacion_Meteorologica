/*
; -------------------------------------------------------
;################################################################################
;# TÃ­tulo: Modulo Administración EEPROM                                        #
;#                                                                              #
;#  VersiÃ³n:    2.0             Fecha:  10/03/2019                             #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                             #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n:  Incluye biblioteca de funciones para utilizar memoria EEPROM #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 05/02/2019 | G.Zugman | Inicial                                     #
;#    2.0 | 10/03/2019 | G.Zugman | Final                                       #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */
#include <EEPROM.h>




/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_EEPROM(void);                        -
  -                                             -
  - AcciÃ³n:     Carga la memoria EEPROM en RAM                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   -                                  -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_EEPROM(void)
{

    EEPROM.begin(100);

}
/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Guardar_EEPROM(int , String );                        -
  -                                             -
  - AcciÃ³n:     Guarda strings de hasta 50 bytes en la EEPROM                             -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   -                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Conexion_WiFi()                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */
void Guardar_EEPROM(int addr, String a) {
  int tamano = a.length(); 
  char inchar[50]; 
  a.toCharArray(inchar, tamano+1);
  for (int i = 0; i < tamano; i++) {
    EEPROM.write(addr+i, inchar[i]);
  }
  for (int i = tamano; i < 50; i++) {
    EEPROM.write(addr+i, 255);
  }
  EEPROM.commit();
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    String Leer_EEPROM(int);                        -
  -                                             -
  - AcciÃ³n:     Lee strings de hasta 50 bytes en la EEPROM                             -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   -                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Conexion_WiFi()                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

//-----------------Función para leer la EEPROM------------------------
String Leer_EEPROM(int addr) {
   byte lectura;
   String strlectura;
   for (int i = addr; i < addr+50; i++) {
      lectura = EEPROM.read(i);
      if (lectura != 255) {
        strlectura += (char)lectura;
      }
   }
   return strlectura;
}
