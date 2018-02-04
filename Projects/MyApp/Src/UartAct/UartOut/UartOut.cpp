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
#include "fw_evt.h"
#include "UartAct.h"
#include "UartOut.h"
#include "event.h"


#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(x, y)          


Q_DEFINE_THIS_FILE

namespace APP {

void UartOut::DmaCompleteCallback(uint8_t id) {
    static uint16_t counter = 0;
    Evt *evt = new Evt(UART_OUT_DMA_DONE, counter++);
    QF::PUBLISH(evt, 0);
}

UartOut::UartOut(uint8_t id, char const *name, QActive *owner, UART_HandleTypeDef &hal) :
    QHsm((QStateHandler)&UartOut::InitialPseudoState), m_id(id), m_name(name), 
    m_nextSequence(0), m_owner(owner),
    m_hal(hal), m_fifo(NULL), m_writeCount(0),
    m_activeTimer(owner, UART_OUT_ACTIVE_TIMER) {}

QState UartOut::InitialPseudoState(UartOut * const me, QEvt const * const e) {
    (void)e;
    me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));
    return Q_TRAN(&UartOut::Root);
}

QState UartOut::Root(UartOut * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UartOut::Stopped);
            break;
        }
        case UART_OUT_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartOutStartCfm(req.GetSeq(), ERROR_STATE);
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

QState UartOut::Stopped(UartOut * const me, QEvt const * const e) {
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
        case UART_OUT_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartOutStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case UART_OUT_START_REQ: {
            LOG_EVENT(e);
            UartOutStartReq const &req = static_cast<UartOutStartReq const &>(*e);
            me->m_fifo = req.GetFifo();
            Q_ASSERT(me->m_fifo);
            me->m_fifo->Reset();
            Evt *evt = new UartOutStartCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartOut::Started);
            break;
        }
        default: {
            status = Q_SUPER(&UartOut::Root);
            break;
        }
    }
    return status;
}

QState UartOut::Started(UartOut * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UartOut::Inactive);
            break;
        }
        case UART_OUT_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartOutStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartOut::Stopped);
            break;
        }
        default: {
            status = Q_SUPER(&UartOut::Root);
            break;
        }
    }
    return status;
}

QState UartOut::Inactive(UartOut * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            //LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            //LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case UART_OUT_WRITE_REQ: {
            //LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new ErrorEvt(UART_OUT_WRITE_CFM, req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            if (me->m_fifo->GetUsedCount() == 0) {
                status = Q_HANDLED();
            } else {
                status = Q_TRAN(&UartOut::Active);
            }
            break;
        }

        default: {
            status = Q_SUPER(&UartOut::Started);
            break;
        }
    }
    return status;
}

QState UartOut::Active(UartOut * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            //LOG_EVENT(e);
            me->m_activeTimer.armX(ACTIVE_TIMEOUT_MS);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            //LOG_EVENT(e);
            me->m_activeTimer.disarm();
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            status = Q_TRAN(&UartOut::Normal);
            break;
        }
        case UART_OUT_CONTINUE: {
            status = Q_TRAN(&UartOut::Active);
            break;
        }
        case UART_OUT_DONE: {
            status = Q_TRAN(&UartOut::Inactive);
            break;
        }
        // TODO Add HW_FAIL
        case UART_OUT_ACTIVE_TIMER: {
            //LOG_EVENT(e);
            Q_ASSERT(me->m_owner);
            Evt *evt = new UartOutFailInd(me->m_nextSequence++, ERROR_TIMEOUT);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartOut::Failed);
            break;
        }
        default: {
            status = Q_SUPER(&UartOut::Started);
            break;
        }
    }
    return status;
}

QState UartOut::Normal(UartOut * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            //LOG_EVENT(e);
            // Zero-copy.
            Fifo &fifo = *(me->m_fifo);
            uint32_t addr = fifo.GetReadAddr();
            uint32_t len = fifo.GetUsedCount();
            if ((addr + len - 1) > fifo.GetMaxAddr()) {
                len = fifo.GetMaxAddr() - addr + 1;
            }
            Q_ASSERT((len > 0) && (len <= fifo.GetUsedCount()));
            // Only applicable to STM32F7.
            //SCB_CleanDCache_by_Addr((uint32_t *)(ROUND_DOWN_32(addr)), ROUND_UP_32(addr + len - ROUND_DOWN_32(addr)));
            HAL_UART_Transmit_DMA(&me->m_hal, (uint8_t*)addr, len);
            me->m_writeCount = len;
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            //LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case UART_OUT_WRITE_REQ: {
            //LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new ErrorEvt(UART_OUT_WRITE_CFM, req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case UART_OUT_DMA_DONE: {
            //LOG_EVENT(e);
            me->m_fifo->IncReadIndex(me->m_writeCount);
            Evt *evt;
            if (me->m_fifo->GetUsedCount()) {
                evt = new Evt(UART_OUT_CONTINUE);
                me->m_owner->postLIFO(evt);
            } else {
                evt = new Evt(UART_OUT_EMPTY_IND, me->m_nextSequence++);
                QF::PUBLISH(evt, me);
                evt = new Evt(UART_OUT_DONE);
                me->m_owner->postLIFO(evt);
            }
            status = Q_HANDLED();
            break;
        }
        case UART_OUT_STOP_REQ: {
            //LOG_EVENT(e);
            // defer event
            me->m_owner->defer(&me->m_deferQueue, e);
            status = Q_TRAN(&UartOut::StopWait);
            break;
        }
        default: {
            status = Q_SUPER(&UartOut::Active);
            break;
        }
    }
    return status;
}

QState UartOut::StopWait(UartOut * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            //LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            //LOG_EVENT(e);
            // recall event
            me->m_owner->recall(&me->m_deferQueue);
            status = Q_HANDLED();
            break;
        }
        case UART_OUT_STOP_REQ: {
            //LOG_EVENT(e);
            // defer event
            me->m_owner->defer(&me->m_deferQueue, e);
            status = Q_HANDLED();
            break;
        }
        case UART_OUT_DMA_DONE: {
            //LOG_EVENT(e);
            me->m_fifo->IncReadIndex(me->m_writeCount);
            Evt *evt = new Evt(UART_OUT_DONE);
            me->m_owner->postLIFO(evt);
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&UartOut::Active);
            break;
        }
    }
    return status;
}

QState UartOut::Failed(UartOut * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            //LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            //LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&UartOut::Started);
            break;
        }
    }
    return status;
}

/*
QState UartOut::MyState(UartOut * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UartOut::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&UartOut::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP

