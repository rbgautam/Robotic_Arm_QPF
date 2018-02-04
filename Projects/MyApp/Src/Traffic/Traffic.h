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

#ifndef TRAFFIC_H_
#define TRAFFIC_H_

#include "qpcpp.h"
#include "fw_evt.h"
#include "hsm_id.h"
#include "bsp.h"
#include "Lamp.h"

#define TRAFFIC_ASSERT(t_) ((t_)? (void)0: Q_onAssert("Traffic.h", (int32_t)__LINE__))

using namespace QP;
using namespace FW;

namespace APP {

class Traffic : public QActive {
public:
    Traffic();
    void Start(uint8_t prio) {
        QActive::start(prio, m_evtQueueStor, ARRAY_COUNT(m_evtQueueStor), NULL, 0);
    }

protected:
    static QState InitialPseudoState(Traffic * const me, QEvt const * const e);
    static QState Root(Traffic * const me, QEvt const * const e);
        static QState Stopped(Traffic * const me, QEvt const * const e);
        static QState Started(Traffic * const me, QEvt const * const e);
            static QState NSGo(Traffic * const me, QEvt const * const e);
                static QState NSMinTimeWait(Traffic * const me, QEvt const * const e);
                static QState NSMinTimeExceeded(Traffic * const me, QEvt const * const e);
            static QState EWSlow(Traffic * const me, QEvt const * const e);
            static QState NSSlow(Traffic * const me, QEvt const * const e);
            static QState EWGo(Traffic * const me, QEvt const * const e);
                static QState EWMinTimeWait(Traffic * const me, QEvt const * const e);
                static QState EWMinTimeExceeded(Traffic * const me, QEvt const * const e);
            
    void PrintUsage();
    
    enum {
        EVT_QUEUE_COUNT = 8
    };
    QEvt const *m_evtQueueStor[EVT_QUEUE_COUNT];
    uint8_t m_id;
    
    
    char const * m_name;
    Lamp m_lampNS;
    Lamp m_lampEW;
    bool m_nsCarWaiting;
    bool m_ewCarWaiting;
    
    QTimeEvt m_waitTimer;
    uint32_t m_nsWaitTimeout; // Timeout for waittimer
    uint32_t m_ewWaitTimeout; // Timeout for waittimer
    uint32_t m_transWaitTimeout; // Timeout for waittimer
};

} // namespace APP

#endif // TRAFFIC_H_
