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
#include "RobotArmUserCtrl.h"
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
RobotArmUserCtrl::RobotArmUserCtrl() :
    QActive((QStateHandler)&RobotArmUserCtrl::InitialPseudoState),
    m_id(ROBOT_USER_CTRL),
    m_name("ROBOT_USER_CTRL"),
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
QState RobotArmUserCtrl::InitialPseudoState(RobotArmUserCtrl * const me, QEvt const * const e) {
    (void)e;

    me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));

    me->subscribe(ROBOT_USER_START_REQ);
    me->subscribe(ROBOT_USER_STOP_REQ);
    me->subscribe(UART_IN_CHAR_IND);

    return Q_TRAN(&RobotArmUserCtrl::Root);
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
QState RobotArmUserCtrl::Root(RobotArmUserCtrl * const me, QEvt const * const e) {
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
            status = Q_TRAN(&RobotArmUserCtrl::Stopped);
            break;
        }
        case ROBOT_USER_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new RobotUserStartCfm(req.GetSeq(), ERROR_STATE);
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
QState RobotArmUserCtrl::Stopped(RobotArmUserCtrl * const me, QEvt const * const e) {
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
        case ROBOT_USER_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new RobotUserStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case ROBOT_USER_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new RobotUserStartCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&RobotArmUserCtrl::Started);
            break;
        }
        default: {
            status = Q_SUPER(&RobotArmUserCtrl::Root);
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
QState RobotArmUserCtrl::Started(RobotArmUserCtrl * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            //me->PrintUsage();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case ROBOT_USER_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new RobotUserStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&RobotArmUserCtrl::Stopped);
            break;
        }
        case UART_IN_CHAR_IND: {
            UartInCharInd const &ind = static_cast<UartInCharInd const &>(*e);
            DEBUG("Rx char %c", ind.GetChar());
            char ch = ind.GetChar();
            switch(ch)
            {
//                case 'a':
//                {
//                    PRINT("  -- Robot Armed --\r\n");
//                    //TODO: Display led pattern for armed robot
//                    //TODO: Add indicators for event
//                    Evt *evt = new RobotArmStartReq();
//                    QF::PUBLISH(evt, me);
//                    break;
//                }
//                case 'k':
//                {
//                    PRINT("  -- Robot Disarmed --\r\n");
//                    //TODO: Add indicators for event
//                    Evt *evt = new RobotArmStopReq();
//                    QF::PUBLISH(evt, me);
//                    break;
//                }
                case 'o':
                {
                    PRINT(" -- Open Claw (Grabber) command  --\r\n");
                    //TODO: Add indicators for event
                    Evt *evt = new OpenClawReq(SERVO_CLAW);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'c':
                {
                    PRINT(" -- Close the claw(grabber) command--\r\n");
                    //TODO: Add indicators for event
                    Evt *evt = new CloseClawReq(SERVO_CLAW);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'f':
                {
                    PRINT(" -- Move arm forward command --\r\n");
                    //TODO: Add indicators for event
                    Evt *evt = new MoveForwardReq(SERVO_SWING);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'b':
                {
                    PRINT(" -- Move arm backward command --\r\n");
                    //TODO: Add indicators for event
                    Evt *evt = new MoveBackwardReq(SERVO_SWING);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'u':
                {
                    PRINT(" -- Move arm up command --\r\n");
                    //TODO: Add indicators for event
                    Evt *evt = new MoveUpReq(SERVO_PICK);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'd':
                {
                    PRINT(" -- Move arm down command --\r\n");
                    //TODO: Add indicators for event
                    Evt *evt = new MoveDownReq(SERVO_PICK);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'l':
                {
                    PRINT(" -- Rotate arm left command --\r\n");
                    //TODO: Add indicators for event
                    Evt *evt = new RotateLeftReq(SERVO_AXIS);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case 'r':
                {
                    PRINT(" -- Rotate arm right command --\r\n");
                    //TODO: Add indicators for event
                    Evt *evt = new RotateRightReq(SERVO_AXIS);
                    QF::PUBLISH(evt, me);
                    break;
                }
                case '?':
                {
                    me->PrintUsage();
                    break;
                }
            }
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&RobotArmUserCtrl::Root);
            break;
        }
    }
    return status;
}


/**
 * Print commands the user can use to simulate the washing machine operations.
 */
void RobotArmUserCtrl::PrintUsage()
{
    PRINT("\r\nRobotic arm control console. Use the following commands to control the Robotic Arm.\r\n");
//    PRINT("    a - (A)rm Robot\r\n");
//    PRINT("    k - Disarm Robot((k)ill switch)\r\n");
    PRINT("    o - (O)pens the claw(grabber)\r\n");
    PRINT("    c - (C)loses the claw(grabber)\r\n");
    PRINT("    f - Move arm (f)orward\r\n");
    PRINT("    b - Move arm (b)ackward\r\n");
    PRINT("    u - Move arm (u)p\r\n");
    PRINT("    d - Move arm (d)own\r\n");
    PRINT("    l - Rotate (l)eft\r\n");
    PRINT("    r - Rotate (r)ight\r\n");
    PRINT("    ? - Print this message\r\n\r\n");   
    
}

} // namespace APP

