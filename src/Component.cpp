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

#include <dspatch/Component.h>

#include <internal/ComponentThread.h>
#include <internal/Wire.h>

#include <condition_variable>
#include <deque>
#include <unordered_set>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class Component
{
public:
    enum class TickStatus
    {
        NotTicked,
        TickStarted,
        Ticking
    };

    explicit Component( DSPatch::Component::ProcessOrder processOrder )
        : processOrder( processOrder )
    {
    }

    void WaitForRelease( int threadNo );
    void ReleaseThread( int threadNo );

    void GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus, DSPatch::Component::TickMode mode );

    void IncRefs( int output );
    void DecRefs( int output );

    const DSPatch::Component::ProcessOrder processOrder;

    int bufferCount = 0;

    std::vector<DSPatch::SignalBus> inputBuses;
    std::vector<DSPatch::SignalBus> outputBuses;

    std::vector<std::vector<std::pair<int, int>>> refs;  // ref_total:ref_counter per output, per buffer
    std::vector<std::deque<std::mutex>> refMutexes;

    std::vector<Wire> inputWires;

    std::deque<ComponentThread> componentThreads;
    std::vector<std::unordered_set<Wire*>> feedbackWires;

    std::vector<TickStatus> tickStatuses;
    std::vector<bool> gotReleases;
    std::deque<std::mutex> releaseMutexes;
    std::deque<std::condition_variable> releaseCondts;

    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
};

}  // namespace internal
}  // namespace DSPatch

Component::Component( ProcessOrder processOrder )
    : p( new internal::Component( processOrder ) )
{
    SetBufferCount( 1 );
}

Component::~Component()
{
    DisconnectAllInputs();

    delete p;
}

bool Component::ConnectInput( Component::SPtr const& fromComponent, int fromOutput, int toInput )
{
    if ( fromOutput >= fromComponent->GetOutputCount() || toInput >= p->inputBuses[0].GetSignalCount() )
    {
        return false;
    }

    // first make sure there are no wires already connected to this input
    DisconnectInput( toInput );

    p->inputWires.emplace_back( fromComponent, fromOutput, toInput );

    // update source output's reference count
    fromComponent->p->IncRefs( fromOutput );

    return true;
}

void Component::DisconnectInput( int inputNo )
{
    // remove wires connected to inputNo from inputWires
    for ( auto it = p->inputWires.begin(); it != p->inputWires.end(); ++it )
    {
        if ( it->toInput == inputNo )
        {
            // update source output's reference count
            it->fromComponent->p->DecRefs( it->fromOutput );

            p->inputWires.erase( it );
            break;
        }
    }
}

void Component::DisconnectInput( Component::SCPtr const& fromComponent )
{
    // remove fromComponent from inputWires
    for ( auto it = p->inputWires.begin(); it != p->inputWires.end(); )
    {
        if ( it->fromComponent == fromComponent )
        {
            // update source output's reference count
            fromComponent->p->DecRefs( it->fromOutput );

            it = p->inputWires.erase( it );
        }
        else
        {
            ++it;
        }
    }
}

void Component::DisconnectAllInputs()
{
    // remove all wires from inputWires
    for ( int i = 0; i < p->inputBuses[0].GetSignalCount(); ++i )
    {
        DisconnectInput( i );
    }
}

// cppcheck-suppress unusedFunction
int Component::GetInputCount() const
{
    return p->inputBuses[0].GetSignalCount();
}

int Component::GetOutputCount() const
{
    return p->outputBuses[0].GetSignalCount();
}

// cppcheck-suppress unusedFunction
std::string Component::GetInputName( int inputNo ) const
{
    if ( inputNo < (int)p->inputNames.size() )
    {
        return p->inputNames[inputNo];
    }
    return "";
}

// cppcheck-suppress unusedFunction
std::string Component::GetOutputName( int outputNo ) const
{
    if ( outputNo < (int)p->outputNames.size() )
    {
        return p->outputNames[outputNo];
    }
    return "";
}

void Component::SetBufferCount( int bufferCount )
{
    // p->bufferCount is the current thread count / bufferCount is new thread count

    if ( bufferCount <= 0 )
    {
        bufferCount = 1;  // there needs to be at least 1 buffer
    }

    // resize vectors
    p->componentThreads.resize( bufferCount );
    p->feedbackWires.resize( bufferCount );

    p->tickStatuses.resize( bufferCount );

    p->inputBuses.resize( bufferCount );
    p->outputBuses.resize( bufferCount );

    p->gotReleases.resize( bufferCount );
    p->releaseMutexes.resize( bufferCount );
    p->releaseCondts.resize( bufferCount );

    p->refs.resize( bufferCount );
    p->refMutexes.resize( bufferCount );

    // init vector values
    for ( int i = 0; i < bufferCount; ++i )
    {
        p->componentThreads[i].Setup( this, i );

        p->tickStatuses[i] = internal::Component::TickStatus::NotTicked;

        p->inputBuses[i].SetSignalCount( p->inputBuses[0].GetSignalCount() );
        p->outputBuses[i].SetSignalCount( p->outputBuses[0].GetSignalCount() );

        p->gotReleases[i] = false;

        p->refs[i].resize( p->refs[0].size() );
        for ( size_t j = 0; j < p->refs[0].size(); ++j )
        {
            // sync output reference counts
            p->refs[i][j] = p->refs[0][j];
        }

        p->refMutexes[i].resize( p->refMutexes[0].size() );
    }

    p->gotReleases[0] = true;

    p->bufferCount = bufferCount;
}

int Component::GetBufferCount() const
{
    return (int)p->inputBuses.size();
}

bool Component::Tick( Component::TickMode mode, int bufferNo )
{
    // continue only if this component has not already been ticked
    if ( p->tickStatuses[bufferNo] == internal::Component::TickStatus::NotTicked )
    {
        // 1. set tickStatus -> TickStarted
        p->tickStatuses[bufferNo] = internal::Component::TickStatus::TickStarted;

        // 2. tick incoming components
        for ( auto& wire : p->inputWires )
        {
            if ( mode == TickMode::Series )
            {
                wire.fromComponent->Tick( mode, bufferNo );
            }
            else if ( mode == TickMode::Parallel )
            {
                if ( !wire.fromComponent->Tick( mode, bufferNo ) )
                {
                    p->feedbackWires[bufferNo].emplace( &wire );
                }
            }
        }

        // 3. set tickStatus -> Ticking
        p->tickStatuses[bufferNo] = internal::Component::TickStatus::Ticking;

        // do tick
        if ( mode == TickMode::Series )
        {
            _DoTick( mode, bufferNo );
        }
        else if ( mode == TickMode::Parallel )
        {
            p->componentThreads[bufferNo].Resume( mode );
        }
    }
    else if ( p->tickStatuses[bufferNo] == internal::Component::TickStatus::TickStarted )
    {
        // return false to indicate that we have already started a tick, and hence, are a feedback component.
        return false;
    }

    // return true to indicate that we are now in "Ticking" state.
    return true;
}

void Component::Reset( int bufferNo )
{
    // wait for ticking to complete
    p->componentThreads[bufferNo].Sync();

    // clear inputs
    p->inputBuses[bufferNo].ClearAllValues();

    // reset tickStatus
    p->tickStatuses[bufferNo] = internal::Component::TickStatus::NotTicked;
}

void Component::SetInputCount_( int inputCount, std::vector<std::string> const& inputNames )
{
    p->inputNames = inputNames;

    for ( auto& inputBus : p->inputBuses )
    {
        inputBus.SetSignalCount( inputCount );
    }
}

void Component::SetOutputCount_( int outputCount, std::vector<std::string> const& outputNames )
{
    p->outputNames = outputNames;

    for ( auto& outputBus : p->outputBuses )
    {
        outputBus.SetSignalCount( outputCount );
    }

    // add reference counters for our new outputs
    for ( auto& ref : p->refs )
    {
        ref.resize( outputCount );
    }
    for ( auto& refMutexes : p->refMutexes )
    {
        refMutexes.resize( outputCount );
    }
}

void Component::_DoTick( Component::TickMode mode, int bufferNo )
{
    // 4. get new inputs from incoming components
    if ( mode == Component::TickMode::Parallel )
    {
        for ( auto& wire : p->inputWires )
        {
            // wait for non-feedback incoming components to finish ticking
            if ( p->feedbackWires[bufferNo].find( &wire ) == p->feedbackWires[bufferNo].end() )
            {
                wire.fromComponent->p->componentThreads[bufferNo].Sync();
            }
            wire.fromComponent->p->GetOutput( bufferNo, wire.fromOutput, wire.toInput, p->inputBuses[bufferNo], mode );
        }
        p->feedbackWires[bufferNo].clear();
    }
    else
    {
        for ( auto& wire : p->inputWires )
        {
            wire.fromComponent->p->GetOutput( bufferNo, wire.fromOutput, wire.toInput, p->inputBuses[bufferNo], mode );
        }
    }

    // You might be thinking: Why not clear the outputs in Reset()?

    // This is because we need components to hold onto their outputs long enough for any
    // loopback wires to grab them during the next tick. The same applies to how we handle
    // output reference counting in internal::Component::GetOutput(), reseting the counter upon
    // the final request rather than in Reset().

    // 5. clear outputs
    p->outputBuses[bufferNo].ClearAllValues();

    if ( p->processOrder == ProcessOrder::InOrder && p->bufferCount > 1 )
    {
        // 6. wait for our turn to process
        p->WaitForRelease( bufferNo );

        // 7. call Process_() with newly aquired inputs
        Process_( p->inputBuses[bufferNo], p->outputBuses[bufferNo] );

        // 8. signal that we're done processing
        p->ReleaseThread( bufferNo );
    }
    else
    {
        // 6. call Process_() with newly aquired inputs
        Process_( p->inputBuses[bufferNo], p->outputBuses[bufferNo] );
    }
}

void internal::Component::WaitForRelease( int threadNo )
{
    if ( gotReleases[threadNo] )
    {
        gotReleases[threadNo] = false;  // reset the release flag
        return;
    }

    std::unique_lock<std::mutex> lock( releaseMutexes[threadNo] );

    if ( !gotReleases[threadNo] )
    {
        releaseCondts[threadNo].wait( lock );  // wait for release
    }
    gotReleases[threadNo] = false;  // reset the release flag
}

void internal::Component::ReleaseThread( int threadNo )
{
    if ( ++threadNo == bufferCount )  // we're actually releasing the next available thread
    {
        threadNo = 0;
    }

    std::lock_guard<std::mutex> lock( releaseMutexes[threadNo] );

    gotReleases[threadNo] = true;
    releaseCondts[threadNo].notify_all();
}

void internal::Component::GetOutput(
    int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus, DSPatch::Component::TickMode mode )
{
    if ( !outputBuses[bufferNo].HasValue( fromOutput ) )
    {
        return;
    }

    auto& signal = outputBuses[bufferNo].GetSignal( fromOutput );
    auto& ref = refs[bufferNo][fromOutput];

    if ( mode == DSPatch::Component::TickMode::Parallel && ref.first > 1 )
    {
        std::lock_guard<std::mutex> lock( refMutexes[bufferNo][fromOutput] );
        if ( ++ref.second != ref.first )
        {
            toBus.SetSignal( toInput, signal );
            return;
        }
        else
        {
            // this is the final reference, reset the counter, move the signal
            ref.second = 0;
        }
    }
    else if ( ++ref.second != ref.first )
    {
        toBus.SetSignal( toInput, signal );
        return;
    }
    else
    {
        // this is the final reference, reset the counter, move the signal
        ref.second = 0;
    }

    toBus.MoveSignal( toInput, signal );
}

void internal::Component::IncRefs( int output )
{
    for ( auto& ref : refs )
    {
        ++ref[output].first;
    }
}

void internal::Component::DecRefs( int output )
{
    for ( auto& ref : refs )
    {
        --ref[output].first;
    }
}
