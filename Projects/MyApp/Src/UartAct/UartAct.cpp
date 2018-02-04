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
#include "event.h"
#include "UartAct.h"

#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(x, y)


Q_DEFINE_THIS_FILE

namespace APP {

UART_HandleTypeDef UartAct::m_hal;  
  
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *hal) {
    UartOut::DmaCompleteCallback(UART2_OUT);
}

UART_HandleTypeDef *UartAct::GetHal(uint8_t id) {
    // id for future use.
    return &m_hal;
}

UartAct::UartAct(uint8_t id, char const *name, char const *inName, char const *outName,
                 USART_TypeDef *dev) :
    QActive((QStateHandler)&UartAct::InitialPseudoState),
    m_id(id), m_name(name), m_nextSequence(0), m_savedInSeq(0), m_cfmCount(0),
    m_uartIn(UART2_IN, inName, this, m_hal),
    m_uartOut(UART2_OUT, outName, this, m_hal),
    m_outFifo(NULL), m_inFifo(NULL),
    m_stateTimer(this, UART_ACT_STATE_TIMER) {
    Q_ASSERT(dev);
    memset(&m_hal, 0, sizeof(m_hal));
    m_hal.Instance = dev;
}

QState UartAct::InitialPseudoState(UartAct * const me, QEvt const * const e) {
    (void)e;
    me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));
    
    me->subscribe(UART_ACT_START_REQ);
    me->subscribe(UART_ACT_STOP_REQ);
    me->subscribe(UART_ACT_STATE_TIMER);
    me->subscribe(UART_ACT_START);
    me->subscribe(UART_ACT_DONE);
    me->subscribe(UART_ACT_FAIL);
    
    me->subscribe(UART_OUT_START_REQ);
    me->subscribe(UART_OUT_START_CFM);
    me->subscribe(UART_OUT_STOP_REQ);
    me->subscribe(UART_OUT_STOP_CFM);
    me->subscribe(UART_OUT_FAIL_IND);
    me->subscribe(UART_OUT_WRITE_REQ);
    me->subscribe(UART_OUT_ACTIVE_TIMER);
    me->subscribe(UART_OUT_DONE);
    me->subscribe(UART_OUT_DMA_DONE);
    me->subscribe(UART_OUT_CONTINUE);
    me->subscribe(UART_OUT_HW_FAIL);
    
    me->subscribe(UART_IN_START_REQ);
    me->subscribe(UART_IN_START_CFM);
    me->subscribe(UART_IN_STOP_REQ);
    me->subscribe(UART_IN_STOP_CFM);
    me->subscribe(UART_IN_STATE_TIMER);
    me->subscribe(UART_IN_DONE);
    me->subscribe(UART_IN_DATA_RDY);
    
    return Q_TRAN(&UartAct::Root);
}

QState UartAct::Root(UartAct * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            me->m_uartIn.Init();
            me->m_uartOut.Init();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG: {
            status = Q_TRAN(&UartAct::Stopped);
            break;
        }
        case UART_ACT_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartActStartCfm(req.GetSeq(), ERROR_STATE);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case UART_ACT_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            me->m_savedInSeq = req.GetSeq();
            status = Q_TRAN(&UartAct::Stopping);
            break;
        }
        
        case UART_OUT_START_REQ:
        case UART_OUT_STOP_REQ:
        case UART_OUT_WRITE_REQ:
        case UART_OUT_ACTIVE_TIMER:
        case UART_OUT_DONE:
        case UART_OUT_DMA_DONE:
        case UART_OUT_CONTINUE:
        case UART_OUT_HW_FAIL: {
            me->m_uartOut.dispatch(e);
            break;
        };
        case UART_IN_START_REQ:
        case UART_IN_STOP_REQ:
        case UART_IN_STATE_TIMER:
        case UART_IN_DONE:
        case UART_IN_DATA_RDY: {
            me->m_uartIn.dispatch(e);
            break;
        }
        
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState UartAct::Stopped(UartAct * const me, QEvt const * const e) {
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
        case UART_ACT_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UartActStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case UART_ACT_START_REQ: {
            LOG_EVENT(e);
            UartActStartReq const &req = static_cast<UartActStartReq const &>(*e);
            // For now serial configuration is fixed. In the future, it can be passed in with START_REQ.
            me->m_hal.Init.BaudRate = 115200;
            me->m_hal.Init.WordLength = UART_WORDLENGTH_8B;
            me->m_hal.Init.StopBits = UART_STOPBITS_1;
            me->m_hal.Init.Parity = UART_PARITY_NONE;
            me->m_hal.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
            me->m_hal.Init.Mode = UART_MODE_TX_RX;
            HAL_UART_DeInit(&me->m_hal);
            HAL_StatusTypeDef halStatus = HAL_UART_Init(&me->m_hal);
            if(halStatus == HAL_OK) {
                me->m_savedInSeq = req.GetSeq();
                me->m_outFifo = req.GetOutFifo();
                me->m_inFifo = req.GetInFifo();
                Evt *evt = new Evt(UART_ACT_START);
                me->postLIFO(evt);
            } else {
                DEBUG("HAL_UART_Init failed(%d", halStatus);
                Evt *evt = new UartActStartCfm(req.GetSeq(), ERROR_HAL);
                QF::PUBLISH(evt, me);
            }
            status = Q_HANDLED();
            break;
        }
        case UART_ACT_START: {
            LOG_EVENT(e);
            status = Q_TRAN(&UartAct::Starting);
            break;
        }
        default: {
            status = Q_SUPER(&UartAct::Root);
            break;
        }
    }
    return status;
}

QState UartAct::Starting(UartAct * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            uint32_t timeout = UartActStartReq::TIMEOUT_MS;
            Q_ASSERT(timeout > UartInStartReq::TIMEOUT_MS);
            Q_ASSERT(timeout > UartOutStartReq::TIMEOUT_MS);
            me->m_stateTimer.armX(timeout);
            me->m_cfmCount = 0;
            
            Q_ASSERT(me->m_outFifo && me->m_inFifo);
            Evt *evt = new UartOutStartReq(me->m_nextSequence++, me->m_outFifo);
            // TODO - Save sequence number for comparison.
            QF::PUBLISH(evt, me);
            evt = new UartInStartReq(me->m_nextSequence++, me->m_inFifo);
            // TODO - Save sequence number for comparison.
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            me->m_stateTimer.disarm();
            status = Q_HANDLED();
            break;
        }
        case UART_OUT_START_CFM:
        case UART_IN_START_CFM: {
            LOG_EVENT(e);
            me->HandleCfm(ERROR_EVT_CAST(*e), 2);
            status = Q_HANDLED();
            break;
        }
        case UART_ACT_FAIL:
        case UART_ACT_STATE_TIMER: {
            LOG_EVENT(e);
            Evt *evt;
            if (e->sig == UART_ACT_FAIL) {
                ErrorEvt const &fail = ERROR_EVT_CAST(*e);
                evt = new UartActStartCfm(me->m_savedInSeq,
                                         fail.GetError(), fail.GetReason());
            } else {
                evt = new UartActStartCfm(me->m_savedInSeq, ERROR_TIMEOUT);
            }
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartAct::Stopping);
            break;
        }
        case UART_ACT_DONE: {
            LOG_EVENT(e);
            Evt *evt = new UartActStartCfm(me->m_savedInSeq, ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartAct::Started);
            break;
        }
        default: {
            status = Q_SUPER(&UartAct::Root);
            break;
        }
    }
    return status;
}

QState UartAct::Stopping(UartAct * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            uint32_t timeout = UartActStopReq::TIMEOUT_MS;
            Q_ASSERT(timeout > UartInStopReq::TIMEOUT_MS);
            Q_ASSERT(timeout > UartOutStopReq::TIMEOUT_MS);
            me->m_stateTimer.armX(timeout);
            me->m_cfmCount = 0;

            Evt *evt = new UartInStopReq(me->m_nextSequence++);
            QF::PUBLISH(evt, me);
            evt = new UartOutStopReq(me->m_nextSequence++);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            me->m_stateTimer.disarm();
            status = Q_HANDLED();
            // recall event
            me->recall(&me->m_deferQueue);
            break;
        }
        case UART_ACT_STOP_REQ: {
            LOG_EVENT(e);
            // defer event
            me->defer(&me->m_deferQueue, e);
            status = Q_HANDLED();
            break;
        }
        case UART_IN_STOP_CFM:
        case UART_OUT_STOP_CFM: {
            LOG_EVENT(e);
            me->HandleCfm(ERROR_EVT_CAST(*e), 2);
            status = Q_HANDLED();
            break;
        }
        case UART_ACT_FAIL:
        case UART_ACT_STATE_TIMER: {
            LOG_EVENT(e);
            Q_ASSERT(0);
            // Will not reach here.
            status = Q_HANDLED();
            break;
        }
        case UART_ACT_DONE: {
            LOG_EVENT(e);
            Evt *evt = new UartActStopCfm(me->m_savedInSeq, ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartAct::Stopped);
            break;
        }
        default: {
            status = Q_SUPER(&UartAct::Root);
            break;
        }
    }
    return status;
}

QState UartAct::Started(UartAct * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UartAct::Normal);
            break;
        }
        default: {
            status = Q_SUPER(&UartAct::Root);
            break;
        }
    }
    return status;
}

QState UartAct::Normal(UartAct * const me, QEvt const * const e) {
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
        case UART_IN_FAIL_IND:
        case UART_OUT_FAIL_IND: {
            LOG_EVENT(e);
            ErrorEvt const &ind = ERROR_EVT_CAST(*e);
            Evt *evt = new UartActFailInd(me->m_nextSequence++,
                                         ind.GetError(), ind.GetReason());
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&UartAct::Failed);
            break;
        }
        default: {
            status = Q_SUPER(&UartAct::Started);
            break;
        }
    }
    return status;
}

QState UartAct::Failed(UartAct * const me, QEvt const * const e) {
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
        default: {
            status = Q_SUPER(&UartAct::Started);
            break;
        }
    }
    return status;
}

void UartAct::HandleCfm(ErrorEvt const &e, uint8_t expectedCnt) {
    if (e.GetError() == ERROR_SUCCESS) {
        // TODO - Compare seqeuence number.
        if(++m_cfmCount == expectedCnt) {
            Evt *evt = new Evt(UART_ACT_DONE);
            postLIFO(evt);
        }
    } else {
        Evt *evt = new UartActFail(e.GetError(), e.GetReason());
        postLIFO(evt);
    }
}

/*
QState UartAct::MyState(UartAct * const me, QEvt const * const e) {
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
            status = Q_TRAN(&UartAct::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&UartAct::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP
