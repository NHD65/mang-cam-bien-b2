#ifndef __USART1__
#define __USART1__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"


void USART1_DEBUG_Init(unsigned int baudrates);			
void USART1_DEBUG_SendString(char *varString);			
void USART1_DEBUG_SendChar(char varChar);					
void USART1_DEBUG_Int_Number(int32_t varNumber);						
void USART1_DEBUG_Float_Number(float varNumber);
void USART1_DEBUG_ShowString(void);									

int8_t USART1_DEBUG_Compare(char *string);					
int8_t USART1_DEBUG_Strstr(char *string);						
int8_t USART1_DEBUG_Flag(void);											

void USART1_Reset_Count(void);											

#ifdef __cplusplus
}
#endif

#endif


