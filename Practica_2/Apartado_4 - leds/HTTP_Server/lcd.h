#ifndef __LCD_H
#define __LCD_H


#include "main.h"
#include "Driver_SPI.h"
#include "RTE_Device.h"
#include "stdio.h"


void LCD_reset(void);
void delay(uint32_t n_microsegundos);
void initPines_GPIOs (void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void LCD_init (void);
void LCD_update(void);
void LCD_symbolToLocalBuffer_L1(uint8_t symbol);
void LCD_symbolToLocalBuffer_L2(uint8_t symbol);
void LCD_symbolToLocalBuffer(uint8_t line, uint8_t symbol);
void L1(void);
void LCD_Clean(void);

void texto_SBM(void);
void escribir_hora(void);
void Thlcd (void *argument);
int Init_Thlcd (void);
//int Init_MsgQueue (void); 

void escribir_temperatura(void);
void pagina3(void);
void cuadrado(void);



void texto_estado_radio_manual(void);
void texto_estado_radio_memoria(void);
void texto_estado_cambio_hora(void);
void escribir_volumen(void);
void escribir_hora_2(void);
void escribir_texto(void);
void escribir_frecuencia(void);
void escribir_posicionMemoria_Frecuencia(void);



void horas_seleccionadas(void);
void minutos_seleccionados(void);
void segundos_seleccionados(void);
void bm_seleccionados(void);

void modificar_minutos(void);
void modificar_segundos(void);
void modificar_horas(void);
void modificar_BM(void);

void escribir_lineasWeb(void);





#endif /* __LCD_H */
