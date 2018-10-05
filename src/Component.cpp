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

#include <dspatch/Component.h>

#include <dspatch/Circuit.h>

#include <internal/ComponentThread.h>
#include <internal/Wire.h>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class Component
{
public:
    Component()
        : bufferCount( 0 )
        , isAutoTickRunning( false )
        , isAutoTickPaused( false )
        , pauseCount( 0 )
        , hasTicked( false )
        , componentThread( new ComponentThread )
    {
    }

    void WaitForRelease( int threadNo );
    void ReleaseThread( int threadNo );

    std::weak_ptr<DSPatch::Circuit> parentCircuit;

    int bufferCount;

    std::vector<DSPatch::SignalBus> inputBuses;
    std::vector<DSPatch::SignalBus> outputBuses;

    bool isAutoTickRunning;
    bool isAutoTickPaused;
    int pauseCount;

    std::vector<Wire> inputWires;

    bool hasTicked;

    std::unique_ptr<internal::ComponentThread> componentThread;

    std::vector<std::unique_ptr<bool>> hasTickeds;  // bool pointers ensure that parallel threads will only read from this vector
    std::vector<bool> gotReleases;                  // bool pointers not used here as only 1 thread writes to this vector at a time
    std::vector<std::unique_ptr<std::mutex>> releaseMutexes;
    std::vector<std::unique_ptr<std::condition_variable>> releaseCondts;

    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
};

}  // namespace internal
}  // namespace DSPatch

Component::Component()
    : p( new internal::Component() )
{
    p->inputBuses.resize( 1 );
    p->outputBuses.resize( 1 );
}

Component::~Component()
{
    if ( p->parentCircuit.lock() != nullptr )
    {
        p->parentCircuit.lock()->RemoveComponent( shared_from_this() );
    }

    StopAutoTick();
    _SetBufferCount( 0 );
    DisconnectAllInputs();
}

void Component::Tick()
{
    // continue only if this component has not already been ticked
    if ( !p->hasTicked )
    {
        // 1. set hasTicked flag
        p->hasTicked = true;

        // 2. get outputs required from input components
        for ( size_t i = 0; i < p->inputWires.size(); i++ )
        {
            auto wire = p->inputWires[i];
            wire.linkedComponent->Tick();

            auto signal = wire.linkedComponent->p->outputBuses[0]._GetSignal( wire.fromSignalIndex );
            p->inputBuses[0]._SetSignal( wire.toSignalIndex, signal );
        }

        // 3. clear all outputs
        p->outputBuses[0].ClearAllValues();

        // 4. call Process_() with newly aquired inputs
        Process_( p->inputBuses[0], p->outputBuses[0] );
    }
}

void Component::Reset()
{
    // clear all inputs
    p->inputBuses[0].ClearAllValues();

    // reset hasTicked flag
    p->hasTicked = false;
}

void Component::StartAutoTick()
{
    auto currentParent = p->parentCircuit.lock();

    if ( currentParent )
    {
        currentParent->StartAutoTick();
    }
    else if ( p->componentThread->IsStopped() )
    {
        if ( !p->componentThread->IsInitialised() )
        {
            p->componentThread->Initialise( shared_from_this() );
        }

        p->componentThread->Start();

        p->isAutoTickRunning = true;
        p->isAutoTickPaused = false;
    }
    else
    {
        ResumeAutoTick();
    }
}

void Component::StopAutoTick()
{
    auto currentParent = p->parentCircuit.lock();

    if ( currentParent )
    {
        currentParent->StopAutoTick();
    }
    else if ( !p->componentThread->IsStopped() )
    {
        p->componentThread->Stop();

        p->isAutoTickRunning = false;
        p->isAutoTickPaused = false;
    }
}

void Component::PauseAutoTick()
{
    auto currentParent = p->parentCircuit.lock();

    if ( currentParent )
    {
        currentParent->PauseAutoTick();
    }
    else if ( !p->componentThread->IsStopped() && p->isAutoTickRunning )
    {
        ++p->pauseCount;
        p->componentThread->Pause();
        p->isAutoTickPaused = true;
        p->isAutoTickRunning = false;
    }
}

void Component::ResumeAutoTick()
{
    auto currentParent = p->parentCircuit.lock();

    if ( currentParent )
    {
        currentParent->ResumeAutoTick();
    }
    else if ( p->isAutoTickPaused && --p->pauseCount == 0 )
    {
        p->componentThread->Resume();
        p->isAutoTickPaused = false;
        p->isAutoTickRunning = true;
    }
}

bool Component::ConnectInput( Component::SPtr const& fromComponent, int fromOutput, int toInput )
{
    if ( fromOutput >= fromComponent->p->outputBuses[0].GetSignalCount() || toInput >= p->inputBuses[0].GetSignalCount() )
    {
        return false;
    }

    PauseAutoTick();

    // first make sure there are no wires already connected to this input
    DisconnectInput( toInput );

    p->inputWires.push_back( internal::Wire( fromComponent, fromOutput, toInput ) );

    // update source signal's dependent count
    for ( size_t i = 0; i < fromComponent->p->outputBuses.size(); i++ )
    {
        fromComponent->p->outputBuses[i]._GetSignal( fromOutput )->_IncDeps();
    }

    ResumeAutoTick();

    return true;
}

void Component::DisconnectInput( int inputNo )
{
    PauseAutoTick();

    // remove wires connected to inputNo from inputWires
    for ( size_t i = 0; i < p->inputWires.size(); i++ )
    {
        auto wire = p->inputWires[i];
        if ( wire.toSignalIndex == inputNo )
        {
            p->inputWires.erase( p->inputWires.begin() + i );

            // update source signal's dependent count
            for ( size_t i = 0; i < wire.linkedComponent->p->outputBuses.size(); i++ )
            {
                wire.linkedComponent->p->outputBuses[i]._GetSignal( wire.fromSignalIndex )->_DecDeps();
            }
        }
    }

    ResumeAutoTick();
}

void Component::DisconnectInput( Component::SCPtr const& fromComponent )
{
    PauseAutoTick();

    // remove fromComponent from inputWires
    for ( size_t i = 0; i < p->inputWires.size(); i++ )
    {
        auto wire = p->inputWires[i];
        if ( wire.linkedComponent == fromComponent )
        {
            DisconnectInput( wire.toSignalIndex );
        }
    }

    ResumeAutoTick();
}

void Component::DisconnectAllInputs()
{
    PauseAutoTick();

    // remove all wires from inputWires
    for ( int i = 0; i < p->inputBuses[0].GetSignalCount(); i++ )
    {
        DisconnectInput( i );
    }

    ResumeAutoTick();
}

int Component::GetInputCount() const
{
    return p->inputBuses[0].GetSignalCount();
}

int Component::GetOutputCount() const
{
    return p->outputBuses[0].GetSignalCount();
}

std::string Component::GetInputName( int inputNo ) const
{
    if ( inputNo < (int)p->inputNames.size() )
    {
        return p->inputNames[inputNo];
    }
    return "";
}

std::string Component::GetOutputName( int outputNo ) const
{
    if ( outputNo < (int)p->outputNames.size() )
    {
        return p->outputNames[outputNo];
    }
    return "";
}

void Component::SetInputCount_( int inputCount, std::vector<std::string> const& inputNames )
{
    p->inputNames = inputNames;
    for ( size_t i = 0; i < p->inputBuses.size(); i++ )
    {
        p->inputBuses[i].SetSignalCount( inputCount );
    }
}

void Component::SetOutputCount_( int outputCount, std::vector<std::string> const& outputNames )
{
    p->outputNames = outputNames;
    for ( size_t i = 0; i < p->outputBuses.size(); i++ )
    {
        p->outputBuses[i].SetSignalCount( outputCount );
    }
}

void Component::_SetParentCircuit( Circuit::SPtr const& parentCircuit )
{
    auto currentParent = p->parentCircuit.lock();

    if ( currentParent != parentCircuit && parentCircuit != shared_from_this() )
    {
        p->parentCircuit = std::weak_ptr<Circuit>();

        // if this component is part of another circuit, remove it from that circuit first
        if ( currentParent != nullptr )
        {
            currentParent->RemoveComponent( shared_from_this() );
        }
        else
        {
            // This component is moving to a ciruit, stop its auto-tick
            StopAutoTick();
        }

        p->parentCircuit = parentCircuit;

        // this method is called from within AddComponent() so don't call AddComponent() here
    }
}

Circuit::SCPtr Component::_GetParentCircuit()
{
    return p->parentCircuit.lock();
}

void Component::_SetBufferCount( int bufferCount )
{
    // p->bufferCount is the current thread count / bufferCount is new thread count

    // resize local buffer array
    p->hasTickeds.resize( bufferCount );

    // create excess hasTickeds (if new buffer count is more than current)
    for ( int i = p->bufferCount; i < bufferCount; i++ )
    {
        p->hasTickeds[i] = std::unique_ptr<bool>( new bool() );
    }

    p->inputBuses.resize( bufferCount == 0 ? 1 : bufferCount );
    p->outputBuses.resize( bufferCount == 0 ? 1 : bufferCount );

    p->gotReleases.resize( bufferCount );
    p->releaseMutexes.resize( bufferCount );
    p->releaseCondts.resize( bufferCount );

    for ( int i = p->bufferCount; i < bufferCount; i++ )
    {
        if ( !p->releaseCondts[i] )
        {
            p->releaseMutexes[i] = std::unique_ptr<std::mutex>( new std::mutex() );
            p->releaseCondts[i] = std::unique_ptr<std::condition_variable>( new std::condition_variable() );
        }

        *p->hasTickeds[i] = false;
        p->gotReleases[i] = false;

        p->inputBuses[i].SetSignalCount( p->inputBuses[0].GetSignalCount() );
        p->outputBuses[i].SetSignalCount( p->outputBuses[0].GetSignalCount() );

        for ( int j = 0; j < p->inputBuses[0].GetSignalCount(); ++j )
        {
            // update source signal's dependent count
            p->inputBuses[i]._GetSignal( j )->_SetDeps( p->inputBuses[0]._GetSignal( j )->_Deps() );
        }
        for ( int j = 0; j < p->outputBuses[0].GetSignalCount(); ++j )
        {
            // update source signal's dependent count
            p->outputBuses[i]._GetSignal( j )->_SetDeps( p->outputBuses[0]._GetSignal( j )->_Deps() );
        }
    }

    if ( bufferCount > 0 )
    {
        p->gotReleases[0] = true;
    }

    p->bufferCount = bufferCount;
}

int Component::_GetBufferCount()
{
    return p->inputBuses.size();
}

bool Component::_MoveInputSignal( int bufferIndex, int signalIndex, Signal::SPtr const& signal )
{
    if ( (size_t)bufferIndex < p->inputBuses.size() && signalIndex < p->inputBuses[bufferIndex].GetSignalCount() )
    {
        p->inputBuses[bufferIndex]._MoveSignal( signalIndex, signal );
        return true;
    }
    return false;
}

Signal::SPtr Component::_GetOutputSignal( int bufferIndex, int signalIndex )
{
    if ( (size_t)bufferIndex < p->inputBuses.size() && signalIndex < p->outputBuses[bufferIndex].GetSignalCount() )
    {
        return p->outputBuses[bufferIndex]._GetSignal( signalIndex );
    }
    return nullptr;
}

void Component::_ThreadTick( int threadNo )
{
    // continue only if this component has not already been ticked
    if ( *p->hasTickeds[threadNo] == false )
    {
        // 1. set hasTicked flag
        *p->hasTickeds[threadNo] = true;

        // 2. get outputs required from input components
        for ( size_t i = 0; i < p->inputWires.size(); i++ )
        {
            auto wire = p->inputWires[i];
            wire.linkedComponent->_ThreadTick( threadNo );

            auto signal = wire.linkedComponent->p->outputBuses[threadNo]._GetSignal( wire.fromSignalIndex );
            p->inputBuses[threadNo]._SetSignal( wire.toSignalIndex, signal );
        }

        // 3. clear all outputs
        p->outputBuses[threadNo].ClearAllValues();

        // 4. wait for your turn to process.
        p->WaitForRelease( threadNo );

        // 5. call Process_() with newly aquired inputs
        Process_( p->inputBuses[threadNo], p->outputBuses[threadNo] );

        // 6. signal that you're done processing.
        p->ReleaseThread( threadNo );
    }
}

void Component::_ThreadReset( int threadNo )
{
    // clear all inputs
    p->inputBuses[threadNo].ClearAllValues();

    // reset hasTicked flag
    *p->hasTickeds[threadNo] = false;
}

void internal::Component::WaitForRelease( int threadNo )
{
    std::unique_lock<std::mutex> lock( *releaseMutexes[threadNo] );
    if ( !gotReleases[threadNo] )
    {
        releaseCondts[threadNo]->wait( lock );  // wait for resume
    }
    gotReleases[threadNo] = false;  // reset the release flag
}

void internal::Component::ReleaseThread( int threadNo )
{
    int nextThread = threadNo + 1;

    if ( nextThread >= bufferCount )
    {
        nextThread = 0;
    }

    releaseMutexes[nextThread]->lock();
    gotReleases[nextThread] = true;
    releaseCondts[nextThread]->notify_one();
    releaseMutexes[nextThread]->unlock();
}
