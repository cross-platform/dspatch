/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
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

#include <dspatch/Circuit.h>

#include <internal/CircuitThread.h>
#include <internal/Wire.h>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{
    class IoComponent : public DSPatch::Component
    {
        virtual void Process_( DSPatch::SignalBus const& inputs, DSPatch::SignalBus& outputs ) override
        {
            for ( int i = 0; i < inputs.GetSignalCount(); ++i )
            {
                // this component sits between the circuit's I/O and internal component I/O
                // it simply passes its incoming signals through to its outputs
                outputs.SetValue( i, inputs, i );
            }
        }
    };

    class Circuit
    {
    public:
        Circuit()
            : components( std::make_shared<std::vector<DSPatch::Component::SPtr>>() )
            , currentThreadIndex( 0 )
            , inputComp( std::make_shared<IoComponent>() )
            , outputComp( std::make_shared<IoComponent>() )
        {
        }

        bool FindComponent( DSPatch::Component::SCPtr const& component, int& returnIndex ) const;

        std::shared_ptr<std::vector<DSPatch::Component::SPtr>> components;

        std::vector<std::unique_ptr<internal::CircuitThread>> circuitThreads;
        int currentThreadIndex;

        IoComponent::SPtr inputComp;
        IoComponent::SPtr outputComp;
    };
}
}

Circuit::Circuit( int threadCount )
    : p( new internal::Circuit() )
{
    SetThreadCount( threadCount );
}

Circuit::~Circuit()
{
    StopAutoTick();
    SetThreadCount( 0 );
    RemoveAllComponents();
}

void Circuit::PauseAutoTick()
{
    // pause auto tick
    Component::PauseAutoTick();

    // manually tick until 0
    while ( p->currentThreadIndex != 0 )
    {
        Tick();
        Reset();
    }

    // sync all threads
    for ( size_t i = 0; i < p->circuitThreads.size(); i++ )
    {
        p->circuitThreads[i]->Sync();
    }
}

void Circuit::SetInputCount( int inputCount )
{
    PauseAutoTick();
    SetInputCount_( inputCount );

    p->inputComp->SetInputCount_( inputCount );
    p->inputComp->SetOutputCount_( inputCount );

    ResumeAutoTick();
}

void Circuit::SetOutputCount( int outputCount )
{
    PauseAutoTick();
    SetOutputCount_( outputCount );

    p->outputComp->SetInputCount_( outputCount );
    p->outputComp->SetOutputCount_( outputCount );

    ResumeAutoTick();
}

void Circuit::SetThreadCount( int threadCount )
{
    if ( ( size_t ) threadCount != p->circuitThreads.size() )
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

int Circuit::AddComponent( Component::SPtr const& component )
{
    if ( component.get() != this && component != nullptr )
    {
        int componentIndex;

        if ( component->_GetParentCircuit() != nullptr && component->_GetParentCircuit() != shared_from_this() )
        {
            return -1;  // if the component is already part of another circuit
        }
        else if ( p->FindComponent( component, componentIndex ) )
        {
            return componentIndex;  // if the component is already in the array
        }

        // components within the circuit need to have as many buffers as there are threads in the circuit
        component->_SetParentCircuit( std::static_pointer_cast<Circuit>( shared_from_this() ) );
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

    // set the removed component's parent circuit to nullptr
    if ( ( *p->components )[componentIndex]->_GetParentCircuit() != nullptr )
    {
        ( *p->components )[componentIndex]->_SetParentCircuit( nullptr );
    }
    // setting a component's parent to nullptr (above) calls _RemoveComponent (hence the following code will run)
    else if ( p->components->size() != 0 )
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
    if ( p->FindComponent( fromComponent, fromComponentIndex ) &&
         p->FindComponent( toComponent, toComponentIndex ) )
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
    if ( ( size_t ) fromComponent >= p->components->size() || ( size_t ) toComponent >= p->components->size() )
    {
        return false;
    }

    PauseAutoTick();
    bool result = ( *p->components )[toComponent]->ConnectInput( ( *p->components )[fromComponent], fromOutput, toInput );
    ResumeAutoTick();

    return result;
}

bool Circuit::ConnectInToIn( int fromInput, Component::SCPtr const& toComponent, int toInput )
{
    int toComponentIndex;
    if ( p->FindComponent( toComponent, toComponentIndex ) )
    {
        return ConnectInToIn( fromInput, toComponentIndex, toInput );
    }

    return false;
}

bool Circuit::ConnectInToIn( int fromInput, int toComponent, int toInput )
{
    if ( ( size_t ) toComponent >= p->components->size() )
    {
        return false;
    }

    PauseAutoTick();

    ( *p->components )[toComponent]->ConnectInput( p->inputComp, fromInput, toInput );

    ResumeAutoTick();

    return true;
}

bool Circuit::ConnectOutToOut( Component::SCPtr const& fromComponent, int fromOutput, int toOutput )
{
    int fromComponentIndex;
    if ( p->FindComponent( fromComponent, fromComponentIndex ) )
    {
        return ConnectOutToOut( fromComponentIndex, fromOutput, toOutput );
    }

    return false;
}

bool Circuit::ConnectOutToOut( int fromComponent, int fromOutput, int toOutput )
{
    if ( ( size_t ) fromComponent >= p->components->size() )
    {
        return false;
    }

    PauseAutoTick();

    p->outputComp->ConnectInput( ( *p->components )[fromComponent], fromOutput, toOutput );

    ResumeAutoTick();

    return true;
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

void Circuit::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    // process in a single thread if this circuit has no threads
    // =========================================================
    if ( p->circuitThreads.size() == 0 )
    {
        // set all internal component inputs from connected circuit inputs
        for ( int i = 0; i < inputs.GetSignalCount(); ++i )
        {
            p->inputComp->_MoveInputSignal( 0, i, inputs._GetSignal( i ) );
        }

        // tick all internal components
        for ( size_t i = 0; i < p->components->size(); i++ )
        {
            ( *p->components )[i]->Tick();
        }
        p->outputComp->Tick();

        // reset all internal components
        for ( size_t i = 0; i < p->components->size(); i++ )
        {
            ( *p->components )[i]->Reset();
        }
        p->inputComp->Reset();
        p->outputComp->Reset();

        // set all circuit outputs from connected internal component outputs
        for ( int i = 0; i < outputs.GetSignalCount(); ++i )
        {
            outputs._MoveSignal( i, p->outputComp->_GetOutputSignal( 0, i ) );
        }
    }
    // process in multiple threads if this circuit has threads
    // =======================================================
    else
    {
        p->circuitThreads[p->currentThreadIndex]->Sync();  // sync with thread x

        p->circuitThreads[p->currentThreadIndex]->Resume();  // resume thread x

        if ( ( size_t )++p->currentThreadIndex >= p->circuitThreads.size() )  // shift to thread x+1
        {
            p->currentThreadIndex = 0;
        }
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
