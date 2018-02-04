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

#include "qpcpp.h"
#include "fw_log.h"
#include "fw_evt.h"
#include "hsm_id.h"
#include "System.h"
#include "event.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"
// Test only.
#include "Test.h"
#include "LedPattern.h"

/*
#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(format_, ...)
*/

Q_DEFINE_THIS_FILE

using namespace FW;
using namespace APP;

void LCD_Config(void);

namespace APP {

System::System() :
    QActive((QStateHandler)&System::InitialPseudoState), 
    m_id(SYSTEM), m_name("SYSTEM"), m_nextSequence(0),
    m_uart2OutFifo(m_uart2OutFifoStor, UART_OUT_FIFO_ORDER),
    m_uart2InFifo(m_uart2InFifoStor, UART_IN_FIFO_ORDER),
    m_stateTimer(this, SYSTEM_STATE_TIMER),
    m_testTimer(this, SYSTEM_TEST_TIMER) {}

QState System::InitialPseudoState(System * const me, QEvt const * const e) {
    (void)e;
    me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));

    me->subscribe(SYSTEM_START_REQ);
    me->subscribe(SYSTEM_STOP_REQ);
    me->subscribe(SYSTEM_STATE_TIMER);
    me->subscribe(SYSTEM_TEST_TIMER);
    me->subscribe(SYSTEM_DONE);
    me->subscribe(SYSTEM_FAIL);
    me->subscribe(UART_ACT_START_CFM);
    me->subscribe(UART_IN_CHAR_IND);
    me->subscribe(USER_BTN_START_CFM);
    me->subscribe(USER_BTN_UP_IND);
    me->subscribe(USER_BTN_DOWN_IND);
    me->subscribe(USER_LED_START_CFM);
    me->subscribe(USER_LED_ON_CFM);
    me->subscribe(USER_LED_OFF_CFM);
    me->subscribe(ROBOT_USER_START_CFM);
    me->subscribe(ROBOT_USER_STOP_CFM);
    me->subscribe(ROBOTARM_START_CFM);
    me->subscribe(ROBOTARM_STOP_CFM);

    return Q_TRAN(&System::Root);
}

QState System::Root(System * const me, QEvt const * const e) {
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
            status = Q_TRAN(&System::Stopped);
            break;
        }
        case SYSTEM_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new SystemStartCfm(req.GetSeq(), ERROR_STATE);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            me->m_savedInSeq = req.GetSeq();
            status = Q_TRAN(&System::Stopping2);
            break;
        }
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState System::Stopped(System * const me, QEvt const * const e) {
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
        case SYSTEM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new SystemStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_START_REQ: {
            LOG_EVENT(e);
            SystemStartReq const &req = static_cast<SystemStartReq const &>(*e);
            me->m_savedInSeq = req.GetSeq();
            status = Q_TRAN(&System::Starting1);
            break;
        }
        default: {
            status = Q_SUPER(&System::Root);
            break;
        }
    }
    return status;
}

QState System::Starting1(System * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            uint32_t timeout = SystemStartReq::TIMEOUT_MS / 2;
            Q_ASSERT(timeout > UartActStartReq::TIMEOUT_MS);
            me->m_stateTimer.armX(timeout);
            me->m_cfmCount = 0;
            
            Evt *evt = new UartActStartReq(me->m_nextSequence++, &me->m_uart2OutFifo, &me->m_uart2InFifo);
            // TODO - Save sequence for comparison.
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
        case UART_ACT_START_CFM: {
            LOG_EVENT(e);
            me->HandleCfm(ERROR_EVT_CAST(*e), 1);
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_FAIL:
        case SYSTEM_STATE_TIMER: {
            LOG_EVENT(e);
            Evt *evt;
            if (e->sig == SYSTEM_FAIL) {
                ErrorEvt const &fail = ERROR_EVT_CAST(*e);
                evt = new SystemStartCfm(me->m_savedInSeq,
                                         fail.GetError(), fail.GetReason());
            } else {
                evt = new SystemStartCfm(me->m_savedInSeq, ERROR_TIMEOUT);
            }
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&System::Stopping1);
            break;
        }
        case SYSTEM_DONE: {
            LOG_EVENT(e);
            Log::AddInterface(&me->m_uart2OutFifo, UART_OUT_WRITE_REQ);
            status = Q_TRAN(&System::Starting2);
            break;
        }
        default: {
            status = Q_SUPER(&System::Root);
            break;
        }
    }
    return status;
}

QState System::Starting2(System * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            uint32_t timeout = SystemStartReq::TIMEOUT_MS / 2;
            Q_ASSERT(timeout > UserLedStartReq::TIMEOUT_MS);
            Q_ASSERT(timeout > UserBtnStartReq::TIMEOUT_MS);
            Q_ASSERT(timeout > RobotUserStartReq::TIMEOUT_MS);
            Q_ASSERT(timeout > RobotArmStartReq::TIMEOUT_MS);
            me->m_stateTimer.armX(timeout);
            me->m_cfmCount = 0;
            
            Evt *evt = new UserLedStartReq(me->m_nextSequence++);
            // TODO - Save sequence number for comparison.
            QF::PUBLISH(evt, me);
            evt = new UserBtnStartReq(me->m_nextSequence++);
            // TODO - Save sequence number for comparison.
            QF::PUBLISH(evt, me);
             evt = new RobotUserStartReq(me->m_nextSequence++);
            // TODO - Save sequence number for comparison.
            QF::PUBLISH(evt, me);
            evt = new RobotArmStartReq();
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
        case USER_LED_START_CFM:
        case USER_BTN_START_CFM:
        case ROBOT_USER_START_CFM:
        case ROBOTARM_START_CFM: {
            LOG_EVENT(e);
            me->HandleCfm(ERROR_EVT_CAST(*e), 3);       
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_FAIL:
        case SYSTEM_STATE_TIMER: {
            LOG_EVENT(e);
            Evt *evt;
            if (e->sig == SYSTEM_FAIL) {
                ErrorEvt const &fail = ERROR_EVT_CAST(*e);
                evt = new SystemStartCfm(me->m_savedInSeq,
                                         fail.GetError(), fail.GetReason());
            } else {
                evt = new SystemStartCfm(me->m_savedInSeq, ERROR_TIMEOUT);
            }
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&System::Stopping2);
            break;
        }
        case SYSTEM_DONE: {
            LOG_EVENT(e);
            Evt *evt = new SystemStartCfm(me->m_savedInSeq, ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&System::Started);
            break;
        }
        default: {
            status = Q_SUPER(&System::Root);
            break;
        }
    }
    return status;
}

QState System::Stopping1(System * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            uint32_t timeout = SystemStopReq::TIMEOUT_MS / 2;
            Q_ASSERT(timeout > UartActStopReq::TIMEOUT_MS);
            me->m_stateTimer.armX(timeout);
            me->m_cfmCount = 0;

            Log::DeleteInterface();            
            Evt *evt = new UartActStopReq(me->m_nextSequence++);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            me->m_stateTimer.disarm();
            // recall event
            me->recall(&me->m_deferQueue);
            status = Q_HANDLED();            
            break;
        }
        case SYSTEM_STOP_REQ: {
            LOG_EVENT(e);
            // defer event
            me->defer(&me->m_deferQueue, e);
            status = Q_HANDLED();
            break;
        }
        case UART_ACT_STOP_CFM: {
            LOG_EVENT(e);
            me->HandleCfm(ERROR_EVT_CAST(*e), 1);
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_FAIL:
        case SYSTEM_STATE_TIMER: {
            LOG_EVENT(e);
            Q_ASSERT(0);
            // Will not reach here.
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_DONE: {
            LOG_EVENT(e);
            Evt *evt = new SystemStopCfm(me->m_savedInSeq, ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&System::Stopped);
            break;
        }
        default: {
            status = Q_SUPER(&System::Root);
            break;
        }
    }
    return status;
}

QState System::Stopping2(System * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            uint32_t timeout = SystemStopReq::TIMEOUT_MS / 2;
            Q_ASSERT(timeout > UserLedStopReq::TIMEOUT_MS);
            Q_ASSERT(timeout > UserBtnStopReq::TIMEOUT_MS);
            Q_ASSERT(timeout > RobotUserStopReq::TIMEOUT_MS);
            Q_ASSERT(timeout > RobotArmStopReq::TIMEOUT_MS);
            me->m_stateTimer.armX(timeout);
            me->m_cfmCount = 0;

            Evt *evt = new UserLedStopReq(me->m_nextSequence++);
            QF::PUBLISH(evt, me);
            evt = new UserBtnStopReq(me->m_nextSequence++);
            QF::PUBLISH(evt, me);
            evt = new RobotUserStopReq(me->m_nextSequence++);
            QF::PUBLISH(evt, me);
            
            evt = new RobotArmStopReq();
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            me->m_stateTimer.disarm();
            // recall event
            me->recall(&me->m_deferQueue);
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_STOP_REQ: {
            LOG_EVENT(e);
            // defer event
            me->defer(&me->m_deferQueue, e);
            status = Q_HANDLED();
            break;
        }
        case USER_LED_STOP_CFM:
        case USER_BTN_STOP_CFM:
        case ROBOT_USER_STOP_CFM:
        case ROBOTARM_STOP_CFM: {
            LOG_EVENT(e);
            me->HandleCfm(ERROR_EVT_CAST(*e), 3);
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_FAIL:
        case SYSTEM_STATE_TIMER: {
            LOG_EVENT(e);
            Q_ASSERT(0);
            // Will not reach here.
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_DONE: {
            LOG_EVENT(e);
            status = Q_TRAN(&System::Stopping1);
            break;
        }
        default: {
            status = Q_SUPER(&System::Root);
            break;
        }
    }
    return status;
}

QState System::Started(System * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            //me->m_testTimer.armX(2000, 2000);
            Evt *evt = new UserLedPatternReq(me->m_nextSequence++, 0, true, 0);
            //Evt *evt = new UserLedPatternReq(me->m_nextSequence++, 0, false, 0);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            // Test only.
            me->m_testTimer.disarm();
            status = Q_HANDLED();
            break;
        }
        case SYSTEM_TEST_TIMER: {
            //LOG_EVENT(e);          
            /*
            static int testcount = 10000;
            char msg[100];
            snprintf(msg, sizeof(msg), "This is a UART DMA transmission testing number %d.", testcount++);
            DEBUG("Writing %s", msg);
            */
            
            status = Q_HANDLED();
            break;
        }
        case USER_BTN_UP_IND: {
            LOG_EVENT(e);
            //Evt *evt = new UserLedOffReq(me->m_nextSequence++);
            //QF::PUBLISH(evt, me);
            Evt *evt = new UserLedOffReq(me->m_nextSequence++, 1);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;  
            }
        case USER_BTN_DOWN_IND: {
            LOG_EVENT(e);            
            //Evt *evt = new UserLedOnReq(me->m_nextSequence++);
            //Evt *evt = new UserLedPatternReq(me->m_nextSequence++, 1, false, 1);
            Evt *evt = new UserLedPatternReq(me->m_nextSequence++, 1, true, 1);
            QF::PUBLISH(evt, me);
            //evt = new UserLedOffReq(me->m_nextSequence++, 0);
            //QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;  
        }
        case USER_LED_ON_CFM: 
        case USER_LED_PATTERN_CFM: 
        case USER_LED_OFF_CFM: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;    
        }
//        case UART_IN_CHAR_IND: {
//            UartInCharInd const &ind = static_cast<UartInCharInd const &>(*e);
//            //DEBUG("Rx char %c", ind.GetChar());
//            
//            // Week 2 C++ test examples.
//            //TestFunction();
//            // LedPattern testing.
//            /*
//            uint32_t patternIdx, intervalIdx;
//            for (patternIdx = 0; patternIdx < TEST_LED_PATTERN_SET.GetCount(); patternIdx++) {
//                DEBUG("LED Pattern %d", patternIdx);
//                LedPattern const &pattern = TEST_LED_PATTERN_SET.GetPattern(patternIdx);
//                for (intervalIdx = 0; intervalIdx < pattern.GetCount(); intervalIdx++) {
//                    LedInterval const &interval = pattern.GetInterval(intervalIdx);
//                    DEBUG("    interval[%d] = {%d, %d}", intervalIdx, interval.GetLevelPermil(), interval.GetDurationMs());
//                }
//            } 
//            */
//            status = Q_HANDLED();
//            break;
//        }
        default: {
            status = Q_SUPER(&System::Root);
            break;
        }
    }
    return status;
}

void System::HandleCfm(ErrorEvt const &e, uint8_t expectedCnt) {
    if (e.GetError() == ERROR_SUCCESS) {
        // TODO - Compare seqeuence number.
        if(++m_cfmCount == expectedCnt) {
            Evt *evt = new Evt(SYSTEM_DONE);
            postLIFO(evt);
        }
    } else {
        Evt *evt = new SystemFail(e.GetError(), e.GetReason());
        postLIFO(evt);
    }
}

} // namespace APP
