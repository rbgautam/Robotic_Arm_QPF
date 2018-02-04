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
#include "event.h"
#include "fw_macro.h"
#include "fw_log.h"

Q_DEFINE_THIS_FILE

namespace APP {
  
char const * const eventName[] = {
    "NULL",
    "ENTRY",
    "EXIT",
    "INIT",
    "SYSTEM_START_REQ",
    "SYSTEM_START_CFM",
    "SYSTEM_STOP_REQ",
    "SYSTEM_STOP_CFM",
    "SYSTEM_STATE_TIMER",
    "SYSTEM_TEST_TIMER",
    "SYSTEM_DONE",
    "SYSTEM_FAIL",
    
    "UART_ACT_START_REQ",
    "UART_ACT_START_CFM",
    "UART_ACT_STOP_REQ",
    "UART_ACT_STOP_CFM",
    "UART_ACT_FAIL_IND",
    "UART_ACT_STATE_TIMER",
    "UART_ACT_START",
    "UART_ACT_DONE",
    "UART_ACT_FAIL",
      
    "UART_OUT_START_REQ",
    "UART_OUT_START_CFM",
    "UART_OUT_STOP_REQ",
    "UART_OUT_STOP_CFM",
    "UART_OUT_FAIL_IND",
    "UART_OUT_WRITE_REQ", 
    "UART_OUT_WRITE_CFM", 
    "UART_OUT_EMPTY_IND",
    "UART_OUT_ACTIVE_TIMER",
    "UART_OUT_DONE",
    "UART_OUT_DMA_DONE",
    "UART_OUT_CONTINUE",
    "UART_OUT_HW_FAIL",
    
    "UART_IN_START_REQ",
    "UART_IN_START_CFM",
    "UART_IN_STOP_REQ",
    "UART_IN_STOP_CFM",
    "UART_IN_CHAR_IND",
    "UART_IN_FAIL_IND",
    "UART_IN_STATE_TIMER",
    "UART_IN_DONE",
    "UART_IN_DATA_RDY",
    
    "USER_BTN_START_REQ",
    "USER_BTN_START_CFM",
    "USER_BTN_STOP_REQ",
    "USER_BTN_STOP_CFM",
    "USER_BTN_UP_IND",
    "USER_BTN_DOWN_IND",
    "USER_BTN_STATE_TIMER",
    "USER_BTN_TRIG",
    "USER_BTN_UP",
    "USER_BTN_DOWN",
    
    "USER_LED_START_REQ",
    "USER_LED_START_CFM",
    "USER_LED_STOP_REQ",
    "USER_LED_STOP_CFM",
    "USER_LED_ON_REQ",
    "USER_LED_ON_CFM",
    "USER_LED_PATTERN_REQ",
    "USER_LED_PATTERN_CFM",
    "USER_LED_OFF_REQ",
    "USER_LED_OFF_CFM", 
    "USER_LED_INTERVAL_TIMER",
    "USER_LED_DONE",
    "USER_LED_NEXT_INTERVAL",
    "USER_LED_LAST_INTERVAL",
    "USER_LED_LOAD_PATTERN",
    "USER_LED_PATTERN_END",
    "USER_LED_NEW_PATTERN",

    // Washing machine control simulator
    "USER_SIM_START_REQ",
    "USER_SIM_START_CFM",
    "USER_SIM_STOP_REQ",
    "USER_SIM_STOP_CFM",
    "OPEN_DOOR_IND",
    "CLOSE_DOOR_IND",
    "START_PAUSE_BUTTON_IND",
    "CYCLE_SELECTED_IND",

    // Washing machine
    "WASH_START_REQ",
    "WASH_START_CFM",
    "WASH_STOP_REQ",
    "WASH_STOP_CFM",

    // Internal washing machine events
    "iWASH_START",
    "iWASH_CLOSE",
    "iWASH_OPEN",
    "iWASH_DONE",
    "WASH_TIMEOUT",
    "RINSE_TIMEOUT",
    "SPIN_TIMEOUT",
    
    // Washing machine sensor events
    "WASH_FILLED_IND",
    "WASH_DRAINED_IND",

    // Demo
    "DEMO_A",
    "DEMO_B",
    "DEMO_C",
    "DEMO_D",    
    "DEMO_E",
    "DEMO_F",
    "DEMO_G",
    "DEMO_H",
    "DEMO_I",
    
    // Traffic
    "TRAFFIC_START_REQ",
    "TRAFFIC_START_CFM",
    "TRAFFIC_STOP_REQ",
    "TRAFFIC_STOP_CFM",
    "TRAFFIC_CAR_NS_REQ",
    "TRAFFIC_CAR_EW_REQ",
    "TRAFFIC_WAIT_TO",   
    "LAMP_RED_REQ",      
    "LAMP_YELLOW_REQ",   
    "LAMP_GREEN_REQ", 
    
    //RobotArm
    "ROBOT_USER_START_REQ",
    "ROBOT_USER_START_CFM",
    "ROBOT_USER_STOP_REQ",
    "ROBOT_USER_STOP_CFM",
    "ROBOTARM_START_REQ",
    "ROBOTARM_START_CFM",
    "ROBOTARM_STOP_REQ",
    "ROBOTARM_STOP_CFM",
    "ROBOTARM_MOTOR_AXIS_REQ",         
    "ROBOTARM_MOTOR_SWING_REQ",         
    "ROBOTARM_MOTOR_CLAW_REQ",         
    "ROBOTARM_MOTOR_PICK_REQ",         
    "ROBOTARM_WAIT_TO", 
    
    "DISARM_ROBOT_SIG",
    "ARM_ROBOT_SIG",
    "OPEN_CLAW_SIG",
    "CLOSE_CLAW_SIG",
    "MOVE_FORWARD_SIG",
    "MOVE_BACKWARD_SIG",
    "MOVE_UP_SIG",
    "MOVE_DOWN_SIG",
    "ROTATE_LEFT_SIG",
    "ROTATE_RIGHT_SIG",
    
    
    "MOTOR_FORWARD_REQ",
    "MOTOR_STOP_REQ",
    "MOTOR_BACKWARD_REQ",
    
    
    
};

char const * GetEvtName(QP::QSignal sig) {
    if (sig >= ARRAY_COUNT(eventName)) {
        PRINT("Assert: eventName array does not match enum in event.h!\r\n");
    }
    Q_ASSERT(sig < ARRAY_COUNT(eventName));
    if (sig < MAX_PUB_SIG) {
        return eventName[sig];
    }
    return "(UNKNOWN)";
}

}