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

#include "dspatch/Circuit.h"
#include "dspatch/Plugin.h"

/**

\mainpage The Refreshingly Simple C++ Dataflow Framework

\section intro_sec Introduction
    DSPatch, pronounced "dispatch", is a powerful C++ dataflow framework. DSPatch is not limited to
    any particular domain or data type, from reactive programming to stream processing, DSPatch's
    generic, object-oriented API allows you to create virtually any graph processing system imaginable.

    DSPatch is designed around the concept of a "circuit" that contains "components" interconnected
    via "wires" that transfer "signals" to and from I/O "buses".

    The two most important classes to consider are DSPatch::Component and DSPatch::Circuit. In
    order to route data to and from components they must be added to a circuit, where they can be
    wired together.

    The DSPatch engine takes care of data transfers between interconnected components. When data is
    ready for a component to process, a callback: "Process_()" is executed in that component. For a
    component to form part of a DSPatch circuit, designers simply have to derive their component
    from the DSPatch::Component base class, configure the component's IO buses, and implement the
    virtual Process_() callback method.

\n

\section features_sec Features
    - <b>Automatic branch synchronization</b> - The result of data diverging across parallel
    branches is guaranteed to arrive synchronized at a converging point.
    - <b>Component plugins</b> - Package components into plugins to be dynamically loaded into
    other host applications.
    - <b>Cross-platform</b> - DSPatch is built and tested daily on Linux, Mac and Windows.
    <a href="https://www.youtube.com/watch?v=u3A9x9bpbdo"><b>Here</b></a> we see DSPatch running
    flawlessly on a BeagleBone!
    - <b>Easy-to-use header-only API</b> - DSPatch is modelled around real-world circuit
    entities and concepts, making code more readable and easy to understand.
    - <b>High performance multi-buffering</b> - Utilize parallel multi-buffering via
    Circuit::SetBufferCount() to maximize dataflow efficiency in stream processing circuits.
    - <b>High performance multi-threading</b> - Utilize parallel multi-threading via
    Circuit::SetThreadCount() to maximize dataflow efficiency across parallel branches.
    - <b>Feedback loops</b> - Create true closed-circuit systems by feeding component outputs back
    into previous component inputs (supported in multi-buffered circuits but not multi-threaded).
    - <b>Optimised signal transfers</b> - Wherever possible, data between components is transferred
    via move rather than copy.
    - <b>Run-time adaptive signal types</b> - Component inputs can accept values of run-time
    varying types allowing you to create more flexible, multi-purpose component processes.
    - <b>Run-time circuit wiring</b> - Connect and disconnect wires on the fly whilst maintaining
    steady dataflow through the system.

\n

\section start_sec Getting Started
    1. Download DSPatch:
        - <a href="https://github.com/cross-platform/dspatch"><b>GitHub repository</b></a>
        - <a href="https://github.com/cross-platform/dspatch-template"><b>DSPatch project template</b></a>
    2. \ref tutorial_sec "Read the tutorials"
    3. <a href="https://github.com/cross-platform/dspatchables/tree/master/Components"><b>Browse some example components</b></a>
    4. <a href="annotated.html"><b>Refer to the API docs</b></a>

\n

\section tutorial_sec Tutorials

\subsection create_component 1. Creating a component
    In order to create a new component, we must derive our component class from the
    DSPatch::Component base class, configure component IO, and implement the inherited virtual
    "Process_()" method.

    Lets take a look at how we would go about creating a very simple boolean logic "AND" component.
    This component will accept 2 boolean input values and output the result of: input 1 && input 2.

    We begin by deriving our new "AndBool" component from Component:

    \code
//  1. Derive AndBool class from Component
//  ======================================
class AndBool final : public DSPatch::Component
{
    \endcode

    The next step is to configure our component's input and output buses. This is achieved by
    calling the base protected methods: SetInputCount_() and SetOutputCount_() respectively from our
    component's constructor. In our component's case, we require 2 inputs and 1 output, therefore
    our constructor code will look like this:

    \code
public:
//  2. Configure component IO buses
//  ===============================
    AndBool()
    {
        // add 2 inputs
        SetInputCount_( 2 );

        // add 1 output
        SetOutputCount_( 1 );
    }
    \endcode

    Lastly, our component must implement the virtual Process_() method. This is where our component
    does its work. The Process_() method provides us with 2 arguments: the input bus and the
    output bus. It is our duty as the component designer to pull the inputs we require out of the
    input bus, process them accordingly, then populate the output bus with the results.

    Our component's process method will look something like this:

    \code
protected:
//  3. Implement virtual Process_() method
//  ======================================
    void Process_( DSPatch::SignalBus& inputs, DSPatch::SignalBus& outputs ) override
    {
        // create some local pointers to hold our input values
        auto bool1 = inputs.GetValue<bool>( 0 );
        auto bool2 = inputs.GetValue<bool>( 1 );

        // check first that our component has received valid inputs
        if( bool1 && bool2 )
        {
            // set the output as the result of bool1 AND bool2
            outputs.SetValue( 0, *bool1 && *bool2 );
        }
    }
};
    \endcode

    Our component is now ready to form part of a DSPatch circuit. Next we'll look at how we can add
    our component to a circuit and route it to and from other components.

\n

\subsection use_component 2. Building a circuit
    In order for us to get any real use out of our components, we need them to interact with
    each other. This is where the DSPatch::Circuit class comes in. A circuit is a workspace for
    adding and routing components. In this section we will have a look at how to create a simple
    DSPatch application that generates random boolean pairs, performs a logic AND on each pair,
    then prints the result to the screen.

    First we must include the DSPatch header and any other headers that contain components we
    wish to use in our application:

    \code
#include "components.h"

#include <DSPatch.h>
    \endcode

    Next, we must instantiate our circuit object and all component objects needed for our
    circuit. Lets say we had 2 other components included with "AndBool" (from the first tutorial):
    "GenBool" (generates a random boolean value then outputs the result) and "PrintBool"
    (receives a boolean value and outputs it to the console):

    \code
int main()
{
//  1. Create a circuit where we can route our components
//  =====================================================
    auto circuit = std::make_shared<DSPatch::Circuit>();

//  2. Create instances of the components needed for our circuit
//  ============================================================
    auto genBool1 = std::make_shared<GenBool>();
    auto genBool2 = std::make_shared<GenBool>();
    auto andBool = std::make_shared<AndBool>();
    auto printBool = std::make_shared<PrintBool>();
    \endcode

    Now that we have a circuit and some components, lets add all of our components to the circuit:

    \code
//  3. Add component instances to circuit
//  =====================================
    circuit->AddComponent( genBool1 );
    circuit->AddComponent( genBool2 );
    circuit->AddComponent( andBool );
    circuit->AddComponent( printBool );
    \endcode

    We are now ready to begin wiring the circuit:

    \code
//  4. Wire up the components inside the circuit
//  ============================================
    circuit->ConnectOutToIn( genBool1, 0, andBool, 0 );
    circuit->ConnectOutToIn( genBool2, 0, andBool, 1 );
    circuit->ConnectOutToIn( andBool, 0, printBool, 0 );
    \endcode

    The code above results in the following wiring configuration:
    \verbatim
  __________            _________
 |          |          |         |
 | genBool1 |-0 ===> 0-|         |           ___________
 |__________|          |         |          |           |
  __________           | andBool |-0 ===> 0-| printBool |
 |          |          |         |          |___________|
 | genBool2 |-0 ===> 1-|         |
 |__________|          |_________|

    \endverbatim

    Lastly, in order for our circuit to do any work it must be ticked. This is performed by
    repeatedly calling the circuit's Tick() method. This method can be called manually in a loop,
    or alternatively, by calling StartAutoTick(), a seperate thread will spawn, automatically
    calling Tick() continuously.

    Furthermore, to boost performance in stream processing circuits like this one, multi-buffering
    can be enabled via the SetBufferCount() method:

    <b>NOTE:</b> If none of the parallel branches in your circuit are time-consuming (⪆10μs),
    multi-buffering (or even zero buffering) will almost always outperform multi-threading
    (via SetThreadCount()). The contention overhead caused by multiple threads processing a single
    tick must be made negligible by time-consuming parallel components for any performance
    improvement to be seen.

    \code
//  5. Tick the circuit
//  ===================

    // Circuit tick method 1: Manual
    for( int i = 0; i < 10; ++i )
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
    \endcode

    That's it! Enjoy using DSPatch!

    (<b>NOTE:</b> The source code for the above tutorials can be found under the "tutorial" folder
    in the <a href="https://github.com/cross-platform/dspatch"><b>DSPatch root directory</b></a>).

*/
