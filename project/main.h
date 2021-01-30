/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.h 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    13-November-2013
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
//#include "stm32f4xx_it.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void TimingDelay_Decrement(void);
extern void My_GPIO_Init(void);
extern void My_TIMER_Init(void);
extern void My_PWM_Init(void);
extern void EXTILine_Config(void);
extern void USART_DMA_Configuration(unsigned int BaudRate);
extern void delay_ms(uint32_t milliSeconds);
extern void delay_us(uint32_t microSeconds);
extern void MyDelay(__IO uint32_t number);
extern void SendUSART(char* data); 
extern char* GetUSART(void);
extern int GetInt(void);
extern void Encoder_Init(void);

#define MAIN_BAUDRATE 115200
#define MAIN_STOPBITS USART_StopBits_2
#define MAIN_PARITY USART_Parity_No //USART_Parity_Even
#define BUFF_SIZE_RX 13
#define BUFF_SIZE_TX 13
//#define COUNTER_MAX 390*4*25
#define COUNTER_MIN	1000
//#define COUNTER_ROUND 390*4

extern int16_t COUNTER_MAX, COUNTER_ROUND;

extern char RXBuffer[BUFF_SIZE_RX];
extern char TXBuffer[BUFF_SIZE_TX];
void checkSpeed(void);
void sendAngle(void);
void readData(void);
void PID_control(float setpoint_RPM, float measure_RPM);
void IntToString(int32_t u, char *y);

struct motor_Values{
	int16_t counter;
	int16_t counter_per_round;
	float angle;
	float actual_angle;
	float calib_angle_zero;
	float calib_angle_span;
} extern mainMotor;
union ByteToFloat{
	float myfloat;
	char mybyte[4];
} extern m_data;
struct calibPararameters{
	float zero;
	float span;
} extern calibPara;

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
