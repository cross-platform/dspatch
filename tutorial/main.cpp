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

#include "components.h"

#include <DSPatch.h>

//  The code below results in the following wiring configuration:
//   __________            _________
//  |          |          |         |
//  | genBool1 |-0 ===> 0-|         |           ___________
//  |__________|          |         |          |           |
//   __________           | andBool |-0 ===> 0-| printBool |
//  |          |          |         |          |___________|
//  | genBool2 |-0 ===> 1-|         |
//  |__________|          |_________|

int main()
{
    // 1. Create a circuit where we can route our components
    // =====================================================
    auto circuit = std::make_shared<DSPatch::Circuit>();

    // 2. Create instances of the components needed for our circuit
    // ============================================================
    auto genBool1 = std::make_shared<GenBool>();
    auto genBool2 = std::make_shared<GenBool>();
    auto andBool = std::make_shared<AndBool>();
    auto printBool = std::make_shared<PrintBool>();

    // 3. Add component instances to circuit
    // =====================================
    circuit->AddComponent( genBool1 );
    circuit->AddComponent( genBool2 );
    circuit->AddComponent( andBool );
    circuit->AddComponent( printBool );

    // 4. Wire up the components inside the circuit
    // ============================================
    circuit->ConnectOutToIn( genBool1, 0, andBool, 0 );
    circuit->ConnectOutToIn( genBool2, 0, andBool, 1 );
    circuit->ConnectOutToIn( andBool, 0, printBool, 0 );

    // 5. Tick the circuit
    // ===================

    // Circuit tick method 1: Manual
    for ( int i = 0; i < 10; ++i )
    {
        circuit->Tick();
    }

    // Circuit tick method 2: Automatic
    std::cout << "Press any key to begin circuit auto-tick.";
    getchar();
    circuit->StartAutoTick();

    // Increase circuit buffer count for higher performance
    getchar();
    circuit->SetBufferCount( 4 );

    // Press any key to quit
    getchar();
    return 0;
}
