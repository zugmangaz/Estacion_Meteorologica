/*
; -------------------------------------------------------
;################################################################################
;# TÃ­tulo: Modulo Formato File system                                          #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  07/02/2019                             #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                             #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n:  Incluye biblioteca de funciones para utilizar un file system #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 07/02/2019 | G.Zugman | Inicial                                     #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */

#include <FS.h>

void Inicializar_File_System()
{

SPIFFS.begin();

}
