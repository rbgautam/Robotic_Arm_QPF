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
#include "UartIn.h"
#include "event.h"

#undef LOG_EVENT
#define LOG_EVENT(e)            

//Q_DEFINE_THIS_FILE


namespace APP {

void UartIn::RxCallback(uint8_t id) {
    static uint16_t counter = 0;
    Evt *evt = new Evt(UART_IN_DATA_RDY, counter++);
    QF::PUBLISH(evt, 0);
}

void UartIn::EnableRxInt() {
    // Enable Data Register not empty Interrupt.
    QF_CRIT_STAT_TYPE crit;
    QF_CRIT_ENTRY(crit);
    SET_BIT(m_hal.Instance->CR1, USART_CR1_RXNEIE);
    QF_CRIT_EXIT(crit);
}
    
void UartIn::DisableRxInt() {
    QF_CRIT_STAT_TYPE crit;
    QF_CRIT_ENTRY(crit);
    CLEAR_BIT(m_hal.Instance->CR1, USART_CR1_RXNEIE);
    QF_CRIT_EXIT(crit);
}

UartIn::UartIn(uint8_t id, char const *name, QActive *owner, UART_HandleTypeDef &hal) :
    QHsm((QStateHandler)&UartIn::InitialPseudoState), m_id(id), m_name(name), 
    m_nextSequence(0), m_owner(owner),
    m_hal(hal), m_stateTimer(owner, UART_IN_STATE_TIMER) {}

QState UartIn::InitialPseudoState(UartIn * const me, QEvt const * const e) {
    (void)e;
    return Q_TRAN(&UartIn::Root);
}

QState UartIn::Root(UartIn * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UartIn::Stopped);
            break;
        }
        case UART_IN_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartInStartCfm(req.GetSeq(), ERROR_STATE);
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

QState UartIn::Stopped(UartIn * const me, QEvt const * const e) {
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
        case UART_IN_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartInStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case UART_IN_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartInStartCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartIn::Started);
            break;
        }
        default: {
            status = Q_SUPER(&UartIn::Root);
            break;
        }
    }
    return status;
}

QState UartIn::Started(UartIn * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            me->EnableRxInt();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            me->DisableRxInt();
            status = Q_HANDLED();
            break;
        }
        case UART_IN_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartInStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartIn::Stopped);
            break;
        }
        case UART_IN_DATA_RDY: {
            //LOG_EVENT(e);
            char ch = me->m_hal.Instance->DR & (uint8_t)0x00FFU; 
            Evt *evt = new UartInCharInd(me->m_nextSequence++, ch);
            QF::PUBLISH(evt, me);
            me->EnableRxInt();
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&UartIn::Root);
            break;
        }
    }
    return status;
}

/*
QState UartIn::MyState(UartIn * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UartIn::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&UartIn::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP

