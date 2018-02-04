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

#include "hsm_id.h"
#include "fw_log.h"
#include "UserBtn.h"
#include "event.h"
#include "bsp.h"

#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(x, y)

//Q_DEFINE_THIS_FILE

namespace APP {

// User Btn uses PC.13 (i.e. EXTI13)
void UserBtn::ConfigGpioInt() {
    GPIO_InitTypeDef   GPIO_InitStructure;
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    NVIC_SetPriority(EXTI15_10_IRQn, EXTI15_10_PRIO);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void UserBtn::EnableGpioInt() {
    QF_CRIT_STAT_TYPE crit;
    QF_CRIT_ENTRY(crit);
    EXTI->IMR = BIT_SET(EXTI->IMR, GPIO_PIN_13, 0);
    QF_CRIT_EXIT(crit);
}

void UserBtn::DisableGpioInt() {
    QF_CRIT_STAT_TYPE crit;
    QF_CRIT_ENTRY(crit);
    EXTI->IMR = BIT_CLR(EXTI->IMR, GPIO_PIN_13, 0);
    QF_CRIT_EXIT(crit);
}

void UserBtn::GpioIntCallback(uint8_t id) {
    static uint16_t counter = 0; 
    Evt *evt = new Evt(USER_BTN_TRIG, counter++);
    QF::PUBLISH(evt, 0);
    DisableGpioInt();
}

UserBtn::UserBtn() :
    QActive((QStateHandler)&UserBtn::InitialPseudoState), 
    m_id(USER_BTN), m_name("USER_BTN"), m_nextSequence(0), 
    m_stateTimer(this, USER_BTN_STATE_TIMER) {}

QState UserBtn::InitialPseudoState(UserBtn * const me, QEvt const * const e) {
    (void)e;
    
    me->subscribe(USER_BTN_START_REQ);
    me->subscribe(USER_BTN_STOP_REQ);
    me->subscribe(USER_BTN_STATE_TIMER);
    me->subscribe(USER_BTN_TRIG);
    me->subscribe(USER_BTN_UP);
    me->subscribe(USER_BTN_DOWN);
    
    return Q_TRAN(&UserBtn::Root);
}

QState UserBtn::Root(UserBtn * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UserBtn::Stopped);
            break;
        }
        case USER_BTN_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserBtnStartCfm(req.GetSeq(), ERROR_STATE);
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

QState UserBtn::Stopped(UserBtn * const me, QEvt const * const e) {
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
        case USER_BTN_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserBtnStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case USER_BTN_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
             Evt *evt = new UserBtnStartCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UserBtn::Started);
            break;
        }
        default: {
            status = Q_SUPER(&UserBtn::Root);
            break;
        }
    }
    return status;
}

QState UserBtn::Started(UserBtn * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            ConfigGpioInt();            
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            DisableGpioInt();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            status = Q_TRAN(&UserBtn::Up);
            break;
        }
        case USER_BTN_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserBtnStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UserBtn::Stopped);
            break;
        }
        default: {
            status = Q_SUPER(&UserBtn::Root);
            break;
        }
    }
    return status;
}

QState UserBtn::Up(UserBtn * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            Evt *evt = new Evt(USER_BTN_UP_IND, me->m_nextSequence++);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case USER_BTN_TRIG: {
            LOG_EVENT(e);
            EnableGpioInt();
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
                Evt *evt = new Evt(USER_BTN_DOWN, me->m_nextSequence++);
                me->postLIFO(evt);
            }
            status = Q_HANDLED();
            break;
        }
        case USER_BTN_DOWN: {
            status = Q_TRAN(&UserBtn::Down);
            break;
        }
        default: {
            status = Q_SUPER(&UserBtn::Started);
            break;
        }
    }
    return status;
}

QState UserBtn::Down(UserBtn * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            Evt *evt = new Evt(USER_BTN_DOWN_IND, me->m_nextSequence++);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case USER_BTN_TRIG: {
            LOG_EVENT(e);
            EnableGpioInt();
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) != GPIO_PIN_RESET) {
                Evt *evt = new Evt(USER_BTN_UP, me->m_nextSequence++);
                me->postLIFO(evt);
            }
            status = Q_HANDLED();
            break;
        }
        case USER_BTN_UP: {
            status = Q_TRAN(&UserBtn::Up);
            break;
        }
        default: {
            status = Q_SUPER(&UserBtn::Started);
            break;
        }
    }
    return status;
}


/*
QState UserBtn::MyState(UserBtn * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UserBtn::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&UserBtn::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP
