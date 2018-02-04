/*******************************************************************************
 * Copyright (C) Lawrence Lo (https://github.com/galliumstudio). 
 * All rights reserved.
 *
 * This program is open source software: you can redistribute it and/or
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
#include "UserLed.h"
#include "event.h"
#include "LedPattern.h"

#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(x, y)

Q_DEFINE_THIS_FILE

namespace APP {

#define  PERIOD_VALUE           (uint32_t)(1000 - 1)  /* Period Value  */
#define  DEFAULT_LEVEL_PERMIL   (uint32_t)((PERIOD_VALUE + 1)/2) /* Capture Compare 1 Value  */
  
// User LED uses PA.05 (TIM2_CH1)
void UserLed::InitPwm() {
    // Initialize GPIO pin and clocks.
    GPIO_InitTypeDef   GPIO_InitStruct;
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* TIMx Peripheral clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* Enable all GPIO Channels Clock requested */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PA.05 (TIM3_Channel1) in output, push-pull, alternate function mode
    */
    /* Common configuration for all channels */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Initialize PWM timer.
    /* Compute the prescaler value to have TIM3 counter clock equal to 21000000 Hz */
    uint32_t uhPrescalerValue = (uint32_t)((SystemCoreClock) / 21000000) - 1;

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
    m_timHandle.Instance = TIM2;
    m_timHandle.Init.Prescaler         = uhPrescalerValue;
    m_timHandle.Init.Period            = PERIOD_VALUE;
    m_timHandle.Init.ClockDivision     = 0;
    m_timHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    m_timHandle.Init.RepetitionCounter = 0;
    if (HAL_TIM_PWM_Init(&m_timHandle) != HAL_OK) {
        Q_ASSERT(0);
    }
    
    /*##-2- Configure the PWM channels #########################################*/
    /* Common configuration for all channels */
    m_timConfig.OCMode       = TIM_OCMODE_PWM1;
    m_timConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    m_timConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    m_timConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    m_timConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    m_timConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;    
}        
    
void UserLed::DeInitPwm() {
    // TODO
}

void UserLed::ConfigPwm(uint32_t levelPermil) {  
    m_timConfig.Pulse = levelPermil;
    if (HAL_TIM_PWM_ConfigChannel(&m_timHandle, &m_timConfig, TIM_CHANNEL_1) != HAL_OK) {
        Q_ASSERT(0);
    }
}

void UserLed::StartPwm() {
    if (HAL_TIM_PWM_Start(&m_timHandle, TIM_CHANNEL_1) != HAL_OK) {
        Q_ASSERT(0);
    }
}

void UserLed::StopPwm() {
    if (HAL_TIM_PWM_Stop(&m_timHandle, TIM_CHANNEL_1) != HAL_OK) {
        Q_ASSERT(0);
    }            
}
  
UserLed::UserLed() :
    QActive((QStateHandler)&UserLed::InitialPseudoState),
    m_id(USER_LED), m_name("USER_LED"), m_nextSequence(0), 
    m_currPattern(NULL), m_intervalIndex(0), m_currLayer(0),
    m_intervalTimer(this, USER_LED_INTERVAL_TIMER) {}

QState UserLed::InitialPseudoState(UserLed * const me, QEvt const * const e) {
    (void)e;
    
    me->subscribe(USER_LED_START_REQ);
    me->subscribe(USER_LED_STOP_REQ);
    me->subscribe(USER_LED_ON_REQ);
    me->subscribe(USER_LED_PATTERN_REQ);
    me->subscribe(USER_LED_OFF_REQ);  
    me->subscribe(USER_LED_INTERVAL_TIMER);
    me->subscribe(USER_LED_DONE);
    me->subscribe(USER_LED_NEXT_INTERVAL);
    me->subscribe(USER_LED_LAST_INTERVAL);
    me->subscribe(USER_LED_LOAD_PATTERN);
    me->subscribe(USER_LED_PATTERN_END);
    me->subscribe(USER_LED_NEW_PATTERN);
    
    return Q_TRAN(&UserLed::Root);
}

QState UserLed::Root(UserLed * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UserLed::Stopped);
            break;
        }
        case USER_LED_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserLedStartCfm(req.GetSeq(), ERROR_STATE);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState UserLed::Stopped(UserLed * const me, QEvt const * const e) {
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
        case USER_LED_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserLedStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case USER_LED_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserLedStartCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UserLed::Started);
            break;
        }
        default: {
            status = Q_SUPER(&UserLed::Root);
            break;
        }
    }
    return status;
}

QState UserLed::Started(UserLed * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);       
            BSP_LED_Init(LED2);
            me->InitPwm();
            me->ConfigPwm(DEFAULT_LEVEL_PERMIL);
            me->m_currLayer = 0;
            me->InvalidateAllPatternInfo();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            me->StopPwm();
            me->DeInitPwm();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            status = Q_TRAN(&UserLed::Idle);
            break;
        }
        case USER_LED_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserLedStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UserLed::Stopped);
            break;
        }
        /*
        case USER_LED_ON_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserLedOnCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            
            //BSP_LED_On(LED2);
            me->ConfigPwm(1000);
            me->StartPwm();
           
            status = Q_HANDLED();
            break;
        }
        case USER_LED_OFF_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserLedOffCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);   
            
            //BSP_LED_Off(LED2);
            me->StopPwm();

            status = Q_HANDLED();
            break;
        }
        */      
        case USER_LED_PATTERN_REQ: {
            LOG_EVENT(e);
            UserLedPatternReq const &req = static_cast<UserLedPatternReq const &>(*e);
            Q_ASSERT(req.GetLayer() < MAX_LAYER);
            me->SavePatternInfo(req.GetLayer(), req.GetPatternIndex(), req.IsRepeat());
            Evt *evt = new UserLedPatternCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me); 
            evt = new UserLedNewPattern(req.GetLayer());
            me->postLIFO(evt);
            status = Q_HANDLED();
            break;
        }      
        case USER_LED_LOAD_PATTERN: {
            LOG_EVENT(e);
            uint32_t index = me->GetCurrPatternInfo().GetIndex();
            me->m_currPattern = &TEST_LED_PATTERN_SET.GetPattern(index);
            me->m_intervalIndex = 0;
            status = Q_TRAN(&UserLed::Active);
            break;
        }
        default: {
            status = Q_SUPER(&UserLed::Root);
            break;
        }
    }
    return status;
}

QState UserLed::Idle(UserLed * const me, QEvt const * const e) {
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
        case USER_LED_OFF_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserLedOffCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);   
            status = Q_HANDLED();
            break;
        }   
        case USER_LED_NEW_PATTERN: {
            LOG_EVENT(e);
            UserLedNewPattern const &newPattern = static_cast<UserLedNewPattern const &>(*e);
            Q_ASSERT(newPattern.GetLayer() < MAX_LAYER);
            me->m_currLayer = newPattern.GetLayer();
            Evt *evt = new Evt(USER_LED_LOAD_PATTERN);
            me->postLIFO(evt);
            status = Q_HANDLED();
            break;
        }  
        default: {
            status = Q_SUPER(&UserLed::Started);
            break;
        }
    }
    return status;
}

