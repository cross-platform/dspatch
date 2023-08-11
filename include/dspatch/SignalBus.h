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

#include <dspatch/Common.h>

#include <any>

#include <vector>

namespace DSPatch
{

/// Signal container

/**
A SignalBus contains signals. Via the Process_() method, a Component receives signals into
its "inputs" SignalBus and provides signals to its "outputs" SignalBus. The SignalBus class
provides public getters and setters for manipulating its internal signal values directly,
abstracting the need to retrieve and interface with the contained Signals themself.
*/

class DLLEXPORT SignalBus final
{
public:
    NONCOPYABLE( SignalBus );

    inline SignalBus();
    inline SignalBus( SignalBus&& );

    inline void SetSignalCount( int signalCount );
    inline int GetSignalCount() const;

    inline std::any* GetSignal( int signalIndex );

    inline bool HasValue( int signalIndex ) const;

    template <typename ValueType>
    inline ValueType* GetValue( int signalIndex ) const;

    template <typename ValueType>
    inline void SetValue( int signalIndex, const ValueType& newValue );

    template <typename ValueType>
    inline void MoveValue( int signalIndex, ValueType&& newValue );

    inline void SetSignal( int toSignalIndex, const std::any& fromSignal );
    inline void MoveSignal( int toSignalIndex, std::any& fromSignal );

    inline void ClearAllValues();

    inline const std::type_info& GetType( int signalIndex ) const;

private:
    std::vector<std::any> _signals;
};

inline SignalBus::SignalBus() = default;

// cppcheck-suppress missingMemberCopy
inline SignalBus::SignalBus( SignalBus&& rhs )
    : _signals( std::move( rhs._signals ) )
{
}

inline void SignalBus::SetSignalCount( int signalCount )
{
    _signals.resize( signalCount );
}

inline int SignalBus::GetSignalCount() const
{
    return (int)_signals.size();
}

inline std::any* SignalBus::GetSignal( int signalIndex )
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return &_signals[signalIndex];
    }
    else
    {
        return nullptr;
    }
}

inline bool SignalBus::HasValue( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex].has_value();
    }
    else
    {
        return false;
    }
}

template <typename ValueType>
inline ValueType* SignalBus::GetValue( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        try
        {
            return const_cast<ValueType*>( &std::any_cast<const ValueType&>( _signals[signalIndex] ) );
        }
        catch ( const std::exception& )
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}

template <typename ValueType>
inline void SignalBus::SetValue( int signalIndex, const ValueType& newValue )
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        _signals[signalIndex].emplace<ValueType>( newValue );
    }
}

template <typename ValueType>
inline void SignalBus::MoveValue( int signalIndex, ValueType&& newValue )
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        _signals[signalIndex].emplace<ValueType>( std::move( newValue ) );
    }
}

inline void SignalBus::SetSignal( int toSignalIndex, const std::any& fromSignal )
{
    if ( (size_t)toSignalIndex < _signals.size() )
    {
        _signals[toSignalIndex] = fromSignal;
    }
}

inline void SignalBus::MoveSignal( int toSignalIndex, std::any& fromSignal )
{
    if ( (size_t)toSignalIndex < _signals.size() && fromSignal.has_value() )
    {
        _signals[toSignalIndex].swap( fromSignal );
    }
}

inline void SignalBus::ClearAllValues()
{
    for ( auto& signal : _signals )
    {
        signal.reset();
    }
}

inline const std::type_info& SignalBus::GetType( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex].type();
    }
    else
    {
        return typeid( void );
    }
}

}  // namespace DSPatch
