/************************************************************************
DSPatch - The C++ Flow-Based Programming Framework
Copyright (c) 2012-2018 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LICENSE included in the packaging of this file.
Please review the following information to ensure the GNU Lesser
General Public License version 3.0 requirements will be met:
http://www.gnu.org/copyleft/lgpl.html.

Other Usage
Alternatively, this file may be used in accordance with the terms and
conditions contained in a signed written agreement between you and
Marcus Tomlinson.

DSPatch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
************************************************************************/

#pragma once

#include <dspatch/Circuit.h>
#include <dspatch/Plugin.h>

/**

\mainpage The C++ Flow-Based Programming Framework

\section intro_sec Introduction
    DSPatch, pronounced "dispatch", is a powerful C++
    <a href="http://www.jpaulmorrison.com/fbp/">flow-based programming</a> framework. DSPatch is
    not limited to any particular domain or data type, its generic, object-oriented API allows you
    to create almost any dataflow system imaginable, from simple logic circuits to high performance
    audio process chains.

    DSPatch is designed around the concept of a "circuit" that contains "components"
    interconnected via "wires" that transfer "signals" to and from component I/O "buses".

    The two most important classes to consider are DSPatch::Component and DSPatch::Circuit.
    In order to route data to and from components they must be added to a circuit, where they can
    be wired together.

    The DSPatch engine takes care of data transfers between interconnected components. When data
    is ready for a component to process, a callback: "Process_()" is executed in that component.
    For a component to form part of a DSPatch circuit, designers simply have to derive their
    component from the DSPatch::Component base class, configure the component's IO buses, and
    implement the virtual Process_() callback method.

\n

\section features_sec Features
    - <b>Automatic branch synchronization</b> - The result of data diverging across parallel
    branches is guaranteed to arrive synchronized at a converging point.
    - <b>Component plugins</b> - Package components into plugins to be dynamically loaded into
    other host applications.
    - <b>Cross-platform</b> - DSPatch is built and tested daily on Linux, Mac and Windows.
    <a href="https://www.youtube.com/watch?v=u3A9x9bpbdo"><b>Here</b></a> we see DSPatch running
    flawlessly on a BeagleBone!
    - <b>Easy-to-use object-oriented API</b> - DSPatch is modelled around real-world circuit
    entities and concepts, making code more readable and easy to understand.
    - <b>Feedback loops</b> - Create true closed-circuit systems by feeding resultant signals back
    into previous component inputs.
    - <b>High performance parallel processing</b> - Circuits use advanced multi-threaded scheduling
    to maximize dataflow efficiency across parallel branches.
    - <b>High performance stream processing</b> - Utilize multi-buffering in stream processing
    circuits to further boost dataflow efficiency.
    - <b>Optimised signal transfers</b> - Wherever possible, data between components is transferred
    via move rather than copy.
    - <b>Run-time adaptive signal types</b> - Component inputs can accept values of run-time
    varying types allowing you to create more flexible, multi-purpose component processes.
    - <b>Run-time circuit wiring</b> - Connect and disconnect wires on the fly whilst maintaining
    steady dataflow through the system.

\n

\section start_sec Getting Started
    1. Download DSPatch:
        - Source Code:
            - <a href="https://github.com/MarcusTomlinson/DSPatch"><b>GitHub</b></a>
        - Binaries (Linux):
            - <a href="https://github.com/MarcusTomlinson/DSPatch/archive/build/master/linux-gcc.zip"><b>GCC</b></a> -
<a href="https://github.com/MarcusTomlinson/DSPatch/archive/build/master/linux-clang.zip"><b>Clang</b></a>
        - Binaries (Mac):
            - <a href="https://github.com/MarcusTomlinson/DSPatch/archive/build/master/osx-gcc.zip"><b>GCC</b></a> -
<a href="https://github.com/MarcusTomlinson/DSPatch/archive/build/master/osx-clang.zip"><b>Clang</b></a>
        - Binaries (Windows):
            - <a href="https://github.com/MarcusTomlinson/DSPatch/archive/build/master/win-mingw-x86.zip"><b>MinGW</b></a> -
<a href="https://github.com/MarcusTomlinson/DSPatch/archive/build/master/win-msvc-x86.zip"><b>MSVC</b></a>
    2. \ref tutorial_sec "Read the tutorials"
    3. <a href="https://github.com/MarcusTomlinson/DSPatchables/tree/master/Components"><b>Browse some example components</b></a>
    4. <a href="annotated.html"><b>Refer to the API docs</b></a>

\n

\section tutorial_sec Tutorials

\subsection create_component 1. Creating a component
    In order to create a new component, we must derive our component class from the
    DSPatch::Component base class, configure component IO, and implement the inherited virtual
    "Process_()" method.

    Lets take a look at how we would go about creating a very simple boolean logic "AND" component.
    This component will accept 2 boolean input values and output the result of: input 1 && input 2.

    We begin by deriving our new "And" component from Component:

    \code
// 1. Derive And class from Component
// ==================================
class And final : public Component
{
    \endcode

    The next step is to configure our component's input and output buses. This is achieved by
    calling the base protected methods: SetInputCount_() and SetOutputCount_() respectively from our
    component's constructor. In our component's case, we require 2 inputs and 1 output, therefore
    our constructor code will look like this:

    \code
public:
    // 2. Configure component IO buses
    // ===============================
    And()
    {
        // add 2 inputs
        SetInputCount_( 2 );

        // add 1 output
        SetOutputCount_( 1 );
    }
    \endcode

    Lastly, our component must implement the virtual Process_() method. This is where our component
    does it's work. The Process_() method provides us with 2 arguments: the input bus and the
    output bus. It is our duty as the component designer to pull the inputs we require out of the
    input bus, process them accordingly, then populate the output bus with the results.

    Our component's process method will look something like this:

    \code
protected:
    // 3. Implement virtual Process_() method
    // ======================================
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override
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
#include <DSPatch.h>
#include <components.h>

using namespace DSPatch;
    \endcode

    Next, we must instantiate our circuit object and all component objects needed for our
    circuit. Lets say we had 2 other components included with "And" (from the first tutorial):
    "RandBool" (generates a random boolean value then outputs the result) and "PrintBool"
    (receives a boolean value and outputs it to the console):

    \code
int main()
{
    // 1. Create a circuit where we can route our components
    // =====================================================
    auto circuit = std::make_shared<Circuit>();

    // 2. Create instances of the components needed for our circuit
    // ============================================================
    auto randBoolGen1 = std::make_shared<RandBool>();
    auto randBoolGen2 = std::make_shared<RandBool>();
    auto logicAnd = std::make_shared<And>();
    auto boolPrinter = std::make_shared<PrintBool>();
    \endcode

    Now that we have a circuit and some components, lets add all of our components to the circuit:

    \code
    // 3. Add component instances to circuit
    // =====================================
    circuit->AddComponent( randBoolGen1 );
    circuit->AddComponent( randBoolGen2 );
    circuit->AddComponent( logicAnd );
_   circuit->AddComponent( boolPrinter );
    \endcode

    We are now ready to begin wiring the circuit:

    \code
    // 4. Wire up the components inside the circuit
    // ============================================
    circuit->ConnectOutToIn( randBoolGen1, 0, logicAnd, 0 );
    circuit->ConnectOutToIn( randBoolGen2, 0, logicAnd, 1 );
_   circuit->ConnectOutToIn( logicAnd, 0, boolPrinter, 0 );
    \endcode

    The code above results in the following wiring configuration:
    \code
      ______________            __________
     |              |          |          |
     | randBoolGen1 |-0 ===> 0-|          |           _____________
     |______________|          |          |          |             |
      ______________           | logicAnd |-0 ===> 0-| boolPrinter |
     |              |          |          |          |_____________|
     | randBoolGen2 |-0 ===> 1-|          |
     |______________|          |__________|
    _
    \endcode

    Lastly, in order for our circuit to do any work it must be ticked. This is performed by
    repeatedly calling the circuit's Tick() method. This method can be called manually in a loop
    from the main application thread, or alternatively, by calling StartAutoTick(), a seperate
    thread will spawn, automatically calling Tick() continuously.

    Furthermore, to boost performance in stream processing circuits like this one, multi-buffering
    can be enabled via the SetBufferCount() method:

    \code
    // 5. Tick the circuit
    // ===================

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
    in the <a href="https://github.com/MarcusTomlinson/DSPatch"><b>DSPatch root directory</b></a>).

*/
