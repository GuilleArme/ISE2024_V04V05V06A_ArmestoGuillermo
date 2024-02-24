#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"
#include "Driver_I2C.h"
#include <math.h>

 /*----------------------------------------------------------------------------
 *      estructura cola lcd
 *---------------------------------------------------------------------------*/

typedef struct{
	char texto1[30];
  char texto2[30];
  //uint8_t linea;
}MSGQUEUE_OBJ_t_lcd;



int Init_MsgQueue_lcd (void);