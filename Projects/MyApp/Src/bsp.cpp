///***************************************************************************
// Product: DPP example, STM32 NUCLEO-L152RE board, preemptive QK kernel
// Last updated for version 5.6.5
// Last updated on  2016-07-05
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// http://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************

#include <string.h>
#include "qpcpp.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "bsp.h"

//Q_DEFINE_THIS_FILE

#define ENABLE_BSP_PRINT

static UART_HandleTypeDef usart;

/* top of stack (highest address) defined in the linker script -------------*/
extern int CSTACK$$Limit;

void BspInit() {
    // STM32F7xx HAL library initialization
    HAL_Init();

#ifdef ENABLE_BSP_PRINT
    // USART2 (TX=PA2, RX=PA3) is used as the virtual COM port in ST-Link.
    usart.Instance = USART2;
    usart.Init.BaudRate = 115200;
    usart.Init.WordLength = UART_WORDLENGTH_8B;
    usart.Init.StopBits = UART_STOPBITS_1;
    usart.Init.Parity = UART_PARITY_NONE;
    usart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    usart.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&usart);
    char const *test = "BspInit success\n\r";
    BspWrite(test, strlen(test));
#endif // ENABLE_BSP_PRINT
}

void BspWrite(char const *buf, uint32_t len) {
     HAL_UART_Transmit(&usart, (uint8_t *)buf, len, 0xFFFF);
}

uint32_t GetSystemMs() {
    return HAL_GetTick() * BSP_MSEC_PER_TICK;
}

// Override the one defined in stm32f7xx_hal.c.
// Callback by HAL_Init() called from BspInit().
// Initialize SysTick interrupt as required by some HAL functions.
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
    (void)TickPriority;
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
    NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIO);
    return HAL_OK;
}

// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // assigning all priority bits for preemption-prio. and none to sub-prio.
    NVIC_SetPriorityGrouping(0U);

    // Note SysTick has been initialized in HAL_InitTick(), which is called back
    // by HAL_Init() called from BspInit().
    // set priorities of ALL ISRs used in the system, see NOTE00
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    // DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    //NVIC_SetPriority(EXTI0_1_IRQn,   DPP::EXTI0_1_PRIO);
    // ...

    // enable IRQs...
    //NVIC_EnableIRQ(EXTI0_1_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QXK::onIdle(void) {
    // toggle the User LED on and then off (not enough LEDs, see NOTE01)
    QF_INT_DISABLE();
    //GPIOA->BSRR |= (LED_LD2);        // turn LED[n] on
    //GPIOA->BSRR |= (LED_LD2 << 16);  // turn LED[n] off
    QF_INT_ENABLE();


#if defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    // !!!CAUTION!!!
    // The WFI instruction stops the CPU clock, which unfortunately disables
    // the JTAG port, so the ST-Link debugger can no longer connect to the
    // board. For that reason, the call to __WFI() has to be used with CAUTION.
    //
    // NOTE: If you find your board "frozen" like this, strap BOOT0 to VDD and
    // reset the board, then connect with ST-Link Utilities and erase the part.
    // The trick with BOOT(0) is it gets the part to run the System Loader
    // instead of your broken code. When done disconnect BOOT0, and start over.
    //
    //__WFI();   Wait-For-Interrupt
#endif
}

//............................................................................
extern "C" void Q_onAssert(char const * const module, int loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;

    // Gallium - TBD
    char buf[200];
    HAL_Delay(100);
    QF_INT_DISABLE();
    snprintf(buf, sizeof(buf), "**** ASSERT: %s %d ****\n\r", module, loc);
    BspWrite(buf, strlen(buf));
    for (;;) {
    }
    //NVIC_SystemReset();
}


/*****************************************************************************
* The function assert_failed defines the error/assertion handling policy
* for the application. After making sure that the stack is OK, this function
* calls Q_onAssert, which should NOT return (typically reset the CPU).
*
* NOTE: the function Q_onAssert should NOT return.
*****************************************************************************/
extern "C" __stackless void assert_failed(char const *module, int loc) {
    /* re-set the SP in case of stack overflow */
    __asm volatile (
        "  MOV sp,%0\n\t"
        : : "r" (&CSTACK$$Limit));
    Q_onAssert(module, loc); /* call the application-specific QP handler */

    for (;;) { /* should not be reached, but just in case loop forever... */
    }
}


} // namespace QP

