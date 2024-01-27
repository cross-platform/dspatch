/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2024, Marcus Tomlinson

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

#include <dspatch/Circuit.h>

#include "internal/AutoTickThread.h"
#include "internal/CircuitThread.h"

#include <algorithm>
#include <map>
#include <set>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class Circuit final
{
public:
    inline void Optimize();

    int pauseCount = 0;

    int bufferCount = 0;
    int currentBuffer = 0;

    AutoTickThread autoTickThread;

    std::vector<DSPatch::Component*> components;
    std::set<DSPatch::Component::SPtr> componentsSet;
    std::map<int, std::set<DSPatch::Component*>> componentsMap;

    std::vector<CircuitThread> circuitThreads;

    bool circuitDirty = false;
};

}  // namespace internal
}  // namespace DSPatch

Circuit::Circuit()
    : p( new internal::Circuit() )
{
}

Circuit::~Circuit()
{
    StopAutoTick();
    SetBufferCount( 0 );

    delete p;
}

bool Circuit::AddComponent( const Component::SPtr& component )
{
    if ( !component || p->componentsSet.find( component ) != p->componentsSet.end() )
    {
        return false;
    }

    // components within the circuit need to have as many buffers as there are threads in the circuit
    component->SetBufferCount( p->bufferCount, p->currentBuffer );

    PauseAutoTick();
    p->components.emplace_back( component.get() );
    ResumeAutoTick();

    p->componentsSet.emplace( component );

    return true;
}

bool Circuit::RemoveComponent( const Component::SPtr& component )
{
    if ( p->componentsSet.find( component ) == p->componentsSet.end() )
    {
        return false;
    }

    auto findFn = [&component]( auto comp ) { return comp == component.get(); };

    if ( auto it = std::find_if( p->components.begin(), p->components.end(), findFn ); it != p->components.end() )
    {
        PauseAutoTick();

        DisconnectComponent( component );

        p->components.erase( it );

        ResumeAutoTick();

        p->componentsSet.erase( component );

        return true;
    }

    return false;
}

// cppcheck-suppress unusedFunction
void Circuit::RemoveAllComponents()
{
    PauseAutoTick();

    p->components.clear();

    ResumeAutoTick();

    p->componentsSet.clear();
}

int Circuit::GetComponentCount() const
{
    return (int)p->components.size();
}

bool Circuit::ConnectOutToIn( const Component::SPtr& fromComponent, int fromOutput, const Component::SPtr& toComponent, int toInput )
{
    if ( p->componentsSet.find( fromComponent ) == p->componentsSet.end() ||
         p->componentsSet.find( toComponent ) == p->componentsSet.end() )
    {
        return false;
    }

    PauseAutoTick();

    bool result = toComponent->ConnectInput( fromComponent, fromOutput, toInput );

    p->circuitDirty = result;

    ResumeAutoTick();

    return result;
}

bool Circuit::DisconnectComponent( const Component::SPtr& component )
{
    if ( p->componentsSet.find( component ) == p->componentsSet.end() )
    {
        return false;
    }

    PauseAutoTick();

    // remove component from _inputComponents and _inputWires
    component->DisconnectAllInputs();

    // remove any connections this component has to other components
    for ( auto comp : p->components )
    {
        comp->DisconnectInput( component );
    }

    ResumeAutoTick();

    return true;
}

// cppcheck-suppress unusedFunction
void Circuit::DisconnectAllComponents()
{
    PauseAutoTick();

    for ( auto component : p->components )
    {
        component->DisconnectAllInputs();
    }

    ResumeAutoTick();
}

void Circuit::SetBufferCount( int bufferCount )
{
    if ( p->bufferCount == bufferCount )
    {
        return;
    }

    PauseAutoTick();

    p->bufferCount = bufferCount;

    // stop all threads
    for ( auto& circuitThread : p->circuitThreads )
    {
        circuitThread.Stop();
    }

    // resize thread array
    p->circuitThreads.resize( p->bufferCount );

    // initialise and start all threads
    for ( int i = 0; i < p->bufferCount; ++i )
    {
        p->circuitThreads[i].Start( &p->components, i );
    }

    if ( p->currentBuffer >= p->bufferCount )
    {
        p->currentBuffer = 0;
    }

    // set all components to the new buffer count
    for ( auto component : p->components )
    {
        component->SetBufferCount( p->bufferCount, p->currentBuffer );
    }

    ResumeAutoTick();
}

int Circuit::GetBufferCount() const
{
    return p->bufferCount;
}

void Circuit::Tick()
{
    if ( p->circuitDirty )
    {
        p->Optimize();
    }

    // process in a single thread if this circuit has no threads
    // =========================================================
    if ( p->bufferCount == 0 )
    {
        // tick all internal components
        for ( auto component : p->components )
        {
            component->Tick();
        }
    }
    // process in multiple threads if this circuit has threads
    // =======================================================
    else
    {
        p->circuitThreads[p->currentBuffer].SyncAndResume();  // sync and resume thread x

        if ( ++p->currentBuffer == p->bufferCount )
        {
            p->currentBuffer = 0;
        }
    }
}

void Circuit::Sync()
{
    // sync all threads
    for ( auto& circuitThread : p->circuitThreads )
    {
        circuitThread.Sync();
    }
}

void Circuit::StartAutoTick()
{
    if ( p->autoTickThread.IsStopped() )
    {
        p->autoTickThread.Start( this );
    }
    else
    {
        ResumeAutoTick();
    }
}

void Circuit::StopAutoTick()
{
    if ( !p->autoTickThread.IsStopped() )
    {
        p->autoTickThread.Stop();
    }

    Sync();
}

void Circuit::PauseAutoTick()
{
    if ( !p->autoTickThread.IsStopped() && ++p->pauseCount == 1 && !p->autoTickThread.IsPaused() )
    {
        p->autoTickThread.Pause();
    }

    Sync();
}

void Circuit::ResumeAutoTick()
{
    if ( p->autoTickThread.IsPaused() && --p->pauseCount == 0 )
    {
        p->autoTickThread.Resume();
    }
}

void Circuit::Optimize()
{
    if ( p->circuitDirty )
    {
        PauseAutoTick();
        p->Optimize();
        ResumeAutoTick();
    }
}

inline void internal::Circuit::Optimize()
{
    std::vector<DSPatch::Component*> orderedComponents;
    orderedComponents.reserve( components.size() );

    std::map<int, std::set<DSPatch::Component*>> orderedComponentsMap;

    // scan for optimal component order
    for ( auto component : components )
    {
        component->_Scan( orderedComponents );
        component->_ParallelScan( orderedComponentsMap );
    }

    // reset all isScanning flags
    for ( auto component : components )
    {
        component->_EndScan();
        component->_EndParallelScan();
    }

    components = std::move( orderedComponents );
    componentsMap = std::move( orderedComponentsMap );

    circuitDirty = false;
}
