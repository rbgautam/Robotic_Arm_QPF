/*******************************************************************************
 * Copyright (C) Lawrence Lo (https://github.com/galliumstudio).
 * All rights reserved.
 *
 * Author: Katie Elliott
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
#include "AOUserCtrlSimulator.h"
#include "event.h"

#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(x, y)


//Q_DEFINE_THIS_FILE

using namespace FW;
using namespace APP;


namespace APP {

/**
 ******************************************************************************
 *
 * Class constructor
 *
 ******************************************************************************
 */
AOUserCtrlSimulator::AOUserCtrlSimulator() :
    QActive((QStateHandler)&AOUserCtrlSimulator::InitialPseudoState),
    m_id(USER_SIM),
    m_name("USER_CTRL_SIM"),
    m_nextSequence(0)
    {}

/**
 * The InitialPseudoState is the top level state before transitioning
 * into the the first user state. Here we subscribe to events of
 * interest to this active object.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_RET_TRAN
 */
QState AOUserCtrlSimulator::InitialPseudoState(AOUserCtrlSimulator * const me, QEvt const * const e) {
    (void)e;

    me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));

    me->subscribe(USER_SIM_START_REQ);
    me->subscribe(USER_SIM_STOP_REQ);
    me->subscribe(UART_IN_CHAR_IND);

    return Q_TRAN(&AOUserCtrlSimulator::Root);
}

/**
 * This is the root state of the active object. Note that there is a hidden
 * top state above it provided by the base class.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOUserCtrlSimulator::Root(AOUserCtrlSimulator * const me, QEvt const * const e) {
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
            status = Q_TRAN(&AOUserCtrlSimulator::Stopped);
            break;
        }
        case USER_SIM_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserSimStartCfm(req.GetSeq(), ERROR_STATE);
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

/**
 * This is the state where the state machine waits to be started by the system manager.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOUserCtrlSimulator::Stopped(AOUserCtrlSimulator * const me, QEvt const * const e) {
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
        case USER_SIM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserSimStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case USER_SIM_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserSimStartCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&AOUserCtrlSimulator::Started);
            break;
        }
        default: {
            status = Q_SUPER(&AOUserCtrlSimulator::Root);
            break;
        }
    }
    return status;
}

/**
 * In the Started state, the state machine is ready for user input, which will 
 * simulate use of the washing machine.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOUserCtrlSimulator::Started(AOUserCtrlSimulator * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            me->PrintCommands();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case USER_SIM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new UserSimStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&AOUserCtrlSimulator::Stopped);
            break;
        }
        case UART_IN_CHAR_IND: {
            UartInCharInd const &ind = static_cast<UartInCharInd const &>(*e);
            DEBUG("Rx char %c", ind.GetChar());
            char ch = ind.GetChar();
            switch(ch)
            {
                case 'o':
                {
                    PRINT("  -- Open door selected --\r\n");
                    Evt *evt = new UserSimOpenDoorInd();
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'c':
                {
                    PRINT("  -- Close door selected --\r\n");
                    Evt *evt = new UserSimCloseDoorInd();
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 's':
                {
                    PRINT(" -- Start/Pause button pressed --\r\n");
                    Evt *evt = new UserSimStartPauseInd();
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'n':
                {
                    PRINT(" -- NORMAL cycle selected --\r\n");
                    Evt *evt = new UserSimCycleSelectedInd(UserSimCycleSelectedInd::NORMAL);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'd':
                {
                    PRINT(" -- DELICATE cycle selected --\r\n");
                    Evt *evt = new UserSimCycleSelectedInd(UserSimCycleSelectedInd::DELICATE);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'b':
                {
                    PRINT(" -- BULKY cycle selected --\r\n");
                    Evt *evt = new UserSimCycleSelectedInd(UserSimCycleSelectedInd::BULKY);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 't':
                {
                    PRINT(" -- TOWEL cycle selected --\r\n");
                    Evt *evt = new UserSimCycleSelectedInd(UserSimCycleSelectedInd::TOWELS);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case '?':
                {
                    me->PrintCommands();
                    break;
                }
            }
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&AOUserCtrlSimulator::Root);
            break;
        }
    }
    return status;
}


/**
 * Print commands the user can use to simulate the washing machine operations.
 */
void AOUserCtrlSimulator::PrintCommands()
{
    PRINT("\r\nWashing machine simulator. Use the following commands to simulate washing.\r\n");
    PRINT("    o - Opens the door\r\n");
    PRINT("    c - Closes the door\r\n");
    PRINT("    s - Start/Pause button\r\n");
    PRINT("    n - Select wash NORMAL\r\n");
    PRINT("    d - Select wash DELICATE\r\n");
    PRINT("    b - Select wash BULKY\r\n");
    PRINT("    t - Select wash TOWELS\r\n");
    PRINT("    ? - Print this message\r\n\r\n");
    
}

} // namespace APP

