/*******************************************************************************
 * Copyright (C) Lawrence Lo (https://github.com/galliumstudio). 
 * All rights reserved.
 *
 * This program is open source software: you can Stopistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
 ******************************************************************************/

#include "stm32f4xx_nucleo.h"
#include "hsm_id.h"
#include "fw_log.h"
#include "TouchBtn.h"
#include "event.h"



#define  PERIOD_VALUE       (uint32_t)(20000 - 1)  /* Period Value  */
#define  PULSE1_VALUE       (uint32_t)((PERIOD_VALUE + 1)/2) /* Capture Compare 1 Value  */

/*
#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(x, y)
*/


Q_DEFINE_THIS_FILE


namespace APP {
 const uint16_t SERVO_CHANNEL[SERVOn] = {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4}; 
  const uint16_t SERVO_COMP_REG[SERVOn] = {TIM4->CCR1, TIM4->CCR2, TIM4->CCR3, TIM4->CCR4};   
 TouchBtn::TouchBtn(uint8_t id, char const *name, QActive *owner, TouchBtn_TypeDef btnName) :
    QHsm((QStateHandler)&TouchBtn::InitialPseudoState), m_id(id), m_name(name), 
    m_owner(owner), m_touchBtnName(btnName) {}
    
    bool TouchBtn::InitPwm(TouchBtn * const me) {  
      //Configure GPIO
       // TIM4 Clock Enable
      __HAL_RCC_TIM4_CLK_ENABLE();
//    // GPIOB Clock Enable
      //SERVO_GPIO_CLK_ENABLE(); 
      //GPIO_Struct
      
//      BSP_SERVO_PWM_Init(me->m_touchBtnName);
      
      
      
      
      
      /* Compute the prescaler value to have TIM3 counter clock equal to 21000000 Hz */
      uint32_t uhPrescalerValue = (uint16_t) 84;
      //(uint32_t)((SystemCoreClock) / 21000000) - 1;
      
      /*##-1- Configure the TIM peripheral #######################################*/
      /* -----------------------------------------------------------------------
      TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles.
      
      In this example TIM3 input clock (TIM3CLK) is set to APB1 clock x 2,
      since APB1 prescaler is equal to 2.
      TIM2CLK = APB1CLK*2
      APB1CLK = HCLK/2
      => TIM2CLK = HCLK = SystemCoreClock
      
      To get TIM3 counter clock at 16 MHz, the prescaler is computed as follows:
      Prescaler = (TIM2CLK / TIM3 counter clock) - 1
      Prescaler = ((SystemCoreClock) /15 MHz) - 1
      
      To get TIM3 output clock at 21 KHz, the period (ARR)) is computed as follows:
      ARR = (TIM3 counter clock / TIM3 output clock) - 1
      = 665
      
      TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR + 1)* 100 = 50%
      TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR + 1)* 100 = 37.5%
      TIM3 Channel3 duty cycle = (TIM3_CCR3/ TIM3_ARR + 1)* 100 = 25%
      TIM3 Channel4 duty cycle = (TIM3_CCR4/ TIM3_ARR + 1)* 100 = 12.5%
      
      Note:
      SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
      Each time the core clock (HCLK) changes, user had to update SystemCoreClock
      variable value. Otherwise, any configuration based on this variable will be incorrect.
      This variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetSysClockFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
      ----------------------------------------------------------------------- */
      
      /* Initialize TIMx peripheral as follows:
      + Prescaler = ((SystemCoreClock) / 21000000) - 1
      + Period = (1000 - 1)
      + ClockDivision = 0
      + Counter direction = Up
      */
      m_timHandle.Instance = TIM4;
      
      m_timHandle.Init.Prescaler         = uhPrescalerValue;
      m_timHandle.Init.Period            = 19999;
      m_timHandle.Init.ClockDivision     = 0;
      m_timHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
      m_timHandle.Init.RepetitionCounter = 0;
      if (HAL_TIM_PWM_Init(&m_timHandle) != HAL_OK) {
        return false;
      }
      
      /*##-2- Configure the PWM channels #########################################*/
      /* Common configuration for all channels */
      m_timConfig.OCMode       = TIM_OCMODE_PWM1;
      m_timConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
      m_timConfig.OCFastMode   = TIM_OCFAST_DISABLE;
      m_timConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
      m_timConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
      m_timConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
      
      /* Set the pulse value for channel 1 */
      m_timConfig.Pulse = PULSE1_VALUE;
//      uint32_t Channelx = SERVO_CHANNEL[me->m_touchBtnName];
//      if (HAL_TIM_PWM_ConfigChannel(&m_timHandle, &m_timConfig, Channelx) != HAL_OK) {
//        return false;
//      }
      
      return true;
    }
    
    
    
    void TouchBtn::ConfigPwm(TouchBtn * const me) {  
//      uint32_t Channelx = SERVO_CHANNEL[me->m_touchBtnName];
//      if (HAL_TIM_PWM_ConfigChannel(&m_timHandle, &m_timConfig, Channelx) != HAL_OK) {
//        Q_ASSERT(0);
//      }
    }
    
    void TouchBtn::StartPwm(TouchBtn * const me) {
//      uint32_t Channelx = SERVO_CHANNEL[me->m_touchBtnName];
//      if (HAL_TIM_PWM_Start(&m_timHandle, Channelx) != HAL_OK) {
//        Q_ASSERT(0);
//      }
    }
    
    void TouchBtn::StopPwm(TouchBtn * const me) {
//      uint32_t Channelx = SERVO_CHANNEL[me->m_touchBtnName];
//      if (HAL_TIM_PWM_Stop(&m_timHandle, Channelx ) != HAL_OK) { //TIM_CHANNEL_1
//        Q_ASSERT(0);
//      }            
    }
    
    
    void TouchBtn::MoveForward(TouchBtn * const me)
    {
      int servoConfig = SERVO1;//me->m_touchBtnName;
      
      int minInterval = 600;
      int maxInterval = 1500;
      if(servoConfig == SERVO1  || servoConfig == SERVO4){
         minInterval = 600;
         maxInterval = 2100;
        
      }
      
      
      
      for(int i = minInterval; i<=maxInterval; )
      {
        //TIM4->CCR1 = i;       // 600 == 0.6 ms  -> 0' 1500 == 1.5ms -> 90'  2100 == 2.1ms ->
        //__HAL_TIM_SET_COMPARE
        if(servoConfig == SERVO1)
          TIM4->CCR1 = i; 
        if(servoConfig == SERVO2)
          TIM4->CCR2 = i; 
        if(servoConfig == SERVO3)
          TIM4->CCR3 = i; 
        if(servoConfig == SERVO4)
          TIM4->CCR4 = i; 
        
        HAL_Delay(20);
        i=i+20;
      }
      
    }
    
    void TouchBtn::MoveBackward(TouchBtn * const me)
    {
      int servoConfig = SERVO1;//me->m_touchBtnName;
      
      int minInterval = 600;
      int maxInterval = 1500;
      
      if(servoConfig == SERVO1 || servoConfig == SERVO4){
        minInterval = 600;
        maxInterval = 2100;
        
      }
      
      
      
      for(int i = maxInterval; i>minInterval; )
      {
        // SERVO_COMP_REG[me->m_touchBtnName] = i;       // 600 == 0.6 ms  -> 0'
        
        if(servoConfig == SERVO1)
          TIM4->CCR1 = i; 
        if(servoConfig == SERVO2)
          TIM4->CCR2 = i; 
        if(servoConfig == SERVO3)
          TIM4->CCR3 = i; 
        if(servoConfig == SERVO4)
          TIM4->CCR4 = i; 
        
        HAL_Delay(20);
        i=i-20;
      }
      
      
    }
    
    
    
    QState TouchBtn::InitialPseudoState(TouchBtn * const me, QEvt const * const e) {
      (void)e;
      //    me->subscribe(Motor_Stop_REQ);      
      //    me->subscribe(Motor_Backward_REQ);   
      //    me->subscribe(Motor_Forward_REQ);    
      return Q_TRAN(&TouchBtn::Root);
    }
    
