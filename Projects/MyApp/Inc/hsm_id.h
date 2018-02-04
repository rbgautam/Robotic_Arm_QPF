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

#ifndef HSM_ID_H
#define HSM_ID_H

namespace APP {

enum {
    SYSTEM = 1,
    UART2_ACT,
    UART2_IN,
    UART2_OUT,
    USER_BTN,
    USER_LED,
    USER_SIM,
    WASH,
    DEMO,
    TRAFFIC,
        LAMP_NS,        // Orthogonal regions in Traffic active object
        LAMP_EW,        // Orthogonal regions in Traffic active object
    ROBOT_USER_CTRL,    
    ROBOTARM,
        SERVO_AXIS,
        SERVO_SWING,
        SERVO_CLAW,
        SERVO_PICK,
        
    HSM_COUNT
};

// Higher value corresponds to higher priority.
// The maximum priority is defined in qf_port.h as QF_MAX_ACTIVE (32)
enum
{
    PRIO_UART2_ACT  = 30,
    PRIO_CONSOLE    = 28,
    PRIO_SYSTEM     = 26,
    PRIO_USER_BTN   = 24,
    PRIO_USER_LED   = 22,
    PRIO_USER_SIM   = 18,
    PRIO_WASH       = 12,
    PRIO_DEMO       = 11,
    PRIO_TRAFFIC    = 10,
    PRIO_ROBOTUSER  = 9,
    PRIO_ROBOTARM   = 8,
    
};

} // namespace APP

#endif // HSM_ID_H
