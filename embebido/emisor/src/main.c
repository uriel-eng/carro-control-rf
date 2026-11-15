#include "stm32f1xx_hal.h"

//Implementedas
void SystemClock_Config();
void SystemLed_Config();
void DirectionButton_Config();
void DirectionButton_Read();
void UpdateButton();
//Por implementar
void DirectionButton_Processing();
void RF_Config();
void RF_SendData();

//Estructuras
typedef struct{
    
    GPIO_PinState currentState;
    GPIO_PinState lastState;
    uint32_t lastChange;
    
} ButtonState;

//Configuracion de botones control
ButtonState forward={GPIO_PIN_RESET,GPIO_PIN_RESET,0};
ButtonState backward={GPIO_PIN_RESET,GPIO_PIN_RESET,0};
ButtonState left={GPIO_PIN_RESET,GPIO_PIN_RESET,0};
ButtonState right={GPIO_PIN_RESET,GPIO_PIN_RESET,0};

int main(){
    //Configuracion de sistema
    HAL_Init();
    SystemClock_Config();
    SystemLed_Config();
    //Vector de direccion
    float directionVector[]={0,0};
    
    //UpdateButton(&forward,GPIOA,GPIO_PIN_0);
    while(1){
        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
        HAL_Delay(500);
        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
        HAL_Delay(500);
        
        
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
    RCC_ClkInitStruct.AHBCLKDivider=RCC_SYSCL_DIV1;
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

void DirectionButton_Config(){
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.Pin=GPIO_PIN_0 |
                        GPIO_PIN_1 |
                        GPIO_PIN_2 |
                        GPIO_PIN_3;
    GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull=GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
    
}
//Analizar vida temporal de las variables
//@Decrape
void DirectionButton_Read(){
    //Esto en global
    ButtonState forward={GPIO_PIN_RESET,GPIO_PIN_RESET,0};
    ButtonState backward={GPIO_PIN_RESET,GPIO_PIN_RESET,0};
    ButtonState left={GPIO_PIN_RESET,GPIO_PIN_RESET,0};
    ButtonState right={GPIO_PIN_RESET,GPIO_PIN_RESET,0};
    
    UpdateButton(&forward,GPIOA,GPIO_PIN_0);
    
}

void DirectionButton_Processing(float *directionVector){
    directionVector[0]=right.currentState-left.currentState;
    directionVector[1]=forward.currentState-backward.currentState;
    
    
}

void UpdateButton(ButtonState* btn,GPIO_TypeDef *port,uint16_t pin){
    
    GPIO_PinState currentSate=HAL_GPIO_ReadPin(port,pin);
    
    if(currentState!=btn->lastSate){
        
        btn->lastChange=HAL_GetTick();
        btn->lastState=currentState;
    
    }
    
    if((HAL_GetTick()-btn->lastChange)>=20)
        btn->currentState=currentState;
    
}