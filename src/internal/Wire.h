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

#pragma once

#include <dspatch/Component.h>

namespace DSPatch
{

class Component;

namespace internal
{

/// Connection between two components

/**
Components process and transfer data between each other in the form of signals via interconnected
"wires". Each wire contains references to the linked component, the source output signal, and the
destination input signal. The Wire struct simply stores these references for use in retrieving and
providing signals across component connections.
*/

struct Wire final
{
    Wire( DSPatch::Component::SPtr const& newLinkedComponent, int newFromSignalIndex, int newToSignalIndex )
        : fromComponent( newLinkedComponent )
        , fromOutput( newFromSignalIndex )
        , toInput( newToSignalIndex )
    {
    }

    DSPatch::Component::SPtr fromComponent;
    int fromOutput;
    int toInput;
};

}  // namespace internal
}  // namespace DSPatch
