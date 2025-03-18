#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

char getC, string_nhan[100];
int8_t Flag=0;											
uint16_t count1=0, count2=0, vitri; 

void USART1_DEBUG_Init(unsigned int baudrate)
{							
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef uart;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	gpio.GPIO_Pin=GPIO_Pin_9;
	gpio.GPIO_Mode=GPIO_Mode_AF_PP;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
	
	gpio.GPIO_Pin=GPIO_Pin_10;
	gpio.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
	
	uart.USART_BaudRate=baudrate;
	uart.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	uart.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	uart.USART_Parity=USART_Parity_No;
	uart.USART_StopBits=USART_StopBits_1;
	uart.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART1, &uart);
	USART_ITConfig(USART1,USART_IT_RXNE, ENABLE);
	NVIC_EnableIRQ(USART1_IRQn);
	USART_Cmd(USART1,ENABLE);
}	

void USART1_IRQHandler()
{													
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET){
		getC = USART_ReceiveData(USART1);
		if(getC == '\n'){
			string_nhan[count1]='\0';
			Flag = 1;
		}
		else{
			string_nhan[count1]=getC;
			count1++;
		}
	}
}

void USART1_DEBUG_SendChar(char varChar)
{				
	USART_SendData(USART1, varChar);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}		

void USART1_DEBUG_SendString(char *varString)
{				
	while(*varString != '\0'){
		USART1_DEBUG_SendChar(*varString++);
	}
}


void USART1_DEBUG_Int_Number(int32_t varNumber)
{						
	uint8_t count_element=0;
	int32_t number=varNumber;
	uint8_t string_number[10];
	if(varNumber<0){
		number=-number;
	}
	while(number){
		string_number[count_element]=(uint8_t)(number%10)+48;
		count_element++;
		number/=10;
	}
	if(varNumber<0){
		string_number[count_element]='-';
		count_element++;
	}
		string_number[count_element]='\0';
	while(count_element--){
		USART1_DEBUG_SendChar(string_number[count_element]);
	}
}	

void USART1_DEBUG_Float_Number(float varNumber)
{
	uint8_t count_element=0;

	int32_t number=varNumber*10000;
	uint8_t string_number[10];
	if(varNumber<0){
		number=-number;
	}
	if(number%10>=5){
			number=(number/10)+1;
	}else{
			number=number/10;
	}
	while(number){
		string_number[count_element]=(uint8_t)(number%10)+48;
		if(count_element==3){
			string_number[count_element]='.';
			count_element++;
			string_number[count_element]=(uint8_t)(number%10)+48;
		}
		count_element++;
		number/=10;
	}
	if(varNumber<0){
		string_number[count_element]='-';
		count_element++;
	}
	string_number[count_element]='\0';
	while(count_element--){
		USART1_DEBUG_SendChar(string_number[count_element]);
	}
}

int8_t USART1_DEBUG_Flag(void)
{											
	if(Flag == 1)
	{
		Flag=0;
		return 1;
	}
	return 0;
}

int8_t USART1_DEBUG_Compare(char *string){							
	while(string[count2] != '\0'){
		count2++;
	}
	if(count2 == count1-1){ 	
		for(vitri = 0; vitri < count2; vitri++){
			if(string_nhan[vitri] != string[vitri]){
				return 0;
			}
		}
	}
	else{
		return 0;
	}
	return 1;
}

int8_t USART1_DEBUG_Strstr(char *string){								
	uint16_t i,j, temp;
	while(string[count2] != '\0'){
		count2++;
	}
	for(i = 0; i < count1; i++){
		temp=i;
		for(j = 0; j < count2; j++){
			if(string_nhan[temp] != string[j]){
				break;
			}
			temp++;
		}
		if(j == count2){
			return 1;
		}
	}
	return 0;
}

void USART1_Reset_Count(void){									
	count1 = 0;
	count2 = 0;
}

void USART1_DEBUG_ShowString(void)
{							
	USART1_DEBUG_SendString(string_nhan);
}
