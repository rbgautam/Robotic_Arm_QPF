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

#ifndef Motor_H
#define Motor_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#include "qpcpp.h"
#include "fw_evt.h"
#include "hsm_id.h"

using namespace QP;
using namespace FW;

namespace APP {

class Motor : public QHsm {
public:
    Motor(uint8_t id, char const *name, QActive *owner, Servo_TypeDef servoName);
    
    void Init() { 
      QHsm::init(); 
    }
    void Print(Motor * const me, char* mode);

protected:
    static QState InitialPseudoState(Motor * const me, QEvt const * const e);
    static QState Root(Motor * const me, QEvt const * const e);
        static QState Stop(Motor * const me, QEvt const * const e);
        static QState Forward(Motor * const me, QEvt const * const e);
        static QState Backward(Motor * const me, QEvt const * const e);
    
 
    bool InitPwm(Motor * const me);
   
    void ConfigPwm(Motor * const me);
    void StartPwm(Motor * const me);
    void StopPwm(Motor * const me);    
    
    volatile uint32_t msTicks;      //counts 1ms timeTicks
    void  MoveForward(Motor * const me);
    void MoveBackward(Motor * const mes);
    void MoveFull180();
    
     // Structures for configuration
    GPIO_TypeDef GPIO_Struct;
    
    TIM_HandleTypeDef m_timHandle;
    TIM_OC_InitTypeDef m_timConfig;
    
    uint8_t m_id;
    char const * m_name;  
    QActive *m_owner;
    Servo_TypeDef m_servoName;
};

} // namespace APP

#endif // Motor_H
