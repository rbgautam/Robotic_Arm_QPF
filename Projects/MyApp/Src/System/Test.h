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

#ifndef TEST_H
#define TEST_H

#include "bsp.h"
#include <stdint.h>
#include "fw_log.h"
#include "qassert.h"

//#define TEST_ASSERT(t_) ((t_)? (void)0: Q_onAssert("Test.h", (int_t)__LINE__))

using namespace FW;

namespace APP {

// Declaration of TestFunction() to be called in System.cpp in System::Start()
// under case UART_IN_CHAR_IND.
// This function is defined in Test.cpp. It contains testing code used in the
// class of Week 2. 
void TestFunction();

// Before we have structure, we define variables individually. They are loosely
// related to each other, and can be hard to maintain.
/*
uint32_t a; // unsigned long a;
uint32_t b;
*/

// Before C++, we can still use structure to group related variables together.
// Note the following typedef only defines the type of structure only. It does 
// not yet define an instance of the struct that actually takes up memory.
// We can have functions that act on the an instance of a structure, like
// TestSum() below.
/*
typedef struct {
    uint32_t a;
    uint32_t b;
} TestStruct;

uint32_t TestSum(TestStruct *ts) {
  uint32_t sum = ts->a + ts->b;
  PRINT("SUM = %d\n\r", sum);
}
*/

// With C++ Encapsulation, we can combine data and functions acting on those
// data together in something called a class.
class TestClass {
// Private means the following data members can only be accessed by member 
// functions of this class.    
private:
    // Data members.
    // Note, not only can we have primitive types, such as integers, booleans,
    // etc, we can have objects of other classes as members. See LedPattern
    // assignment for examples.
    uint32_t m_a;
    uint32_t m_b;    

// Public means the following methods can be accessed or called from anywhere,
// including our TestFunction() in global scope.
public:
    // The keyword "static" when used within a class definition means that the 
    // member variable m_c is shared by all objects/instances of this class, 
    // i.e. there is only one copy of it, instead of having one per object.    
    static uint32_t m_c;
    
    // Constructor. This one initializes m_a and m_b with the parameters a and b
    // passed in respectively. Note a constructor does not have an return type, 
    // not even void. See TestFunction() to see how this is used.
    // The single colon here marks the beginning of the "member initialization
    // list". As its name implies, it initializes all data members (in the order
    // of the their appearence in the class definition, rather than in the 
    // order they are listed here. Try swap m_a(a) and m_b(b) below and you will
    // get a warning.
    // When we use inheritance, i.e. deriving from base class, we also use the
    // member initialization list to initializae base class objects. We will
    // see plenty of such examples when we look at QP application later. 
    TestClass(uint32_t a, uint32_t b) : m_a(a), m_b(b) {
        PRINT("TestClass constructor a=%d, b=%d\n\r", a, b);
    }
    // Subtle difference between initialization and assignment.
    // The following version is different from the one above in that it 
    // does NOT initialize the data members m_a and m_b. Rather it assigns 
    // values to them via "m_a = a" and "m_b = b". While it does not make a
    // difference for primitive types (like this case), it makes differences
    // for class types (recall a data member can be an object of another class)
    // since initialization is done through "constructor" and assignment is done
    // through "assignment operator". In short, use member initialization list
    // in the version above rather than the one below; or you may get a 
    // compiler error complaining a member object is uninitialized.
    /*
    TestClass(uint32_t a, uint32_t b) {
        m_a = a;
        m_b = b;
    }
    */
    
    // Simple accessor functions to return private data members as needed.
    // It may seem trival, but with this we can control which one to expose, 
    // limit read access only, and optionally provide any necessary 
    // transformation or checking before returning the data. 
    // We follow this practice in our LedPattern example.
    uint32_t GetA() const { return this->m_a; }
    uint32_t GetB() const { return this->m_b; }
    
    // This dummy example shows just one use of the implicit "this pointer"
    // available to any non-static memmber functions. Its use is not strictly
    // required, but it makes it clearer which m_a we are referring to.
    /*
    void AssignA(TestClass &tc) { this->m_a = tc.m_a; }
    */
    
    // Member function that calculates the sum of data members m_a and m_b.
    // For testing purposes, it also increments the static data member m_c.
    // Since there is only one copy of m_c, we see that every time Sum() is
    // called, regardless to which object it is called on, the "same" m_c is
    // incremented. Hence in our examples, it jumps from 123 to 124 and finally
    // to 125. If there were two different copies (one per object), we would see
    // it jumps from 123 to 124 in both calls of Sum().
    uint32_t Sum() const { m_c++; return m_a + m_b; }
};
  
// The following is another example illustrating the concepts of inheritance and
// polymorphism.
// First we define the base class TestBase. It has a data member m_id and a
// a virtual member function named Print(). Being virtual means that 
// when this function is overriden by a derived class (see TestDerived1 below), 
// and when you call it via a pointer to the base class object (i.e. TestBase *pb),
// it will call the Print() function of the actual class type that the object 
// referenced by pb belongs to.
// Surely this is hard to understand and hard to explain. If you get stuck here,
// don't worry. I will find another example to explain better.
class TestBase {
public:
    TestBase(uint32_t id = 0) : m_id(id) {}
    virtual void Print() {
        PRINT("TestBase m_id = %d\n\r", m_id);
    }
protected:
    uint32_t m_id;
};

class TestDerived1 : public TestBase {
public:
    // Example of default constructor. If no parameter is provided id assumes
    // 127.
    // See how it uses the member initialization list to initialize the base
    // class by calling the base class constructor.
    TestDerived1(uint32_t id = 127) : TestBase(id) {}
    
    // This member function overrides the virtual function defined in the base class.
    // Note the keywork "virtual" is optional in derived classes and is omitted here.
    void Print() {
        PRINT("TestDerived1 m_id = 0x%x\n\r", m_id);
    }
    // This derived class does not have any data members, but it "inherits" them
    // from all of its base class(es). In this case it inherits m_id from TestBase.
};


}

#endif //