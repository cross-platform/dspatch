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

#include <dspatch/SignalBus.h>

#include <condition_variable>
#include <string>

namespace DSPatch
{

namespace internal
{
class Component;
}  // namespace internal

/// Abstract base class for DSPatch components

/**
Classes derived from Component can be added to a Circuit and routed to and from other Components.

On construction, derived classes must configure the component's IO buses by calling
SetInputCount_() and SetOutputCount_() respectively.

Derived classes must also implement the virtual method: Process_(). The Process_() method is a
callback from the DSPatch engine that occurs when a new set of input signals is ready for
processing. The Process_() method has 2 arguments: the input bus, and the output bus. This method's
purpose is to pull its required inputs out of the input bus, process these inputs, and populate the
output bus with the results (see SignalBus).

In order for a component to do any work it must be ticked. This is performed by repeatedly calling
the Tick() and Reset() methods. The Tick() method is responsible for acquiring the next set of
input signals from component input wires and populating the component's input bus. To insure that
these inputs are up-to-date, the dependent component first calls all of its input components'
Tick() methods - hence recursively called in all components going backward through the circuit. The
acquired input bus is then passed to the Process_() method. The Reset() method informs the
component that the last circuit traversal has completed and hence can execute the next Tick()
request.
*/

class DLLEXPORT Component
{
public:
    NONCOPYABLE( Component );
    DEFINE_PTRS( Component );

    enum class ProcessOrder
    {
        InOrder,
        OutOfOrder
    };

    Component( ProcessOrder processOrder = ProcessOrder::InOrder );
    virtual ~Component();

    bool ConnectInput( Component::SPtr const& fromComponent, int fromOutput, int toInput );

    void DisconnectInput( int inputNo );
    void DisconnectInput( Component::SCPtr const& fromComponent );
    void DisconnectAllInputs();

    int GetInputCount() const;
    int GetOutputCount() const;

    std::string GetInputName( int inputNo ) const;
    std::string GetOutputName( int outputNo ) const;

    void SetBufferCount( int bufferCount );
    int GetBufferCount() const;

    void Tick( int bufferNo = 0 );
    void Reset( int bufferNo = 0 );

protected:
    virtual void Process_( SignalBus const&, SignalBus& ) = 0;

    void SetInputCount_( int inputCount, std::vector<std::string> const& inputNames = {} );
    void SetOutputCount_( int outputCount, std::vector<std::string> const& outputNames = {} );

private:
    std::unique_ptr<internal::Component> p;
};

}  // namespace DSPatch
