/*
; -------------------------------------------------------
;################################################################################
;# TÃ­tulo: Modulo de medicion a traves del ADC                                 #
;#                                                                              #
;#  VersiÃ³n:    1.0             Fecha:  10/02/2019                             #
;#  Autor:      G. Zugman       Tab:  0                                         #
;#  CompilaciÃ³n:  Usar Arduino IDE                                             #
;#  Uso:      -                                                                 #
;#  --------------------------------------------------------------------------  #
;#  DescripciÃ³n:                                                               #
;#  ------------------------------------------------------------------------    #
;#  Revisiones:                                                                 #
;#    1.0 | 05/02/2019 | G.Zugman | Inicial                                     #
;#  ------------------------------------------------------------------------    #
;#  TODO:                                                                       #
;#    -                                                                         #
;################################################################################
 */
#include <MAX11603.h>
#include <DHT_U.h>
//#include <Adafruit_TSL2561_U.h>
#include <Thread.h>
#include <math.h>
#include <Tiempo.h>
#include <MQ135.h>
#include <Mediciones.h>
//#include <Lista_Circular_con_Archivo.h>
/*--------------------------
     Definiciones   
 --------------------------*/


#define CANTIDAD_MEDICIONES_A_GUARDAR   100
#define BYTES_POR_MEDICION              20
#define BUFFER_MEDICIONES     BYTES_POR_MEDICION * CANTIDAD_MEDICIONES_A_GUARDAR

#define LONGITUD_MAC_ADDRESS    17

/*
#define SENSOR_TEMPERATURA      1
#define SENSOR_HUMEDAD          2
#define SENSOR_ILUMINACION      3
#define SENSOR_SONIDO           4
#define SENSOR_GAS1             5
//#define SENSOR_GAS2             6

#define ULTIMO_SENSOR       5
*/

/*---------------------------------
     Umbrales de falla sensores   
 ----------------------------------*/

#define UMBRAL_FALLA_SENSOR_TEMPERATURA   NAN
#define UMBRAL_FALLA_SENSOR_HUMEDAD       NAN
#define UMBRAL_FALLA_SENSOR_ILUMINACION   0xFFF0
#define UMBRAL_FALLA_SENSOR_SONIDO        -48
#define UMBRAL_INFERIOR_FALLA_SENSOR_GAS1 0
#define UMBRAL_SUPERIOR_FALLA_SENSOR_GAS1 200000
#define UMBRAL_INFERIOR_FALLA_SENSOR_GAS2 0
#define UMBRAL_SUPERIOR_FALLA_SENSOR_GAS2 200000


/*--------------------------
     Tiempos Mediciones   
 --------------------------*/

#define TIEMPO_ESPERA_INICIAL         15000
#define TIEMPO_PERIODO_MEDICIONES     60000   //  (30000 milisegundos)  
#define TIEMPO_MEDIR_ADC              100       //  (100 milisegundos)
#define TIEMPO_LECTURA_ADC            100       //  (100 milisegundos)
#define TIEMPO_MEDIR_TEMP_HUMEDAD     100       //  (100 milisegundos)
#define TIEMPO_LECTURA_TEMP_HUMEDAD   100       //  (100 milisegundos)
#define TIEMPO_ESPERA_PARA_MEDIR_LUZ  100       //  (100 milisegundos)
#define TIEMPO_HABILITAR_LUZ          100       //  (100 milisegundos)
#define TIEMPO_MEDIR_LUZ              100       //  (100 milisegundos)
#define TIEMPO_AJUSTE_LUZ             100       //  (100 milisegundos)
#define TIEMPO_GUARDADO_MEDICION_LUZ  100       //  (100 milisegundos)
#define TIEMPO_EVALUAR_PUBLICACION    100       //  (100 milisegundos)
#define TIEMPO_MAX_SIN_PUBLICACIONES  600000    // (600 segundos)


#define TIEMPO_TICKER_MEDICIONES  100             // 100 milisegundos


/*--------------------------------------
     Ticks para maquina de estados   
 ---------------------------------------*/
#define TICKS_MEDICIONES_POR_SEG  1000/(TIEMPO_TICKER_MEDICIONES)        // 10 Ticks/seg

#define TICKS_ESPERA_INICIAL            TIEMPO_ESPERA_INICIAL         / TIEMPO_TICKER_MEDICIONES
#define TICKS_PERIODO_MEDICIONES        TIEMPO_PERIODO_MEDICIONES     / TIEMPO_TICKER_MEDICIONES
#define TICKS_MEDIR_ADC                 TIEMPO_MEDIR_ADC              / TIEMPO_TICKER_MEDICIONES
#define TICKS_LECTURA_ADC               TIEMPO_LECTURA_ADC            / TIEMPO_TICKER_MEDICIONES
#define TICKS_MEDIR_TEMP_HUMEDAD        TIEMPO_MEDIR_TEMP_HUMEDAD     / TIEMPO_TICKER_MEDICIONES
#define TICKS_LECTURA_TEMP_HUMEDAD      TIEMPO_LECTURA_TEMP_HUMEDAD   / TIEMPO_TICKER_MEDICIONES
#define TICKS_ESPERA_PARA_MEDIR_LUZ     TIEMPO_ESPERA_PARA_MEDIR_LUZ  / TIEMPO_TICKER_MEDICIONES
#define TICKS_MEDIR_LUZ                 TIEMPO_MEDIR_LUZ              / TIEMPO_TICKER_MEDICIONES
#define TICKS_HABILITAR_LUZ             TIEMPO_HABILITAR_LUZ          / TIEMPO_TICKER_MEDICIONES
#define TICKS_AJUSTE_LUZ                TIEMPO_AJUSTE_LUZ             / TIEMPO_TICKER_MEDICIONES
#define TICKS_GUARDADO_MEDICION_LUZ     TIEMPO_GUARDADO_MEDICION_LUZ  / TIEMPO_TICKER_MEDICIONES
#define TICKS_EVALUAR_PUBLICACION       TIEMPO_EVALUAR_PUBLICACION    / TIEMPO_TICKER_MEDICIONES
#define TICKS_MAX_SIN_PUBLICACIONES     TIEMPO_MAX_SIN_PUBLICACIONES  / TIEMPO_TICKER_MEDICIONES
//#define TICKS_IDLE_MEDICIONES           TIEMPO_IDLE_MEDICIONES        / TIEMPO_TICKER_MEDICIONES

//#define TICKS_IDLE_MEDICIONES     TICKS_PERIODO_MEDICIONES - TICKS_MEDIR_ADC - TICKS_LECTURA_ADC - TICKS_MEDIR_TEMP_HUMEDAD - TICKS_LECTURA_TEMP_HUMEDAD - TICKS_ESPERA_PARA_MEDIR_LUZ - TICKS_MEDIR_LUZ - TICKS_AJUSTE_LUZ - TICKS_GUARDADO_MEDICION_LUZ - TICKS_EVALUAR_PUBLICACION

// ------------------------------------------------------
// -               Estados Mediciones                   -
// ------------------------------------------------------
typedef void(*Retorno_funcion)(void);

Retorno_funcion   Rutina_Estado_IDLE_MEDICIONES(void);

Retorno_funcion   Rutina_Estado_MEDIR_ADC(void);
Retorno_funcion   Rutina_Estado_LEER_MEDICION_ADC(void);

Retorno_funcion   Rutina_Estado_MEDIR_HUMEDAD(void);
Retorno_funcion   Rutina_Estado_LEER_MEDICION_HUMEDAD(void);

Retorno_funcion   Rutina_Estado_HABILITAR_LUZ(void);
Retorno_funcion   Rutina_Estado_MEDIR_LUZ(void);
Retorno_funcion   Rutina_Estado_VERIFICAR_AJUSTAR_MEDICION_LUZ(void);
Retorno_funcion   Rutina_Estado_GUARDAR_MEDICION_LUZ(void);

Retorno_funcion   Rutina_Estado_EVALUAR_PUBLICACION(void);

Retorno_funcion Puntero_Proximo_Estado_Mediciones;

void Enviar_Medicion(void);
/*--------------------------------            
-   Variable de uso del modulo   -
----------------------------------*/


Thread Thread_Mediciones = Thread();

extern class DHT_Unified Sensor_De_Humedad;
//extern class Adafruit_TSL2561_Unified Sensor_De_Luz_TSL;

class Lista Lista_Mediciones;

int Time_Out_Sin_Mediciones;
int Tick_Mediciones;
unsigned char Num_Sensor;
//enum Numeracion_Sensor {SENSOR_TEMPERATURA, SENSOR_HUMEDAD, SENSOR_ILUMINACION, SENSOR_SONIDO, SENSOR_GAS1, SENSOR_GAS2} Num_Sensor;


struct Informacion_Sensor Data_Sensor[ULTIMO_SENSOR];
//const struct Informacion_Sensor Data_Basica = {1,"","","",0,25,25,25,25,25,25,25,25,"",TICKS_MAX_SIN_PUBLICACIONES,""};
extern struct Tiempo Fecha_Hora_Actual;
extern byte Mac_Address[LONGITUD_MAC_ADDRESS+1];



unsigned char Audio;
unsigned char Envolvente;
float Ruido_dB;
unsigned char Gas1;
unsigned char Gas2;
float PPM_Gas1;
float PPM_Gas2;
//sensors_event_t Evento_Luz;
sensors_event_t Evento_Humedad;
sensors_event_t Evento_Temperatura;
//uint16_t Luz_broadband, Luz_ir;

//char *Buffer_Medicion_a_Guaradar;

unsigned char Falla_Sensores;
bool Alerta_Mediciones;
extern bool Falla_Conexion;


/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_ADC(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el ADC                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   -                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Inicializar_Mediciones                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */
  
void Inicializar_ADC(void)
{
  AgregarDatoColaEnvioI2C(DIRECCION_ESCRITURA_ADC_MAX11603);
  AgregarDatoColaEnvioI2C(2);
  AgregarDatoColaEnvioI2C(SETUP_BYTE_DEFAULT);
  AgregarDatoColaEnvioI2C(CONFIGURATION_BYTE_DEFAULT);
  
}

/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Inicializar_Mediciones(void);                        -
  -                                             -
  - AcciÃ³n:     Inicializa el la maquina de estados para medir                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Puntero_Proximo_Estado_Mediciones / Tick_Mediciones                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Setup                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */

void Inicializar_Mediciones(void)
{

//  Wire.begin(MASTER_SDA,MASTER_SCL)             // Define los pines del bus I2C, primero SDA y segundo SCL
  Serial.println(F("\nBusco la lista perdida"));
  Lista_Mediciones.Reparar_Lista();
  Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_IDLE_MEDICIONES;
  Tick_Mediciones = TICKS_ESPERA_INICIAL;
  Time_Out_Sin_Mediciones = TICKS_PERIODO_MEDICIONES;
  Inicializar_ADC();
  for(Num_Sensor=0; Num_Sensor<ULTIMO_SENSOR; Num_Sensor++)
  {
      strcpy(Data_Sensor[Num_Sensor].Status,"normal");
      Data_Sensor[Num_Sensor].Time_Out_Sin_Publicaciones = TICKS_MAX_SIN_PUBLICACIONES;  
  }
  Serial.println(F("Mediciones Inicializada"));
  Thread_Mediciones.onRun(Maquina_Mediciones);
  Thread_Mediciones.setInterval(TIEMPO_TICKER_MEDICIONES);
  controll.add(&Thread_Mediciones);

}



//------------------------------------------------------
/* ----------------------------------------------------------------------------------------------
  -                                             -
  - FunciÃ³n:    void Maquina_Mediciones(void);                        -
  -                                             -
  - AcciÃ³n:     Ejecuta la maquina de estados que realiza las mediciones                 -
  - Recibe:     -                                   -
  - Devuelve:   -                                   -
  - Modifica:   Tick_Mediciones                              -
  - Destruye:   -                                     -
  - Llama a:    -                                     -
  - Llamada por:  Ticker_Mediciones                                  -
  - Macros usados:  -                                   -
  - Nivel de STACK:                                    -
  -                                             -
  ---------------------------------------------------------------------------------------------- */


void Maquina_Mediciones()
{
  for(Num_Sensor=0; Num_Sensor<ULTIMO_SENSOR; Num_Sensor++)
  {     
      if(Data_Sensor[Num_Sensor].Time_Out_Sin_Publicaciones)
          Data_Sensor[Num_Sensor].Time_Out_Sin_Publicaciones--;
  }
  if(Time_Out_Sin_Mediciones)
      Time_Out_Sin_Mediciones--;
  if(!--Tick_Mediciones)
      Puntero_Proximo_Estado_Mediciones();
  return;    
  
}


/* -----------------------------------------------------
                  Estados Mediciones
  ------------------------------------------------------ */  
//------------------------   1   ------------------------------
Retorno_funcion  Rutina_Estado_IDLE_MEDICIONES(void)
{
        if(!Data_Sensor[0].Numero_Sensor || Fecha_Hora_Actual.Ano == 0)
        {
            Serial.println(F("No se han configurado los sensores aun, no mido"));
            Tick_Mediciones = TICKS_ESPERA_INICIAL;
            Time_Out_Sin_Mediciones = TICKS_PERIODO_MEDICIONES;
            Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_IDLE_MEDICIONES;     
        }
        else
        {
            Time_Out_Sin_Mediciones = TICKS_PERIODO_MEDICIONES;
            Tick_Mediciones = TICKS_MEDIR_ADC;
            Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_MEDIR_ADC;
        }

  return Puntero_Proximo_Estado_Mediciones;
}


//------------------------   2   ------------------------------
Retorno_funcion  Rutina_Estado_MEDIR_ADC(void)
{

      Serial.println(F("Solicito medir"));
      AgregarDatoColaEnvioI2C(DIRECCION_LECTURA_ADC_MAX11603);
      AgregarDatoColaEnvioI2C(CANTIDAD_DE_CANALES_ADC_MAX11603);

      Tick_Mediciones = TICKS_LECTURA_ADC;     
      Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_LEER_MEDICION_ADC;
      return Puntero_Proximo_Estado_Mediciones;

}

//------------------------     3    ------------------------------
Retorno_funcion  Rutina_Estado_LEER_MEDICION_ADC(void)
{

      if(CantidadEnColaReciboI2C()==CANTIDAD_DE_CANALES_ADC_MAX11603)
      {
          Audio=       RetirarDatoColaReciboI2C();
          Envolvente=  RetirarDatoColaReciboI2C();
          Gas1 =       RetirarDatoColaReciboI2C();
          Gas2 =       RetirarDatoColaReciboI2C();
    
          Ruido_dB = 40*log10((float)Envolvente)+10;

          Tick_Mediciones = TICKS_MEDIR_TEMP_HUMEDAD;     
          Serial.printf("Datos!!! %d %3.2f dB %d %d \n",Audio, Ruido_dB, Gas1, Gas2);
          Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_MEDIR_HUMEDAD;
      }
      else
      {
          Tick_Mediciones = TICKS_LECTURA_ADC;     
          Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_LEER_MEDICION_ADC;
      }        
      return Puntero_Proximo_Estado_Mediciones;

}

//------------------------     4      ------------------------------
Retorno_funcion  Rutina_Estado_MEDIR_HUMEDAD(void)
{
 
  
    Sensor_De_Humedad.temperature().getEvent(&Evento_Temperatura);
    Sensor_De_Humedad.humidity().getEvent(&Evento_Humedad);

    Tick_Mediciones = TICKS_LECTURA_TEMP_HUMEDAD;         
    Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_LEER_MEDICION_HUMEDAD;
    return Puntero_Proximo_Estado_Mediciones;

}

//------------------------      5      ------------------------------
Retorno_funcion  Rutina_Estado_LEER_MEDICION_HUMEDAD(void)
{
  float MQ135_Rs, MQ135_Corrected_Rs;
  float PPM_Temp, PPM_Humedad;
  
  if(isnan(Evento_Temperatura.temperature))
      PPM_Temp = 20;
  else
      PPM_Temp = Evento_Temperatura.temperature;
  if(isnan(Evento_Humedad.relative_humidity))
      PPM_Humedad = 50;
  else
      PPM_Humedad = Evento_Humedad.relative_humidity;

  Serial.printf("Temperatura %3.1f ºC \n",Evento_Temperatura.temperature);
  Serial.printf("Humedad %4.1f %% \n",Evento_Humedad.relative_humidity);

//  Ecuaciones  Rs = (ADC_FULL_SCALE(255)/Gas -1) * Rl(1000)
//              Corrected_Rs = Rs/(1.6979 - 0.012 * Temp - 0.00612 * Humedad)
//              PPM = 121.4517 * (Corrected_Rs / Ro(28000))^-2.78054

  MQ135_Rs = ((float)ADC_FULL_SCALE/(float)Gas1 - 1)* MQ135_R_L;
  Serial.printf("Rs = %8.2f \n",MQ135_Rs);
  MQ135_Corrected_Rs = MQ135_Rs/(1.6979 - 0.012 * PPM_Temp - 0.00612 * PPM_Humedad);
  Serial.printf("Corrected_Rs = %8.2f \n",MQ135_Corrected_Rs);
  PPM_Gas1 = MQ135_Scaling_Factor * pow((float)MQ135_Corrected_Rs / MQ135_R_O, MQ135_Exponential_Factor);

#ifdef SENSOR_GAS2

  MQ135_Rs = ((float)ADC_FULL_SCALE/(float)Gas2 - 1)* MQ135_R_L;
  MQ135_Corrected_Rs = MQ135_Rs/(1.6979 - 0.012*Evento_Temperatura.temperature - 0.00612*Evento_Humedad.relative_humidity);
  PPM_Gas2 = MQ135_Scaling_Factor*pow((float)MQ135_Corrected_Rs/MQ135_R_O,MQ135_Exponential_Factor);

#endif

  Tick_Mediciones = TICKS_EVALUAR_PUBLICACION;         
  Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_EVALUAR_PUBLICACION;
  return Puntero_Proximo_Estado_Mediciones;

}
/*
//------------------------      6      ------------------------------
Retorno_funcion  Rutina_Estado_HABILITAR_LUZ(void)
{
  Sensor_De_Luz_TSL.enable();

  Tick_Mediciones = TICKS_ESPERA_PARA_MEDIR_LUZ;
  Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_MEDIR_LUZ;
  return Puntero_Proximo_Estado_Mediciones;

}

//------------------------      7        ------------------------------
Retorno_funcion  Rutina_Estado_MEDIR_LUZ(void)
{
  
  Luz_broadband = Sensor_De_Luz_TSL.read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW);
  Luz_ir = Sensor_De_Luz_TSL.read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW);

  Tick_Mediciones = TICKS_AJUSTE_LUZ;
  Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_VERIFICAR_AJUSTAR_MEDICION_LUZ;
  return Puntero_Proximo_Estado_Mediciones;
  
}

//------------------------      8      ------------------------------
Retorno_funcion  Rutina_Estado_VERIFICAR_AJUSTAR_MEDICION_LUZ(void)
{
  if(Sensor_De_Luz_TSL.verificar_medicion(Luz_broadband,Luz_ir) == true)
  {
    Tick_Mediciones = TICKS_GUARDADO_MEDICION_LUZ;
    Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_GUARDAR_MEDICION_LUZ;
    Sensor_De_Luz_TSL.disable();
  }
  else
  {
      Tick_Mediciones = TICKS_ESPERA_PARA_MEDIR_LUZ;
      Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_MEDIR_LUZ;
  }
      
  return Puntero_Proximo_Estado_Mediciones;

}

//-------------------------     9       -----------------------------
Retorno_funcion  Rutina_Estado_GUARDAR_MEDICION_LUZ(void)
{

  memset(&Evento_Luz, 0, sizeof(sensors_event_t));
  Evento_Luz.version   = sizeof(sensors_event_t);
  Evento_Luz.type      = SENSOR_TYPE_LIGHT;
  Evento_Luz.timestamp = millis();
  Evento_Luz.light = Sensor_De_Luz_TSL.calculateLux(Luz_broadband, Luz_ir);
  
  Serial.printf("%.1f Lumenes \n ",Evento_Luz.light);
  
  Tick_Mediciones = TICKS_EVALUAR_PUBLICACION;
  Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_EVALUAR_PUBLICACION;
  return Puntero_Proximo_Estado_Mediciones;

}

*/
//-------------------------     10       -----------------------------
Retorno_funcion  Rutina_Estado_EVALUAR_PUBLICACION(void)
{

      Alerta_Mediciones = false;
      Falla_Sensores = 0;

      for(Num_Sensor=0; Num_Sensor<ULTIMO_SENSOR; Num_Sensor++)
      {
          Data_Sensor[Num_Sensor].Lectura_Anterior = Data_Sensor[Num_Sensor].Lectura_Sensor;
//          Serial.printf("Sensor: %d, Lectura Anterior: %8.2f \n",Data_Sensor[Num_Sensor].Numero_Sensor, Data_Sensor[Num_Sensor].Lectura_Anterior);
//          Serial.printf("Sensor: %d, Time_Out_Sin_Publicaciones: %d \n",Data_Sensor[Num_Sensor].Numero_Sensor , Data_Sensor[Num_Sensor].Time_Out_Sin_Publicaciones);
          sprintf(Data_Sensor[Num_Sensor].Sensor_id,"%s-%d",Mac_Address,Num_Sensor+1);
//          sprintf(Data_Sensor[Num_Sensor].Status,"normal");
          
          switch(Num_Sensor)       // Guardo Lecturas y verifico fallas en sensores
          {
              case SENSOR_TEMPERATURA:     // Sensor numero 1-Temperatura
                    Data_Sensor[Num_Sensor].Lectura_Sensor = Evento_Temperatura.temperature;
                    if(isnan(Data_Sensor[Num_Sensor].Lectura_Sensor) || Data_Sensor[Num_Sensor].Lectura_Sensor == UMBRAL_FALLA_SENSOR_TEMPERATURA)
                    {    
                        Data_Sensor[Num_Sensor].Lectura_Sensor = 0;
                        Falla_Sensores += 1 << Num_Sensor;
                        Serial.printf("Falla sensor de Temp: %d\n",Falla_Sensores);
                        strcpy(Data_Sensor[Num_Sensor].Status,"outofservice");
                    }
                    break;
              case SENSOR_HUMEDAD:     // Sensor numero 2-Humedad
                    Data_Sensor[Num_Sensor].Lectura_Sensor = Evento_Humedad.relative_humidity;
                    if(isnan(Data_Sensor[Num_Sensor].Lectura_Sensor) || Data_Sensor[Num_Sensor].Lectura_Sensor == UMBRAL_FALLA_SENSOR_HUMEDAD)
                    {    
                        Data_Sensor[Num_Sensor].Lectura_Sensor = 0;
                        Falla_Sensores += 1 << Num_Sensor;
                        Serial.printf("Falla sensor de Humedad: %d\n",Falla_Sensores);
                        strcpy(Data_Sensor[Num_Sensor].Status,"outofservice");
                    }
                    break;
/*              case SENSOR_LUZ:     // Sensor numero 3-Iluminacion
                    Data_Sensor[Num_Sensor].Lectura_Sensor = Evento_Luz.light;
                    if(Data_Sensor[Num_Sensor].Lectura_Sensor > UMBRAL_FALLA_SENSOR_ILUMINACION)
                    {    
                        Falla_Sensores += 1 << Num_Sensor;
                        strcpy(Data_Sensor[Num_Sensor].Status,"outofservice");
                    }
                    break;
*/
              case SENSOR_SONIDO:     // Sensor numero 4-Sonido
                    Data_Sensor[Num_Sensor].Lectura_Sensor = Ruido_dB;
                    if(Data_Sensor[Num_Sensor].Lectura_Sensor < UMBRAL_FALLA_SENSOR_SONIDO)
                    {    
                        Falla_Sensores += 1 << Num_Sensor;
                        Serial.printf("Falla sensor de Sonido: %d\n",Falla_Sensores);
                        strcpy(Data_Sensor[Num_Sensor].Status,"outofservice");
                    }
                    break;
              case SENSOR_GAS1:     // Sensor numero 5-Gas1
                    Data_Sensor[Num_Sensor].Lectura_Sensor = PPM_Gas1;
                    if(Data_Sensor[Num_Sensor].Lectura_Sensor <= UMBRAL_INFERIOR_FALLA_SENSOR_GAS1 || Data_Sensor[Num_Sensor].Lectura_Sensor >= UMBRAL_SUPERIOR_FALLA_SENSOR_GAS1)
                    {    
                        Falla_Sensores += 1 << Num_Sensor;
                        Serial.printf("Falla sensor de Gas1: %d\n",Falla_Sensores);
                        strcpy(Data_Sensor[Num_Sensor].Status,"outofservice");
                    }
                    break;
/*              case SENSOR_GAS2:
                    Data_Sensor[Num_Sensor].Lectura_Sensor = PPM_Gas2;
                    if(Data_Sensor[Num_Sensor].Lectura_Sensor <= UMBRAL_INFERIOR_FALLA_SENSOR_GAS2 || Data_Sensor[Num_Sensor].Lectura_Sensor >= UMBRAL_SUPERIOR_FALLA_SENSOR_GAS2)
                    {    
                        Falla_Sensores += 1 << Num_Sensor;
                        strcpy(Data_Sensor[Num_Sensor].Status,"outofservice");
                    }
                    break;
*/
              default:
                    break;
          }
//          Serial.println("Led Verde");       
//          goto Enviar_Medicion;

          Data_Sensor[Num_Sensor].Variacion_Medicion = (Data_Sensor[Num_Sensor].Lectura_Anterior - Data_Sensor[Num_Sensor].Lectura_Sensor);
       
          if(Data_Sensor[Num_Sensor].Variacion_Medicion < 0)
              Data_Sensor[Num_Sensor].Variacion_Medicion *= -1;

//          Serial.printf("Variacion: %8.2f \n",Data_Sensor[Num_Sensor].Variacion_Medicion);
          Serial.printf("Evaluando Sensor: %d\n",Num_Sensor);
          
// Verifico si alguna de las siguientes condiciones se cumple para realizar una publicacion
          if(!strcmp(Data_Sensor[Num_Sensor].Status,"outofservice"))                     // 6 - Status igual a outofservice
          {    
              Serial.printf("El Sensor %d estaba Fallando. Status actual: %d\n",Num_Sensor,Falla_Sensores);
              if(! (Falla_Sensores & (1 << Num_Sensor)) )                                  // Si dejo de Fallar el sensor 
                 strcpy(Data_Sensor[Num_Sensor].Status,"onservice");                     // 7 - Status igual a onservice
              Enviar_Medicion();
          }
          else if(Fecha_Hora_Actual.Hora == 23 && Fecha_Hora_Actual.Minuto == 59)             // 8 - Hora 23:59
                  Enviar_Medicion();
          else if(Fecha_Hora_Actual.Hora == 00 && Fecha_Hora_Actual.Minuto == 00)             // 9 - Hora 00:00
                  Enviar_Medicion();
          else if(!Data_Sensor[Num_Sensor].Time_Out_Sin_Publicaciones)                        // 10 - Sin publicaciones por 10 minutos
                  Enviar_Medicion();
          else if(Data_Sensor[Num_Sensor].Variacion_Medicion > Data_Sensor[Num_Sensor].Delta)      // 1 - Variacion mayor al Delta
          {
                Alerta_Mediciones = true;
                Serial.println(F("Alerta de Medicion: Delta \n"));
                Enviar_Medicion();
          }
          else if(Data_Sensor[Num_Sensor].Lectura_Sensor > Data_Sensor[Num_Sensor].Highest)   // 2 - Medicion mayor a Highest
          {
                Alerta_Mediciones = true;
                Serial.println(F("Alerta de Medicion: Highest \n"));
                Enviar_Medicion();
          }
          else if(Data_Sensor[Num_Sensor].Lectura_Sensor > Data_Sensor[Num_Sensor].High)      // 3 - Medicion mayor a High
          {
                Alerta_Mediciones = true;
                Serial.println(F("Alerta de Medicion: High \n"));
                Enviar_Medicion();
          }
          else if(Data_Sensor[Num_Sensor].Lectura_Sensor < Data_Sensor[Num_Sensor].Low)       // 4 - Medicion menor a Low
          {
                Alerta_Mediciones = true;
                Serial.println(F("Alerta de Medicion: Low \n"));
                Enviar_Medicion();
          }
          else if(Data_Sensor[Num_Sensor].Lectura_Sensor < Data_Sensor[Num_Sensor].Lowest)    // 5 - Medicion menor a Lowest
          {
                Alerta_Mediciones = true;
                Serial.println(F("Alerta de Medicion: Lowest \n"));
                Enviar_Medicion();
          }
          else sprintf(Data_Sensor[Num_Sensor].JSON_Serializado,"\0");                        // 11 - Si no hay condiciones para publicar, anulo el JSON         
      }


      Tick_Mediciones = Time_Out_Sin_Mediciones;
      Puntero_Proximo_Estado_Mediciones=(Retorno_funcion)&Rutina_Estado_IDLE_MEDICIONES;
      return Puntero_Proximo_Estado_Mediciones;
}


void Enviar_Medicion(void)
{
          sprintf(Data_Sensor[Num_Sensor].Read_Time,"%s",Fecha_Hora_Actual.Char_Fecha_Hora_Actual);
//          Serial.printf("Sensor: %d, Fecha_Hora_Actual: %s \n",Data_Sensor[Num_Sensor].Numero_Sensor , Data_Sensor[Num_Sensor].Read_Time);
//          sprintf(Data_Sensor[Num_Sensor].JSON_Serializado,"{\n\"nroSensor\":%d,\n\"serial\":\"%s\",\n\"readTime\":\"%s\",\n\"metric\":\"%s\",\n\"value\":%8.2f,\n\"lowest\":%8.2f,\n\"low\":%8.2f,\n\"high\":%8.2f,\n\"highest\":%8.2f,\n\"delta\":%8.2f,\n\"status\":\"%s\"\n}",
          sprintf(Data_Sensor[Num_Sensor].JSON_Serializado,"{\"nroSensor\":%d,\"serial\":\"%s\",\"readTime\":\"%s\",\"metric\":\"%s\",\"value\":%8.2f,\"lowest\":%8.2f,\"low\":%8.2f,\"high\":%8.2f,\"highest\":%8.2f,\"delta\":%8.2f,\"status\":\"%s\"}",  \
                                      Data_Sensor[Num_Sensor].Numero_Sensor,  \
                                      Data_Sensor[Num_Sensor].Sensor_id,      \
                                      Data_Sensor[Num_Sensor].Read_Time,      \
                                      Data_Sensor[Num_Sensor].Unidad,         \
                                      Data_Sensor[Num_Sensor].Lectura_Sensor, \
                                      Data_Sensor[Num_Sensor].Lowest,         \
                                      Data_Sensor[Num_Sensor].Low,            \
                                      Data_Sensor[Num_Sensor].High,           \
                                      Data_Sensor[Num_Sensor].Highest,        \
                                      Data_Sensor[Num_Sensor].Delta,          \
                                      Data_Sensor[Num_Sensor].Status);
//          Serial.printf("JSON Serializado: %s\n",Data_Sensor[Num_Sensor].JSON_Serializado);
          if(Falla_Conexion)
          { 
              if(!Lista_Mediciones.Agregar_Dato_Lista(Data_Sensor[Num_Sensor].JSON_Serializado))
                  Serial.println(F("NO SE PUDO GUARDAR DATOS\n"));
              else
              {
                  Serial.println(F("Se guardo medicion en memoria Flash\n")); 
                  sprintf(Data_Sensor[Num_Sensor].JSON_Serializado,"\0");
              }

          }
          Data_Sensor[Num_Sensor].Time_Out_Sin_Publicaciones = TICKS_MAX_SIN_PUBLICACIONES;
          if(!strcmp(Data_Sensor[Num_Sensor].Status,"onservice"))                             // Pasa de estado onservice a normal
                strcpy(Data_Sensor[Num_Sensor].Status,"normal");
}
