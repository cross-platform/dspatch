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

#include <dspatch/Signal.h>

using namespace DSPatch;

Signal::Signal()
{
}

Signal::~Signal()
{
    delete _valueHolder;
}

bool Signal::HasValue() const
{
    return _hasValue;
}

bool Signal::CopySignal( Signal::SPtr const& fromSignal )
{
    if ( fromSignal != nullptr && fromSignal->_hasValue )
    {
        if ( _valueHolder != nullptr && fromSignal->_valueHolder != nullptr &&
             _valueHolder->GetType() == fromSignal->_valueHolder->GetType() )
        {
            _valueHolder->SetValue( fromSignal->_valueHolder );
        }
        else
        {
            delete _valueHolder;
            _valueHolder = fromSignal->_valueHolder->GetCopy();
        }

        _hasValue = true;
        return true;
    }
    else
    {
        return false;
    }
}

bool Signal::MoveSignal( Signal::SPtr const& fromSignal )
{
    if ( fromSignal != nullptr && fromSignal->_hasValue )
    {
        // You might be thinking: Why std::swap and not std::move here?

        // This is a really nifty little optimisation actually. When we move a signal value from an
        // output to an input (or vice-versa within a component) we move it's type_info along with
        // it. If you look at SetValue(), you'll see that type_info is really useful in determining
        // whether we have to delete and copy (re)construct our contained value, or can simply copy
        // assign. To avoid the former as much as possible, a swap is done between source and
        // target signals such that, between these two points, just two value holders need to be
        // constructed, and shared back and forth from then on.

        std::swap( fromSignal->_valueHolder, _valueHolder );
        fromSignal->_hasValue = false;

        _hasValue = true;
        return true;
    }
    else
    {
        return false;
    }
}

void Signal::ClearValue()
{
    _hasValue = false;
}

std::type_info const& Signal::GetType() const
{
    if ( _valueHolder != nullptr )
    {
        return _valueHolder->GetType();
    }
    else
    {
        return typeid( void );
    }
}
