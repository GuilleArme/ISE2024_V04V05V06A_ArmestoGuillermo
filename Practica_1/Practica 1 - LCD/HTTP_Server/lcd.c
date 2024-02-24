#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "lcd.h"
#include "Arial12x12.h"
#include "stdio.h"
#include "string.h"
#include "HTTP_Server_CGI.h"

extern osStatus_t status_lcd;
static GPIO_InitTypeDef GPIO_InitStruct;



  extern ARM_DRIVER_SPI Driver_SPI1;
  ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
  
  ARM_SPI_STATUS stat;
  
  unsigned char buffer[512];
  
  TIM_HandleTypeDef htim7;
  
  uint16_t positionL1;
  uint16_t positionL2;
  
  uint32_t valor1 = 1234; 
  char linea1[19];
  
  int i;

  void delay(uint32_t n_microsegundos){ 

    __HAL_RCC_TIM7_CLK_ENABLE();	


    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 83; //El APB1 es 84 y queremos una cadencia de 1uz
    htim7.Init.Period = (n_microsegundos-1);
    //USAR EL PARAMETRO DE ENTRADA DE LA FUNCIÓN

    HAL_TIM_Base_Init(&htim7);
    HAL_TIM_Base_Start_IT(&htim7);
    
    while((TIM7->CNT) < (n_microsegundos-1)){}
      
    HAL_TIM_Base_Stop(&htim7);
    HAL_TIM_Base_DeInit(&htim7);

  }



  //Inicializas los pines (Reset = PA6; AO = PF13 ; CS = PD14): 

  

  void initPines_GPIOs (void){ 
    
    /*Enable clock to GPIO-D,GPIO_A,GPIO-F*/
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    
    /*Set GPIOA pin RESET*/

    GPIO_InitStruct.Pin = GPIO_PIN_6 ;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*Set GPIOF pin A0*/
    GPIO_InitStruct.Pin = GPIO_PIN_13 ;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  

    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct); 
    
    /*Set GPIOD pin CS*/
    
    GPIO_InitStruct.Pin = GPIO_PIN_14 ;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  

    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);  
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); //RESET
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET); //A0
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); //CS = Activo a nivel bajo
    
  }
  
  
  void LCD_reset(void){
    
    //Primero inicializas el SPI
    SPIdrv->Initialize(NULL);
    SPIdrv->PowerControl(ARM_POWER_FULL);
    SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 20000000);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    delay(1); 
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

    delay(1001);//Retardo de 1ms después del reset(lo pedian en el enunciado) -> 1000us = 1ms -> El 1 es el TRW, nos dicen que mínimo tiene que ser de 1us
  
}
  



  //FUNCION QUE ESCRIBE UN DATO EN EL LCD
  void LCD_wr_data(unsigned char data){
  // Seleccionar CS = 0;
  // Seleccionar A0 = 1;
    
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
    
    SPIdrv->Send(&data, sizeof(data));
    do{
      stat = SPIdrv->GetStatus();
    }
    while (stat.busy);
    
  // Seleccionar CS = 1;
    
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  
  }
  
  
  //FUNCION QUE ESCRIBE UN COMANDO EN EL LCD
  void LCD_wr_cmd(unsigned char cmd){
  // Seleccionar CS = 0;
  // Seleccionar A0 = 0;
    
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
		
    SPIdrv->Send(&cmd, sizeof(cmd));
    do{
      stat = SPIdrv->GetStatus();
    }
    while (stat.busy);//Espera hasta que se libere el bus SPI

  // Seleccionar CS = 1;
    
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); //CS = 1
    
  }

  //AJUSTAR LA PANTALLA
  void LCD_init (void){
    
    LCD_wr_cmd(0xAE); //Display off
    LCD_wr_cmd(0xA2); //Fija el valor de la relación de la tensión de polarización del LCD a 1/9
    LCD_wr_cmd(0xA0); //El direccionamiento de la RAM de datos del display es la normal
    LCD_wr_cmd(0xC8); //El scan en las salidas COM es el normal
    LCD_wr_cmd(0x22); //Fija la relación de resistencias interna a 2
    LCD_wr_cmd(0x2F); //Power on
    LCD_wr_cmd(0x40); //Display empieza en la línea 0
    LCD_wr_cmd(0xAF); //Display ON
    LCD_wr_cmd(0x81); //Contraste
    LCD_wr_cmd(0x17); //Valor Contraste
    LCD_wr_cmd(0xA4); //Display all points normal
    LCD_wr_cmd(0xA6); //LCD Display normal
  
  }
  
 
  void LCD_update(void){
    
    int i;
    LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
    LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
    LCD_wr_cmd(0xB0); // Página 0

    for(i=0;i<128;i++){
    LCD_wr_data(buffer[i]);
    }

    LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
    LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
    LCD_wr_cmd(0xB1); // Página 1

    for(i=128;i<256;i++){
      LCD_wr_data(buffer[i]);
    }

    LCD_wr_cmd(0x00);
    LCD_wr_cmd(0x10);
    LCD_wr_cmd(0xB2); //Página 2
    for(i=256;i<384;i++){
      LCD_wr_data(buffer[i]);
    }

    LCD_wr_cmd(0x00);
    LCD_wr_cmd(0x10);
    LCD_wr_cmd(0xB3); // Pagina 3


    for(i=384;i<512;i++){
      LCD_wr_data(buffer[i]);
    }
    
  }
  
  void LCD_symbolToLocalBuffer_L1(uint8_t symbol){ // 1º y 2º página
  
    uint8_t i, value1, value2;
    uint16_t offset=0;
  
    offset=25*(symbol - ' ');
  
    for(i=0; i<12; i++){
  
      value1=Arial12x12[offset+i*2+1];
      value2=Arial12x12[offset+i*2+2];

      buffer[i+positionL1]=value1;
      buffer[i+128+positionL1]=value2;
      
    }
    
    positionL1=positionL1+Arial12x12[offset];
    
  }
  
  
  void LCD_symbolToLocalBuffer_L2(uint8_t symbol){ //3º y 4º página

    uint8_t i, value1, value2;
    uint16_t offset=0;
  
    offset=25*(symbol - ' ');
  
    for(i=0; i<12; i++){
  
      value1=Arial12x12[offset+i*2+1];
      value2=Arial12x12[offset+i*2+2];
  
      buffer[i+256+positionL2]=value1; 
      buffer[i+384+positionL2]=value2;
      
    }
    
   positionL2=positionL2+Arial12x12[offset];
    
  }
  
  void LCD_symbolToLocalBuffer(uint8_t line, uint8_t symbol){
    if(line==1){
      LCD_symbolToLocalBuffer_L1(symbol);
    }
    if(line==2){
      LCD_symbolToLocalBuffer_L2(symbol);
    }
  }
  
 
  void L1(void){
    sprintf(linea1, "Prueba valor1: %d", valor1);//si fuese un float en vez de uint32_t sería %.5f2
    for(i=0; i<19; i++){
      LCD_symbolToLocalBuffer(1, linea1[i]);
    }
  }
  
  void LCD_Clean(void){
    memset(buffer, 0 , 512u); //implica añadir la libreria: #include "string.h"
    LCD_update();
  }
 
	
	/*----------------------------------------------------------------------------
  *                       ESCRITURA DE UNA PAGINA ENTERA          
	*---------------------------------------------------------------------------*/
	
	void pagina3(){//Pintar toda la página3
	//Cada página ocupa 128, luego 1º) 0-127; 2º) 128-256; 3º) 257-386; 4º) 387-514
		int i; //No puedes crearlo dentro del for
		for(i = 256; i< 387; i++){
				buffer[i] = 0xff;
		}
		LCD_update();
	}
	

	
