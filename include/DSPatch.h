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
    to create almost any system imaginable, from simple logic circuits to high performance audio
    process chains.

    DSPatch is designed around the concept of a "circuit" that contains "components"
    interconnected via "wires" that transfer "signals" to and from component I/O "buses". For
    more detail on how DSPatch works internally, check out the <a href="spec_page.html">
    <b>DSPatch Design Specification</b></a>.

    The two most important classes to consider are DSPatch::Component and DSPatch::Circuit.
    In order to route data to and from components they must be added to a circuit, where they can
    be wired together.

    The DSPatch engine takes care of data transfer between interconnected components. When data
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
    - <b>Easy-to-use object-oriented API</b> - DSPatch is modeled around real-world circuit
    entities and concepts, making code more readable and easy to understand.
    - <b>Feedback loops</b> - Create true closed-circuit systems by feeding resultant signals back
    into previous component inputs.
    - <b>High performance parallel processing</b> - Circuits use advanced multi-threaded scheduling
    to maximize data flow efficiency.
    - <b>Integrated circuits</b> - Build circuits within circuits to encapsulate complex component
    networks into single circuit components.
    - <b>Optimised signal transfers</b> - Wherever possible, data between components is transfered
    via move rather than copy.
    - <b>Run-time adaptive signal types</b> - Component inputs can accept values of run-time
    varying types allowing you to create more flexible, multi-purpose component processes.
    - <b>Run-time circuit wiring</b> - Connect and disconnect wires on the fly whilst maintaining
    steady data flow through the system.
    - <b>Run-time thread count adjustment</b> - Specify at run-time, the number of threads in which
    you require a circuit to process.

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

    Lastly, in order for our circuit to do any work it must be ticked over. This is performed by
    repeatedly calling the circuit's Tick() and Reset() methods. These methods can be called
    manually in a loop from the main application thread, or alternatively, by calling
    StartAutoTick(), a seperate thread will spawn, automatically calling Tick() and Reset()
    continuously.

    A circuit's thread count can be adjusted at runtime, allowing us to increase / decrease the
    number of threads use by the circuit as required during execution:

    \code
    // 5. Tick the circuit
    // ===================

    // Circuit tick method 1: Manual
    for( int i = 0; i < 10; i++ )
    {
        circuit->Tick();
        circuit->Reset();
    }

    // Circuit tick method 2: Automatic
    std::cout << "Press any key to begin circuit auto-tick.";
    getchar();
    circuit->StartAutoTick();

    // Increase Circuit Thread count for higher performance
    getchar();
    circuit->SetThreadCount( 4 );

    // Press any key to quit
    getchar();
    return 0;
}
    \endcode

    That's it! Enjoy using DSPatch!

    (<b>NOTE:</b> The source code for the above tutorials can be found under the "tutorial" folder
    in the <a href="https://github.com/MarcusTomlinson/DSPatch"><b>DSPatch root directory</b></a>).

\page spec_page DSPatch Design Specification

1. The Circuit Concept:
-----------------------

A circuit is comprised of a collection of interconnected components. Each
component has 2 signal buses, on one end of the component there are inputs
(the "input bus"), and on the other end there are outputs (the "output bus").
Components within the circuit are connected to each other via wires. Each
wire carries a signal from one component's output to another component's
input. A circuit can also comprise of interconnected circuits - in this case
a circuit acts as a component within another circuit.

______________________________________________________________________________

2. The DSPatch Circuit System:
---------------------------------

<b>2.1 Structure:</b>

The nouns above are the classes we require in order to model our circuit in
code. Each component will contain an array of input wires. Each wire contains
references to the source component, the source output, and the destination input.
The signal bus class will contain an array of signals, and lastly, the Circuit
class will be derived from Component and will contain an array of internal
components.

<b>2.2 Behavior:</b>

<b>2.2.1 Component:</b>

