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

#include "qpcpp.h"
#include "fw_log.h"
#include "fw_evt.h"
#include "hsm_id.h"
#include "AOWashingMachine.h"
#include "event.h"
// Test only.
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"


// Constants used within this file.
static const uint16_t NORMAL_WASH_TIME_MS = 3000; 
static const uint16_t NORMAL_RINSE_TIME_MS = 3000; 
static const uint16_t NORMAL_WASH_TEMP = 100;  // Degrees F
static const uint16_t NORMAL_RINSE_TEMP = 100; // Degrees F

static const uint16_t DELICATE_WASH_TIME_MS = 3000; 
static const uint16_t DELICATE_RINSE_TIME_MS = 3000;
static const uint16_t DELICATE_WASH_TEMP = 70;  // Degrees F
static const uint16_t DELICATE_RINSE_TEMP = 70; // Degrees F

static const uint16_t BULKY_WASH_TIME_MS = 10000;
static const uint16_t BULKY_RINSE_TIME_MS = 10000;
static const uint16_t BULKY_WASH_TEMP = 100;  // Degrees F
static const uint16_t BULKY_RINSE_TEMP = 100; // Degrees F

static const uint16_t TOWELS_WASH_TIME_MS = 6000;
static const uint16_t TOWELS_RINSE_TIME_MS = 6000;
static const uint16_t TOWELS_WASH_TEMP = 150;  // Degrees F
static const uint16_t TOWELS_RINSE_TEMP = 150; // Degrees F

static const uint16_t SPIN_TIME_MS = 5000;

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
AOWashingMachine::AOWashingMachine() :
    QActive((QStateHandler)&AOWashingMachine::InitialPseudoState),
    m_id(SYSTEM),
    m_name("WASH"),
    m_nextSequence(0),
    m_washTimer(this, WASH_TIMEOUT),
    m_rinseTimer(this, RINSE_TIMEOUT),
    m_spinTimer(this, SPIN_TIMEOUT)
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
QState AOWashingMachine::InitialPseudoState(AOWashingMachine * const me, QEvt const * const e)
{
    me->m_deferQueue.init(me->m_deferQueueStor, ARRAY_COUNT(me->m_deferQueueStor));

    me->subscribe(WASH_START_REQ);
    me->subscribe(WASH_STOP_REQ);
    me->subscribe(WASH_FILLED_IND);
    me->subscribe(WASH_DRAINED_IND);
    me->subscribe(CYCLE_SELECTED_IND);
    me->subscribe(START_PAUSE_BUTTON_IND);
    me->subscribe(CLOSE_DOOR_IND);
    me->subscribe(OPEN_DOOR_IND);

    return Q_TRAN(&AOWashingMachine::Root);
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
QState AOWashingMachine::Root(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_TRAN(&AOWashingMachine::Stopped);
            break;
        }
        default:
        {
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
QState AOWashingMachine::Stopped(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case WASH_STOP_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new WashStopCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_HANDLED();
            break;
        }
        case WASH_START_REQ: {
            LOG_EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new WashStartCfm(req.GetSeq(), ERROR_SUCCESS);
            QF::PUBLISH(evt, me);
            status = Q_TRAN(&AOWashingMachine::Started);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::Root);
            break;
        }
    }
    return status;
}

/**
 * In the Started state, the washing machine is ready to be used.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::Started(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);

            // Initialize cycle parameters to NORMAL wash
            me->m_cycleParams.washTime = NORMAL_WASH_TIME_MS;
            me->m_cycleParams.rinseTime = NORMAL_RINSE_TIME_MS;
            me->m_cycleParams.washTemperature = NORMAL_WASH_TEMP;
            me->m_cycleParams.rinseTemperature = NORMAL_RINSE_TEMP;

            // Initialize the history to the first step in the wash cycle.
            me->m_history = &FillingWash;

            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            if (me->CheckDoorSensor() == DOOR_OPEN)
            {
                status = Q_TRAN(&AOWashingMachine::DoorOpen);
            }
            else
            {
                status = Q_TRAN(&AOWashingMachine::DoorClosed);
            }
            break;
        }
        case CYCLE_SELECTED_IND:
        {
            LOG_EVENT(e);

            UserSimCycleSelectedInd const &ind = static_cast<UserSimCycleSelectedInd const &>(*e);

            switch (ind.GetCycleType())
            {
                case UserSimCycleSelectedInd::DELICATE:
                {
                    me->m_cycleParams.washTime = DELICATE_WASH_TIME_MS;
                    me->m_cycleParams.rinseTime = DELICATE_RINSE_TIME_MS;
                    me->m_cycleParams.washTemperature = DELICATE_WASH_TEMP;
                    me->m_cycleParams.rinseTemperature = DELICATE_RINSE_TEMP;
                    break;
                }
                case UserSimCycleSelectedInd::TOWELS:
                {
                    me->m_cycleParams.washTime = TOWELS_WASH_TIME_MS;
                    me->m_cycleParams.rinseTime = TOWELS_RINSE_TIME_MS;
                    me->m_cycleParams.washTemperature = TOWELS_WASH_TEMP;
                    me->m_cycleParams.rinseTemperature = TOWELS_RINSE_TEMP;
                    break;
                }
                case UserSimCycleSelectedInd::BULKY:
                {
                    me->m_cycleParams.washTime = BULKY_WASH_TIME_MS;
                    me->m_cycleParams.rinseTime = BULKY_RINSE_TIME_MS;
                    me->m_cycleParams.washTemperature = BULKY_WASH_TEMP;
                    me->m_cycleParams.rinseTemperature = BULKY_RINSE_TEMP;
                    break;
                }
                case UserSimCycleSelectedInd::NORMAL:
                default:
                {
                    me->m_cycleParams.washTime = NORMAL_WASH_TIME_MS;
                    me->m_cycleParams.rinseTime = NORMAL_RINSE_TIME_MS;
                    me->m_cycleParams.washTemperature = NORMAL_WASH_TEMP;
                    me->m_cycleParams.rinseTemperature = NORMAL_RINSE_TEMP;
                    break;
                }
            }

            me->m_history = &FillingWash;
            me->PrintCycleParams();
            
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::Root);
            break;
        }
    }
    return status;
}

/**
 * This state models the behavior when the door is open.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::DoorOpen(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_doorState = DOOR_OPEN;
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case START_PAUSE_BUTTON_IND:
        {
            LOG_EVENT(e);
            me->ErrorBeep();
            status = Q_HANDLED();
            break;
        }
        case CLOSE_DOOR_IND:
        {
            LOG_EVENT(e);
            Evt *evt = new Evt(iWASH_CLOSE);
            me->postLIFO(evt);
            status = Q_HANDLED();
            break;
        }
        case iWASH_CLOSE:
        {
            status = Q_TRAN(&AOWashingMachine::DoorClosed);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::Started);
            break;
        }
    }
    return status;
}

/**
 * This state models the behavior when the door is closed. This is where the washing happens.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */

QState AOWashingMachine::DoorClosed(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_doorState = DOOR_CLOSED;
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case Q_INIT_SIG:
        {
            status = Q_TRAN(&AOWashingMachine::DoorUnlocked);
            break;
        }
        case iWASH_OPEN:
        {
            status = Q_TRAN(&AOWashingMachine::DoorOpen);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::Started);
            break;
        }
    }
    return status;
}

/**
 * When the door is unlocked, the user is allowed to open the door, select the cycle, 
 * and start a wash cycle.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::DoorUnlocked(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->UnlockDoor();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case OPEN_DOOR_IND:
        {
            LOG_EVENT(e);
            Evt *evt = new Evt(iWASH_OPEN);
            me->postLIFO(evt);
            status = Q_HANDLED();
            break;
        }
        case START_PAUSE_BUTTON_IND:
        {
            LOG_EVENT(e);
            // Go to the state pointed to by m_history.
            status = Q_TRAN(me->m_history);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::DoorClosed);
            break;
        }
    }
    return status;
}

/**
 * In the Locked state, the washing machine is actively doing its cycle. 
 * The user is not allowed to open the door because the machine is full
 * of water. Since a cycle is running, the user cannot select a new cycle type. 
 * The user can pause the cycle. 
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::DoorLocked(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->LockDoor();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case START_PAUSE_BUTTON_IND:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&AOWashingMachine::DoorUnlocked);
            break;
        }
        case iWASH_DONE:
        {
            status = Q_TRAN(&AOWashingMachine::DoorUnlocked);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::DoorClosed);
            break;
        }
    }
    return status;
}

/**
 * In the FillingWash state, the washing machine is being filled with water in 
 * preparation for washing. 
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::FillingWash(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_history = &FillingWash;
            me->StartFilling();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            me->StopFilling();
            status = Q_HANDLED();
            break;
        }
        case WASH_FILLED_IND:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&AOWashingMachine::Washing);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::DoorLocked);
            break;
        }
    }
    return status;
}

/**
 * In this state the wash cycle is run for the time specified in the cycle parameters.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::Washing(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_history = &Washing;
            me->m_washTimer.armX(me->m_cycleParams.washTime);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            me->m_washTimer.disarm();
            me->StopWashing();
            me->StartDraining();
            status = Q_HANDLED();
            break;
        }
        case WASH_TIMEOUT:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&AOWashingMachine::DrainingWash);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::DoorLocked);
            break;
        }
    }
    return status;
}

/**
 * In this state the wash cycle is complete and the water is drained from 
 * the machine. 
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::DrainingWash(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_history = &DrainingWash;
            me->StartDraining();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case WASH_DRAINED_IND:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&AOWashingMachine::FillingRinse);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::DoorLocked);
            break;
        }
    }
    return status;
}

/**
 * In this state the washing machine is filled with water in preparation for rinsing.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::FillingRinse(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_history = &FillingRinse;
            me->StartFilling();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            me->StopFilling();
            status = Q_HANDLED();
            break;
        }
        case WASH_FILLED_IND:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&AOWashingMachine::Rinsing);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::DoorLocked);
            break;
        }
    }
    return status;
}

/**
 * In this state the rinse cycle runs for the time specified in the cycle parameters.
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::Rinsing(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_history = &Rinsing;
            me->StartRinsing();
            me->m_rinseTimer.armX(me->m_cycleParams.rinseTime);
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            me->m_rinseTimer.disarm();
            me->StopRinsing();
            me->StartDraining();
            status = Q_HANDLED();
            break;
        }
        case RINSE_TIMEOUT:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&AOWashingMachine::DrainingRinse);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::DoorLocked);
            break;
        }
    }
    return status;
}

/**
 * In this state the rinse water is drained from the machine. 
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::DrainingRinse(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_history = &DrainingRinse;
            me->StartDraining();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        case WASH_DRAINED_IND:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&AOWashingMachine::Spinning);
            break;
        }
        default:
        {
            status = Q_SUPER(&AOWashingMachine::DoorLocked);
            break;
        }
    }
    return status;
}

/**
 * In this state the spinning begins. After spinning, the wash cycle is complete, and we
 * go back to the DoorUnlocked state. 
 *
 * @param me - Pointer to class
 * @param pEvent - The event to process
 *
 * @return Q_HANDLED if the event was processed, the containing state if not
 * processed.
 */
