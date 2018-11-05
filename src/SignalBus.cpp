/************************************************************************
DSPatch - The C++ Flow-Based Programming Framework
Copyright (c) 2012-2018 Marcus Tomlinson

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

#include <dspatch/SignalBus.h>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class SignalBus
{
};

}  // namespace internal
}  // namespace DSPatch

SignalBus::SignalBus()
    : p( new internal::SignalBus() )
{
}

SignalBus::SignalBus( SignalBus&& rhs )
{
    _signals = rhs._signals;
}

SignalBus::~SignalBus()
{
}

void SignalBus::SetSignalCount( int signalCount )
{
    int fromSize = _signals.size();

    _signals.resize( signalCount );

    for ( int i = fromSize; i < signalCount; ++i )
    {
        _signals[i] = std::make_shared<Signal>();
    }
}

int SignalBus::GetSignalCount() const
{
    return _signals.size();
}

Signal::SPtr SignalBus::GetSignal( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex];
    }
    else
    {
        return nullptr;
    }
}

bool SignalBus::HasValue( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex]->HasValue();
    }
    else
    {
        return false;
    }
}

bool SignalBus::CopySignal( int toSignalIndex, Signal::SPtr const& fromSignal )
{
    if ( (size_t)toSignalIndex < _signals.size() )
    {
        return _signals[toSignalIndex]->CopySignal( fromSignal );
    }
    else
    {
        return false;
    }
}

bool SignalBus::MoveSignal( int toSignalIndex, Signal::SPtr const& fromSignal )
{
    if ( (size_t)toSignalIndex < _signals.size() )
    {
        return _signals[toSignalIndex]->MoveSignal( fromSignal );
    }
    else
    {
        return false;
    }
}

void SignalBus::ClearAllValues()
{
    for ( auto& signal : _signals )
    {
        signal->ClearValue();
    }
}

std::type_info const& SignalBus::GetType( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex]->GetType();
    }
    else
    {
        return typeid( void );
    }
}
