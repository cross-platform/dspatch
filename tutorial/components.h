/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2025, Marcus Tomlinson

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

#include <DSPatch.h>

#include <cstdlib>
#include <ctime>
#include <iostream>

// AndBool:
// AndBool has 2 inputs and 1 output.
// This component performs a logic AND on 2 boolean input values and outputs the result.

// 1. Derive AndBool class from Component
// ======================================
class AndBool final : public DSPatch::Component
{
public:
    // 2. Configure component IO buses
    // ===============================
    AndBool()
        // the order in which buffers are Process_()'ed is not important
        : Component( ProcessOrder::OutOfOrder )
    {
        // add 2 inputs
        SetInputCount_( 2 );

        // add 1 output
        SetOutputCount_( 1 );
    }

protected:
    // 3. Implement virtual Process_() method
    // ======================================
    void Process_( DSPatch::SignalBus& inputs, DSPatch::SignalBus& outputs ) override
    {
        // create some local pointers to hold our input values
        auto bool1 = inputs.GetValue<bool>( 0 );
        auto bool2 = inputs.GetValue<bool>( 1 );

        // check first that our component has received valid inputs
        if ( bool1 && bool2 )
        {
            // set the output as the result of bool1 AND bool2
            outputs.SetValue( 0, *bool1 && *bool2 );
        }
    }
};

// GenBool:
// GenBool has 1 output.
// This component generates a random boolean value then outputs the result.

class GenBool final : public DSPatch::Component
{
public:
    GenBool()
        // the order in which buffers are Process_()'ed is not important
        : Component( ProcessOrder::OutOfOrder )
    {
        // add 1 output
        SetOutputCount_( 1 );

        // seed randomizer
        srand( static_cast<unsigned int>( time( nullptr ) ) );
    }

protected:
    void Process_( DSPatch::SignalBus&, DSPatch::SignalBus& outputs ) override
    {
        // set output as randomized true / false
        outputs.SetValue( 0, rand() % 2 == 0 );
    }
};

// PrintBool:
// PrintBool has 1 input.
// This component receives a boolean value and outputs it to the console.

class PrintBool final : public DSPatch::Component
{
public:
    PrintBool()
        // here, the order in which buffers are Process_()'ed is important
        : Component( ProcessOrder::InOrder )
    {
        // add 1 input
        SetInputCount_( 1 );
    }

protected:
    void Process_( DSPatch::SignalBus& inputs, DSPatch::SignalBus& ) override
    {
        // create a local stack variable to hold input value
        auto inputBool = inputs.GetValue<bool>( 0 );

        // get boolean value from inputs bus
        if ( inputBool )
        {
            // print "true" / "false" depending on boolean value received
            if ( *inputBool )
            {
                std::cout << "true" << '\n';
            }
            else
            {
                std::cout << "false" << '\n';
            }
        }
    }
};
