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

#include "SignalBus.h"

#include <algorithm>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace DSPatch
{

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

class Component
{
public:
    Component( const Component& ) = delete;
    Component& operator=( const Component& ) = delete;

    using SPtr = std::shared_ptr<Component>;

    enum class ProcessOrder
    {
        InOrder,
        OutOfOrder
    };

    Component( ProcessOrder processOrder = ProcessOrder::InOrder );
    virtual ~Component();

    bool ConnectInput( const Component::SPtr& fromComponent, int fromOutput, int toInput );

    void DisconnectInput( int inputNo );
    void DisconnectInput( const Component::SPtr& fromComponent );
    void DisconnectAllInputs();

    int GetInputCount() const;
    int GetOutputCount() const;

    std::string GetInputName( int inputNo ) const;
    std::string GetOutputName( int outputNo ) const;

    void SetBufferCount( int bufferCount, int startBuffer );
    int GetBufferCount() const;

    void Tick( int bufferNo );
    void TickParallel( int bufferNo );

    void Scan( std::vector<Component*>& components );
    void ScanParallel( std::vector<std::vector<DSPatch::Component*>>& componentsMap, int& scanPosition );
    void EndScan();

protected:
    inline virtual void Process_( SignalBus&, SignalBus& ) = 0;

    void SetInputCount_( int inputCount, const std::vector<std::string>& inputNames = {} );
    void SetOutputCount_( int outputCount, const std::vector<std::string>& outputNames = {} );

private:
    class AtomicFlag final
    {
    public:
        AtomicFlag( const AtomicFlag& ) = delete;
        AtomicFlag& operator=( const AtomicFlag& ) = delete;

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

    void _WaitForRelease( int bufferNo );
    void _ReleaseNextBuffer( int bufferNo );

    void _GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus );
    void _GetOutputParallel( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus );

    void _IncRefs( int output );
    void _DecRefs( int output );

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

inline Component::Component( ProcessOrder processOrder )
    : _processOrder( processOrder )
{
    SetBufferCount( 1, 0 );
}

inline Component::~Component() = default;

inline bool Component::ConnectInput( const Component::SPtr& fromComponent, int fromOutput, int toInput )
{
    if ( fromOutput >= fromComponent->GetOutputCount() || toInput >= GetInputCount() )
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
            return true;
        }

        // update source output's reference count
        it->fromComponent->_DecRefs( it->fromOutput );

        // clear input
        for ( auto& inputBus : _inputBuses )
        {
            inputBus.ClearValue( toInput );
        }

        // replace wire
        it->fromComponent = fromComponent.get();
        it->fromOutput = fromOutput;
    }
    else
    {
        // add new wire
        _inputWires.emplace_back( Wire{ fromComponent.get(), fromOutput, toInput } );
    }

    // update source output's reference count
    fromComponent->_IncRefs( fromOutput );

    return true;
}

inline void Component::DisconnectInput( int inputNo )
{
    // remove wires connected to inputNo from _inputWires
    auto findFn = [&inputNo]( const auto& wire ) { return wire.toInput == inputNo; };

    if ( auto it = std::find_if( _inputWires.begin(), _inputWires.end(), findFn ); it != _inputWires.end() )
    {
        // update source output's reference count
        it->fromComponent->_DecRefs( it->fromOutput );

        // clear input
        for ( auto& inputBus : _inputBuses )
        {
            inputBus.ClearValue( inputNo );
        }

        // remove wire
        _inputWires.erase( it );
    }
}

inline void Component::DisconnectInput( const Component::SPtr& fromComponent )
{
    // remove fromComponent from _inputWires
    auto findFn = [&fromComponent]( const auto& wire ) { return wire.fromComponent == fromComponent.get(); };

    for ( auto it = std::find_if( _inputWires.begin(), _inputWires.end(), findFn ); it != _inputWires.end();
          it = std::find_if( it, _inputWires.end(), findFn ) )
    {
        // update source output's reference count
        fromComponent->_DecRefs( it->fromOutput );

        // clear input
        for ( auto& inputBus : _inputBuses )
        {
            inputBus.ClearValue( it->toInput );
        }

        // remove wire
        it = _inputWires.erase( it );
    }
}

inline void Component::DisconnectAllInputs()
{
    // update all source output reference counts
    for ( const auto& wire : _inputWires )
    {
        wire.fromComponent->_DecRefs( wire.fromOutput );
    }

    // clear all inputs
    for ( auto& inputBus : _inputBuses )
    {
        inputBus.ClearAllValues();
    }

    // remove all wires
    _inputWires.clear();
}

inline int Component::GetInputCount() const
{
    return _inputBuses[0].GetSignalCount();
}

inline int Component::GetOutputCount() const
{
    return _outputBuses[0].GetSignalCount();
}

// cppcheck-suppress unusedFunction
inline std::string Component::GetInputName( int inputNo ) const
{
    if ( inputNo < (int)_inputNames.size() )
    {
        return _inputNames[inputNo];
    }
    return "";
}

// cppcheck-suppress unusedFunction
inline std::string Component::GetOutputName( int outputNo ) const
{
    if ( outputNo < (int)_outputNames.size() )
    {
        return _outputNames[outputNo];
    }
    return "";
}

inline void Component::SetBufferCount( int bufferCount, int startBuffer )
{
    // _bufferCount is the current thread count / bufferCount is new thread count

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

    const auto inputCount = GetInputCount();
    const auto outputCount = GetOutputCount();
    const auto refCount = _refs[0].size();

    // init vector values
    for ( int i = 0; i < bufferCount; ++i )
    {
        _inputBuses[i].SetSignalCount( inputCount );
        _outputBuses[i].SetSignalCount( outputCount );

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

inline int Component::GetBufferCount() const
{
    return _bufferCount;
}

inline void Component::Tick( int bufferNo )
{
    auto& inputBus = _inputBuses[bufferNo];

    for ( const auto& wire : _inputWires )
    {
        // get new inputs from incoming components
        wire.fromComponent->_GetOutput( bufferNo, wire.fromOutput, wire.toInput, inputBus );
    }

    if ( _bufferCount != 1 && _processOrder == ProcessOrder::InOrder )
    {
        // wait for our turn to process
        _WaitForRelease( bufferNo );

        // call Process_() with newly aquired inputs
        Process_( inputBus, _outputBuses[bufferNo] );

        // signal that we're done processing
        _ReleaseNextBuffer( bufferNo );
    }
    else
    {
        // call Process_() with newly aquired inputs
        Process_( inputBus, _outputBuses[bufferNo] );
    }
}

inline void Component::TickParallel( int bufferNo )
{
    auto& inputBus = _inputBuses[bufferNo];

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
        Process_( inputBus, _outputBuses[bufferNo] );

        // signal that we're done processing
        _ReleaseNextBuffer( bufferNo );
    }
    else
    {
        // call Process_() with newly aquired inputs
        Process_( inputBus, _outputBuses[bufferNo] );
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

inline void Component::Scan( std::vector<Component*>& components )
{
    // continue only if this component has not already been scanned
    if ( _scanPosition != -1 )
    {
        return;
    }

    // initialize _scanPosition
    _scanPosition = 0;

    for ( const auto& wire : _inputWires )
    {
        // scan incoming components
        wire.fromComponent->Scan( components );
    }

    components.emplace_back( this );
}

inline void Component::ScanParallel( std::vector<std::vector<DSPatch::Component*>>& componentsMap, int& scanPosition )
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

    // insert component at _scanPosition
    if ( _scanPosition == (int)componentsMap.size() )
    {
        componentsMap.emplace_back( std::vector<DSPatch::Component*>{} );
        componentsMap[_scanPosition].reserve( componentsMap.capacity() );
    }
    componentsMap[_scanPosition].emplace_back( this );
}

inline void Component::EndScan()
{
    // reset _scanPosition
    _scanPosition = -1;
}

inline void Component::SetInputCount_( int inputCount, const std::vector<std::string>& inputNames )
{
    _inputNames = inputNames;

    for ( auto& inputBus : _inputBuses )
    {
        inputBus.SetSignalCount( inputCount );
    }

    _inputWires.reserve( inputCount );
}

inline void Component::SetOutputCount_( int outputCount, const std::vector<std::string>& outputNames )
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

inline void Component::_WaitForRelease( int bufferNo )
{
    _releaseFlags[bufferNo].WaitAndClear();
}

inline void Component::_ReleaseNextBuffer( int bufferNo )
{
    if ( ++bufferNo == _bufferCount )  // release the next available buffer
    {
        _releaseFlags[0].Set();
    }
    else
    {
        _releaseFlags[bufferNo].Set();
    }
}

inline void Component::_GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
{
    auto& signal = *_outputBuses[bufferNo].GetSignal( fromOutput );

    if ( !signal.has_value() )
    {
        toBus.ClearValue( toInput );
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

inline void Component::_GetOutputParallel( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus )
{
    auto& signal = *_outputBuses[bufferNo].GetSignal( fromOutput );
    auto& ref = _refs[bufferNo][fromOutput];

    // wait for this output to be ready
    ref.readyFlag.WaitAndClear();

    if ( !signal.has_value() )
    {
        toBus.ClearValue( toInput );
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

inline void Component::_IncRefs( int output )
{
    for ( auto& ref : _refs )
    {
        ++ref[output].total;
    }
}

inline void Component::_DecRefs( int output )
{
    for ( auto& ref : _refs )
    {
        --ref[output].total;
    }
}

}  // namespace DSPatch
