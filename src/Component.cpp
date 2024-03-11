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
    inline AtomicFlag() = default;

    inline AtomicFlag( AtomicFlag&& )
    {
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
    std::atomic_flag flag = { true };  // true here actually means unset / cleared
};

struct RefCounter final
{
    int count = 0;
    int total = 0;
    AtomicFlag readyFlag;
};

class Component final
{
public:
    inline explicit Component( DSPatch::Component::ProcessOrder processOrder )
        : _processOrder( processOrder )
    {
    }

    inline void _WaitForRelease( int threadNo );
    inline void _ReleaseNextThread( int threadNo );

    inline void _GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus );
    inline void _GetOutputParallel( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus );

    inline void _IncRefs( int output );
    inline void _DecRefs( int output );

    const DSPatch::Component::ProcessOrder _processOrder;

    int _bufferCount = 0;

    std::vector<DSPatch::SignalBus> _inputBuses;
    std::vector<DSPatch::SignalBus> _outputBuses;

    std::vector<std::vector<RefCounter>> _refs;  // RefCounter per output, per buffer

    std::vector<Wire> _inputWires;

    std::vector<AtomicFlag> _releaseFlags;

    std::vector<std::string> _inputNames;
    std::vector<std::string> _outputNames;

    int _scanPosition = -1;
};

}  // namespace internal
}  // namespace DSPatch

Component::Component( ProcessOrder processOrder )
    : p( new internal::Component( processOrder ) )
{
    SetBufferCount( 1, 0 );
}

Component::~Component()
{
    delete p;
}

bool Component::ConnectInput( const Component::SPtr& fromComponent, int fromOutput, int toInput )
{
    if ( fromOutput >= fromComponent->GetOutputCount() || toInput >= p->_inputBuses[0].GetSignalCount() )
    {
        return false;
    }

    // first make sure there are no wires already connected to this input
    auto findFn = [&toInput]( const auto& wire ) { return wire.toInput == toInput; };

    if ( auto it = std::find_if( p->_inputWires.begin(), p->_inputWires.end(), findFn ); it != p->_inputWires.end() )
    {
        if ( it->fromComponent == fromComponent.get() && it->fromOutput == fromOutput )
        {
            // this wire already exists
            return false;
        }

        // update source output's reference count
        it->fromComponent->p->_DecRefs( it->fromOutput );

        p->_inputWires.erase( it );
    }

    p->_inputWires.emplace_back( internal::Wire{ fromComponent.get(), fromOutput, toInput } );

    // update source output's reference count
    fromComponent->p->_IncRefs( fromOutput );

    return true;
}

void Component::DisconnectInput( int inputNo )
{
    // remove wires connected to inputNo from inputWires
    auto findFn = [&inputNo]( const auto& wire ) { return wire.toInput == inputNo; };

    if ( auto it = std::find_if( p->_inputWires.begin(), p->_inputWires.end(), findFn ); it != p->_inputWires.end() )
    {
        // update source output's reference count
        it->fromComponent->p->_DecRefs( it->fromOutput );

        p->_inputWires.erase( it );
    }
}

void Component::DisconnectInput( const Component::SPtr& fromComponent )
{
    // remove fromComponent from inputWires
    auto findFn = [&fromComponent]( const auto& wire ) { return wire.fromComponent == fromComponent.get(); };

    for ( auto it = std::find_if( p->_inputWires.begin(), p->_inputWires.end(), findFn ); it != p->_inputWires.end();
          it = std::find_if( it, p->_inputWires.end(), findFn ) )
    {
        // update source output's reference count
        fromComponent->p->_DecRefs( it->fromOutput );

        it = p->_inputWires.erase( it );
    }
}

void Component::DisconnectAllInputs()
{
    // remove all wires from inputWires
    for ( const auto& inputWires : p->_inputWires )
    {
        // update source output's reference count
        inputWires.fromComponent->p->_DecRefs( inputWires.fromOutput );
    }

    p->_inputWires.clear();
}

