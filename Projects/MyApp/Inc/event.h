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

#ifndef EVENT_H
#define EVENT_H

#include "qpcpp.h"
#include "fw_error.h"
#include "fw_evt.h"
#include "fw_pipe.h"

using namespace FW;

namespace APP {

enum {
    SYSTEM_START_REQ = QP::Q_USER_SIG,
    SYSTEM_START_CFM,
    SYSTEM_STOP_REQ,
    SYSTEM_STOP_CFM,
    SYSTEM_STATE_TIMER,
    SYSTEM_TEST_TIMER,
    SYSTEM_DONE,
    SYSTEM_FAIL,
    
    UART_ACT_START_REQ,
    UART_ACT_START_CFM,
    UART_ACT_STOP_REQ,
    UART_ACT_STOP_CFM,
    UART_ACT_FAIL_IND,
    UART_ACT_STATE_TIMER,
    UART_ACT_START,
    UART_ACT_DONE,
    UART_ACT_FAIL,
    
    UART_OUT_START_REQ,
    UART_OUT_START_CFM,
    UART_OUT_STOP_REQ,
    UART_OUT_STOP_CFM,
    UART_OUT_FAIL_IND,
    UART_OUT_WRITE_REQ,     // of type Evt
    UART_OUT_WRITE_CFM,     // of type ErrorEvt
    UART_OUT_EMPTY_IND,     // of type Evt
    UART_OUT_ACTIVE_TIMER,
    UART_OUT_DONE,
    UART_OUT_DMA_DONE,
    UART_OUT_CONTINUE,
    UART_OUT_HW_FAIL,
    
    UART_IN_START_REQ,
    UART_IN_START_CFM,
    UART_IN_STOP_REQ,
    UART_IN_STOP_CFM,
    UART_IN_CHAR_IND,   // For testing only (by-passing fifo)
    UART_IN_FAIL_IND,
    UART_IN_STATE_TIMER,
    UART_IN_DONE,
    UART_IN_DATA_RDY,   // of type Evt
    
    USER_BTN_START_REQ,
    USER_BTN_START_CFM,
    USER_BTN_STOP_REQ,
    USER_BTN_STOP_CFM,
    USER_BTN_UP_IND,    // of type Evt
    USER_BTN_DOWN_IND,  // of type Evt
    USER_BTN_STATE_TIMER,
    USER_BTN_TRIG,
    USER_BTN_UP,
    USER_BTN_DOWN,
    
    USER_LED_START_REQ,
    USER_LED_START_CFM,
    USER_LED_STOP_REQ,
    USER_LED_STOP_CFM,
    USER_LED_ON_REQ,            // Deprecated
    USER_LED_ON_CFM,            // Deprecated
    USER_LED_PATTERN_REQ,
    USER_LED_PATTERN_CFM,
    USER_LED_OFF_REQ,
    USER_LED_OFF_CFM,   
    USER_LED_INTERVAL_TIMER,
    USER_LED_DONE,
    USER_LED_NEXT_INTERVAL,
    USER_LED_LAST_INTERVAL,
    USER_LED_LOAD_PATTERN,
    USER_LED_PATTERN_END,
    USER_LED_NEW_PATTERN,

    // Washing machine control simulator
    USER_SIM_START_REQ,
    USER_SIM_START_CFM,
    USER_SIM_STOP_REQ,
    USER_SIM_STOP_CFM,
    OPEN_DOOR_IND,
    CLOSE_DOOR_IND,
    START_PAUSE_BUTTON_IND,
    CYCLE_SELECTED_IND,

    // Washing machine
    WASH_START_REQ,
    WASH_START_CFM,
    WASH_STOP_REQ,
    WASH_STOP_CFM,

    // Internal washing machine events
    iWASH_START,
    iWASH_CLOSE,
    iWASH_OPEN,
    iWASH_DONE,
    WASH_TIMEOUT,
    RINSE_TIMEOUT,
    SPIN_TIMEOUT,


    // Washing machine sensor events
    WASH_FILLED_IND,
    WASH_DRAINED_IND,

    // Demo
    DEMO_A,
    DEMO_B,
    DEMO_C,
    DEMO_D,    
    DEMO_E,
    DEMO_F,
    DEMO_G,
    DEMO_H,
    DEMO_I,
    
    // Traffic
    TRAFFIC_START_REQ,
    TRAFFIC_START_CFM,
    TRAFFIC_STOP_REQ,
    TRAFFIC_STOP_CFM,
    TRAFFIC_CAR_NS_REQ,         // of type Evt (no CFM)
    TRAFFIC_CAR_EW_REQ,         // of type Evt (no CFM)
    TRAFFIC_WAIT_TO,            // of type QTimeEvt
    //Lamp
    LAMP_RED_REQ,               // of type LampRedReq (no CFM)
    LAMP_YELLOW_REQ,            // of type LampYellowReq (no CFM)
    LAMP_GREEN_REQ,             // of type LampGreenReq (no CFM)
    
     // RobotArm
    ROBOT_USER_START_REQ,
    ROBOT_USER_START_CFM,
    
    ROBOT_USER_STOP_REQ,
    ROBOT_USER_STOP_CFM,
    ROBOTARM_START_REQ,
    ROBOTARM_START_CFM,
    ROBOTARM_STOP_REQ,
    ROBOTARM_STOP_CFM,
    //tODO/; REMOVE THE UNNEEDED EVENTS
    ROBOTARM_MOTOR_AXIS_REQ,         // of type Evt (no CFM)
    ROBOTARM_MOTOR_SWING_REQ,         // of type Evt (no CFM)
    ROBOTARM_MOTOR_CLAW_REQ,         // of type Evt (no CFM)
    ROBOTARM_MOTOR_PICK_REQ,         // of type Evt (no CFM)
    
    DISARM_ROBOT_SIG,
    ARM_ROBOT_SIG,
    OPEN_CLAW_SIG,
    CLOSE_CLAW_SIG,
    MOVE_FORWARD_SIG,
    MOVE_BACKWARD_SIG,
    MOVE_UP_SIG,
    MOVE_DOWN_SIG,
    ROTATE_LEFT_SIG,
    ROTATE_RIGHT_SIG,
    
    
    
    
    
    ROBOTARM_WAIT_TO,            // of type QTimeEvt

   
    
    //Motor
    MOTOR_FORWARD_REQ,
    MOTOR_STOP_REQ,
    MOTOR_BACKWARD_REQ,
    
    MAX_PUB_SIG
};

char const * GetEvtName(QP::QSignal sig);

class SystemStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 600
    };
    SystemStartReq(uint16_t seq) :
        Evt(SYSTEM_START_REQ, seq) {}
};

class SystemStartCfm : public ErrorEvt {
public:
    SystemStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(SYSTEM_START_CFM, seq, error, reason) {}
};

class SystemStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 600
    };
    SystemStopReq(uint16_t seq) :
        Evt(SYSTEM_STOP_REQ, seq) {}
};

class SystemStopCfm : public ErrorEvt {
public:
    SystemStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(SYSTEM_STOP_CFM, seq, error, reason) {}
};

class SystemFail : public ErrorEvt {
public:
    SystemFail(Error error, Reason reason) :
        ErrorEvt(SYSTEM_FAIL, 0, error, reason) {}
};

class UartActStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 200
    };
    UartActStartReq(uint16_t seq, Fifo *outFifo, Fifo *inFifo) :
        Evt(UART_ACT_START_REQ, seq), m_outFifo(outFifo), m_inFifo(inFifo) {}
    Fifo *GetOutFifo() const { return m_outFifo; }
    Fifo *GetInFifo() const { return m_inFifo; }
private:
    Fifo *m_outFifo;
    Fifo *m_inFifo;
};

class UartActStartCfm : public ErrorEvt {
public:
    UartActStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_ACT_START_CFM, seq, error, reason) {}
};

class UartActStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 200
    };
    UartActStopReq(uint16_t seq) :
        Evt(UART_ACT_STOP_REQ, seq) {}
};

class UartActStopCfm : public ErrorEvt {
public:
    UartActStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_ACT_STOP_CFM, seq, error, reason) {}
};

class UartActFailInd : public ErrorEvt {
public:
    UartActFailInd(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_ACT_FAIL_IND, seq, error, reason) {}
};

class UartActFail : public ErrorEvt {
public:
    UartActFail(Error error, Reason reason) :
        ErrorEvt(UART_ACT_FAIL, 0, error, reason) {}
};

class UartOutStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UartOutStartReq(uint16_t seq, Fifo *fifo) :
        Evt(UART_OUT_START_REQ, seq), m_fifo(fifo) {}
    Fifo *GetFifo() const { return m_fifo; }
private:
    Fifo *m_fifo;
};

class UartOutStartCfm : public ErrorEvt {
public:
    UartOutStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_OUT_START_CFM, seq, error, reason) {}
};

class UartOutStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UartOutStopReq(uint16_t seq) :
        Evt(UART_OUT_STOP_REQ, seq) {}
};

class UartOutStopCfm : public ErrorEvt {
public:
    UartOutStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_OUT_STOP_CFM, seq, error, reason) {}
};

class UartOutFailInd : public ErrorEvt {
public:
    UartOutFailInd(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_OUT_FAIL_IND, seq, error, reason) {}
};

class UartInStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UartInStartReq(uint16_t seq, Fifo *fifo) :
        Evt(UART_IN_START_REQ, seq), m_fifo(fifo) {}
    Fifo *GetFifo() const { return m_fifo; }
private:
    Fifo *m_fifo;
};

class UartInStartCfm : public ErrorEvt {
public:
    UartInStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_IN_START_CFM, seq, error, reason) {}
};

class UartInStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UartInStopReq(uint16_t seq) :
        Evt(UART_IN_STOP_REQ, seq) {}
};

class UartInStopCfm : public ErrorEvt {
public:
    UartInStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_IN_STOP_CFM, seq, error, reason) {}
};

class UartInCharInd : public Evt {
public:
    UartInCharInd(uint16_t seq, char ch) :
        Evt(UART_IN_CHAR_IND, seq), m_ch(ch) {}
    char GetChar() const { return m_ch; }
private:
    char m_ch;
};

class UartInFailInd : public ErrorEvt {
public:
    UartInFailInd(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(UART_IN_FAIL_IND, seq, error, reason) {}
};

class UserBtnStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserBtnStartReq(uint16_t seq) :
        Evt(USER_BTN_START_REQ, seq) {}
};

class UserBtnStartCfm : public ErrorEvt {
public:
    UserBtnStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_BTN_START_CFM, seq, error, reason) {}
};

class UserBtnStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserBtnStopReq(uint16_t seq) :
        Evt(USER_BTN_STOP_REQ, seq) {}
};

class UserBtnStopCfm : public ErrorEvt {
public:
    UserBtnStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_BTN_STOP_CFM, seq, error, reason) {}
};

class UserLedStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserLedStartReq(uint16_t seq) :
        Evt(USER_LED_START_REQ, seq) {}
};

class UserLedStartCfm : public ErrorEvt {
public:
    UserLedStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_LED_START_CFM, seq, error, reason) {}
};

class UserLedStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserLedStopReq(uint16_t seq) :
        Evt(USER_LED_STOP_REQ, seq) {}
};

class UserLedStopCfm : public ErrorEvt {
public:
    UserLedStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_LED_STOP_CFM, seq, error, reason) {}
};

class UserLedOnReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserLedOnReq(uint16_t seq) :
        Evt(USER_LED_ON_REQ, seq) {}
};

class UserLedOnCfm : public ErrorEvt {
public:
    UserLedOnCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_LED_ON_CFM, seq, error, reason) {}
};

class UserLedPatternReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserLedPatternReq(uint16_t seq, uint32_t patternIndex, bool isRepeat = false, uint32_t layer = 0) :
        Evt(USER_LED_PATTERN_REQ, seq), m_patternIndex(patternIndex),
        m_isRepeat(isRepeat), m_layer(layer) {}
        
    uint32_t GetPatternIndex() const { return m_patternIndex; }
    bool IsRepeat() const { return m_isRepeat; }
    uint32_t GetLayer() const { return m_layer; }
private:
    uint32_t m_patternIndex;
    bool m_isRepeat;
    uint32_t m_layer;
};

class UserLedPatternCfm : public ErrorEvt {
public:
    UserLedPatternCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_LED_PATTERN_CFM, seq, error, reason) {}
};

class UserLedOffReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserLedOffReq(uint16_t seq, uint32_t layer) :
        Evt(USER_LED_OFF_REQ, seq),  m_layer(layer) {}
    uint32_t GetLayer() const { return m_layer; }        
private:
    uint32_t m_layer;        
};

class UserLedOffCfm : public ErrorEvt {
public:
    UserLedOffCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_LED_OFF_CFM, seq, error, reason) {}
};
   
class UserLedNewPattern: public Evt {
public:
    UserLedNewPattern(uint32_t layer) :
        Evt(USER_LED_NEW_PATTERN, 0), m_layer(layer) {}
    uint32_t GetLayer() const { return m_layer; }
private:
    uint32_t m_layer;
};


class UserSimStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserSimStartReq(uint16_t seq) :
        Evt(USER_SIM_START_REQ, seq) {}
};
 
class UserSimStartCfm : public ErrorEvt {
public:
    UserSimStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_SIM_START_CFM, seq, error, reason) {}
};

class UserSimStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    UserSimStopReq(uint16_t seq) :
        Evt(USER_SIM_STOP_REQ, seq) {}
};

class UserSimStopCfm : public ErrorEvt {
public:
    UserSimStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(USER_SIM_STOP_CFM, seq, error, reason) {}
};


class UserSimOpenDoorInd : public Evt {
public:
    UserSimOpenDoorInd() :
        Evt(OPEN_DOOR_IND) {}
};

class UserSimCloseDoorInd : public Evt {
public:
    UserSimCloseDoorInd() :
        Evt(CLOSE_DOOR_IND) {}
};

class UserSimStartPauseInd : public Evt {
public:
    UserSimStartPauseInd() :
        Evt(START_PAUSE_BUTTON_IND) {}
};

class UserSimCycleSelectedInd : public Evt {
public:
    typedef enum {
        NORMAL,
        DELICATE,
        BULKY,
        TOWELS
    } CycleType;
    UserSimCycleSelectedInd(CycleType cycle) :
        Evt(CYCLE_SELECTED_IND), m_cycleType(cycle) {}
    CycleType GetCycleType() const { return m_cycleType; }
private:
    CycleType m_cycleType;
};

class WashStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    WashStartReq(uint16_t seq) :
        Evt(WASH_START_REQ, seq) {}
};

class WashStartCfm : public ErrorEvt {
public:
    WashStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(WASH_START_CFM, seq, error, reason) {}
};

class WashStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    WashStopReq(uint16_t seq) :
        Evt(WASH_STOP_REQ, seq) {}
};

class WashStopCfm : public ErrorEvt {
public:
    WashStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(WASH_STOP_CFM, seq, error, reason) {}
};

class TrafficStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    TrafficStartReq(uint16_t seq) :
        Evt(TRAFFIC_START_REQ, seq) {}
};

class TrafficStartCfm : public ErrorEvt {
public:
    TrafficStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(TRAFFIC_START_CFM, seq, error, reason) {}
};

class TrafficStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    TrafficStopReq(uint16_t seq) :
        Evt(TRAFFIC_STOP_REQ, seq) {}
};

class TrafficStopCfm : public ErrorEvt {
public:
    TrafficStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(TRAFFIC_STOP_CFM, seq, error, reason) {}
};

class LampReq : public Evt {
public:
    LampReq(QP::QSignal sig, uint8_t lampId) :
        Evt(sig, 0), m_lampId(lampId) {}
        uint8_t GetLampId() const { return m_lampId; }
private:
    uint8_t m_lampId;   // either LAMP_NS or LAMP_EW
};

class LampRedReq : public LampReq {
public:
    LampRedReq(uint8_t lampId) :
        LampReq(LAMP_RED_REQ, lampId) {}
};

class LampYellowReq : public LampReq {
public:
    LampYellowReq(uint8_t lampId) :
        LampReq(LAMP_YELLOW_REQ, lampId) {}
};

class LampGreenReq : public LampReq {
public:
    LampGreenReq(uint8_t lampId) :
        LampReq(LAMP_GREEN_REQ, lampId) {}
};

class RobotArmStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    RobotArmStartReq() :
        Evt(ROBOTARM_START_REQ) {}
};

class RobotArmStartCfm : public ErrorEvt {
public:
    RobotArmStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(ROBOTARM_START_CFM, seq, error, reason) {}
};

class RobotArmStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    RobotArmStopReq() :
        Evt(ROBOTARM_STOP_REQ) {}
};

class RobotArmStopCfm : public ErrorEvt {
public:
    RobotArmStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(ROBOTARM_STOP_CFM, seq, error, reason) {}
};

class MotorReq : public Evt {
public:
    MotorReq(QP::QSignal sig, uint8_t motorId) :
        Evt(sig, 0), m_motorId(motorId) {}
        uint8_t GetMotorId() const { return m_motorId; }
private:
    uint8_t m_motorId;   // either LAMP_NS or LAMP_EW
};

class OpenClawReq : public MotorReq {
public:
    OpenClawReq(uint8_t motorId) :
        MotorReq(MOTOR_FORWARD_REQ, motorId) {}
};

class CloseClawReq : public MotorReq {
public:
    CloseClawReq(uint8_t motorId) :
        MotorReq(MOTOR_BACKWARD_REQ, motorId) {}
};

class MoveForwardReq : public MotorReq {
public:
    MoveForwardReq(uint8_t motorId) :
        MotorReq(MOTOR_FORWARD_REQ, motorId) {}
};

class MoveBackwardReq : public MotorReq {
public:
    MoveBackwardReq(uint8_t motorId) :
        MotorReq(MOTOR_BACKWARD_REQ, motorId) {}
};

class MoveUpReq : public MotorReq {
public:
    MoveUpReq(uint8_t motorId) :
        MotorReq(MOTOR_FORWARD_REQ, motorId) {}
};


class MoveDownReq : public MotorReq {
public:
    MoveDownReq(uint8_t motorId) :
        MotorReq(MOTOR_BACKWARD_REQ, motorId) {}
};

class RotateLeftReq : public MotorReq {
public:
    RotateLeftReq(uint8_t motorId) :
        MotorReq(MOTOR_BACKWARD_REQ, motorId) {}
};


class RotateRightReq : public MotorReq {
public:
    RotateRightReq(uint8_t motorId) :
        MotorReq(MOTOR_FORWARD_REQ, motorId) {}
};


class RobotUserStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    RobotUserStartReq(uint16_t seq) :
        Evt(ROBOT_USER_START_REQ, seq) {}
};
 
class RobotUserStartCfm : public ErrorEvt {
public:
    RobotUserStartCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(ROBOT_USER_START_CFM, seq, error, reason) {}
};

//:TODO: add evets for arm and disarm robot


class RobotUserStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    RobotUserStopReq(uint16_t seq) :
        Evt(ROBOT_USER_STOP_REQ, seq) {}
};

class RobotUserStopCfm : public ErrorEvt {
public:
    RobotUserStopCfm(uint16_t seq, Error error, Reason reason = 0) :
        ErrorEvt(ROBOT_USER_STOP_CFM, seq, error, reason) {}
};


}

#endif