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
};

}  // namespace internal
}  // namespace DSPatch

Signal::Signal()
    : p( new internal::Signal() )
    , _value( std::make_shared<RunType>() )
{
}

Signal::~Signal()
{
}

bool Signal::HasValue() const
{
    return _value->HasValue();
}

bool Signal::CopySignal( Signal::SPtr const& newSignal )
{
    return _value->CopyRunType( newSignal->_value );
}

bool Signal::MoveSignal( Signal::SPtr const& newSignal )
{
    return _value->MoveRunType( newSignal->_value );
}

void Signal::ClearValue()
{
    _value->ClearValue();
}
