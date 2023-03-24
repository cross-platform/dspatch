/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2023, Marcus Tomlinson

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

#include <internal/AutoTickThread.h>
#include <internal/CircuitThread.h>

#include <deque>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class Circuit
{
public:
    bool FindComponent( const DSPatch::Component::SCPtr& component, int& returnIndex ) const;

    int pauseCount = 0;
    size_t currentThreadNo = 0;

    AutoTickThread autoTickThread;

    std::vector<DSPatch::Component::SPtr> components;

    std::deque<CircuitThread> circuitThreads;

    DSPatch::ThreadPool::SPtr threadPool;
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
    SetThreadPool( nullptr );
    RemoveAllComponents();
}

int Circuit::AddComponent( const Component::SPtr& component )
{
    if ( component != nullptr )
    {
        int componentIndex;

        if ( p->FindComponent( component, componentIndex ) )
        {
            return componentIndex;  // if the component is already in the array
        }

        // components within the circuit need to have as many buffers as there are threads in the circuit
        component->SetThreadPool( p->threadPool );

        PauseAutoTick();
        p->components.emplace_back( component );
        ResumeAutoTick();

        return (int)p->components.size() - 1;
    }

    return -1;
}

void Circuit::RemoveComponent( const Component::SCPtr& component )
{
    int componentIndex;

    if ( p->FindComponent( component, componentIndex ) )
    {
        RemoveComponent( componentIndex );
    }
}

void Circuit::RemoveComponent( int componentIndex )
{
    PauseAutoTick();

    DisconnectComponent( componentIndex );

    if ( !p->components.empty() )
    {
        p->components.erase( p->components.begin() + componentIndex );
    }

    ResumeAutoTick();
}

void Circuit::RemoveAllComponents()
{
    for ( size_t i = 0; i < p->components.size(); ++i )
    {
        RemoveComponent( (int)i-- );  // size drops as one is removed
    }
}

// cppcheck-suppress unusedFunction
int Circuit::GetComponentCount() const
{
    return (int)p->components.size();
}

bool Circuit::ConnectOutToIn( const Component::SCPtr& fromComponent, int fromOutput, const Component::SCPtr& toComponent, int toInput )
{
    int toComponentIndex;
    int fromComponentIndex;
    if ( p->FindComponent( fromComponent, fromComponentIndex ) && p->FindComponent( toComponent, toComponentIndex ) )
    {
        return ConnectOutToIn( fromComponentIndex, fromOutput, toComponentIndex, toInput );
    }

    return false;
}

bool Circuit::ConnectOutToIn( const Component::SCPtr& fromComponent, int fromOutput, int toComponent, int toInput )
{
    int fromComponentIndex;
    if ( p->FindComponent( fromComponent, fromComponentIndex ) )
    {
        return ConnectOutToIn( fromComponentIndex, fromOutput, toComponent, toInput );
    }

    return false;
}

bool Circuit::ConnectOutToIn( int fromComponent, int fromOutput, const Component::SCPtr& toComponent, int toInput )
{
    int toComponentIndex;
    if ( p->FindComponent( toComponent, toComponentIndex ) )
    {
        return ConnectOutToIn( fromComponent, fromOutput, toComponentIndex, toInput );
    }

    return false;
}

bool Circuit::ConnectOutToIn( int fromComponent, int fromOutput, int toComponent, int toInput )
{
    if ( (size_t)fromComponent >= p->components.size() || (size_t)toComponent >= p->components.size() )
    {
        return false;
    }

    PauseAutoTick();
    bool result = p->components[toComponent]->ConnectInput( p->components[fromComponent], fromOutput, toInput );
    ResumeAutoTick();

    return result;
}

void Circuit::DisconnectComponent( const Component::SCPtr& component )
{
    int componentIndex;

    if ( p->FindComponent( component, componentIndex ) )
    {
        DisconnectComponent( componentIndex );
    }
}

void Circuit::DisconnectComponent( int componentIndex )
{
    PauseAutoTick();

    // remove component from _inputComponents and _inputWires
    p->components[componentIndex]->DisconnectAllInputs();

    // remove any connections this component has to other components
    for ( auto& component : p->components )
    {
        component->DisconnectInput( p->components[componentIndex] );
    }

    ResumeAutoTick();
}

void Circuit::SetThreadPool( const ThreadPool::SPtr& threadPool )
{
    auto bufferCount = 0;
    if ( threadPool )
    {
        bufferCount = threadPool->GetBufferCount();
    }

    PauseAutoTick();

    // stop all threads
    for ( auto& circuitThread : p->circuitThreads )
    {
        circuitThread.Stop();
    }

    // resize thread array
    p->circuitThreads.resize( bufferCount );

    // initialise and start all threads
    for ( size_t i = 0; i < p->circuitThreads.size(); ++i )
    {
        p->circuitThreads[i].Start( &p->components, (int)i );
    }

    // set all components to the new buffer count
    for ( auto& component : p->components )
    {
        component->SetThreadPool( threadPool );
    }

    p->threadPool = threadPool;

    p->currentThreadNo = 0;

    ResumeAutoTick();
}

void Circuit::Tick()
{
    // process in a single thread if this circuit has no threads
    // =========================================================
    if ( p->circuitThreads.empty() )
    {
        // tick all internal components
        for ( auto& component : p->components )
        {
            component->Tick();
        }

        // reset all internal components
        for ( auto& component : p->components )
        {
            component->Reset();
        }
    }
    // process in multiple threads if this circuit has threads
    // =======================================================
    else
    {
        p->circuitThreads[p->currentThreadNo].SyncAndResume();  // sync and resume thread x

        if ( ++p->currentThreadNo == p->circuitThreads.size() )
        {
            p->currentThreadNo = 0;
        }
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

        // manually tick until 0
        while ( p->currentThreadNo != 0 )
        {
            Tick();
        }

        // sync all threads
        for ( auto& circuitThread : p->circuitThreads )
        {
            circuitThread.Sync();
        }
    }
}

void Circuit::PauseAutoTick()
{
    if ( p->autoTickThread.IsStopped() )
    {
        return;
    }

    if ( ++p->pauseCount == 1 && !p->autoTickThread.IsPaused() )
    {
        p->autoTickThread.Pause();

        // manually tick until 0
        while ( p->currentThreadNo != 0 )
        {
            Tick();
        }

        // sync all threads
        for ( auto& circuitThread : p->circuitThreads )
        {
            circuitThread.Sync();
        }
    }
}

void Circuit::ResumeAutoTick()
{
    if ( p->autoTickThread.IsPaused() && --p->pauseCount == 0 )
    {
        p->autoTickThread.Resume();
    }
}

bool internal::Circuit::FindComponent( const DSPatch::Component::SCPtr& component, int& returnIndex ) const
{
    for ( size_t i = 0; i < components.size(); ++i )
    {
        if ( components[i] == component )
        {
            returnIndex = (int)i;
            return true;
        }
    }

    return false;
}
