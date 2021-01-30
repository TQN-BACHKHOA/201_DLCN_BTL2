#include "main.h"
USART_InitTypeDef				  USART_InitStruct;
GPIO_InitTypeDef 				  GPIO_InitStruct;
NVIC_InitTypeDef					NVIC_InitStruct;
EXTI_InitTypeDef					EXTI_InitStruct;
TIM_TimeBaseInitTypeDef 	TIM_BaseStruct;
TIM_ICInitTypeDef 				TIM_ICInitStruct;
TIM_OCInitTypeDef 				TIM_OCStruct;
DMA_InitTypeDef 					DMA_InitStructure;

char RXBuffer[BUFF_SIZE_RX];
char TXBuffer[BUFF_SIZE_TX] = {'$','A','N','G','L',',','0','0','0','0','0','\r','\n'};
union ByteToFloat m_data;
struct motor_Values mainMotor = {0, 150, 0, 0, 0, 0};
struct calibPararameters calibPara = {0, 1};
int16_t COUNTER_MAX = 0;
int16_t COUNTER_ROUND = 0;

int main(void){
	SystemInit();
	USART_DMA_Configuration(MAIN_BAUDRATE);
	My_GPIO_Init();
	My_TIMER_Init();
	My_PWM_Init();
	Encoder_Init();
//	EXTILine_Config();
	while(1){
		COUNTER_MAX = mainMotor.counter_per_round*4*25;
		COUNTER_ROUND = mainMotor.counter_per_round*4;
		sendAngle();
		delay_ms(20);
	}
}

void sendAngle(){
	char checksum_Tx = 0;
	m_data.myfloat = mainMotor.angle;
	for (int i = 0; i < 4; i++){
		TXBuffer[6+i] = m_data.mybyte[3-i];
		checksum_Tx += m_data.mybyte[3-i];
	}
	TXBuffer[10] = checksum_Tx;
	DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
  DMA1_Stream3->NDTR = BUFF_SIZE_TX;
  DMA_Cmd(DMA1_Stream3, ENABLE);
}

void IntToString(int32_t u, char *y){
	int32_t a;
	if(u >= 0){
		a = u;
		y[3] = a%10 + 0x30; a = a/10;
		y[2] = a%10 + 0x30; a = a/10;
		y[1] = a + 0x30;
		y[0] = 0x2B;
	} else{
		a = -u;
		y[3] = a%10 + 0x30; a = a/10;
		y[2] = a%10 + 0x30; a = a/10;
		y[1] = a + 0x30;
		y[0] = 0x2D;
	}
}

void USART_DMA_Configuration(unsigned int BaudRate){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3); 
	
	/* Khoi tao chân TX & RX Uart*/
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	USART_InitStruct.USART_BaudRate = BaudRate;

	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = MAIN_STOPBITS;
	USART_InitStruct.USART_Parity = MAIN_PARITY;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStruct);

	USART_Cmd(USART3, ENABLE);
	/* Enable USART3 DMA */
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	/* Configure DMA Initialization Structure */
	
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull ;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(USART3->DR)) ;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;

	/* Configure TX DMA */
	DMA_InitStructure.DMA_BufferSize = BUFF_SIZE_TX;
	DMA_InitStructure.DMA_Channel = DMA_Channel_4 ;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)TXBuffer ;
	DMA_Init(DMA1_Stream3,&DMA_InitStructure);
	DMA_Cmd(DMA1_Stream3, ENABLE);
	
	/* Configure RX DMA */
	DMA_InitStructure.DMA_BufferSize = BUFF_SIZE_RX;
	DMA_InitStructure.DMA_Channel = DMA_Channel_4 ;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)&RXBuffer;
	DMA_Init(DMA1_Stream1,&DMA_InitStructure);	
	DMA_Cmd(DMA1_Stream1, ENABLE);
	
	/* Enable DMA Interrupt to the highest priority */
	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	/* Transfer complete interrupt mask */
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC, ENABLE);
}

void delay_ms(uint32_t milliSeconds){
	while (milliSeconds--){
		TIM_SetCounter(TIM6, 0);
		TIM_Cmd(TIM6, ENABLE);
		while (TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) != SET);
		TIM_Cmd(TIM6, DISABLE);
		TIM_ClearFlag(TIM6, TIM_FLAG_Update);
	}
}

void delay_us(uint32_t microSeconds){
	TIM_SetCounter(TIM6, 0);
	TIM_Cmd(TIM6, ENABLE);
	while (TIM_GetCounter(TIM6) < microSeconds);
	TIM_Cmd(TIM6, DISABLE);
}

void MyDelay(__IO uint32_t number){
	while(number--){}
}


