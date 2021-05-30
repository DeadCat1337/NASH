#include "it.h"
#include "math.h"
#include "stdint.h"

volatile uint32_t msTicks; 

void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}


void SysTick_Handler(void) {
  msTicks++;                        /* increment counter necessary in Delay() */
}

extern USART_HandleTypeDef huart2;

//void USART1_IRQHandler(void)
//{
//  /* USER CODE BEGIN USART1_IRQn 0 */
//    if ( USART1->ISR & UART_IT_TXE) {

//    }

//    if ( USART1->ISR & UART_IT_RXNE) {
//        HAL_UART_Receive_IT(&huart1,rx_data,buff_size_rx);
//        if(rx_data[pointer]=='\0') {
//              pointer=0;
//              readCommand(rx_data);
//              clearBuffer(rx_data,buff_size_rx);
//        } else {
//          pointer++;
//          if(pointer>=buff_size_rx) {
//              pointer=0;
//          }
//        }
//    }
//    /* USER CODE END USART1_IRQn 0 */
//    HAL_UART_IRQHandler(&huart1);
//    /* USER CODE BEGIN USART1_IRQn 1 */



//  /* USER CODE END USART1_IRQn 1 */
//}

void HardFault_Handler( void ) {
	//uint8_t str[]="\r\nHARD_FAULT\r\n";
	HAL_USART_Transmit(&huart2, (uint8_t*)"\r\nHARD_FAULT\r\n", 14, HAL_MAX_DELAY);
	//printf("\r\nHARD FAULT\r\n");
	while(1);
}