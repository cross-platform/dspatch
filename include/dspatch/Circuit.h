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
DisconnectOutToIn provide a means of routing component outputs to component inputs.

*N.B. Each component input can only accept one wire at a time. When a wire is connected to an input
that already has a connected wire, that wire is replaced with the new one. One output, on the other
hand, can be distributed to multiple inputs.

For process intensive circuits, multi-threaded processing can be enabled via the SetThreadCount()
method. The Circuit class allows the user to specify the number of threads in which he/she requires
the circuit to process (0 threads: multi-threading disabled). A circuit's thread count can be
adjusted at runtime, allowing the user to increase / decrease the number of threads as required
during execution.

Circuit is derived from Component and therefore inherits all Component behavior. This means that a
circuit object needs to be Tick()ed and Reset()ed as a component (see Component). The Circuit
Process_() method simply runs through it's internal array of components and calls each component's
Tick() and Reset() methods.
*/

class DLLEXPORT Circuit final : public std::enable_shared_from_this<Circuit>
{
public:
    NONCOPYABLE( Circuit );
    DEFINE_PTRS( Circuit );

    Circuit();
    virtual ~Circuit();

    int AddComponent( Component::SPtr const& component );

    void RemoveComponent( Component::SCPtr const& component );
    void RemoveComponent( int component );
    void RemoveAllComponents();

    int GetComponentCount() const;

    bool ConnectOutToIn( Component::SCPtr const& fromComponent, int fromOutput, Component::SCPtr const& toComponent, int toInput );
    bool ConnectOutToIn( Component::SCPtr const& fromComponent, int fromOutput, int toComponent, int toInput );
    bool ConnectOutToIn( int fromComponent, int fromOutput, Component::SCPtr const& toComponent, int toInput );
    bool ConnectOutToIn( int fromComponent, int fromOutput, int toComponent, int toInput );

    void DisconnectComponent( Component::SCPtr const& component );
    void DisconnectComponent( int componentIndex );

    void SetThreadCount( int threadCount );
    int GetThreadCount() const;

    void Tick();

    void StartAutoTick();
    void StopAutoTick();
    void PauseAutoTick();
    void ResumeAutoTick();

private:
    std::unique_ptr<internal::Circuit> p;
};

}  // namespace DSPatch
