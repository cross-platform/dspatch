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

#include <dspatch/Component.h>

namespace DSPatch
{

namespace internal
{
class Circuit;
}

/// Workspace for adding and routing components

/**
Components can be added to a Circuit and routed to and from other Components. ConnectOutToIn and
DisconnectOutToIn provide a means of routing component outputs to other component inputs, while
ConnectInToIn / DisconnectInToIn and ConnectOutToOut / DisconnectOutToOut route the circuit's IO
signals to and from it's internal components.

*N.B. Each component input can only accept one wire at a time. When a wire is connected to an input
that already has a connected wire, that wire is replaced with the new one. One output, on the other
hand, can be distributed to multiple inputs.

For process intensive circuits, multi-threaded processing can be enabled via the SetThreadCount()
method. The Circuit class allows the user to specify the number of threads in which he/she requires
the circuit to process (0 threads: multi-threading disabled). A circuit's thread count can be
adjusted at runtime, allowing the user to increase / decrease the number of threads as required
during execution.

Circuit is derived from Component and therefore inherits all Component behavior. This means that a
Circuit can be added to, and routed within another Circuit as a component. This also means a
circuit object needs to be Tick()ed and Reset()ed as a component (see Component). The Circuit
Process_() method simply runs through it's internal array of components and calls each component's
Tick() and Reset() methods.
*/

class DLLEXPORT Circuit final : public Component
{
public:
    NONCOPYABLE( Circuit );
    DEFINE_PTRS( Circuit );

    Circuit( int threadCount = 0 );
    virtual ~Circuit();

    virtual void PauseAutoTick() override;

    void SetInputCount( int inputCount );
    void SetOutputCount( int outputCount );

    void SetThreadCount( int threadCount );
    int GetThreadCount() const;

    int AddComponent( Component::SPtr const& component );

    void RemoveComponent( Component::SCPtr const& component );
    void RemoveComponent( int component );
    void RemoveAllComponents();

    int GetComponentCount() const;

    // Component output to component input
    bool ConnectOutToIn( Component::SCPtr const& fromComponent, int fromOutput, Component::SCPtr const& toComponent, int toInput );
    bool ConnectOutToIn( Component::SCPtr const& fromComponent, int fromOutput, int toComponent, int toInput );
    bool ConnectOutToIn( int fromComponent, int fromOutput, Component::SCPtr const& toComponent, int toInput );
    bool ConnectOutToIn( int fromComponent, int fromOutput, int toComponent, int toInput );

    // Circuit input to component input
    bool ConnectInToIn( int fromInput, Component::SCPtr const& toComponent, int toInput );
    bool ConnectInToIn( int fromInput, int toComponent, int toInput );

    // Component output to circuit output
    bool ConnectOutToOut( Component::SCPtr const& fromComponent, int fromOutput, int toOutput );
    bool ConnectOutToOut( int fromComponent, int fromOutput, int toOutput );

    void DisconnectComponent( Component::SCPtr const& component );
    void DisconnectComponent( int componentIndex );

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

private:
    std::unique_ptr<internal::Circuit> p;
};

}  // namespace DSPatch
