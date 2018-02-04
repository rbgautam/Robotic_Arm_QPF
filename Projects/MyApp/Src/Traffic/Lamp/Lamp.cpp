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

#include "hsm_id.h"
#include "fw_log.h"
#include "Lamp.h"
#include "event.h"

/*
#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(x, y)
*/


//Q_DEFINE_THIS_FILE


namespace APP {

Lamp::Lamp(uint8_t id, char const *name, QActive *owner) :
    QHsm((QStateHandler)&Lamp::InitialPseudoState), m_id(id), m_name(name), 
    m_owner(owner) {}

QState Lamp::InitialPseudoState(Lamp * const me, QEvt const * const e) {
    (void)e;
//    me->subscribe(LAMP_RED_REQ);      
//    me->subscribe(LAMP_YELLOW_REQ);   
//    me->subscribe(LAMP_GREEN_REQ);    
    return Q_TRAN(&Lamp::Root);
}

QState Lamp::Root(Lamp * const me, QEvt const * const e) {
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
            status = Q_TRAN(&Lamp::Red);
            break;
        }
        
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}


QState Lamp::Red(Lamp * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            me->Print(me, "RED");
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        
        case LAMP_GREEN_REQ:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&Lamp::Green);
            break;
        }
        
        default: {
            status = Q_SUPER(&Lamp::Root);
            break;
        }
    }
    return status;
}


QState Lamp::Yellow(Lamp * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            me->Print(me,"YELLOW");
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        
        case LAMP_RED_REQ:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&Lamp::Red);
            break;
        }
        default: {
            status = Q_SUPER(&Lamp::Root);
            break;
        }
    }
    return status;
}

QState Lamp::Green(Lamp * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LOG_EVENT(e);
            me->Print(me ,"GREEN");
            status = Q_HANDLED();
            break;
        }
        case Q_EXIT_SIG: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        
        case LAMP_YELLOW_REQ:
        {
            LOG_EVENT(e);
            status = Q_TRAN(&Lamp::Yellow);
            break;
        }
        
        default: {
            status = Q_SUPER(&Lamp::Root);
            break;
        }
    }
    return status;
}

void Lamp::Print(Lamp * const me, char* color){
   PRINT("\r\n %s = %s\r\n", me->m_name, color);
    
}

/*
QState Lamp::MyState(Lamp * const me, QEvt const * const e) {
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
            status = Q_TRAN(&Lamp::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&Lamp::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP

