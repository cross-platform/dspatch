/************************************************************************
DSPatch - The C++ Flow-Based Programming Framework
Copyright (c) 2012-2018 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LGPLv3.txt included in the packaging of this
file. Please review the following information to ensure the GNU Lesser
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

namespace DSPatch
{

class Circuit;

namespace internal
{
class Component;
class CircuitThread;
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

In order for a component to do any work it must be ticked over. This is performed by repeatedly
calling the Tick() and Reset() methods. The Tick() method is responsible for acquiring the next set
of input signals from component input wires and populating the component's input bus. To insure
that these inputs are up-to-date, the dependent component first calls all of its input components'
Tick() methods - hence recursively called in all components going backward through the circuit
(This is what's classified as a "pull system"). The acquired input bus is then passed to the
Process_() method. The Reset() method then informs the component that the last circuit traversal
has completed and hence can execute the next Tick() request. A component's Tick() and Reset()
methods can be called in a loop from the main application thread, or alternatively, by calling
StartAutoTick(), a separate thread will spawn, automatically calling Tick() and Reset() methods
continuously (This is most commonly used to tick over an instance of Circuit).
*/

class DLLEXPORT Component : public std::enable_shared_from_this<Component>
{
public:
    NONCOPYABLE( Component );
    DEFINE_PTRS( Component );

    Component();
    virtual ~Component();

    void Tick();
    void Reset();

    virtual void StartAutoTick();
    virtual void StopAutoTick();
    virtual void PauseAutoTick();
    virtual void ResumeAutoTick();

    bool ConnectInput( Component::SPtr const& fromComponent, int fromOutput, int toInput );

    void DisconnectInput( int inputNo );
    void DisconnectInput( Component::SCPtr const& fromComponent );
    void DisconnectAllInputs();

    int GetInputCount() const;
    int GetOutputCount() const;

    std::string GetInputName( int inputNo ) const;
    std::string GetOutputName( int outputNo ) const;

protected:
    virtual void Process_( SignalBus const&, SignalBus& ) = 0;

    void SetInputCount_( int inputCount, std::vector<std::string> const& inputNames = {} );
    void SetOutputCount_( int outputCount, std::vector<std::string> const& outputNames = {} );

private:
    // Private methods required by Circuit

    void _SetParentCircuit( std::shared_ptr<Circuit> const& parentCircuit );
    std::shared_ptr<Circuit const> _GetParentCircuit();

    void _SetBufferCount( int bufferCount );
    int _GetBufferCount();

    bool _MoveInputSignal( int bufferIndex, int signalIndex, Signal::SPtr const& signal );
    Signal::SPtr _GetOutputSignal( int bufferIndex, int signalIndex );

    // Private methods required by CircuitThread

    void _ThreadTick( int threadNo );
    void _ThreadReset( int threadNo );

private:
    friend class Circuit;
    friend class internal::CircuitThread;

    std::unique_ptr<internal::Component> p;
};

}  // namespace DSPatch
