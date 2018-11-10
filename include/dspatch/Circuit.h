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
Components can be added to a Circuit via the AddComponent() method, and routed to and from other
components via the ConnectOutToIn() methods.

<b>NOTE:</b> Each component input can only accept a single "wire" at a time. When a wire is
connected to an input that already has a connected wire, that wire is replaced with the new one.
One output, on the other hand, can be distributed to multiple inputs.

For process intensive circuits, multi-threaded processing can be enabled via the SetThreadCount()
method (0 threads = multi-threading disabled). A circuit's thread count can be adjusted at runtime,
allowing the user to increase / decrease the number of threads as required during execution.

The Circuit Tick() method runs through it's internal array of components and calls each component's
Tick() and Reset() methods once. A circuit's Tick() method can be called in a loop from the main
application thread, or alternatively, by calling StartAutoTick(), a separate thread will spawn,
automatically calling Tick() continuously until PauseAutoTick() or StopAutoTick() is called.
*/

class DLLEXPORT Circuit final
{
public:
    NONCOPYABLE( Circuit );
    DEFINE_PTRS( Circuit );

    Circuit();
    ~Circuit();

    int AddComponent( Component::SPtr const& component );

    void RemoveComponent( Component::SCPtr const& component );
    void RemoveComponent( int componentIndex );
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
