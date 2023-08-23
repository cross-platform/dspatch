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

#include <algorithm>
#include <condition_variable>
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

    void WaitForRelease( int threadNo ) const;
    void ReleaseNextThread( int threadNo );

    void GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus, DSPatch::Component::TickMode mode );

    void IncRefs( int output );
    void DecRefs( int output );

    const DSPatch::Component::ProcessOrder processOrder;

    int bufferCount = 0;

    std::vector<DSPatch::SignalBus> inputBuses;
    std::vector<DSPatch::SignalBus> outputBuses;

    std::vector<std::vector<std::pair<int, int>>> refs;  // ref_total:ref_counter per output, per buffer
    std::vector<std::vector<std::mutex*>> refMutexes;

    std::vector<Wire> inputWires;

    std::vector<ComponentThread> componentThreads;
    std::vector<std::unordered_set<const Wire*>> feedbackWires;

    std::vector<TickStatus> tickStatuses;
    std::vector<std::atomic_flag*> releaseFlags;

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

bool Component::ConnectInput( const Component::SPtr& fromComponent, int fromOutput, int toInput )
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
    auto findFn = [&inputNo]( const auto& wire ) { return wire.toInput == inputNo; };

    if ( auto it = std::find_if( p->inputWires.begin(), p->inputWires.end(), findFn ); it != p->inputWires.end() )
    {
        // update source output's reference count
        it->fromComponent->p->DecRefs( it->fromOutput );

        p->inputWires.erase( it );
    }
}

void Component::DisconnectInput( const Component::SPtr& fromComponent )
{
    // remove fromComponent from inputWires
    auto findFn = [&fromComponent]( const auto& wire ) { return wire.fromComponent == fromComponent; };

    for ( auto it = std::find_if( p->inputWires.begin(), p->inputWires.end(), findFn ); it != p->inputWires.end();
          it = std::find_if( it, p->inputWires.end(), findFn ) )
    {
        // update source output's reference count
        fromComponent->p->DecRefs( it->fromOutput );

        it = p->inputWires.erase( it );
    }
}

void Component::DisconnectAllInputs()
{
    // remove all wires from inputWires
    for ( const auto& inputWires : p->inputWires )
    {
        // update source output's reference count
        inputWires.fromComponent->p->DecRefs( inputWires.fromOutput );
    }

    p->inputWires.clear();
}

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

    auto oldSize = p->releaseFlags.size();

    p->releaseFlags.resize( bufferCount );

    for ( size_t i = p->releaseFlags.size(); i < oldSize; ++i )
    {
        delete p->releaseFlags[i];
    }
    for ( size_t i = oldSize; i < p->releaseFlags.size(); ++i )
    {
        p->releaseFlags[i] = new std::atomic_flag();
    }

    p->refs.resize( bufferCount );
    p->refMutexes.resize( bufferCount );

    // init vector values
    for ( int i = 0; i < bufferCount; ++i )
    {
        p->componentThreads[i].Setup( this, i );

        p->tickStatuses[i] = internal::Component::TickStatus::NotTicked;

        p->inputBuses[i].SetSignalCount( p->inputBuses[0].GetSignalCount() );
        p->outputBuses[i].SetSignalCount( p->outputBuses[0].GetSignalCount() );

        p->releaseFlags[i]->test_and_set();

        p->refs[i].resize( p->refs[0].size() );
        for ( size_t j = 0; j < p->refs[0].size(); ++j )
        {
            // sync output reference counts
            p->refs[i][j] = p->refs[0][j];
        }

        oldSize = p->refMutexes[i].size();

        p->refMutexes[i].resize( p->refMutexes[0].size() );

        for ( size_t j = p->refMutexes[i].size(); j < oldSize; ++j )
        {
            delete p->refMutexes[i][j];
        }
        for ( size_t j = oldSize; j < p->refMutexes[i].size(); ++j )
        {
            p->refMutexes[i][j] = new std::mutex();
        }
    }

    p->releaseFlags[0]->clear();

    p->bufferCount = bufferCount;
}

int Component::GetBufferCount() const
{
    return (int)p->inputBuses.size();
}

void Component::Tick( Component::TickMode mode, int bufferNo )
{
    // continue only if this component has not already been ticked
    if ( p->tickStatuses[bufferNo] == internal::Component::TickStatus::Ticking )
    {
        return;
    }

    if ( mode == TickMode::Series )
    {
        // set tickStatus -> Ticking
        p->tickStatuses[bufferNo] = internal::Component::TickStatus::Ticking;

        auto& inputBus = p->inputBuses[bufferNo];
        auto& outputBus = p->outputBuses[bufferNo];

        for ( const auto& wire : p->inputWires )
        {
            // tick incoming components
            wire.fromComponent->Tick( TickMode::Series, bufferNo );

            // get new inputs from incoming components
            wire.fromComponent->p->GetOutput( bufferNo, wire.fromOutput, wire.toInput, inputBus, TickMode::Series );
        }

        // You might be thinking: Why not clear the outputs in Reset()?

        // This is because we need components to hold onto their outputs long enough for any
        // loopback wires to grab them during the next tick. The same applies to how we handle
        // output reference counting in internal::Component::GetOutput(), reseting the counter upon
        // the final request rather than in Reset().

        // clear outputs
        outputBus.ClearAllValues();

        if ( p->bufferCount != 1 && p->processOrder == ProcessOrder::InOrder )
        {
            // wait for our turn to process
            p->WaitForRelease( bufferNo );

            // call Process_() with newly aquired inputs
            Process_( inputBus, outputBus );

            // signal that we're done processing
            p->ReleaseNextThread( bufferNo );
        }
        else
        {
            // call Process_() with newly aquired inputs
            Process_( inputBus, outputBus );
        }
    }
    else if ( p->tickStatuses[bufferNo] == internal::Component::TickStatus::NotTicked )
    {
        // set tickStatus -> TickStarted
        p->tickStatuses[bufferNo] = internal::Component::TickStatus::TickStarted;

        auto& feedbackWires = p->feedbackWires[bufferNo];

        // tick incoming components
        for ( const auto& wire : p->inputWires )
        {
            try
            {
                wire.fromComponent->Tick( TickMode::Parallel, bufferNo );
            }
            catch ( const std::exception& )
            {
                feedbackWires.emplace( &wire );
            }
        }

        // set tickStatus -> Ticking
        p->tickStatuses[bufferNo] = internal::Component::TickStatus::Ticking;

        p->componentThreads[bufferNo].Resume();
    }
    else
    {
        // throw an exception to indicate that we have already started a tick, and hence, are a feedback component.
        throw std::exception();
    }
}

void Component::Reset( Component::TickMode mode, int bufferNo )
{
    if ( mode == TickMode::Parallel )
    {
        // wait for ticking to complete
        p->componentThreads[bufferNo].Sync();
    }

    // clear inputs
    p->inputBuses[bufferNo].ClearAllValues();

    // reset tickStatus
    p->tickStatuses[bufferNo] = internal::Component::TickStatus::NotTicked;
}

void Component::SetInputCount_( int inputCount, const std::vector<std::string>& inputNames )
{
    p->inputNames = inputNames;

    for ( auto& inputBus : p->inputBuses )
    {
        inputBus.SetSignalCount( inputCount );
    }
}

void Component::SetOutputCount_( int outputCount, const std::vector<std::string>& outputNames )
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
        auto oldSize = refMutexes.size();

        refMutexes.resize( outputCount );

        for ( size_t i = refMutexes.size(); i < oldSize; ++i )
        {
            delete refMutexes[i];
        }
        for ( size_t i = oldSize; i < refMutexes.size(); ++i )
        {
            refMutexes[i] = new std::mutex();
        }
    }
}

void Component::_TickParallel( int bufferNo )
{
    auto& inputBus = p->inputBuses[bufferNo];
    auto& outputBus = p->outputBuses[bufferNo];

    auto& feedbackWires = p->feedbackWires[bufferNo];

    for ( const auto& wire : p->inputWires )
    {
        // wait for non-feedback incoming components to finish ticking
        if ( feedbackWires.find( &wire ) == feedbackWires.end() )
        {
            wire.fromComponent->p->componentThreads[bufferNo].Sync();
        }
        else
        {
            feedbackWires.erase( &wire );
        }

        // get new inputs from incoming components
        wire.fromComponent->p->GetOutput( bufferNo, wire.fromOutput, wire.toInput, inputBus, Component::TickMode::Parallel );
    }

    // clear outputs
    outputBus.ClearAllValues();

    if ( p->bufferCount != 1 && p->processOrder == ProcessOrder::InOrder )
    {
        // wait for our turn to process
        p->WaitForRelease( bufferNo );

        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );

        // signal that we're done processing
        p->ReleaseNextThread( bufferNo );
    }
    else
    {
        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );
    }
}

void internal::Component::WaitForRelease( int threadNo ) const
{
    auto releaseFlag = releaseFlags[threadNo];

    while ( releaseFlag->test_and_set( std::memory_order_acquire ) )
    {
        YieldThread();
    }
}

void internal::Component::ReleaseNextThread( int threadNo )
{
    if ( ++threadNo == bufferCount )  // we're actually releasing the next available thread
    {
        releaseFlags[0]->clear( std::memory_order_release );
    }
    else
    {
        releaseFlags[threadNo]->clear( std::memory_order_release );
    }
}

void internal::Component::GetOutput(
    int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus, DSPatch::Component::TickMode mode )
{
    if ( !outputBuses[bufferNo].HasValue( fromOutput ) )
    {
        return;
    }

    auto& ref = refs[bufferNo][fromOutput];

    if ( ref.first == 1 )
    {
        // there's only one reference, move the signal immediately
        toBus.MoveSignal( toInput, *outputBuses[bufferNo].GetSignal( fromOutput ) );
        return;
    }
    else if ( mode == DSPatch::Component::TickMode::Parallel )
    {
        std::lock_guard<std::mutex> lock( *refMutexes[bufferNo][fromOutput] );
        if ( ++ref.second != ref.first )
        {
            toBus.SetSignal( toInput, *outputBuses[bufferNo].GetSignal( fromOutput ) );
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
        toBus.SetSignal( toInput, *outputBuses[bufferNo].GetSignal( fromOutput ) );
        return;
    }
    else
    {
        // this is the final reference, reset the counter, move the signal
        ref.second = 0;
    }

    toBus.MoveSignal( toInput, *outputBuses[bufferNo].GetSignal( fromOutput ) );
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
