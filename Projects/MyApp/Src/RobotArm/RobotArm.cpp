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
#include "RobotArm.h"
#include "event.h"

/*
#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(x, y)
*/

Q_DEFINE_THIS_FILE

namespace APP {
  
void RobotArm::PrintUsage() {
    // Print usage.
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

    
     


RobotArm::RobotArm() :
    QActive((QStateHandler)&RobotArm::InitialPseudoState),
    m_id(ROBOTARM), m_name("RobotArm"),
    m_motorClaw(SERVO_CLAW,"SERVO_CLAW",this, SERVO1),
    m_motorSwing(SERVO_SWING,"SERVO_SWING",this, SERVO2),
    m_motorPick(SERVO_PICK,"SERVO_PICK",this, SERVO3),
    m_motorAxis(SERVO_AXIS,"SERVO_AXIS",this, SERVO4),
    m_waitTimer(this, ROBOTARM_WAIT_TO)
    {}

QState RobotArm::InitialPseudoState(RobotArm * const me, QEvt const * const e) {
    (void)e;
    me->subscribe(UART_IN_CHAR_IND);
    me->subscribe(ROBOTARM_START_REQ);
    me->subscribe(ROBOTARM_STOP_REQ);
    me->subscribe(ROBOTARM_MOTOR_AXIS_REQ);
    me->subscribe(ROBOTARM_MOTOR_SWING_REQ);
    me->subscribe(ROBOTARM_MOTOR_CLAW_REQ);        
    me->subscribe(ROBOTARM_MOTOR_PICK_REQ);         
    me->subscribe(ROBOTARM_WAIT_TO);   
    me->subscribe(MOTOR_FORWARD_REQ);      
    me->subscribe(MOTOR_STOP_REQ);   
    me->subscribe(MOTOR_BACKWARD_REQ);    
    
    return Q_TRAN(&RobotArm::Root);
}

QState RobotArm::Root(RobotArm * const me, QEvt const * const e) {
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
            LOG_EVENT(e);
            status = Q_TRAN(&RobotArm::Stopped);
            break;
        }
        case ROBOTARM_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new RobotArmStartCfm(req.GetSeq(), ERROR_STATE);
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

QState RobotArm::Stopped(RobotArm * const me, QEvt const * const e) {
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
        case ROBOTARM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new RobotArmStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case ROBOTARM_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new RobotArmStartCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&RobotArm::Started);
            break;
        }
        default: {
            status = Q_SUPER(&RobotArm::Root);
            break;
        }
    }
    return status;
}

QState RobotArm::Started(RobotArm * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);       
            status = Q_HANDLED();
       
            // Initialize regions.
            me->m_motorClaw.Init();
            me->m_motorSwing.Init();
            me->m_motorPick.Init();
            me->m_motorAxis.Init();
            
            me->PrintUsage();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
      
        case ROBOTARM_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new RobotArmStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&RobotArm::Stopped);
            break;
        }
        // Dispatch events to orthogonal regions.
        case MOTOR_FORWARD_REQ:
        case MOTOR_STOP_REQ:
        case MOTOR_BACKWARD_REQ: {
            MotorReq const &req = static_cast<MotorReq const &>(*e);
            
    
            if (req.GetMotorId() == SERVO_AXIS) {
                me->m_motorAxis.dispatch(e);
            } else if (req.GetMotorId() == SERVO_SWING) {
                me->m_motorSwing.dispatch(e);
            } else if (req.GetMotorId() == SERVO_PICK) {
                me->m_motorPick.dispatch(e);
            }else if (req.GetMotorId() == SERVO_CLAW) {
                me->m_motorClaw.dispatch(e);
            }else {
                Q_ASSERT(0);
            }
            status = Q_HANDLED();
            break;
        }
        
        
        
        default: {
            status = Q_SUPER(&RobotArm::Root);
            break;
        }
    }
    return status;
}





/*
QState RobotArm::MyState(RobotArm * const me, QEvt const * const e) {
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
            LOG_EVENT(e);
            status = Q_TRAN(&RobotArm::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&RobotArm::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP
