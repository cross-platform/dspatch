/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2021, Marcus Tomlinson

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

#include <dspatch/SignalBus.h>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class SignalBus
{
public:
    Signal::SPtr nullSignal = nullptr;
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

Signal::SPtr const& SignalBus::GetSignal( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex];
    }
    else
    {
        return p->nullSignal;
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
