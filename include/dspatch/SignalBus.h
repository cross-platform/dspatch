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

#pragma once

#include <dspatch/Signal.h>

#include <vector>

namespace DSPatch
{

/// Signal container

/**
A SignalBus contains Signals (see Signal). Via the Process_() method, a Component receives signals
into it's "inputs" SignalBus and provides signals to it's "outputs" SignalBus. The SignalBus class
provides public getters and setters for manipulating it's internal Signal values directly,
abstracting the need to retrieve and interface with the contained Signals themself.
*/

class DLLEXPORT SignalBus final
{
public:
    NONCOPYABLE( SignalBus );

    inline SignalBus() = default;
    inline SignalBus( SignalBus&& ) = default;
    inline ~SignalBus() = default;

    inline void SetSignalCount( int signalCount );
    inline int GetSignalCount() const;

    inline Signal& GetSignal( int signalIndex );

    inline bool HasValue( int signalIndex ) const;

    template <class ValueType>
    ValueType* GetValue( int signalIndex ) const;

    template <class ValueType>
    bool SetValue( int signalIndex, const ValueType& newValue );

    template <class ValueType>
    bool MoveValue( int signalIndex, ValueType&& newValue );

    inline bool SetSignal( int toSignalIndex, const Signal& fromSignal );
    inline bool MoveSignal( int toSignalIndex, Signal& fromSignal );

    inline void ClearAllValues();

    inline unsigned int GetType( int signalIndex ) const;

private:
    std::vector<Signal> _signals;
    Signal _emptySignal;
};

inline void SignalBus::SetSignalCount( int signalCount )
{
    _signals.resize( signalCount );
}

inline int SignalBus::GetSignalCount() const
{
    return (int)_signals.size();
}

inline Signal& SignalBus::GetSignal( int signalIndex )
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex];
    }
    else
    {
        _emptySignal.ClearValue();
        return _emptySignal;
    }
}

inline bool SignalBus::HasValue( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex].HasValue();
    }
    else
    {
        return false;
    }
}

template <class ValueType>
ValueType* SignalBus::GetValue( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex].GetValue<ValueType>();
    }
    else
    {
        return nullptr;
    }
}

template <class ValueType>
bool SignalBus::SetValue( int signalIndex, const ValueType& newValue )
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        _signals[signalIndex].SetValue( newValue );
        return true;
    }
    else
    {
        return false;
    }
}

template <class ValueType>
bool SignalBus::MoveValue( int signalIndex, ValueType&& newValue )
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        _signals[signalIndex].MoveValue( std::move( newValue ) );
        return true;
    }
    else
    {
        return false;
    }
}

inline bool SignalBus::SetSignal( int toSignalIndex, const Signal& fromSignal )
{
    if ( (size_t)toSignalIndex < _signals.size() )
    {
        return _signals[toSignalIndex].SetSignal( fromSignal );
    }
    else
    {
        return false;
    }
}

inline bool SignalBus::MoveSignal( int toSignalIndex, Signal& fromSignal )
{
    if ( (size_t)toSignalIndex < _signals.size() )
    {
        return _signals[toSignalIndex].MoveSignal( fromSignal );
    }
    else
    {
        return false;
    }
}

inline void SignalBus::ClearAllValues()
{
    for ( auto& signal : _signals )
    {
        signal.ClearValue();
    }
}

inline unsigned int SignalBus::GetType( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex].GetType();
    }
    else
    {
        return type_id<void>;
    }
}

}  // namespace DSPatch
