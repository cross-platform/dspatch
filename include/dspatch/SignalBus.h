/************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2012-2019 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LICENSE included in the packaging of this file.
Please review the following information to ensure the GNU Lesser
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

#pragma once

#include <dspatch/Signal.h>

#include <vector>

namespace DSPatch
{

namespace internal
{
class SignalBus;
}

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
    DEFINE_PTRS( SignalBus );

    SignalBus();
    SignalBus( SignalBus&& );
    ~SignalBus();

    void SetSignalCount( int signalCount );
    int GetSignalCount() const;

    Signal::SPtr const& GetSignal( int signalIndex ) const;

    bool HasValue( int signalIndex ) const;

    template <class ValueType>
    ValueType* GetValue( int signalIndex ) const;

    template <class ValueType>
    bool SetValue( int signalIndex, ValueType const& newValue );

    bool CopySignal( int toSignalIndex, Signal::SPtr const& fromSignal );
    bool MoveSignal( int toSignalIndex, Signal::SPtr const& fromSignal );

    void ClearAllValues();

    std::type_info const& GetType( int signalIndex ) const;

private:
    std::vector<Signal::SPtr> _signals;

    std::unique_ptr<internal::SignalBus> p;
};

template <class ValueType>
ValueType* SignalBus::GetValue( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex]->GetValue<ValueType>();
    }
    else
    {
        return nullptr;
    }
}

template <class ValueType>
bool SignalBus::SetValue( int signalIndex, ValueType const& newValue )
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        _signals[signalIndex]->SetValue( newValue );
        return true;
    }
    else
    {
        return false;
    }
}

}  // namespace DSPatch
