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

bool Signal::CopySignal( Signal::SPtr const& newSignal )
{
    if ( newSignal != nullptr && newSignal->_hasValue )
    {
        if ( _valueHolder != nullptr && newSignal->_valueHolder != nullptr &&
             _valueHolder->GetType() == newSignal->_valueHolder->GetType() )
        {
            _valueHolder->SetValue( newSignal->_valueHolder );
        }
        else
        {
            delete _valueHolder;
            _valueHolder = newSignal->_valueHolder->GetCopy();
        }

        _hasValue = true;
        return true;
    }
    else
    {
        return false;
    }
}

bool Signal::MoveSignal( Signal::SPtr const& newSignal )
{
    if ( newSignal != nullptr && newSignal->_hasValue )
    {
        std::swap( newSignal->_valueHolder, _valueHolder );
        newSignal->_hasValue = false;

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
