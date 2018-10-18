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

namespace DSPatch
{
namespace internal
{

class Signal
{
public:
    int deps = 0;
    int depsServiced = 0;
};

}  // namespace internal
}  // namespace DSPatch

Signal::Signal()
    : _valueAvailable( false )
    , p( new internal::Signal() )
{
}

Signal::~Signal()
{
}

bool Signal::SetSignal( Signal::SPtr const& newSignal )
{
    if ( newSignal != nullptr )
    {
        if ( newSignal->_valueAvailable == false )
        {
            return false;
        }
        else
        {
            if ( newSignal->p->depsServiced == newSignal->p->deps - 1 )
            {
                // This is the incoming signal's last dependent, so we move it.
                newSignal->_signalValue.MoveTo( _signalValue );
                newSignal->_valueAvailable = false;
            }
            else
            {
                _signalValue.CopyFrom( newSignal->_signalValue );
            }

            _valueAvailable = true;
            ++newSignal->p->depsServiced;
            return true;
        }
    }
    else
    {
        return false;
    }
}

bool Signal::MoveSignal( Signal::SPtr const& newSignal )
{
    if ( newSignal != nullptr )
    {
        if ( newSignal->_valueAvailable == false )
        {
            return false;
        }
        else
        {
            newSignal->_signalValue.MoveTo( _signalValue );
            newSignal->_valueAvailable = false;

            _valueAvailable = true;
            return true;
        }
    }
    else
    {
        return false;
    }
}

void Signal::ClearValue()
{
    _valueAvailable = false;
    p->depsServiced = 0;
}

int Signal::_Deps() const
{
    return p->deps;
}

void Signal::_IncDeps()
{
    ++p->deps;
}

void Signal::_DecDeps()
{
    --p->deps;
}

void Signal::_SetDeps( int deps )
{
    p->deps = deps;
}