QState AOWashingMachine::Spinning(AOWashingMachine * const me, QEvt const * const e)
{
    QState status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LOG_EVENT(e);
            me->m_history = &Spinning;
            me->m_spinTimer.armX(SPIN_TIME_MS);
            me->StartSpinning();
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG:
        {
            LOG_EVENT(e);
            me->m_spinTimer.disarm();
            me->StopSpinning();
            if (me->m_history == &AOWashingMachine::FillingWash)
            {
                me->PlayCompletionSong();               
            }
            status = Q_HANDLED();
            break;
        }
        case SPIN_TIMEOUT:
        {
            LOG_EVENT(e);
            me->m_history = &FillingWash;
            Evt *evt = new Evt(iWASH_DONE);
            me->postLIFO(evt);
            break;
        }
        default: {
            status = Q_SUPER(&AOWashingMachine::DoorLocked);
            break;
        }
    }
    return status;
}

// Helper functions
uint8_t AOWashingMachine::CheckDoorSensor()
{
    return m_doorState;
}

void AOWashingMachine::ErrorBeep()
{ 
    PRINT("    ** Error Beep **\r\n"); 
}

void AOWashingMachine::PlayCompletionSong()
{
    PRINT("    ** Wash all done!! **\r\n");
}
 
void AOWashingMachine::StartFilling() 
{ 
    PRINT("    ** Filling started. **\r\n");

    // Send the wash filled event as if a washing machine sensor
    // had been triggered.
    Evt *evt = new Evt(WASH_FILLED_IND);
    QF::PUBLISH(evt, me);
}

void AOWashingMachine::StartWashing() 
{ 
    PRINT("    ** Washing started. **\r\n"); 
}

void AOWashingMachine::StartDraining() 
{ 
    PRINT("    ** Draining started. **\r\n"); 
    // Send the wash drained event as if a washing machine sensor
    // had been triggered. 
    Evt *evt = new Evt(WASH_DRAINED_IND);
    QF::PUBLISH(evt, me);
}

void AOWashingMachine::StartRinsing() 
{ 
    PRINT("    ** Rinsing started. **\r\n"); 
}

void AOWashingMachine::StartSpinning() 
{ 
    PRINT("    ** Spinning started. **\r\n"); 
}

void AOWashingMachine::StopFilling() 
{ 
    PRINT("    ** Filling stopped. **\r\n"); 
}

void AOWashingMachine::StopWashing() 
{ 
    PRINT("    ** Washing stopped. **\r\n"); 
}

void AOWashingMachine::StopDraining() 
{ 
    PRINT("    ** Draining stopped. **\r\n"); 
}

void AOWashingMachine::StopRinsing() 
{ 
    PRINT("    ** Rinsing stopped. **\r\n"); 
}

void AOWashingMachine::StopSpinning() 
{ 
    PRINT("    ** Spinning stopped. **\r\n"); 
}

void AOWashingMachine::LockDoor() 
{ 
    PRINT("    ** Door locked **\r\n"); 
}

void AOWashingMachine::UnlockDoor() 
{ 
    PRINT("    ** Door unlocked. **\r\n"); 
}

void AOWashingMachine::PrintCycleParams()
{
    PRINT("\r\n");
    PRINT("    Wash time = %d s\r\n", m_cycleParams.washTime/1000);
    PRINT("    Rinse time = %d s\r\n", m_cycleParams.rinseTime/1000);
    PRINT("    Wash temperature = %d F\r\n", m_cycleParams.washTemperature);
    PRINT("    Rinse temperature = %d F\r\n", m_cycleParams.rinseTemperature);
    PRINT("\r\n");
}


} // namespace APP
