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
#include "internal/ParallelCircuitThread.h"

#include <algorithm>
#include <set>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class Circuit final
{
public:
    inline void _Optimize();

    int _bufferCount = 0;
    int _threadCount = 0;
    int _currentBuffer = 0;

    AutoTickThread _autoTickThread;

    std::vector<DSPatch::Component*> _components;
    std::set<DSPatch::Component::SPtr> _componentsSet;
    std::vector<DSPatch::Component*> _componentsParallel;

    std::vector<CircuitThread> _circuitThreads;
    std::vector<std::vector<ParallelCircuitThread>> _parallelCircuitThreads;

    bool _circuitDirty = false;
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
    delete p;
}

bool Circuit::AddComponent( const Component::SPtr& component )
{
    if ( !component || p->_componentsSet.find( component ) != p->_componentsSet.end() )
    {
        return false;
    }

    // components within the circuit need to have as many buffers as there are threads in the circuit
    component->SetBufferCount( p->_bufferCount, p->_currentBuffer );

    PauseAutoTick();
    p->_components.emplace_back( component.get() );
    p->_componentsParallel.emplace_back( component.get() );
    ResumeAutoTick();

    p->_componentsSet.emplace( component );

    return true;
}

bool Circuit::RemoveComponent( const Component::SPtr& component )
{
    if ( p->_componentsSet.find( component ) == p->_componentsSet.end() )
    {
        return false;
    }

    auto findFn = [&component]( auto comp ) { return comp == component.get(); };

    if ( auto it = std::find_if( p->_components.begin(), p->_components.end(), findFn ); it != p->_components.end() )
    {
        PauseAutoTick();

        DisconnectComponent( component );

        p->_components.erase( it );

        ResumeAutoTick();

        p->_componentsSet.erase( component );

        return true;
    }

    return false;
}

// cppcheck-suppress unusedFunction
void Circuit::RemoveAllComponents()
{
    PauseAutoTick();

    p->_components.clear();
    p->_componentsParallel.clear();

    ResumeAutoTick();

    p->_componentsSet.clear();
}

int Circuit::GetComponentCount() const
{
    return (int)p->_components.size();
}

bool Circuit::ConnectOutToIn( const Component::SPtr& fromComponent,
                              int fromOutput,
                              const Component::SPtr& toComponent,
                              int toInput )
{
    if ( p->_componentsSet.find( fromComponent ) == p->_componentsSet.end() ||
         p->_componentsSet.find( toComponent ) == p->_componentsSet.end() )
    {
        return false;
    }

    PauseAutoTick();

    bool result = toComponent->ConnectInput( fromComponent, fromOutput, toInput );

    p->_circuitDirty = result;

    ResumeAutoTick();

    return result;
}

bool Circuit::DisconnectComponent( const Component::SPtr& component )
{
    if ( p->_componentsSet.find( component ) == p->_componentsSet.end() )
    {
        return false;
    }

    PauseAutoTick();

    // remove component from _inputComponents and _inputWires
    component->DisconnectAllInputs();

    // remove any connections this component has to other components
    for ( auto comp : p->_components )
    {
        comp->DisconnectInput( component );
    }

    p->_circuitDirty = true;

    ResumeAutoTick();

    return true;
}

// cppcheck-suppress unusedFunction
void Circuit::DisconnectAllComponents()
{
    PauseAutoTick();

    for ( auto component : p->_components )
    {
        component->DisconnectAllInputs();
    }

    ResumeAutoTick();
}

void Circuit::SetBufferCount( int bufferCount )
{
    PauseAutoTick();

    p->_bufferCount = bufferCount;

    // stop all threads
    for ( auto& circuitThread : p->_circuitThreads )
    {
        circuitThread.Stop();
    }

    // resize thread array
    if ( p->_threadCount != 0 )
    {
        p->_circuitThreads.resize( 0 );
        SetThreadCount( p->_threadCount );
    }
    else
    {
        p->_circuitThreads.resize( p->_bufferCount );

        // initialise and start all threads
        for ( int i = 0; i < p->_bufferCount; ++i )
        {
            p->_circuitThreads[i].Start( &p->_components, i );
        }
    }

    if ( p->_currentBuffer >= p->_bufferCount )
    {
        p->_currentBuffer = 0;
    }

    // set all components to the new buffer count
    for ( auto component : p->_components )
    {
        component->SetBufferCount( p->_bufferCount, p->_currentBuffer );
    }

    ResumeAutoTick();
}

int Circuit::GetBufferCount() const
{
    return p->_bufferCount;
}

void Circuit::SetThreadCount( int threadCount )
{
    PauseAutoTick();

    p->_threadCount = threadCount;

    // stop all threads
    for ( auto& circuitThreads : p->_parallelCircuitThreads )
    {
        for ( auto& circuitThread : circuitThreads )
        {
            circuitThread.Stop();
        }
    }

    // resize thread array
    if ( p->_threadCount == 0 )
    {
        p->_parallelCircuitThreads.resize( 0 );
        SetBufferCount( p->_bufferCount );
    }
    else
    {
        p->_parallelCircuitThreads.resize( p->_bufferCount == 0 ? 1 : p->_bufferCount );
        for ( auto& circuitThread : p->_parallelCircuitThreads )
        {
            circuitThread.resize( p->_threadCount );
        }

        // initialise and start all threads
        int i = 0;
        for ( auto& circuitThreads : p->_parallelCircuitThreads )
        {
            int j = 0;
            for ( auto& circuitThread : circuitThreads )
            {
                circuitThread.Start( &p->_componentsParallel, i, j++, p->_threadCount );
            }
            ++i;
        }
    }

    ResumeAutoTick();
}

// cppcheck-suppress unusedFunction
int Circuit::GetThreadCount() const
{
    return p->_threadCount;
}

void Circuit::Tick()
{
    if ( p->_circuitDirty )
    {
        p->_Optimize();
    }

    // process in a single thread if this circuit has no threads
    // =========================================================
    if ( p->_bufferCount == 0 && p->_threadCount == 0 )
    {
        // tick all internal components
        for ( auto component : p->_components )
        {
            component->TickSeries( 0 );
        }

        return;
    }
    // process in multiple threads if this circuit has threads
    // =======================================================
    else if ( p->_threadCount != 0 )
    {
        auto& circuitThreads = p->_parallelCircuitThreads[p->_currentBuffer];

        for ( auto& circuitThread : circuitThreads )
        {
            circuitThread.Sync();
        }
        for ( auto& circuitThread : circuitThreads )
        {
            circuitThread.Resume();
        }
    }
    else
    {
        p->_circuitThreads[p->_currentBuffer].SyncAndResume();  // sync and resume thread x
    }

    if ( p->_bufferCount != 0 && ++p->_currentBuffer == p->_bufferCount )
    {
        p->_currentBuffer = 0;
    }
}

void Circuit::Sync()
{
    // sync all threads
    for ( auto& circuitThread : p->_circuitThreads )
    {
        circuitThread.Sync();
    }
    for ( auto& circuitThreads : p->_parallelCircuitThreads )
    {
        for ( auto& circuitThread : circuitThreads )
        {
            circuitThread.Sync();
        }
    }
}

void Circuit::StartAutoTick()
{
    p->_autoTickThread.Start( this );
}

void Circuit::StopAutoTick()
{
    p->_autoTickThread.Stop();
    Sync();
}

void Circuit::PauseAutoTick()
{
    p->_autoTickThread.Pause();
    Sync();
}

void Circuit::ResumeAutoTick()
{
    p->_autoTickThread.Resume();
}

void Circuit::Optimize()
{
    if ( p->_circuitDirty )
    {
        PauseAutoTick();
        p->_Optimize();
        ResumeAutoTick();
    }
}

inline void internal::Circuit::_Optimize()
{
    // scan for optimal series order -> update components
    {
        std::vector<DSPatch::Component*> orderedComponents;
        orderedComponents.reserve( _components.size() );

        for ( auto component : _components )
        {
            component->ScanSeries( orderedComponents );
        }
        for ( auto component : _components )
        {
            component->EndScan();
        }

        _components = std::move( orderedComponents );
    }

    // scan for optimal parallel order -> update componentsParallel
    {
        std::vector<std::vector<DSPatch::Component*>> componentsMap;

        for ( int i = (int)_components.size() - 1; i >= 0; --i )
        {
            int scanPosition;
            _components[i]->ScanParallel( componentsMap, scanPosition );
        }
        for ( auto component : _components )
        {
            component->EndScan();
        }

        _componentsParallel.clear();
        _componentsParallel.reserve( _components.size() );
        for ( auto& componentsMapEntry : componentsMap )
        {
            _componentsParallel.insert( _componentsParallel.end(), componentsMapEntry.begin(), componentsMapEntry.end() );
        }
    }

    // clear circuitDirty flag
    _circuitDirty = false;
}
