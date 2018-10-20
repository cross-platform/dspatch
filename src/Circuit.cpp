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

#include <dspatch/Circuit.h>

#include <internal/AutoTickThread.h>
#include <internal/CircuitThread.h>
#include <internal/Wire.h>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class Circuit
{
public:
    Circuit()
        : isAutoTickRunning( false )
        , isAutoTickPaused( false )
        , pauseCount( 0 )
        , autoTickThread( new AutoTickThread )
        , components( std::make_shared<std::vector<DSPatch::Component::SPtr>>() )
        , currentThreadIndex( 0 )
    {
    }

    bool FindComponent( DSPatch::Component::SCPtr const& component, int& returnIndex ) const;

    bool isAutoTickRunning;
    bool isAutoTickPaused;
    int pauseCount;

    std::unique_ptr<internal::AutoTickThread> autoTickThread;

    std::shared_ptr<std::vector<DSPatch::Component::SPtr>> components;

    std::vector<std::unique_ptr<internal::CircuitThread>> circuitThreads;
    int currentThreadIndex;
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
    SetThreadCount( 0 );
    RemoveAllComponents();
}

int Circuit::AddComponent( Component::SPtr const& component )
{
    if ( component != nullptr )
    {
        int componentIndex;

        if ( p->FindComponent( component, componentIndex ) )
        {
            return componentIndex;  // if the component is already in the array
        }

        // components within the circuit need to have as many buffers as there are threads in the circuit
        component->_SetBufferCount( p->circuitThreads.size() );

        PauseAutoTick();
        p->components->push_back( component );
        ResumeAutoTick();

        return p->components->size() - 1;
    }

    return -1;
}

void Circuit::RemoveComponent( Component::SCPtr const& component )
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

    if ( p->components->size() != 0 )
    {
        p->components->erase( p->components->begin() + componentIndex );
    }

    ResumeAutoTick();
}

void Circuit::RemoveAllComponents()
{
    for ( size_t i = 0; i < p->components->size(); i++ )
    {
        RemoveComponent( i-- );  // size drops as one is removed
    }
}

int Circuit::GetComponentCount() const
{
    return p->components->size();
}

bool Circuit::ConnectOutToIn( Component::SCPtr const& fromComponent, int fromOutput, Component::SCPtr const& toComponent, int toInput )
{
    int toComponentIndex;
    int fromComponentIndex;
    if ( p->FindComponent( fromComponent, fromComponentIndex ) && p->FindComponent( toComponent, toComponentIndex ) )
    {
        return ConnectOutToIn( fromComponentIndex, fromOutput, toComponentIndex, toInput );
    }

    return false;
}

bool Circuit::ConnectOutToIn( Component::SCPtr const& fromComponent, int fromOutput, int toComponent, int toInput )
{
    int fromComponentIndex;
    if ( p->FindComponent( fromComponent, fromComponentIndex ) )
    {
        return ConnectOutToIn( fromComponentIndex, fromOutput, toComponent, toInput );
    }

    return false;
}

bool Circuit::ConnectOutToIn( int fromComponent, int fromOutput, Component::SCPtr const& toComponent, int toInput )
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
    if ( (size_t)fromComponent >= p->components->size() || (size_t)toComponent >= p->components->size() )
    {
        return false;
    }

    PauseAutoTick();
    bool result = ( *p->components )[toComponent]->ConnectInput( ( *p->components )[fromComponent], fromOutput, toInput );
    ResumeAutoTick();

    return result;
}

void Circuit::DisconnectComponent( Component::SCPtr const& component )
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
    ( *p->components )[componentIndex]->DisconnectAllInputs();

    // remove any connections this component has to other components
    for ( size_t i = 0; i < p->components->size(); ++i )
    {
        ( *p->components )[i]->DisconnectInput( ( *p->components )[componentIndex] );
    }

    ResumeAutoTick();
}

void Circuit::SetThreadCount( int threadCount )
{
    if ( (size_t)threadCount != p->circuitThreads.size() )
    {
        PauseAutoTick();

        // stop all threads
        for ( size_t i = 0; i < p->circuitThreads.size(); i++ )
        {
            p->circuitThreads[i]->Stop();
        }

        // resize thread array
        p->circuitThreads.resize( threadCount );

        // initialise and start all threads
        for ( size_t i = 0; i < p->circuitThreads.size(); i++ )
        {
            if ( !p->circuitThreads[i] )
            {
                p->circuitThreads[i] = std::unique_ptr<internal::CircuitThread>( new internal::CircuitThread() );
            }
            p->circuitThreads[i]->Initialise( p->components, i );
            p->circuitThreads[i]->Start();
        }

        // set all components to the new thread count
        for ( size_t i = 0; i < p->components->size(); i++ )
        {
            ( *p->components )[i]->_SetBufferCount( threadCount );
        }

        ResumeAutoTick();
    }
}

int Circuit::GetThreadCount() const
{
    return p->circuitThreads.size();
}

void Circuit::Tick()
{
    // process in a single thread if this circuit has no threads
    // =========================================================
    if ( p->circuitThreads.size() == 0 )
    {
        // tick all internal components
        for ( size_t i = 0; i < p->components->size(); i++ )
        {
            ( *p->components )[i]->_Tick( 0 );
        }

        // reset all internal components
        for ( size_t i = 0; i < p->components->size(); i++ )
        {
            ( *p->components )[i]->_Reset( 0 );
        }
    }
    // process in multiple threads if this circuit has threads
    // =======================================================
    else
    {
        p->circuitThreads[p->currentThreadIndex]->Sync();  // sync with thread x

        p->circuitThreads[p->currentThreadIndex]->Resume();  // resume thread x

        if ( (size_t)++p->currentThreadIndex >= p->circuitThreads.size() )  // shift to thread x+1
        {
            p->currentThreadIndex = 0;
        }
    }
}

void Circuit::StartAutoTick()
{
    if ( p->autoTickThread->IsStopped() )
    {
        if ( !p->autoTickThread->IsInitialised() )
        {
            p->autoTickThread->Initialise( shared_from_this() );
        }

        p->autoTickThread->Start();

        p->isAutoTickRunning = true;
        p->isAutoTickPaused = false;
    }
    else
    {
        ResumeAutoTick();
    }
}

void Circuit::StopAutoTick()
{
    if ( !p->autoTickThread->IsStopped() )
    {
        p->autoTickThread->Stop();

        p->isAutoTickRunning = false;
        p->isAutoTickPaused = false;
    }
}

void Circuit::PauseAutoTick()
{
    if ( !p->autoTickThread->IsStopped() )
    {
        ++p->pauseCount;
        p->autoTickThread->Pause();
        p->isAutoTickPaused = true;
        p->isAutoTickRunning = false;
    }

    // manually tick until 0
    while ( p->currentThreadIndex != 0 )
    {
        Tick();
    }

    // sync all threads
    for ( size_t i = 0; i < p->circuitThreads.size(); i++ )
    {
        p->circuitThreads[i]->Sync();
    }
}

void Circuit::ResumeAutoTick()
{
    if ( p->isAutoTickPaused && --p->pauseCount == 0 )
    {
        p->autoTickThread->Resume();
        p->isAutoTickPaused = false;
        p->isAutoTickRunning = true;
    }
}

bool internal::Circuit::FindComponent( DSPatch::Component::SCPtr const& component, int& returnIndex ) const
{
    for ( size_t i = 0; i < components->size(); i++ )
    {
        if ( ( *components )[i] == component )
        {
            returnIndex = i;
            return true;
        }
    }

    return false;
}