void Encoder_Init(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	
  GPIO_InitStruct.GPIO_Pin 		= GPIO_Pin_0| GPIO_Pin_1; 
	GPIO_InitStruct.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_PuPd 	= GPIO_PuPd_NOPULL; 		
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM5);
	
	TIM_BaseStruct.TIM_Prescaler 			= 0;
  TIM_BaseStruct.TIM_Period 				= 0xFFFF;//8 chu F thi tot hon	
  TIM_BaseStruct.TIM_ClockDivision 	= TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM5,&TIM_BaseStruct);
	
	TIM_ICInitStruct.TIM_Channel 					= TIM_Channel_1 | TIM_Channel_2;
  TIM_ICInitStruct.TIM_ICFilter 				= 10; 
  TIM_ICInitStruct.TIM_ICPolarity 			= TIM_ICPolarity_Rising;
  TIM_ICInitStruct.TIM_ICPrescaler 			= TIM_ICPSC_DIV4;
  TIM_ICInitStruct.TIM_ICSelection 			= TIM_ICSelection_DirectTI;
  TIM_ICInit(TIM5,&TIM_ICInitStruct);
 
  TIM_EncoderInterfaceConfig(TIM5,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);
  TIM_SetCounter(TIM5, 0);
  TIM_Cmd(TIM5,ENABLE);
  TIM_ClearFlag(TIM5,TIM_FLAG_Update);
}

void My_GPIO_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void My_TIMER_Init(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	TIM_BaseStruct.TIM_Prescaler 						= 83;
	TIM_BaseStruct.TIM_Period 							= 999;
	TIM_BaseStruct.TIM_CounterMode 					= TIM_CounterMode_Up;
	TIM_BaseStruct.TIM_ClockDivision 				= 0;
	TIM_BaseStruct.TIM_RepetitionCounter 		= 0;
	TIM_TimeBaseInit(TIM1, &TIM_BaseStruct);
	TIM_Cmd(TIM1, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	TIM_BaseStruct.TIM_Prescaler 						= 83;
	TIM_BaseStruct.TIM_Period 							= 999; //delay 1ms
	TIM_BaseStruct.TIM_CounterMode 					= TIM_CounterMode_Up;
	TIM_BaseStruct.TIM_ClockDivision 				= 0;
	TIM_BaseStruct.TIM_RepetitionCounter 		= 0;
	TIM_TimeBaseInit(TIM6, &TIM_BaseStruct);
	TIM_UpdateDisableConfig(TIM6, DISABLE);
	TIM_ARRPreloadConfig(TIM6, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	TIM_BaseStruct.TIM_Prescaler 						= 8399;
	TIM_BaseStruct.TIM_Period 							= 99999; //10kHz 10s
	TIM_BaseStruct.TIM_CounterMode 					= TIM_CounterMode_Up;
	TIM_BaseStruct.TIM_ClockDivision 				= 0;
	TIM_BaseStruct.TIM_RepetitionCounter 		= 0;
	TIM_TimeBaseInit(TIM7, &TIM_BaseStruct);
	TIM_UpdateDisableConfig(TIM7, DISABLE);
	TIM_ARRPreloadConfig(TIM7, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_BaseStruct.TIM_Prescaler 						= 8399;
	TIM_BaseStruct.TIM_Period 							= 999; //delay 100ms
	TIM_BaseStruct.TIM_CounterMode 					= TIM_CounterMode_Up;
	TIM_BaseStruct.TIM_ClockDivision 				= 0;
	TIM_BaseStruct.TIM_RepetitionCounter 		= 0;
	TIM_TimeBaseInit(TIM3, &TIM_BaseStruct);
	TIM_UpdateDisableConfig(TIM3, DISABLE);
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	
	NVIC_InitStruct.NVIC_IRQChannel 									= TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority 				= 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd 								= ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void My_PWM_Init(void){
	TIM_OCStruct.TIM_OCMode 					= TIM_OCMode_PWM1; //clear on compare match
	TIM_OCStruct.TIM_OutputState 			= TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OutputNState 		= TIM_OutputNState_Enable;
	TIM_OCStruct.TIM_OCPolarity 			= TIM_OCPolarity_High;
	TIM_OCStruct.TIM_OCNPolarity 			= TIM_OCNPolarity_High;
	
	TIM_OCStruct.TIM_Pulse = 50*1000/100 - 1;
	TIM_OC1Init(TIM1, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	TIM_OC2Init(TIM1, &TIM_OCStruct);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM1,ENABLE);
	TIM_Cmd(TIM1,ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE); 
}

void SendUSART(char* data){ //dia chi bat dau cua chuoi
	for (int i = 0; *(data+i)!= NULL; i++){
		USART_SendData(USART3, *(data+i));
		*(data+i) = NULL;
		while (USART_GetFlagStatus(USART3, USART_SR_TXE) == RESET);
	}
}

char* GetUSART(void){
	uint16_t i = 0;
	memset(RXBuffer, NULL, sizeof(RXBuffer));
	for(i = 0; i < BUFF_SIZE_RX+1; i++){
		while (USART_GetFlagStatus(USART3, USART_SR_RXNE) == RESET);
		RXBuffer[i] = USART_ReceiveData(USART3);
		if (RXBuffer[i] == 10 || RXBuffer[i] == 13) 
			break;
	}
	return RXBuffer;
}

int GetInt(void){
	char* dataReceive = GetUSART();
	int intData;
	sscanf(dataReceive, "%d", &intData);
	return intData;
}







