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

#ifndef USER_LED_H
#define USER_LED_H

#include "qpcpp.h"
#include "fw_evt.h"
#include "hsm_id.h"
#include "bsp.h"
#include "LedPattern.h"

#define USER_LED_ASSERT(t_) ((t_)? (void)0: Q_onAssert("UserLed.h", (int32_t)__LINE__))

using namespace QP;
using namespace FW;

namespace APP {

class UserLed : public QActive {
public:
    UserLed();
    void Start(uint8_t prio) {
        QActive::start(prio, m_evtQueueStor, ARRAY_COUNT(m_evtQueueStor), NULL, 0);
    }

protected:
    static QState InitialPseudoState(UserLed * const me, QEvt const * const e);
    static QState Root(UserLed * const me, QEvt const * const e);
        static QState Stopped(UserLed * const me, QEvt const * const e);
        static QState Started(UserLed * const me, QEvt const * const e);
            static QState Idle(UserLed * const me, QEvt const * const e);
            static QState Active(UserLed * const me, QEvt const * const e);
                static QState Repeating(UserLed * const me, QEvt const * const e);
                static QState Once(UserLed * const me, QEvt const * const e);
    
    class PatternInfo {
    public:
        enum {
            INVALID_INDEX = 0xFFFFFFFF
        };
        // Default constructor
        PatternInfo() : m_index(INVALID_INDEX), m_isRepeat(false) {}
        void Invalidate() { m_index = INVALID_INDEX; }
        bool IsValid() const { return m_index != INVALID_INDEX; }
        void Save(uint32_t index, bool isRepeat) {
            USER_LED_ASSERT(index != INVALID_INDEX);
            m_index = index;
            m_isRepeat = isRepeat;
        }        
        uint32_t GetIndex() const { return m_index; }
        bool IsRepeat() const { return m_isRepeat; }
    private:
        uint32_t m_index;         // pattern index
        bool m_isRepeat;
    };
 
    void InitPwm();
    void DeInitPwm();
    void ConfigPwm(uint32_t levelPermil);
    void StartPwm();
    void StopPwm();
    void InvalidateAllPatternInfo() {
        for (uint32_t i = 0; i < MAX_LAYER; i++) {
            m_patternInfo[i].Invalidate();
        }
    }
    void InvalidatePatternInfo(uint32_t layer) {
        USER_LED_ASSERT(layer < MAX_LAYER);
        m_patternInfo[layer].Invalidate();
    }
    void SavePatternInfo(uint32_t layer, uint32_t index, bool isRepeat) {
        USER_LED_ASSERT(layer < MAX_LAYER);
        m_patternInfo[layer].Save(index, isRepeat);
    }
    // Returns true if one is found; false if none in use.
    bool GetHighestLayerInUse(uint32_t &layer) {
        bool found = false;
        // i must be signed.
        for (int32_t i = MAX_LAYER - 1; i >= 0; i--) {
            if (m_patternInfo[i].IsValid()) {
                found = true;
                layer = i;
                break;
            }
        }        
        return found;
    }
    PatternInfo &GetCurrPatternInfo() {
        USER_LED_ASSERT(m_currLayer < MAX_LAYER);
        return m_patternInfo[m_currLayer];
    }
            
    enum {
        EVT_QUEUE_COUNT = 16
    };
    QEvt const *m_evtQueueStor[EVT_QUEUE_COUNT];
    uint8_t m_id;
    char const * m_name;
    uint16_t m_nextSequence;
    TIM_HandleTypeDef m_timHandle;
    TIM_OC_InitTypeDef m_timConfig;
    LedPattern const *m_currPattern;
    uint32_t m_intervalIndex;
    
    enum {
        MAX_LAYER = 3  
    };
    PatternInfo m_patternInfo[MAX_LAYER];
    uint32_t m_currLayer;
    
    QTimeEvt m_intervalTimer;
};

} // namespace APP

#endif // USER_LED_H
