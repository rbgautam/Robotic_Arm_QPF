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

#ifndef UART_ACT_H
#define UART_ACT_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "qpcpp.h"
#include "fw_evt.h"
#include "fw_pipe.h"
#include "UartIn.h"
#include "UartOut.h"

using namespace QP;
using namespace FW;

namespace APP {

class UartAct : public QActive {
public:
    UartAct(uint8_t id, char const *name, char const *inName, char const *outName,
            USART_TypeDef *dev);
    void Start(uint8_t prio) {
        QActive::start(prio, m_evtQueueStor, ARRAY_COUNT(m_evtQueueStor), NULL, 0);
    }
    static UART_HandleTypeDef *GetHal(uint8_t id);
    
protected:
    static QState InitialPseudoState(UartAct * const me, QEvt const * const e);
    static QState Root(UartAct * const me, QEvt const * const e);
        static QState Stopped(UartAct * const me, QEvt const * const e);
        static QState Starting(UartAct * const me, QEvt const * const e);
        static QState Stopping(UartAct * const me, QEvt const * const e);
        static QState Started(UartAct * const me, QEvt const * const e);
            static QState Normal(UartAct * const me, QEvt const * const e);
            static QState Failed(UartAct * const me, QEvt const * const e);
           
    void HandleCfm(ErrorEvt const &e, uint8_t expectedCnt);

    enum {
        EVT_QUEUE_COUNT = 16,
        DEFER_QUEUE_COUNT = 4
    };
    QEvt const *m_evtQueueStor[EVT_QUEUE_COUNT];
    QEvt const *m_deferQueueStor[DEFER_QUEUE_COUNT];
    QEQueue m_deferQueue;
    uint8_t m_id;
    char const * m_name;
    uint16_t m_nextSequence;
    uint16_t m_savedInSeq;
    uint8_t m_cfmCount;
    
    // TODO - Modify to support multiple instance. Interface with ISR.
    static UART_HandleTypeDef m_hal;
    UartIn m_uartIn;
    UartOut m_uartOut;
    Fifo *m_outFifo;
    Fifo *m_inFifo;

    QTimeEvt m_stateTimer;
};

} // namespace APP

#endif // UART_ACT_H
