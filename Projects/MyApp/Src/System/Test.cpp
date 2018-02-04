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

#include "Test.h"

using namespace FW;

// QP provided macro to define filename used by Q_ASSERT. 
//Q_DEFINE_THIS_FILE

namespace APP {

// Static data member has to be defined in a CPP file outside the class 
// definition (with the exception of constant integral type).
uint32_t TestClass::m_c = 123;

// When an object is defined in "global" scope (vs within a function), the 
// constructor is called even before main() is called! As a result, you wouldn't
// see the debug message printed in the constructor (since the UART port wasn't 
// even initialized).
TestClass globalTestObj(5, 6);

// This is where our test code is located (before we have a unit test framework).
void TestFunction() {
    // Old way of defining struct. Here memory is allocated on stack for struct
    // instances ts1 and ts2.
    /*
    TestStruct ts1 = { 1, 2 };
    TestStruct ts2 = { 3, 4 };
    */
  
    // Here memory is allocated on stack for objects testObj and testObj2.
    TestClass testObj(1,2);
    TestClass testObj2(3,4);
    PRINT("testObj contains %d %d.\n\r", testObj.GetA(), testObj.GetB()); 
    PRINT("testObj2 contains %d %d.\n\r", testObj2.GetA(), testObj2.GetB()); 
    PRINT("Sum of testObj1 = %d (m_c incremented to %d).\n\r", testObj.Sum(), testObj.m_c); 
    PRINT("Sum of testObj2 = %d (m_c incremented to %d).\n\r", testObj2.Sum(), testObj.m_c); 
    
    // Inheritance and polymorphism examples.
    TestBase tb(100);       // m_id of tb has value 100.
    TestDerived1 td1;       // m_id of td1 has value 127 due to default constructor.
                            // Note m_id of td1 is inherited from TestBase.
                            // In memory layout it is located in the base "part"
                            // of the object.
    TestBase *pb = NULL;    // NOT instantiating any objects here. pb is just
                            // a pointer to any object of class TestBase or 
                            // of a class derived from TestBase.
    
    // First set pb to point to object tb (of class TestBase).
    pb = &tb;
    // TestBase::Print() is called.
    pb->Print();
    // Repeat the test by setting pb to point to object td1 (of class TestDerived1).
    pb = &td1;
    // TestDerived1::Print() is called.
    pb->Print();
    // Key point - we can now use a base class pointer (i.e. pb), through 
    // virtual function, to call the "correct" version of the member function 
    // based on the actual type of the object pointed to by the base class 
    // pointer (pb). In short it allows generalization on one hand and 
    // customization on the other.
    
    // Example showing the use of ASSERT, a common way to catch bug.
    //Q_ASSERT((int)pb==0x12345678);
}  

} // namespace APP