int Component::GetInputCount() const
{
    return p->_inputBuses[0].GetSignalCount();
}

int Component::GetOutputCount() const
{
    return p->_outputBuses[0].GetSignalCount();
}

// cppcheck-suppress unusedFunction
std::string Component::GetInputName( int inputNo ) const
{
    if ( inputNo < (int)p->_inputNames.size() )
    {
        return p->_inputNames[inputNo];
    }
    return "";
}

// cppcheck-suppress unusedFunction
std::string Component::GetOutputName( int outputNo ) const
{
    if ( outputNo < (int)p->_outputNames.size() )
    {
        return p->_outputNames[outputNo];
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
    p->_inputBuses.resize( bufferCount );
    p->_outputBuses.resize( bufferCount );

    p->_releaseFlags.resize( bufferCount );

    p->_refs.resize( bufferCount );
    auto refCount = p->_refs[0].size();

    // init vector values
    for ( int i = 0; i < bufferCount; ++i )
    {
        p->_inputBuses[i].SetSignalCount( p->_inputBuses[0].GetSignalCount() );
        p->_outputBuses[i].SetSignalCount( p->_outputBuses[0].GetSignalCount() );

        if ( i == startBuffer )
        {
            p->_releaseFlags[i].Set();
        }
        else
        {
            p->_releaseFlags[i].Clear();
        }

        p->_refs[i].resize( refCount );
        for ( size_t j = 0; j < refCount; ++j )
        {
            // sync output reference counts
            p->_refs[i][j].total = p->_refs[0][j].total;
        }
    }

    p->_bufferCount = bufferCount;
}

int Component::GetBufferCount() const
{
    return (int)p->_inputBuses.size();
}

void Component::TickSeries( int bufferNo )
{
    auto& inputBus = p->_inputBuses[bufferNo];
    auto& outputBus = p->_outputBuses[bufferNo];

    // clear inputs
    inputBus.ClearAllValues();

    for ( const auto& wire : p->_inputWires )
    {
        // get new inputs from incoming components
        wire.fromComponent->p->_GetOutput( bufferNo, wire.fromOutput, wire.toInput, inputBus );
    }

    // clear outputs
    outputBus.ClearAllValues();

    if ( p->_bufferCount != 1 && p->_processOrder == ProcessOrder::InOrder )
    {
        // wait for our turn to process
        p->_WaitForRelease( bufferNo );

        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );

        // signal that we're done processing
        p->_ReleaseNextThread( bufferNo );
    }
    else
    {
        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );
    }
}

void Component::TickParallel( int bufferNo )
{
    auto& inputBus = p->_inputBuses[bufferNo];
    auto& outputBus = p->_outputBuses[bufferNo];

    // clear inputs and outputs
    inputBus.ClearAllValues();
    outputBus.ClearAllValues();

    for ( const auto& wire : p->_inputWires )
    {
        // get new inputs from incoming components
        wire.fromComponent->p->_GetOutputParallel( bufferNo, wire.fromOutput, wire.toInput, inputBus );
    }

    if ( p->_bufferCount != 1 && p->_processOrder == ProcessOrder::InOrder )
    {
        // wait for our turn to process
        p->_WaitForRelease( bufferNo );

        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );

        // signal that we're done processing
        p->_ReleaseNextThread( bufferNo );
    }
    else
    {
        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );
    }

    // signal that our outputs are ready
    for ( auto& ref : p->_refs[bufferNo] )
    {
        // readyFlags are cleared in _GetOutputParallel() which ofc is only called on outputs with refs
        if ( ref.total != 0 )
        {
            ref.readyFlag.Set();
        }
    }
}

void Component::ScanSeries( std::vector<Component*>& components )
{
    // continue only if this component has not already been scanned
    if ( p->_scanPosition != -1 )
    {
        return;
    }

    // initialize scanPosition
    p->_scanPosition = 0;

    for ( const auto& wire : p->_inputWires )
    {
        // scan incoming components
        wire.fromComponent->ScanSeries( components );
    }

    components.emplace_back( this );
}

void Component::ScanParallel( std::vector<std::vector<DSPatch::Component*>>& componentsMap, int& scanPosition )
{
    // continue only if this component has not already been scanned
    if ( p->_scanPosition != -1 )
    {
        scanPosition = p->_scanPosition;
        return;
    }

    // initialize scanPositions
    p->_scanPosition = 0;
    scanPosition = 0;

    for ( const auto& wire : p->_inputWires )
    {
        // scan incoming components
        wire.fromComponent->ScanParallel( componentsMap, scanPosition );

        // ensure we're using the furthest scanPosition detected
        p->_scanPosition = std::max( p->_scanPosition, ++scanPosition );
    }

    // insert component at p->scanPosition
    if ( p->_scanPosition >= (int)componentsMap.size() )
    {
        componentsMap.resize( p->_scanPosition + 1 );
    }
    componentsMap[p->_scanPosition].emplace_back( this );
}

void Component::EndScan()
{
    // reset scanPosition
    p->_scanPosition = -1;
}

void Component::SetInputCount_( int inputCount, const std::vector<std::string>& inputNames )
{
    p->_inputNames = inputNames;

    for ( auto& inputBus : p->_inputBuses )
    {
        inputBus.SetSignalCount( inputCount );
    }

    p->_inputWires.reserve( inputCount );
}

void Component::SetOutputCount_( int outputCount, const std::vector<std::string>& outputNames )
{
    p->_outputNames = outputNames;

    for ( auto& outputBus : p->_outputBuses )
    {
        outputBus.SetSignalCount( outputCount );
    }

    // add reference counters for our new outputs
    for ( auto& ref : p->_refs )
    {
        ref.resize( outputCount );
    }
}

inline void internal::Component::_WaitForRelease( int threadNo )
{
    _releaseFlags[threadNo].WaitAndClear();
}

inline void internal::Component::_ReleaseNextThread( int threadNo )
{
    if ( ++threadNo == _bufferCount )  // we're actually releasing the next available thread
    {
        _releaseFlags[0].Set();
    }
    else
    {
        _releaseFlags[threadNo].Set();
    }
}

inline void internal::Component::_GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
{
    auto& signal = *_outputBuses[bufferNo].GetSignal( fromOutput );

    if ( !signal.has_value() )
    {
        return;
    }

    auto& ref = _refs[bufferNo][fromOutput];

    if ( ref.total == 1 )
    {
        // there's only one reference, move the signal immediately
        toBus.MoveSignal( toInput, signal );
    }
    else if ( ++ref.count != ref.total )
    {
        // this is not the final reference, copy the signal
        toBus.SetSignal( toInput, signal );
    }
    else
    {
        // this is the final reference, reset the counter, move the signal
        ref.count = 0;
        toBus.MoveSignal( toInput, signal );
    }
}

inline void internal::Component::_GetOutputParallel( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
{
    auto& signal = *_outputBuses[bufferNo].GetSignal( fromOutput );
    auto& ref = _refs[bufferNo][fromOutput];

    // wait for this output to be ready
    ref.readyFlag.WaitAndClear();

    if ( !signal.has_value() )
    {
        return;
    }

    if ( ref.total == 1 )
    {
        // there's only one reference, move the signal immediately and return
        toBus.MoveSignal( toInput, signal );
    }
    else if ( ++ref.count != ref.total )
    {
        // this is not the final reference, copy the signal
        toBus.SetSignal( toInput, signal );

        // wake next WaitAndClear()
        ref.readyFlag.Set();
    }
    else
    {
        // this is the final reference, reset the counter, move the signal
        ref.count = 0;
        toBus.MoveSignal( toInput, signal );
    }
}

inline void internal::Component::_IncRefs( int output )
{
    for ( auto& ref : _refs )
    {
        ++ref[output].total;
    }
}

inline void internal::Component::_DecRefs( int output )
{
    for ( auto& ref : _refs )
    {
        --ref[output].total;
    }
}