The Component class will have a Tick() method responsible for acquiring its
next set of inputs from its input wires and populating the component's input
bus. To insure that these inputs are up-to-date, the dependent component
first calls all of its input components' Tick() methods - hence recursively
called in all components going backward through the circuit. The acquired
input bus is then passed into a virtual method: Process_() - it is the
responsibility of the (derived) component creator to implement this virtual
method. The Process_() method has 2 input arguments: the input bus and the
output bus. This method's purpose is to pull its required inputs out of the
input bus, process these inputs, then populate the output bus with the results.
These resultant outputs in the output bus are then acquired by dependent
components via their Tick() methods.

The Component class will also have a Reset() method. For optimization as well
as to avoid feedback deadlocks, a component needs to be aware of whether or
not it has already ticked during a circuit traversal so that if asked to
Tick() again, it can ignore the call. The Reset() method informs the
component that the last circuit traversal has completed and hence may execute
the next Tick() request.

<b>2.2.2 Circuit:</b>

In order to satisfy the statement above ("circuit acts as a component"), the
Circuit class is derived from the Component class. This means that the
Circuit class has both Tick() and Process_() methods. The Tick() method will
execute as normal, acquiring inputs for the circuit to process. This allows
us to expose the IO we require for internal components via the circuit's
input and output buses. Circuit IO-to-component wires, and
component-to-component wires, will be publicly routable via Circuit class
methods. The Circuit class' virtual Process_() method is implemented as such:

- Inputs from the circuit's input bus are moved to their respective
internal component input buses.
- All internal components are Tick()ed.
- All internal components are Reset()ed.
- The circuit output bus is populated with the respective internal component
outputs.

All actions in respect to the circuit and the components within the circuit
will be made available via public methods in the Circuit class. The circuit
object user will be able to add/remove components, connect/disconnect wires,
and set/get circuit IO count.

<b>2.2.3 Signal:</b>

When it comes to transferring signals between components we require the same
level of abstraction for the data being moved around:

The base Component class needs to supply its child class with any number of
inputs and outputs via the virtual Process_() method. These inputs and outputs
may also need to be of different types. This requires a generic way of
containing variables of different types in a single collection - the signal
bus.

The signal class will hold a variable that can be dynamically typed at
run-time, which we'll call "run-type". The run-type and signal classes make
use of template methods to allow object users to set and get the contained
variable as any type they wish. The run-type (and hence, a signal) has the
ability to change type at any point during program execution - this can be
useful for inputs that can accept a number of different types of data (E.g.
Varying sample size in an audio buffer: array of byte / int / float)

From the Process_() method, a derived component can get and set the signals
it requires of the provided signal buses via public methods. As the component
creator is responsible for configuring the component's IO buses, the types
held within those buses can be assumed, and hence, read and written to
accordingly. Built-in typecasting and error checking prevents critical
run-time errors from occurring when signal types are mismatched.

______________________________________________________________________________

3. Parallel Circuit Processing:
-------------------------------

The multi-threading aspect of DSPatch is designed to allow the library user
the ability to specify the number of threads in which he/she requires the circuit
to process, rather than the thread count growing as the system does. So for
example, an application running on a quad core CPU could be limited to 4 threads
in order to allow each core to handle just one thread.

<b>3.1 The Circuit Thread:</b>

Circuit Threads are threads that traverse entire circuits. The circuit runs
through its array of components, calling each components' process method in a
single thread (the Circuit Thread) loop. As each component is done processing,
it hands over control to the next waiting Circuit Thread. Therefore, if you had
5 components in a process chain, and 5 Circuit Threads, at any point in time you
could have one thread per component processing in parallel.

With this in place, you now also have the option to select 0 Circuit Threads.
In this state, the circuit's Tick() and Reset() methods will block the calling
thread while all components in the circuit are processed, whereas with circuit
threads enabled, the calling thread will block only if all Circuit Threads are
busy.

<b>3.2 The Component Thread:</b>

The Component Thread simply ticks a single component over and over. As the
Circuit class inherits from Component, we can use its Component Thread to
"auto-tick" the circuit in order to free up the main application thread for
control.*/
