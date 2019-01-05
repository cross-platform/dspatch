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
