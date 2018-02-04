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

#include "stm32f4xx_nucleo.h"
#include "hsm_id.h"
#include "fw_log.h"
#include "Demo.h"
#include "event.h"

#undef LOG_EVENT
#define LOG_EVENT(e)            
#undef DEBUG
#define DEBUG(format_, ...)

//Q_DEFINE_THIS_FILE

namespace APP {
  
Demo::Demo() :
    QActive((QStateHandler)&Demo::InitialPseudoState),
    m_id(DEMO), m_name("DEMO"), m_foo(0) {}

QState Demo::InitialPseudoState(Demo * const me, QEvt const * const e) {
    (void)e;
    me->subscribe(UART_IN_CHAR_IND);
    return Q_TRAN(&Demo::Root);
}

QState Demo::Root(Demo * const me, QEvt const * const e) {
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
            LOG_EVENT(e);
            me->m_foo = 0;
            status = Q_TRAN(&Demo::S2);
            break;
        }
        case UART_IN_CHAR_IND: {
            UartInCharInd const &ind = static_cast<UartInCharInd const &>(*e);
            char ch = ind.GetChar();
            DEBUG("Rx char %c (foo = %d)", ch, me->m_foo);
            Evt *evt;
            switch(ch) {
                case 'A': evt = new Evt(DEMO_A); break;
                case 'B': evt = new Evt(DEMO_B); break;
                case 'C': evt = new Evt(DEMO_C); break;
                case 'D': evt = new Evt(DEMO_D); break;
                case 'E': evt = new Evt(DEMO_E); break;
                case 'F': evt = new Evt(DEMO_F); break;
                case 'G': evt = new Evt(DEMO_G); break;
                case 'H': evt = new Evt(DEMO_H); break;
                case 'I': evt = new Evt(DEMO_I); break;
                default:  evt = NULL; break;
            }
            if (evt) {
                me->postLIFO(evt);
            }
            status = Q_HANDLED();
            break;
         }
         default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}

QState Demo::S(Demo * const me, QEvt const * const e) {
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
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S11);
            break;
        }
        case DEMO_E: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S11);
            break;
        }
        case DEMO_I: {
            LOG_EVENT(e);
            if (me->m_foo) {
                me->m_foo = 0;
                status = Q_HANDLED();
            } else {
                status = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status = Q_SUPER(&Demo::Root);
            break;
        }
    }
    return status;
}

QState Demo::S1(Demo * const me, QEvt const * const e) {
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
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S11);
            break;
        }
        case DEMO_A: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S1);
            break;
        }
        case DEMO_B: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S11);
            break;
        }
        case DEMO_C: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S2);
            break;
        }
        case DEMO_D: {
            LOG_EVENT(e);
            if (!me->m_foo) {
                me->m_foo = 1;
                status = Q_TRAN(&Demo::S);
            } else {
                status = Q_UNHANDLED();
            }
            break;
        }
        case DEMO_F: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S211);
            break;
        }
        case DEMO_I: {
            LOG_EVENT(e);
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&Demo::S);
            break;
        }
    }
    return status;
}

QState Demo::S11(Demo * const me, QEvt const * const e) {
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
        case DEMO_D: {
            LOG_EVENT(e);
            if (me->m_foo) {
                me->m_foo = 0;
                status = Q_TRAN(&Demo::S1);
            } else {
                status = Q_UNHANDLED();
            }
            break;
        }
        case DEMO_H: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S);
            break;
        }
        case DEMO_G: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S211);
            break;
        }
        default: {
            status = Q_SUPER(&Demo::S1);
            break;
        }
    }
    return status;
}

QState Demo::S2(Demo * const me, QEvt const * const e) {
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
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S211);
            break;
        }
        case DEMO_C: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S1);
            break;
        }
        case DEMO_F: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S11);
            break;
        }
        case DEMO_I: {
            LOG_EVENT(e);
            if (!me->m_foo) {
                me->m_foo = 1;
                status = Q_HANDLED();
            } else {
                status = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status = Q_SUPER(&Demo::S);
            break;
        }
    }
    return status;
}

QState Demo::S21(Demo * const me, QEvt const * const e) {
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
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S211);
            break;
        }
        case DEMO_A: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S21);
            break;
        }
        case DEMO_B: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S211);
            break;
        }
        case DEMO_G: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S11);
            break;
        }
        default: {
            status = Q_SUPER(&Demo::S2);
            break;
        }
    }
    return status;
}

QState Demo::S211(Demo * const me, QEvt const * const e) {
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
        case DEMO_D: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S21);
            break;
        }
        case DEMO_H: {
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::S);
            break;
        }
        default: {
            status = Q_SUPER(&Demo::S21);
            break;
        }
    }
    return status;
}

/*
QState Demo::MyState(Demo * const me, QEvt const * const e) {
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
            LOG_EVENT(e);
            status = Q_TRAN(&Demo::SubState);
            break;
        }
        default: {
            status = Q_SUPER(&Demo::SuperState);
            break;
        }
    }
    return status;
}
*/

} // namespace APP
