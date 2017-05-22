/*============================================================================
 * Licencia: BSD 3 clausulas.
 * Autor: Eric Peria.
 * Fecha: 2017-04-05
 *===========================================================================*/

/*==================[inlcusiones]============================================*/

#include "programa.h"   // <= su propio archivo de cabecera
#include "sapi.h"       // <= Biblioteca sAPI

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

// Guarda la ultima tecla apretada
uint16_t key = 0;

/* Pines del teclado matricial */

// Pines conectados a las Filas --> Salidas (MODO = OUTPUT)
uint8_t keypadRowPins[4] = {
   RS232_TXD, // Row 0
   CAN_RD,    // R1
   CAN_TD,    // R2
   T_COL1     // R3
};

// Pines conectados a las Columnas --> Entradas con pull-up (MODO = INPUT_PULLUP)
uint8_t keypadColPins[4] = {
   T_FIL0,    // Column 0
   T_FIL3,    // C1
   T_FIL2,    // C2
   T_COL0     // C3
};


// Vector para mostrar tecla presionada por UART
uint16_t asciiKeypadKeys[16] = {
                                '1', '2', '3', 'A',
                                '4', '5', '6', 'B',
                                '7', '8', '9', 'C',
                                '*', '0', '#', 'D'
                               };

// Vector para mostrar tecla presionada en el display 7 segmentos
uint16_t keypadKeys[16] = {
                               1,    2,    3, 0x0a,
                               4,    5,    6, 0x0b,
                               7,    8,    9, 0x0c,
                            0x0e,    0, 0x0f, 0x0d
                          };
                          
uint8_t digit = 0;
uint16_t angle_raw = 0;
uint8_t data[4];
                          
/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

void configurarTecladoMatricial( void );
bool_t leerTecladoMatricial( void );                
bool_t valor = 0;
/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void ){

   // ---------- CONFIGURACIONES ------------------------------
   boardConfig(); // Inicializar y configurar la plataforma  
   uartConfig( UART_USB, 115200 ); // Configurar UART a 115200 
  
   configurarTecladoMatricial(); // Configurar teclado matricial

   valor = servoConfig( 0,      SERVO_ENABLE );

   valor = servoConfig( SERVO0, SERVO_ENABLE_OUTPUT );      
   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      if( leerTecladoMatricial() ){
         
         switch (asciiKeypadKeys[key]) {
            case 'A':
               itoa(angle_raw,data,10);
               if ((angle_raw>=0)&&(angle_raw<=180)){
                  uartWriteString(UART_USB,"El servo se mover� al �ngulo ");
                  uartWriteString(UART_USB,data);
                  uartWriteString(UART_USB,"\n\r");
                  servoWrite( SERVO0, angle_raw );
                  delay(500);
               }

               else {
                  uartWriteString(UART_USB,"El valor ingresao es inv�lido");
                  uartWriteString(UART_USB,"\n\r");                                 
               }
               
               digit = 0;
               angle_raw = 0;
               
            break;
         
            case 'B':
               digit = 0;
               angle_raw = 0;
               uartWriteString(UART_USB,"Cancel");
               uartWriteString(UART_USB,"\n\r");
         
            break;
         
            default:
               if (keypadKeys[key] <= 9) {
                  switch(digit){
                     case 0:
                        angle_raw = keypadKeys[key];
                     break;
                     
                     case 1:
                        angle_raw = angle_raw*10 + keypadKeys[key];
                     break;
                     
                     case 2:
                        angle_raw = angle_raw*10 + keypadKeys[key];
                     break;
                     
                     case 3:
                        angle_raw = angle_raw*10 + keypadKeys[key];
                     break;
                  }
                  digit++;
                  if (digit > 3)
                     digit = 0;
                  }
                  //uartWriteByte(UART_USB,asciiKeypadKeys[key]);
                  //uartWriteString(UART_USB,"\n\r");
            break;
         }
         delay(500);
      }
   } 

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta 
   // directamenteno sobre un microcontroladore y no es llamado/ por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

void configurarTecladoMatricial( void ){
   
   uint8_t i = 0;
   
   // Configure Rows as Outputs
   for( i=0; i<4; i++ ){
      gpioConfig( keypadRowPins[i], GPIO_OUTPUT );
   }

   // Configure Columns as Inputs with pull-up resistors enable
   for( i=0; i<4; i++ ){
      gpioConfig( keypadColPins[i], GPIO_INPUT_PULLUP );
   }
}


/* Devuelve TRUE si hay alguna tecla presionada o FALSE (0) en caso contrario.
 * Si hay tecla presionada guarda el valor en la variable key.
 * El valor es un numero de indice entre 0 y 15 */
bool_t leerTecladoMatricial( void ){

   bool_t retVal = FALSE;

   uint16_t r = 0; // Rows
   uint16_t c = 0; // Columns

   // Poner todas las filas en estado BAJO
   for( r=0; r<4; r++ ){
	  gpioWrite( keypadRowPins[r], LOW );
   }

   // Chequear todas las columnas buscando si hay alguna tecla presionada
   for( c=0; c<4; c++ ){

      // Si leo un estado BAJO en una columna entonces puede haber una tecla presionada
      if( !gpioRead( keypadColPins[c] ) ){

         delay( 50 ); // Anti-rebotes de 50 ms

         // Poner todas las filas en estado ALTO excepto la primera
         for( r=1; r<4; r++ ){
            gpioWrite( keypadRowPins[r], HIGH );
         }

         // Buscar que tecla esta presionada
         for( r=0; r<4; r++ ){

            // Poner la Fila[r-1] en estado ALTO y la Fila[r] en estado BAJO
            if( r>0 ){ // Exceptua el indice negativo en el array
               gpioWrite( keypadRowPins[r-1], HIGH );
            }
            gpioWrite( keypadRowPins[r], LOW );

            // Chequear la Columna[c] en Fila[r] para buscar si la tecla esta presionada
            // Si dicha tecla esta oresionada (en estado BAJO) entonces retorna
            // graba la tecla en key y retorna TRUE
            if( !gpioRead( keypadColPins[c] ) ){
               retVal = TRUE;
               key = r * 4 + c;
               /*
                  Formula de las teclas de Teclado Matricial (Keypad)
                  de 4 filas (rows) * 5 columnas (columns)

                     c0 c1 c2 c3 c4
                  r0  0  1  2  3  4
                  r1  5  6  7  8  9   Si se presiona la tecla r[i] c[j]:
                  r2 10 11 12 13 14   valor = (i) * cantidadDeColumnas + (j)
                  r3 15 16 17 18 19
               */
               return retVal;
            }
         }

      }
   }
   return retVal;
}

/*==================[fin del archivo]========================================*/