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

class AtomicFlag final
{
public:
    AtomicFlag() = default;

    AtomicFlag( AtomicFlag&& )
    {
    }

    inline void Wait()
    {
        while ( flag.test_and_set( std::memory_order_acquire ) )
        {
            std::this_thread::yield();
        }
        flag.clear( std::memory_order_release );
    }

    inline void WaitAndClear()
    {
        while ( flag.test_and_set( std::memory_order_acquire ) )
        {
            std::this_thread::yield();
        }
    }

    inline void Set()
    {
        flag.clear( std::memory_order_release );
    }

    inline void Clear()
    {
        flag.test_and_set( std::memory_order_acquire );
    }

private:
    std::atomic_flag flag = true;  // true means NOT set
};

class RefCounter final
{
public:
    RefCounter() = default;

    // cppcheck-suppress missingMemberCopy
    RefCounter( RefCounter&& )
    {
    }

    inline RefCounter& operator=( const RefCounter& other )
    {
        count = other.count;
        total = other.total;
        return *this;
    }

    std::mutex mutex;
    int count = 0;
    int total = 0;
};

class Component final
{
public:
    inline explicit Component( DSPatch::Component::ProcessOrder processOrder )
        : processOrder( processOrder )
    {
    }

    inline void WaitForRelease( int threadNo );
    inline void ReleaseNextThread( int threadNo );

    inline void GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus );
    inline void GetOutputParallel( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus );

    inline void IncRefs( int output );
    inline void DecRefs( int output );

    const DSPatch::Component::ProcessOrder processOrder;

    int bufferCount = 0;

    std::vector<DSPatch::SignalBus> inputBuses;
    std::vector<DSPatch::SignalBus> outputBuses;

    std::vector<std::vector<RefCounter>> refs;  // RefCounter per output, per buffer

    std::vector<Wire> inputWires;

    std::vector<AtomicFlag> releaseFlags;

    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;

    int scanPosition = -1;
    std::vector<AtomicFlag> tickedFlags;
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
    delete p;
}

bool Component::ConnectInput( const Component::SPtr& fromComponent, int fromOutput, int toInput )
{
    if ( fromOutput >= fromComponent->GetOutputCount() || toInput >= p->inputBuses[0].GetSignalCount() )
    {
        return false;
    }

    // first make sure there are no wires already connected to this input
    auto findFn = [&toInput]( const auto& wire ) { return wire.toInput == toInput; };

    if ( auto it = std::find_if( p->inputWires.begin(), p->inputWires.end(), findFn ); it != p->inputWires.end() )
    {
        if ( it->fromComponent == fromComponent.get() && it->fromOutput == fromOutput )
        {
            // this wire already exists
            return false;
        }

        // update source output's reference count
        it->fromComponent->p->DecRefs( it->fromOutput );

        p->inputWires.erase( it );
    }

    p->inputWires.emplace_back( internal::Wire{ fromComponent.get(), fromOutput, toInput } );

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
    auto findFn = [&fromComponent]( const auto& wire ) { return wire.fromComponent == fromComponent.get(); };

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

    // resize vectors
    p->inputBuses.resize( bufferCount );
    p->outputBuses.resize( bufferCount );

    p->releaseFlags.resize( bufferCount );
    p->tickedFlags.resize( bufferCount );

    p->refs.resize( bufferCount );

    // init vector values
    for ( int i = 0; i < bufferCount; ++i )
    {
        p->inputBuses[i].SetSignalCount( p->inputBuses[0].GetSignalCount() );
        p->outputBuses[i].SetSignalCount( p->outputBuses[0].GetSignalCount() );

        if ( i == startBuffer )
        {
            p->releaseFlags[i].Set();
        }
        else
        {
            p->releaseFlags[i].Clear();
        }
        p->tickedFlags[i].Clear();

        p->refs[i].resize( p->refs[0].size() );
        for ( size_t j = 0; j < p->refs[0].size(); ++j )
        {
            // sync output reference counts
            p->refs[i][j] = p->refs[0][j];
        }
    }

    p->bufferCount = bufferCount;
}

int Component::GetBufferCount() const
{
    return (int)p->inputBuses.size();
}

void Component::Tick( int bufferNo )
{
    auto& inputBus = p->inputBuses[bufferNo];
    auto& outputBus = p->outputBuses[bufferNo];

    // clear inputs
    inputBus.ClearAllValues();

    for ( const auto& wire : p->inputWires )
    {
        // get new inputs from incoming components
        wire.fromComponent->p->GetOutput( bufferNo, wire.fromOutput, wire.toInput, inputBus );
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

void Component::SetInputCount_( int inputCount, const std::vector<std::string>& inputNames )
{
    p->inputNames = inputNames;

    for ( auto& inputBus : p->inputBuses )
    {
        inputBus.SetSignalCount( inputCount );
    }

    p->inputWires.reserve( inputCount );
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
}

void Component::_TickParallel( int bufferNo )
{
    auto& inputBus = p->inputBuses[bufferNo];
    auto& outputBus = p->outputBuses[bufferNo];

    // clear inputs
    inputBus.ClearAllValues();

    for ( const auto& wire : p->inputWires )
    {
        // get new inputs from incoming components
        wire.fromComponent->p->GetOutputParallel( bufferNo, wire.fromOutput, wire.toInput, inputBus );
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

    p->tickedFlags[bufferNo].Set();
}

void Component::_ResetParallel( int bufferNo )
{
    p->tickedFlags[bufferNo].Clear();
}

void Component::_Scan( std::vector<Component*>& components,
                       std::vector<std::vector<DSPatch::Component*>>& componentsMap,
                       int& scanPosition )
{
    // continue only if this component has not already been scanned
    if ( p->scanPosition != -1 )
    {
        scanPosition = p->scanPosition == 0 ? 0 : std::max( p->scanPosition, scanPosition );
        return;
    }

    // initialize scanPosition
    p->scanPosition = 0;

    for ( const auto& wire : p->inputWires )
    {
        // scan incoming components
        wire.fromComponent->_Scan( components, componentsMap, scanPosition );
    }

    // set scanPosition
    p->scanPosition = ++scanPosition;

    components.emplace_back( this );

    if ( scanPosition >= (int)componentsMap.size() )
    {
        componentsMap.resize( scanPosition + 1 );
    }
    componentsMap[scanPosition].emplace_back( this );
}

void Component::_EndScan()
{
    // reset scanPosition
    p->scanPosition = -1;
}

inline void internal::Component::WaitForRelease( int threadNo )
{
    releaseFlags[threadNo].WaitAndClear();
}

inline void internal::Component::ReleaseNextThread( int threadNo )
{
    if ( ++threadNo == bufferCount )  // we're actually releasing the next available thread
    {
        releaseFlags[0].Set();
    }
    else
    {
        releaseFlags[threadNo].Set();
    }
}

inline void internal::Component::GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
{
    auto& signal = *outputBuses[bufferNo].GetSignal( fromOutput );

    if ( !signal.has_value() )
    {
        return;
    }

    auto& ref = refs[bufferNo][fromOutput];

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

inline void internal::Component::GetOutputParallel( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
{
    tickedFlags[bufferNo].Wait();

    auto& signal = *outputBuses[bufferNo].GetSignal( fromOutput );

    if ( !signal.has_value() )
    {
        return;
    }

    auto& ref = refs[bufferNo][fromOutput];

    if ( ref.total == 1 )
    {
        // there's only one reference, move the signal immediately
        toBus.MoveSignal( toInput, signal );
        return;
    }

    std::lock_guard<std::mutex> lock( ref.mutex );

    if ( ++ref.count != ref.total )
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
    for ( auto& ref : refs )
    {
        ++ref[output].total;
    }
}

inline void internal::Component::DecRefs( int output )
{
    for ( auto& ref : refs )
    {
        --ref[output].total;
    }
}