    QState TouchBtn::Root(TouchBtn * const me, QEvt const * const e) {
      QState status;
      switch (e->sig) {
      case Q_ENTRY_SIG: {
        LOG_EVENT(e);
        BSP_SERVO_Init(SERVO1); //me->m_touchBtnName
        me->InitPwm(me);
        me->ConfigPwm(me);
        status = Q_HANDLED();
        break;
      }
      case Q_EXIT_SIG: {
        LOG_EVENT(e);
        status = Q_HANDLED();
        break;
      }
      
      case Q_INIT_SIG: {
        status = Q_TRAN(&TouchBtn::Stop);
        break;
      }
      
      default: {
        status = Q_SUPER(&QHsm::top);
        break;
      }
      }
      return status;
    }
    
    
    QState TouchBtn::Stop(TouchBtn * const me, QEvt const * const e) {
      QState status;
      switch (e->sig) {
      case Q_ENTRY_SIG: {
        LOG_EVENT(e);
        me->Print(me, "Stop");
        status = Q_HANDLED();
        break;
      }
      case Q_EXIT_SIG: {
        LOG_EVENT(e);
        status = Q_HANDLED();
        break;
      }
      
      case MOTOR_FORWARD_REQ:
        {
          LOG_EVENT(e);
          me->StartPwm(me);
          me->MoveForward(me);
          me->StopPwm(me);
          HAL_Delay(100);
          status = Q_TRAN(&TouchBtn::Forward);
          break;
        }
        
      case MOTOR_BACKWARD_REQ:
        {
          LOG_EVENT(e);
          me->StartPwm(me);
          me->MoveBackward(me);
          me->StopPwm(me);
          HAL_Delay(100);
          status = Q_TRAN(&TouchBtn::Backward);
          break;
        }
      case MOTOR_STOP_REQ:
        {
          LOG_EVENT(e);
          
          status = Q_TRAN(&TouchBtn::Stop);
          break;
        }
      default: {
        status = Q_SUPER(&TouchBtn::Root);
        break;
      }
      }
      return status;
    }
    
    
    QState TouchBtn::Backward(TouchBtn * const me, QEvt const * const e) {
      QState status;
      switch (e->sig) {
      case Q_ENTRY_SIG: {
        LOG_EVENT(e);

        
        me->Print(me,"Backward");
        status = Q_HANDLED();
        break;
      }
      case Q_EXIT_SIG: {
        LOG_EVENT(e);
        status = Q_HANDLED();
        break;
      }
      case MOTOR_BACKWARD_REQ:
      {
          LOG_EVENT(e);
          me->Print(me,"Already Backward");
          status = Q_HANDLED();
          break;
      }
        
      case MOTOR_FORWARD_REQ:
      {
          LOG_EVENT(e);
          me->StartPwm(me);
          me->MoveForward(me);
          me->StopPwm(me);
          HAL_Delay(100);
          status = Q_TRAN(&TouchBtn::Forward);
          break;
        }
      case MOTOR_STOP_REQ:
        {
          LOG_EVENT(e);
          status = Q_TRAN(&TouchBtn::Stop);
          break;
        }
      default: {
        status = Q_SUPER(&TouchBtn::Root);
        break;
      }
      }
      return status;
    }
    
    QState TouchBtn::Forward(TouchBtn * const me, QEvt const * const e) {
      QState status;
      switch (e->sig) {
      case Q_ENTRY_SIG: {
        LOG_EVENT(e);
        me->Print(me ,"Forward");
        status = Q_HANDLED();
        break;
      }
      case Q_EXIT_SIG: {
        LOG_EVENT(e);
        status = Q_HANDLED();
        break;
      }
      
      case MOTOR_FORWARD_REQ:
        {
          LOG_EVENT(e);
          me->Print(me ,"Already Forward");
          status = Q_HANDLED();
          break;
        }
      case MOTOR_BACKWARD_REQ:
        {
          LOG_EVENT(e);
          me->StartPwm(me);
          me->MoveBackward(me);
          me->StopPwm(me);
          HAL_Delay(100);
          status = Q_TRAN(&TouchBtn::Backward);
          break;
        }
        
      case MOTOR_STOP_REQ:
        {
          LOG_EVENT(e);
          status = Q_TRAN(&TouchBtn::Stop);
          break;
        }
        
      default: {
        status = Q_SUPER(&TouchBtn::Root);
        break;
      }
      }
      return status;
    }
    
    void TouchBtn::Print(TouchBtn * const me, char* mode){
      PRINT("\r\n %s = %s\r\n---------DONE----------\r\n", me->m_name, mode);
      
    }

    
/*
QState TouchBtn::MyState(TouchBtn * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            status = Q_TRAN(&TouchBtn::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&TouchBtn::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP

