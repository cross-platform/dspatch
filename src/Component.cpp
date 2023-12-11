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

#include "internal/Wire.h"

#include <algorithm>
#include <atomic>
#include <thread>

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
        Ticking
    };

    inline explicit Component( DSPatch::Component::ProcessOrder processOrder )
        : processOrder( processOrder )
    {
    }

    inline void WaitForRelease( int threadNo );
    inline void ReleaseNextThread( int threadNo );

    inline void GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus );

    inline void IncRefs( int output );
    inline void DecRefs( int output );

    const DSPatch::Component::ProcessOrder processOrder;

    int bufferCount = 0;

    struct RefCounter final
    {
        int count = 0;
        int total = 0;
    };

    struct MovableAtomicFlag final
    {
        MovableAtomicFlag() = default;

        MovableAtomicFlag( MovableAtomicFlag&& )
        {
        }

        std::atomic_flag flag = ATOMIC_FLAG_INIT;
    };

    struct Buffer final
    {
        std::vector<RefCounter> refs;  // RefCounter per output

        TickStatus tickStatus;
        MovableAtomicFlag releaseFlag;
    };

    std::vector<Buffer> buffers;

    std::vector<DSPatch::SignalBus> inputBuses;
    std::vector<DSPatch::SignalBus> outputBuses;

    std::vector<Wire> inputWires;

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

    p->inputWires.emplace_back( internal::Wire{ fromComponent, fromOutput, toInput } );

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

void Component::SetBufferCount( int bufferCount, int startBuffer )
{
    // p->bufferCount is the current thread count / bufferCount is new thread count

    if ( bufferCount <= 0 )
    {
        bufferCount = 1;  // there needs to be at least 1 buffer
    }

    if ( startBuffer >= bufferCount )
    {
        startBuffer = 0;
    }

    // resize buffers vector
    p->buffers.resize( bufferCount );

    p->inputBuses.resize( bufferCount );
    p->outputBuses.resize( bufferCount );

    auto& firstBuffer = p->buffers[0];

    // init vector values
    for ( int i = 0; i < bufferCount; ++i )
    {
        p->inputBuses[i].SetSignalCount( p->inputBuses[0].GetSignalCount() );
        p->outputBuses[i].SetSignalCount( p->outputBuses[0].GetSignalCount() );

        auto& buffer = p->buffers[i];

        buffer.tickStatus = internal::Component::TickStatus::NotTicked;

        if ( i == startBuffer )
        {
            buffer.releaseFlag.flag.clear();
        }
        else
        {
            buffer.releaseFlag.flag.test_and_set();
        }

        buffer.refs.resize( firstBuffer.refs.size() );
        for ( size_t j = 0; j < firstBuffer.refs.size(); ++j )
        {
            // sync output reference counts
            buffer.refs[j] = firstBuffer.refs[j];
        }
    }

    p->bufferCount = bufferCount;
}

int Component::GetBufferCount() const
{
    return p->bufferCount;
}

void Component::Tick( int bufferNo )
{
    auto& buffer = p->buffers[bufferNo];

    // continue only if this component has not already been ticked
    if ( buffer.tickStatus == internal::Component::TickStatus::Ticking )
    {
        return;
    }

    // set tickStatus -> Ticking
    buffer.tickStatus = internal::Component::TickStatus::Ticking;

    auto& inputBus = p->inputBuses[bufferNo];
    auto& outputBus = p->outputBuses[bufferNo];

    for ( const auto& wire : p->inputWires )
    {
        // tick incoming components
        wire.fromComponent->Tick( bufferNo );

        // get new inputs from incoming components
        wire.fromComponent->p->GetOutput( bufferNo, wire.fromOutput, wire.toInput, inputBus );
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

void Component::Reset( int bufferNo )
{
    // clear inputs
    p->inputBuses[bufferNo].ClearAllValues();

    // reset tickStatus
    p->buffers[bufferNo].tickStatus = internal::Component::TickStatus::NotTicked;
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
    for ( auto& buffer : p->buffers )
    {
        buffer.refs.resize( outputCount );
    }
}

inline void internal::Component::WaitForRelease( int threadNo )
{
    auto& releaseFlag = buffers[threadNo].releaseFlag.flag;

    while ( releaseFlag.test_and_set( std::memory_order_acquire ) )
    {
        std::this_thread::yield();
    }
}

inline void internal::Component::ReleaseNextThread( int threadNo )
{
    if ( ++threadNo == bufferCount )  // we're actually releasing the next available thread
    {
        buffers[0].releaseFlag.flag.clear( std::memory_order_release );
    }
    else
    {
        buffers[threadNo].releaseFlag.flag.clear( std::memory_order_release );
    }
}

inline void internal::Component::GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
{
    auto& signal = *outputBuses[bufferNo].GetSignal( fromOutput );

    if ( !signal.has_value() )
    {
        return;
    }

    auto& ref = buffers[bufferNo].refs[fromOutput];

    if ( ref.total == 1 )
    {
        // there's only one reference, move the signal immediately
        toBus.MoveSignal( toInput, signal );
        return;
    }
    else if ( ++ref.count != ref.total )
    {
        // this is not the final reference, copy the signal
        toBus.SetSignal( toInput, signal );
        return;
    }

    // this is the final reference, reset the counter, move the signal
    ref.count = 0;
    toBus.MoveSignal( toInput, signal );
}

inline void internal::Component::IncRefs( int output )
{
    for ( auto& buffer : buffers )
    {
        ++buffer.refs[output].total;
    }
}

inline void internal::Component::DecRefs( int output )
{
    for ( auto& buffer : buffers )
    {
        --buffer.refs[output].total;
    }
}
