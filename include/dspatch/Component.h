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

#pragma once

#include <dspatch/SignalBus.h>

#include <algorithm>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace DSPatch
{

namespace internal
{
class Component;
}  // namespace internal

/// Abstract base class for DSPatch components

/**
Classes derived from Component can be added to a Circuit and routed to and from other Components.

On construction, derived classes must configure the component's IO buses by calling SetInputCount_() and SetOutputCount_()
respectively.

Derived classes must also implement the virtual method: Process_(). The Process_() method is a callback from the DSPatch engine
that occurs when a new set of input signals is ready for processing. The Process_() method has 2 arguments: the input bus, and the
output bus. This method's purpose is to pull its required inputs out of the input bus, process these inputs, and populate the
output bus with the results (see SignalBus).

In order for a component to do any work it must be ticked. This is performed by repeatedly calling the Tick() method. This method
is responsible for acquiring the next set of input signals from its input wires and populating the component's input bus. The
acquired input bus is then passed to the Process_() method.

<b>PERFORMANCE TIP:</b> If a component is capable of processing its buffers out-of-order within a stream processing circuit,
consider initialising its base with ProcessOrder::OutOfOrder to improve performance. Note however that Process_() must be
thread-safe to operate in this mode.
*/

class DLLEXPORT Component
{
public:
    NONCOPYABLE( Component );

    using SPtr = std::shared_ptr<Component>;

    enum class ProcessOrder
    {
        InOrder,
        OutOfOrder
    };

    inline Component( ProcessOrder processOrder = ProcessOrder::InOrder );
    inline virtual ~Component();

    inline bool ConnectInput( const Component::SPtr& fromComponent, int fromOutput, int toInput );

    inline void DisconnectInput( int inputNo );
    inline void DisconnectInput( const Component::SPtr& fromComponent );
    inline void DisconnectAllInputs();

    inline int GetInputCount() const;
    inline int GetOutputCount() const;

    inline std::string GetInputName( int inputNo ) const;
    inline std::string GetOutputName( int outputNo ) const;

    inline void SetBufferCount( int bufferCount, int startBuffer );
    inline int GetBufferCount() const;

    inline void TickSeries( int bufferNo );
    inline void TickParallel( int bufferNo );

    inline void ScanSeries( std::vector<Component*>& components );
    inline void ScanParallel( std::vector<std::vector<DSPatch::Component*>>& componentsMap, int& scanPosition );
    inline void EndScan();

protected:
    inline virtual void Process_( SignalBus&, SignalBus& ) = 0;

    inline void SetInputCount_( int inputCount, const std::vector<std::string>& inputNames = {} );
    inline void SetOutputCount_( int outputCount, const std::vector<std::string>& outputNames = {} );

private:
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

    struct Wire final
    {
        DSPatch::Component* fromComponent;
        int fromOutput;
        int toInput;
    };

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

Component::Component( ProcessOrder processOrder )
    : _processOrder( processOrder )
{
    SetBufferCount( 1, 0 );
}

Component::~Component() = default;

bool Component::ConnectInput( const Component::SPtr& fromComponent, int fromOutput, int toInput )
{
    if ( fromOutput >= fromComponent->GetOutputCount() || toInput >= _inputBuses[0].GetSignalCount() )
    {
        return false;
    }

    // first make sure there are no wires already connected to this input
    auto findFn = [&toInput]( const auto& wire ) { return wire.toInput == toInput; };

    if ( auto it = std::find_if( _inputWires.begin(), _inputWires.end(), findFn ); it != _inputWires.end() )
    {
        if ( it->fromComponent == fromComponent.get() && it->fromOutput == fromOutput )
        {
            // this wire already exists
            return false;
        }

        // update source output's reference count
        it->fromComponent->_DecRefs( it->fromOutput );

        _inputWires.erase( it );
    }

    _inputWires.emplace_back( Wire{ fromComponent.get(), fromOutput, toInput } );

    // update source output's reference count
    fromComponent->_IncRefs( fromOutput );

    return true;
}

void Component::DisconnectInput( int inputNo )
{
    // remove wires connected to inputNo from inputWires
    auto findFn = [&inputNo]( const auto& wire ) { return wire.toInput == inputNo; };

    if ( auto it = std::find_if( _inputWires.begin(), _inputWires.end(), findFn ); it != _inputWires.end() )
    {
        // update source output's reference count
        it->fromComponent->_DecRefs( it->fromOutput );

        _inputWires.erase( it );
    }
}

void Component::DisconnectInput( const Component::SPtr& fromComponent )
{
    // remove fromComponent from inputWires
    auto findFn = [&fromComponent]( const auto& wire ) { return wire.fromComponent == fromComponent.get(); };

    for ( auto it = std::find_if( _inputWires.begin(), _inputWires.end(), findFn ); it != _inputWires.end();
          it = std::find_if( it, _inputWires.end(), findFn ) )
    {
        // update source output's reference count
        fromComponent->_DecRefs( it->fromOutput );

        it = _inputWires.erase( it );
    }
}

void Component::DisconnectAllInputs()
{
    // remove all wires from inputWires
    for ( const auto& inputWires : _inputWires )
    {
        // update source output's reference count
        inputWires.fromComponent->_DecRefs( inputWires.fromOutput );
    }

    _inputWires.clear();
}

int Component::GetInputCount() const
{
    return _inputBuses[0].GetSignalCount();
}

int Component::GetOutputCount() const
{
    return _outputBuses[0].GetSignalCount();
}

// cppcheck-suppress unusedFunction
std::string Component::GetInputName( int inputNo ) const
{
    if ( inputNo < (int)_inputNames.size() )
    {
        return _inputNames[inputNo];
    }
    return "";
}

// cppcheck-suppress unusedFunction
std::string Component::GetOutputName( int outputNo ) const
{
    if ( outputNo < (int)_outputNames.size() )
    {
        return _outputNames[outputNo];
    }
    return "";
}

void Component::SetBufferCount( int bufferCount, int startBuffer )
{
    // bufferCount is the current thread count / bufferCount is new thread count

    if ( bufferCount <= 0 )
    {
        bufferCount = 1;  // there needs to be at least 1 buffer
    }

    if ( startBuffer >= bufferCount )
    {
        startBuffer = 0;
    }

    // resize vectors
    _inputBuses.resize( bufferCount );
    _outputBuses.resize( bufferCount );

    _releaseFlags.resize( bufferCount );

    _refs.resize( bufferCount );
    auto refCount = _refs[0].size();

    // init vector values
    for ( int i = 0; i < bufferCount; ++i )
    {
        _inputBuses[i].SetSignalCount( _inputBuses[0].GetSignalCount() );
        _outputBuses[i].SetSignalCount( _outputBuses[0].GetSignalCount() );

        if ( i == startBuffer )
        {
            _releaseFlags[i].Set();
        }
        else
        {
            _releaseFlags[i].Clear();
        }

        _refs[i].resize( refCount );
        for ( size_t j = 0; j < refCount; ++j )
        {
            // sync output reference counts
            _refs[i][j].total = _refs[0][j].total;
        }
    }

    _bufferCount = bufferCount;
}

int Component::GetBufferCount() const
{
    return (int)_inputBuses.size();
}

void Component::TickSeries( int bufferNo )
{
    auto& inputBus = _inputBuses[bufferNo];
    auto& outputBus = _outputBuses[bufferNo];

    // clear inputs
    inputBus.ClearAllValues();

    for ( const auto& wire : _inputWires )
    {
        // get new inputs from incoming components
        wire.fromComponent->_GetOutput( bufferNo, wire.fromOutput, wire.toInput, inputBus );
    }

    // clear outputs
    outputBus.ClearAllValues();

    if ( _bufferCount != 1 && _processOrder == ProcessOrder::InOrder )
    {
        // wait for our turn to process
        _WaitForRelease( bufferNo );

        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );

        // signal that we're done processing
        _ReleaseNextThread( bufferNo );
    }
    else
    {
        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );
    }
}