QState UserLed::Active(UserLed * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            LedInterval const &currInterval = me->m_currPattern->GetInterval(me->m_intervalIndex);                        
            me->m_intervalTimer.armX(currInterval.GetDurationMs());
            me->ConfigPwm(currInterval.GetLevelPermil());
            me->StartPwm();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            me->StopPwm();
            me->m_intervalTimer.disarm();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            if (me->GetCurrPatternInfo().IsRepeat()) {
                status = Q_TRAN(&UserLed::Repeating);
            } else {
                status = Q_TRAN(&UserLed::Once);
            }
            break;
        }
        case USER_LED_INTERVAL_TIMER: {
            LOG_EVENT(e);
            uint32_t intervalCount = me->m_currPattern->GetCount();
            Q_ASSERT(intervalCount > 0);
            if (me->m_intervalIndex < (intervalCount - 1)) {
                Evt *evt = new Evt(USER_LED_NEXT_INTERVAL);
                me->postLIFO(evt);
            } else if (me->m_intervalIndex == (intervalCount - 1)) {
                Evt *evt = new Evt(USER_LED_LAST_INTERVAL);
                me->postLIFO(evt);
            } else {
                Q_ASSERT(0);
            }                
            status = Q_HANDLED();        
            break;
        }
        case USER_LED_OFF_REQ: {
            LOG_EVENT(e);
            UserLedOffReq const &req = static_cast<UserLedOffReq const &>(*e);
            Q_ASSERT(req.GetLayer() < MAX_LAYER);
            me->InvalidatePatternInfo(req.GetLayer());
            Evt *evt = new UserLedOffCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me); 
            if (req.GetLayer() == me->m_currLayer) {
                Evt *evt = new Evt(USER_LED_PATTERN_END);
                me->postLIFO(evt);
            }
            status = Q_HANDLED();
            break;
        }        
        case USER_LED_PATTERN_END: {
            LOG_EVENT(e);
            uint32_t highestValidLayer = MAX_LAYER;     // invalid
            bool found = me->GetHighestLayerInUse(highestValidLayer);
            if (!found) {
                me->m_currLayer = 0;
                Evt *evt = new Evt(USER_LED_DONE);
                me->postLIFO(evt);                
            } else {
                Q_ASSERT(highestValidLayer < MAX_LAYER);
                me->m_currLayer = highestValidLayer;
                Evt *evt = new Evt(USER_LED_LOAD_PATTERN);
                me->postLIFO(evt);
            }
            status = Q_HANDLED();
            break;
        }        
        case USER_LED_NEXT_INTERVAL: {
            LOG_EVENT(e);
            me->m_intervalIndex++;
            status = Q_TRAN(&UserLed::Active);
            break;
        }
        case USER_LED_LAST_INTERVAL: {
            LOG_EVENT(e);
            me->m_intervalIndex = 0;
            status = Q_TRAN(&UserLed::Active);  
            break;
        }       
        case USER_LED_DONE: {
            LOG_EVENT(e);
            status = Q_TRAN(&UserLed::Idle);  
            break;
        }   
        default: {
            status = Q_SUPER(&UserLed::Started);
            break;
        }
    }
    return status;
}

QState UserLed::Repeating(UserLed * const me, QEvt const * const e) {
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
        case USER_LED_NEW_PATTERN: {
            LOG_EVENT(e);
            UserLedNewPattern const &newPattern = static_cast<UserLedNewPattern const &>(*e);
            Q_ASSERT(newPattern.GetLayer() < MAX_LAYER);
            if (newPattern.GetLayer() >= me->m_currLayer) {
                me->m_currLayer = newPattern.GetLayer();
                Evt *evt = new Evt(USER_LED_LOAD_PATTERN);
                me->postLIFO(evt);
            }
            status = Q_HANDLED();
            break;
        }          
        default: {
            status = Q_SUPER(&UserLed::Active);
            break;
        }
    }
    return status;
}

QState UserLed::Once(UserLed * const me, QEvt const * const e) {
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
        case USER_LED_LAST_INTERVAL: {
            LOG_EVENT(e);
            me->InvalidatePatternInfo(me->m_currLayer);
            Evt *evt = new Evt(USER_LED_PATTERN_END);
            me->postLIFO(evt);
            status = Q_HANDLED();
            break;
        }
        case USER_LED_NEW_PATTERN: {
            LOG_EVENT(e);
            UserLedNewPattern const &newPattern = static_cast<UserLedNewPattern const &>(*e);
            Q_ASSERT(newPattern.GetLayer() < MAX_LAYER);
            // Condition below must be >, NOT >= since new pattern has been saved in patternInfo[].
            if (newPattern.GetLayer() > me->m_currLayer) {
                me->m_patternInfo[me->m_currLayer].Invalidate();
            }
            if (newPattern.GetLayer() >= me->m_currLayer) {
                me->m_currLayer = newPattern.GetLayer();
                Evt *evt = new Evt(USER_LED_LOAD_PATTERN);
                me->postLIFO(evt);
            }
            status = Q_HANDLED();
            break;
        }        

        default: {
            status = Q_SUPER(&UserLed::Active);
            break;
        }
    }
    return status;
}

/*
QState UserLed::MyState(UserLed * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UserLed::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&UserLed::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP
