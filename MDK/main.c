#include "test.h"

void Timer_Config(void);
void Delay_ms(uint32_t ms);
void Delay_us(uint32_t us);
void USART_Config(void);
void USART_SendString(char *str);
void DHT11_Config(void);
void USART_SendNumber(uint8_t num);

uint16_t u16Tim;
uint8_t u8Buff[5];
uint8_t u8CheckSum;
unsigned int i, j;

void Delay_us(uint32_t us) {
    TIM_SetCounter(TIM2, 0);
    while(TIM_GetCounter(TIM2) < us);
}

void Delay_ms(uint32_t ms) {
    while(ms--) {
        Delay_us(1000);   // 1 ms = 1000 µs (Timer2 d?m ? 1MHz)
    }
}

void USART_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef gpio;
    
    // PA9 - Tx
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    
    // PA10 - Rx
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    USART_InitTypeDef uart;
    
    uart.USART_BaudRate = 9600;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_Mode = USART_Mode_Tx;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &uart);

    USART_Cmd(USART1, ENABLE);
}

void USART_SendString(char *str) {
    while(*str) {
        USART_SendData(USART1, *str);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        str++;
    }
}

void USART_SendNumber(uint8_t num) {
    char buffer[5]; 
    sprintf(buffer, "%d", num); 
    USART_SendString(buffer);  
}

void DHT11_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpio;
    
    // C?u hình chân PB12 cho DHT11 (s? d?ng ch? d? open-drain)
    gpio.GPIO_Pin = GPIO_Pin_12;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);
}

void Timer_Config(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    TIM_TimeBaseInitTypeDef Timer;
    Timer.TIM_Period = 0xFFFF;
    Timer.TIM_Prescaler = 72 - 1;  // V?i xung 72MHz => Timer ch?y 1MHz (1 µs/cycle)
    Timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &Timer);
    TIM_Cmd(TIM2, ENABLE);
}

int main(void) {
    DHT11_Config();
    USART_Config();
    Timer_Config();
    
    while(1) {
        // --- B?t d?u giao ti?p v?i DHT11 ---
        // G?i tín hi?u start: kéo chân PB12 xu?ng LOW trong 20ms, sau dó dua lên HIGH.
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
        Delay_ms(20);
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
        
        // --- Ð?c ph?n h?i t? DHT11 ---
        // 1. Ch? DHT11 kéo xu?ng (ph?n h?i ban d?u)
        TIM_SetCounter(TIM2, 0);
        while(TIM_GetCounter(TIM2) < 10) {
            if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                break;
            }
        }
        u16Tim = TIM_GetCounter(TIM2);
        if(u16Tim >= 10) {
            goto wait;
        }
        
        // 2. Ch? DHT11 kéo lên (ph?n h?i ti?p theo)
        TIM_SetCounter(TIM2, 0);
        while(TIM_GetCounter(TIM2) < 45) {
            if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                break;
            }
        }
        u16Tim = TIM_GetCounter(TIM2);
        if((u16Tim >= 45) || (u16Tim <= 5)) {
            goto wait;
        }
        
        // 3. Ch? DHT11 kéo xu?ng báo hi?u b?t d?u truy?n d? li?u
        TIM_SetCounter(TIM2, 0);
        while(TIM_GetCounter(TIM2) < 90) {
            if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                break;
            }
        }
        u16Tim = TIM_GetCounter(TIM2);
        if((u16Tim >= 90) || (u16Tim <= 70)) {
            goto wait;
        }
        
        // 4. Ch? DHT11 kéo lên báo hi?u bit d?u tiên
        TIM_SetCounter(TIM2, 0);
        while(TIM_GetCounter(TIM2) < 95) {
            if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                break;
            }
        }
        u16Tim = TIM_GetCounter(TIM2);
        if((u16Tim >= 95) || (u16Tim <= 75)) {
            goto wait;
        }
        
        // --- Ð?c 40 bit d? li?u (5 byte) t? DHT11 ---
        for(i = 0; i < 5; i++) {
            u8Buff[i] = 0;   // Kh?i t?o byte
            for(j = 0; j < 8; j++) {
                // Ch? tín hi?u b?t d?u c?a bit (dòng di xu?ng)
                TIM_SetCounter(TIM2, 0);
                while(TIM_GetCounter(TIM2) < 65) {
                    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                        break;
                    }
                }
                u16Tim = TIM_GetCounter(TIM2);
                if((u16Tim >= 65) || (u16Tim <= 45)) {
                    goto wait;
                }
                
                // Ch? tín hi?u c?a bit (dòng lên)
                TIM_SetCounter(TIM2, 0);
                while(TIM_GetCounter(TIM2) < 80) {
                    if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                        break;
                    }
                }
                u16Tim = TIM_GetCounter(TIM2);
                if((u16Tim >= 80) || (u16Tim <= 10)) {
                    goto wait;
                }
                
                u8Buff[i] <<= 1;
                if(u16Tim > 45) {
                    u8Buff[i] |= 1;
                } else {
                    u8Buff[i] &= ~1;
                }
            }
        }
        
        // Tính checksum và ki?m tra
        u8CheckSum = u8Buff[0] + u8Buff[1] + u8Buff[2] + u8Buff[3];
        if(u8CheckSum != u8Buff[4]) {
            goto wait;
        }
        
        // --- In d? li?u qua USART ---
        USART_SendString("Temperature: ");
        USART_SendNumber(u8Buff[2]);
        USART_SendString("*C\n");
        USART_SendString("Humidity: ");
        USART_SendNumber(u8Buff[0]);
        USART_SendString("%\n");
        
wait:
        // Ð?i 3 giây tru?c khi d?c l?n ti?p theo
        Delay_ms(3000);
    }
}