void Component::TickParallel( int bufferNo )
{
    auto& inputBus = _inputBuses[bufferNo];
    auto& outputBus = _outputBuses[bufferNo];

    // clear inputs and outputs
    inputBus.ClearAllValues();
    outputBus.ClearAllValues();

    for ( const auto& wire : _inputWires )
    {
        // get new inputs from incoming components
        wire.fromComponent->_GetOutputParallel( bufferNo, wire.fromOutput, wire.toInput, inputBus );
    }

    if ( _bufferCount != 1 && _processOrder == ProcessOrder::InOrder )
    {
        // wait for our turn to process
        _WaitForRelease( bufferNo );

        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );

        // signal that we're done processing
        _ReleaseNextThread( bufferNo );
    }
    else
    {
        // call Process_() with newly aquired inputs
        Process_( inputBus, outputBus );
    }

    // signal that our outputs are ready
    for ( auto& ref : _refs[bufferNo] )
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
    if ( _scanPosition != -1 )
    {
        return;
    }

    // initialize scanPosition
    _scanPosition = 0;

    for ( const auto& wire : _inputWires )
    {
        // scan incoming components
        wire.fromComponent->ScanSeries( components );
    }

    components.emplace_back( this );
}

void Component::ScanParallel( std::vector<std::vector<DSPatch::Component*>>& componentsMap, int& scanPosition )
{
    // continue only if this component has not already been scanned
    if ( _scanPosition != -1 )
    {
        scanPosition = _scanPosition;
        return;
    }

    // initialize scanPositions
    _scanPosition = 0;
    scanPosition = 0;

    for ( const auto& wire : _inputWires )
    {
        // scan incoming components
        wire.fromComponent->ScanParallel( componentsMap, scanPosition );

        // ensure we're using the furthest scanPosition detected
        _scanPosition = std::max( _scanPosition, ++scanPosition );
    }

    // insert component at scanPosition
    if ( _scanPosition >= (int)componentsMap.size() )
    {
        componentsMap.resize( _scanPosition + 1 );
    }
    componentsMap[_scanPosition].emplace_back( this );
}

void Component::EndScan()
{
    // reset scanPosition
    _scanPosition = -1;
}

void Component::SetInputCount_( int inputCount, const std::vector<std::string>& inputNames )
{
    _inputNames = inputNames;

    for ( auto& inputBus : _inputBuses )
    {
        inputBus.SetSignalCount( inputCount );
    }

    _inputWires.reserve( inputCount );
}

void Component::SetOutputCount_( int outputCount, const std::vector<std::string>& outputNames )
{
    _outputNames = outputNames;

    for ( auto& outputBus : _outputBuses )
    {
        outputBus.SetSignalCount( outputCount );
    }

    // add reference counters for our new outputs
    for ( auto& ref : _refs )
    {
        ref.resize( outputCount );
    }
}

void Component::_WaitForRelease( int threadNo )
{
    _releaseFlags[threadNo].WaitAndClear();
}

void Component::_ReleaseNextThread( int threadNo )
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

void Component::_GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
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

void Component::_GetOutputParallel( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
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

void Component::_IncRefs( int output )
{
    for ( auto& ref : _refs )
    {
        ++ref[output].total;
    }
}

void Component::_DecRefs( int output )
{
    for ( auto& ref : _refs )
    {
        --ref[output].total;
    }
}

}  // namespace DSPatch