/*----------------------------------------------------------------------------
  *                       ESCRITURA DE UN CUADRADO          
*---------------------------------------------------------------------------*/
	
	void cuadrado(){
	  
	  buffer[0]=0xFF;
	  buffer[1]=0x81;
	  buffer[2]=0x81;
	  buffer[3]=0x81;
	  buffer[4]=0x81;
	  buffer[5]=0x81;
	  buffer[6]=0x81;
	  buffer[7]=0xFF;
		
		LCD_update();
	}
	
 
/*----------------------------------------------------------------------------
  *                       ESCRITURA DEL TITULO EN EL LCD            
*---------------------------------------------------------------------------*/

  void texto_SBM(void){
    positionL1 = 2;//Para que se escriba un poco mas centrada

    LCD_symbolToLocalBuffer(1,'S');
    LCD_symbolToLocalBuffer(1,'B');
    LCD_symbolToLocalBuffer(1,'M');
    LCD_symbolToLocalBuffer(1,' ');
    LCD_symbolToLocalBuffer(1,'2');
    LCD_symbolToLocalBuffer(1,'0');
    LCD_symbolToLocalBuffer(1,'2');
    LCD_symbolToLocalBuffer(1,'2');

    LCD_update();
  }
/*----------------------------------------------------------------------------
  * COLA DEL LCD -> Se crea y se rellena en el módulo principal (en este caso, en el test)            
*---------------------------------------------------------------------------*/

	MSGQUEUE_OBJ_t_lcd msg_lcd_recibido;
  
	extern osMessageQueueId_t mid_MsgQueue_lcd; //Identificador de la cola del lcd, creada en el test
/*------------------------------------------------------------------------*/


///*----------------------------------------------------------------------------
//  *                         Thread : HILO DEL LCD                    
//*---------------------------------------------------------------------------*/
 
	osThreadId_t tid_Thlcd;                        // thread id
	 
	void Thlcd (void *argument);                   // thread function
	 
	int Init_Thlcd (void) {
	 
		tid_Thlcd = osThreadNew(Thlcd, NULL, NULL);
		if (tid_Thlcd == NULL) {
			return(-1);
		}

		LCD_Clean();

    
		return(0);
	}
 


	void Thlcd (void *argument) {
		initPines_GPIOs();

		
		while (1) {
      osThreadFlagsWait(0x10, osFlagsWaitAny, osWaitForever);
      osMessageQueueGet(mid_MsgQueue_lcd, &msg_lcd_recibido, NULL, osWaitForever);
      LCD_reset();
      LCD_init();
      LCD_Clean();
      escribir_lineasWeb();
    }
  }
  
 /*----------------------------------------------------------------------------
*                 REPRESENTAR cuenta atras                 
*---------------------------------------------------------------------------*/
    void escribir_lineasWeb(void){
      positionL2=0;
      positionL1=0;
	    for (int i=0; i<strlen(msg_lcd_recibido.texto1); i++){
        LCD_symbolToLocalBuffer(1,msg_lcd_recibido.texto1[i]);
      }
	    for (int i=0; i<strlen(msg_lcd_recibido.texto2); i++){
        LCD_symbolToLocalBuffer(2,msg_lcd_recibido.texto2[i]);
      }
      LCD_update();
  }
///*----------------------------------------------------------------------------
//  *                   ESCRITURA DEL BM EN EL LCD                  
//*---------------------------------------------------------------------------*/	
//  
  void escribir_texto(void){
    int j;
    char linea1[4]; 

    positionL1 = 60;

    sprintf(linea1, "BM"); 
    for(j = 0; j < 4; j++){
      LCD_symbolToLocalBuffer(1, linea1[j]);
    }
    LCD_update();
  }

