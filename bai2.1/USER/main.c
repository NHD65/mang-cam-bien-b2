#include "stm32f10x.h"                  
#include "stm32f10x_gpio.h"             
#include "stm32f10x_rcc.h"              
#include "stm32f10x_usart.h"            
#include "stm32f10x_tim.h"              
#include "usart1.h"                    

void GPIO_Config(void);
void Timer2_Init(void);
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
void DHT11_Start(void);
uint8_t DHT11_Read_Byte(void);
uint8_t DHT11_Read_Data(uint8_t *humidity_int, uint8_t *humidity_dec, uint8_t *temp_int, uint8_t *temp_dec);

int main(void)
{
    uint8_t hum_int, hum_dec, temp_int, temp_dec, checksum;
    USART1_DEBUG_Init(9600);
    Timer2_Init();
    GPIO_Config();
    while(1)
    {
        DHT11_Start();
        checksum = DHT11_Read_Data(&hum_int, &hum_dec, &temp_int, &temp_dec);
        if ((hum_int + hum_dec + temp_int + temp_dec) == checksum)
        {
            USART1_DEBUG_SendString("Do am: ");
            USART1_DEBUG_Int_Number(hum_int);
            USART1_DEBUG_SendChar('.');
            if(hum_dec == 0)
                USART1_DEBUG_SendString("0");
            else
                USART1_DEBUG_Int_Number(hum_dec);
            
            USART1_DEBUG_SendString(", Nhiet do: ");
            USART1_DEBUG_Int_Number(temp_int);
            USART1_DEBUG_SendChar('.');
            if(temp_dec == 0)
                USART1_DEBUG_SendString("0");
            else
                USART1_DEBUG_Int_Number(temp_dec);
            USART1_DEBUG_SendString("\r\n");
        }
        else
        {
            USART1_DEBUG_SendString("Loi checksum!\r\n");
        }
        delay_ms(1000);
    }
}

void Timer2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    TIM_TimeBaseInitTypeDef timerInit;
    timerInit.TIM_CounterMode = TIM_CounterMode_Up;
    timerInit.TIM_Period = 0xFFFF;
    timerInit.TIM_Prescaler = 72 - 1;  
    TIM_TimeBaseInit(TIM2, &timerInit);
    TIM_Cmd(TIM2, ENABLE);
}

void delay_us(uint16_t us)
{
    TIM_SetCounter(TIM2, 0);
    while(TIM_GetCounter(TIM2) < us);
}

void delay_ms(uint16_t ms)
{
    while(ms--)
        delay_us(1000);
}

void GPIO_Config(void)
{
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Pin = GPIO_Pin_12;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);
}

void DHT11_Start(void)
{
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Pin = GPIO_Pin_12;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    delay_ms(20);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
    delay_us(30);
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio);
}

uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, byte = 0;
    for(i = 0; i < 8; i++)
    {
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0);
        TIM_SetCounter(TIM2, 0);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1);
        if(TIM_GetCounter(TIM2) > 45)
            byte = (byte << 1) | 1;
        else
            byte = (byte << 1);
    }
    return byte;
}

uint8_t DHT11_Read_Data(uint8_t *humidity_int, uint8_t *humidity_dec, uint8_t *temp_int, uint8_t *temp_dec)
{
    uint8_t byte[5];
    uint8_t i;
    uint32_t timeout = 0;
    while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1)
    {
        delay_us(1);
        if(++timeout > 100)
            return 0;
    }
    timeout = 0;
    while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0)
    {
        delay_us(1);
        if(++timeout > 100)
            return 0;
    }
    timeout = 0;
    while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1)
    {
        delay_us(1);
        if(++timeout > 100)
            return 0;
    }
    for(i = 0; i < 5; i++)
    {
        byte[i] = DHT11_Read_Byte();
    }
    
    *humidity_int = byte[0];
    *humidity_dec = byte[1];
    *temp_int = byte[2];
    *temp_dec = byte[3];
    
    return byte[4]; 
}
