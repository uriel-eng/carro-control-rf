#include "stm32f1xx_hal.h"

#define RF_PORT GPIOA
#define RF_PIN  GPIO_PIN_4
#define TIME_OUT 20

#define DRIVER_PORT GPIOA
#define FORWARD GPIO_PIN_3
#define BACKWARD GPIO_PIN_2
#define RIGHT GPIO_PIN_1
#define LEFT GPIO_PIN_0

void SystemClock_Config();
void SystemLed_Config();
void RF_Config();
void RF_ReadData();
void Driver_Config();
void Driver_Move(uint8_t);
void Driver_Stop();

int steering=90;
int motor=0;

int main(){
    
    while(1){
        
        
        
    }
    
}

void SystemClock_Config(){  
    
    RCC_OscInitTypeDef RCC_OscInitStruct={0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct={0};
    
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    RCC_ClkInitStruct.ClockType=RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider=RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider=RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider=RCC_HCLK_DIV1;
    
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct,FLASH_LATENCY_2);
    
}

void SystemLed_Config(){
    
    __HAL_RCC_GPIOC_CLK_ENABLE();
    //Configurar PC13 como salida
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.Pin=GPIO_PIN_13;
    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);
    
}

void RF_Config(){
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.Pin =GPIO_PIN_4;
    GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull=GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

}
/*
Si flag_high es 0b00000011 entonces es un uno logico
Si flag_low es 0b00000011 entonces es un cero logico
Otras posibles combinaciones son ruido
*/
void RF_ReadData(void){

    uint16_t packet = 0;
    uint32_t time_start = 0;
    uint32_t time_edge = 0;
    uint32_t time_high = 0;
    uint32_t time_low = 0;

    uint8_t bit_received = 0;

    if(HAL_GPIO_ReadPin(RF_PORT, RF_PIN) == GPIO_PIN_SET){

        time_start = HAL_GetTick();

        for(int i = 11; i >= 0; ){

            if(HAL_GPIO_ReadPin(RF_PORT, RF_PIN) == GPIO_PIN_SET){

                time_edge = HAL_GetTick();

                while(HAL_GPIO_ReadPin(RF_PORT, RF_PIN) == GPIO_PIN_SET){

                    if(HAL_GetTick() - time_start > TIME_OUT){
                        Driver_Stop();
                        return;
                    }

                }

                time_high = HAL_GetTick() - time_edge;
                time_edge = HAL_GetTick();

                while(HAL_GPIO_ReadPin(RF_PORT, RF_PIN) == GPIO_PIN_RESET){

                    if(HAL_GetTick() - time_start > TIME_OUT){
                        Driver_Stop();
                        return;
                    }

                }

                time_low = HAL_GetTick() - time_edge;

                if((time_high >= 2 && time_high <= 4) &&
                   (time_low >= 0 && time_low <= 2)){

                    bit_received = 1;

                }

                else if((time_high >= 0 && time_high <= 2) &&
                        (time_low >= 2 && time_low <= 4)){

                    bit_received = 0;

                }

                else{

                    Driver_Stop();
                    return;

                }

                if(bit_received){

                    packet |= (1U << i);

                }

                i--;

            }

            if(HAL_GetTick() - time_start > TIME_OUT){

                Driver_Stop();
                return;

            }

        }

        // Validar identificador
        if((packet >> 4) == 0xAA){

            uint8_t payload = packet & 0x0F;
            Driver_Move(payload);

        }
        else{

            Driver_Stop();

        }

    }

}

void Driver_Config(){
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.Pin=GPIO_PIN_0 |
                        GPIO_PIN_1 |
                        GPIO_PIN_2 |
                        GPIO_PIN_3;
    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
    
}
/*
Si steering es 0 no se mueve, si es 2 derecha y es 1 izquierda
Si motor es 0 no se mueve, si es 2 avanza y si es 1 atras
*/
void Driver_Move(uint8_t payload){
    
    HAL_GPIO_WritePin(DRIVER_PORT, FORWARD,
        (payload & 0b00001000) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DRIVER_PORT, BACKWARD,
        (payload & 0b00000100) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DRIVER_PORT, RIGHT,
        (payload & 0b00000010) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(DRIVER_PORT, LEFT,
        (payload & 0b00000001) ? GPIO_PIN_SET : GPIO_PIN_RESET);    
    
}

void Driver_Stop(){
    
    HAL_GPIO_WritePin(DRIVER_PORT,FORWARD,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRIVER_PORT,BACKWARD,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRIVER_PORT,RIGHT,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRIVER_PORT,LEFT,GPIO_PIN_RESET);
    
}