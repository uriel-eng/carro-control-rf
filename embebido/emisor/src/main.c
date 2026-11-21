#include "stm32f1xx_hal.h"

#define RF_PORT GPIOA
#define RF_PIN  GPIO_PIN_4

typedef struct{
    
    GPIO_PinState currentState;
    GPIO_PinState lastState;
    uint32_t lastChange;
    
} ButtonState;


//Implementedas
void SystemClock_Config();
void SystemLed_Config();
void DirectionButton_Config();
void UpdateButton(ButtonState* btn,GPIO_TypeDef *port,uint16_t pin);
//Por implementar
void RF_Config();
uint8_t RF_SetPacket();
void RF_SendData();


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
    DirectionButton_Config();
    RF_Config();
    //UpdateButton(&forward,GPIOA,GPIO_PIN_0);
    while(1){
        /*
        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
        HAL_Delay(500);
        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
        HAL_Delay(500);
        */
        UpdateButton(&forward,GPIOA,GPIO_PIN_0);
        UpdateButton(&backward,GPIOA,GPIO_PIN_1);
        UpdateButton(&right,GPIOA,GPIO_PIN_2);
        UpdateButton(&left,GPIOA,GPIO_PIN_3);
        RF_SendData();
        HAL_Delay(10);
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

void UpdateButton(ButtonState* btn,GPIO_TypeDef *port,uint16_t pin){
    
    GPIO_PinState currentState=HAL_GPIO_ReadPin(port,pin);
    
    if(currentState!=btn->lastState){
        
        btn->lastChange=HAL_GetTick();
        btn->lastState=currentState;
    
    }
    
    if((HAL_GetTick()-btn->lastChange)>=20)
        btn->currentState=currentState;
    
}

void RF_Config(){
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct={0};
    GPIO_InitStruct.Pin =GPIO_PIN_4;
    GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

}
uint8_t RF_SetPacket(){
    uint8_t packet=0;
    if(forward.currentState==GPIO_PIN_SET)
        packet=packet | (0x01<<3);
    else
        packet=packet | (0x00<<3);
    if(backward.currentState==GPIO_PIN_SET)
        packet=packet | (0x01<<2);
    else
        packet=packet | (0x00<<2);
    if(right.currentState==GPIO_PIN_SET)
        packet=packet | (0x01<<1);
    else
        packet=packet | (0x00<<1);
    if(left.currentState==GPIO_PIN_SET)
        packet=packet | 0x01;
    else
        packet=packet | 0x00;
    
    return packet;
}
/*
Mi idea es que primero va a mandar un paquete de bits por 433MHz
|start|payload|
Lo que pasa es que voy a mandar un byte de inicio un 0xAA luego voy a mandar la longitud del comando que sera de un nibble , cada bit del nibble determina si el carro se mueve en una direccion, del MSB al LSB va adelante,atras,derecha,izquierda, finalmente el end que es un comando que es 0xFF si recibe start captura lo demas y checa end. Si end no es 0xFF el carro no se mueve
*/
void RF_SendData(){
    uint8_t payload=RF_SetPacket();
    uint16_t packet= (0xAA<<4) | (payload & 0x0F);
    uint8_t bit=0;
    
    for(int8_t i=11;i>=0;i--){
        
        bit=(packet>>i) & 0x01;
        
        HAL_GPIO_WritePin(RF_PORT,RF_PIN,GPIO_PIN_SET);    
            
        if(bit){
            
            HAL_Delay(3);
            HAL_GPIO_WritePin(RF_PORT,RF_PIN,GPIO_PIN_RESET);
            HAL_Delay(1);
             
        }        
        else{
            
            HAL_Delay(1);
            HAL_GPIO_WritePin(RF_PORT,RF_PIN,GPIO_PIN_RESET);
            HAL_Delay(3);
            
        }
        
    }
    
}