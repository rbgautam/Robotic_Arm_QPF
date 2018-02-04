/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "qpcpp.h"
#include "hsm_id.h"
#include "UartAct.h"
#include "UserBtn.h"

/* USER CODE BEGIN 0 */

using namespace FW;
using namespace APP;

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void){
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  QXK_ISR_ENTRY();
  QP::QF::tickX_(0);
  QXK_ISR_EXIT();
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

// UART2 TX DMA
// Must be declared as extern "C" in header.
void DMA1_Stream6_IRQHandler(void) {
    QXK_ISR_ENTRY();
    UART_HandleTypeDef *hal = UartAct::GetHal(UART2_ACT);
    HAL_DMA_IRQHandler(hal->hdmatx);
    QXK_ISR_EXIT();
}

// UART2 RX
// Must be declared as extern "C" in header.
void USART2_IRQHandler(void)
{
    QXK_ISR_ENTRY();
    UART_HandleTypeDef *hal = UartAct::GetHal(UART2_ACT);
    volatile uint32_t isrflags   = READ_REG(hal->Instance->SR);
    if (isrflags & USART_SR_ORE) {
        // Over-run error. Read DR followed by SR to clear flag.
        // Note - ORE will trigger interrupt when RXNE interrupt is enabled.
        uint32_t rxdata = READ_REG(hal->Instance->DR);
    } else {
        // Disable interrupt to avoid re-entering ISR before event is processed.
        CLEAR_BIT(hal->Instance->CR1, USART_CR1_RXNEIE);
        UartIn::RxCallback(UART2_IN);
    } 
    // TX does not use it.
    //HAL_UART_IRQHandler(hal);
    QXK_ISR_EXIT();
}

// User Button (PC.13)
void EXTI15_10_IRQHandler(void)
{
    QXK_ISR_ENTRY();
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET) {
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    }
    // Add other pins (10 to 15) here if needed.
    QXK_ISR_EXIT();  
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    if (pin == GPIO_PIN_13) {
        UserBtn::GpioIntCallback(USER_BTN);
    }
    // Add all other pins here if needed.
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
