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

#include <dspatch/Common.h>
#include <dspatch/RunType.h>

#include <string>
#include <vector>

namespace DSPatch
{

namespace internal
{
class Signal;
}

/// Value container used to carry data between components

/**
Components process and transfer data between each other in the form of "signals" via interconnected
wires. The Signal class holds a single value that can be dynamically typed at runtime. Furthermore,
a Signal has the ability to change it's data type at any point during program execution. This is
designed such that a signal bus can hold any number of different typed variables, as well as to
allow for a variable to dynamically change it's type when needed - this can be useful for inputs
that accept a number of different data types (E.g. Varying sample size in an audio buffer: array of
byte / int / float).
*/

class DLLEXPORT Signal final
{
public:
    NONCOPYABLE( Signal );
    DEFINE_PTRS( Signal );

    Signal();
    virtual ~Signal();

    template <class ValueType>
    void SetValue( ValueType const& newValue );

    template <class ValueType>
    ValueType* GetValue();

    bool SetSignal( Signal::SPtr const& newSignal );
    bool MoveSignal( Signal::SPtr const& newSignal );

    void ClearValue();

private:
    // Private methods required by Component

    int _Deps() const;
    void _IncDeps();
    void _DecDeps();
    void _SetDeps( int deps );

private:
    friend class Component;

    RunType _signalValue;
    bool _valueAvailable;

    std::unique_ptr<internal::Signal> p;
};

template <class ValueType>
void Signal::SetValue( ValueType const& newValue )
{
    _signalValue = newValue;
    _valueAvailable = true;
}

template <class ValueType>
ValueType* Signal::GetValue()
{
    if ( _valueAvailable )
    {
        return RunType::RunTypeCast<ValueType>( &_signalValue );
    }
    else
    {
        return nullptr;  // no value available
    }
}

}  // namespace DSPatch
