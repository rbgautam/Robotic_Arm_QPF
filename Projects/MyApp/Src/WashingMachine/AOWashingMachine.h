/*******************************************************************************
 * Copyright (C) Lawrence Lo (https://github.com/galliumstudio).
 * All rights reserved.
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

#ifndef WASHING_MACHINE_H
#define WASHING_MACHINE_H

#include "qpcpp.h"
#include "fw_pipe.h"
#include "fw_log.h"
#include "hsm_id.h"
#include "event.h"

using namespace QP;
using namespace FW;

namespace APP {

class AOWashingMachine : public QActive {
public:
    AOWashingMachine();
    void Start(uint8_t prio) {
        QActive::start(prio, m_evtQueueStor, ARRAY_COUNT(m_evtQueueStor), NULL, 0);
    }

protected:
    static QState InitialPseudoState(AOWashingMachine * const me, QEvt const * const e);
    static QState Root(AOWashingMachine * const me, QEvt const * const e);
        static QState Stopped(AOWashingMachine * const me, QEvt const * const e);
        static QState Started(AOWashingMachine * const me, QEvt const * const e);
            static QState DoorOpen(AOWashingMachine * const me, QEvt const * const e);
            static QState DoorClosed(AOWashingMachine * const me, QEvt const * const e);
                static QState DoorUnlocked(AOWashingMachine * const me, QEvt const * const e);
                static QState DoorLocked(AOWashingMachine * const me, QEvt const * const e);
                    static QState FillingWash(AOWashingMachine * const me, QEvt const * const e);
                    static QState Washing(AOWashingMachine * const me, QEvt const * const e);
                    static QState DrainingWash(AOWashingMachine * const me, QEvt const * const e);
                    static QState FillingRinse(AOWashingMachine * const me, QEvt const * const e);
                    static QState Rinsing(AOWashingMachine * const me, QEvt const * const e);
                    static QState DrainingRinse(AOWashingMachine * const me, QEvt const * const e);
                    static QState Spinning(AOWashingMachine * const me, QEvt const * const e);

    // Helper functions
    uint8_t CheckDoorSensor();
    void ErrorBeep();
    void PlayCompletionSong();
    void StartFilling();
    void StartWashing();
    void StartDraining();
    void StartRinsing();
    void StartSpinning();

    void StopFilling();
    void StopWashing();
    void StopDraining();
    void StopRinsing();
    void StopSpinning();

    void LockDoor();
    void UnlockDoor();
    void PrintCycleParams(); 


    enum {
        EVT_QUEUE_COUNT = 16,
        DEFER_QUEUE_COUNT = 4
    };

    enum {
        DOOR_OPEN,
        DOOR_CLOSED
    };

    typedef enum {
        NORMAL,
        DELICATE,
        BULKY,
        TOWELS
    } WashType;

    typedef struct {
        uint16_t washTime; // ms
        uint16_t rinseTime; // ms
        uint16_t washTemperature;
        uint16_t rinseTemperature;
        WashType washType;
    } cycleParameters_t ;

    QEvt const *m_evtQueueStor[EVT_QUEUE_COUNT];
    QEvt const *m_deferQueueStor[DEFER_QUEUE_COUNT];
    QEQueue m_deferQueue;
    uint8_t m_id;
    char const *m_name;
    uint16_t m_nextSequence;

    cycleParameters_t m_cycleParams;
    uint16_t m_doorState;

    // m_history is a function pointer used for saving the previous state.
    QState (*m_history) (AOWashingMachine * const me, QEvt const * const e);

    QTimeEvt m_washTimer;
    QTimeEvt m_rinseTimer;
    QTimeEvt m_spinTimer;
};

} // namespace APP

#endif // WASHING_MACHINE_H
