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

#include <dspatch/Common.h>

namespace DSPatch
{

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
    ~Signal();

    bool HasValue() const;

    template <class ValueType>
    ValueType* GetValue();

    template <class ValueType>
    void SetValue( ValueType const& newValue );

    bool CopySignal( Signal::SPtr const& fromSignal );
    bool MoveSignal( Signal::SPtr const& fromSignal );

    void ClearValue();

    std::type_info const& GetType() const;

private:
    struct _ValueHolder
    {
        NONCOPYABLE( _ValueHolder );

        _ValueHolder() = default;
        virtual ~_ValueHolder() = default;

        virtual std::type_info const& GetType() const = 0;
        virtual _ValueHolder* GetCopy() const = 0;
        virtual void SetValue( _ValueHolder* valueHolder ) = 0;
    };

    template <class ValueType>
    struct _Value final : _ValueHolder
    {
        NONCOPYABLE( _Value );

        _Value( ValueType const& value )
            : value( value )
            , type( typeid( ValueType ) )
        {
        }

        virtual std::type_info const& GetType() const override
        {
            return type;
        }

        virtual _ValueHolder* GetCopy() const override
        {
            return new _Value( value );
        }

        virtual void SetValue( _ValueHolder* valueHolder ) override
        {
            value = ( (_Value<ValueType>*)valueHolder )->value;
        }

        ValueType value;
        std::type_info const& type;
    };

    _ValueHolder* _valueHolder = nullptr;
    bool _hasValue = false;
};

template <class ValueType>
ValueType* Signal::GetValue()
{
    // You might be thinking: Why the raw pointer return here?

    // This is mainly for performance, and partly for readability. Performance, because returning a
    // shared_ptr here means having to store the value as a shared_ptr in Signal::_Value too. This
    // adds yet another level of indirection to the value, as well as some reference counting
    // overhead. These Get() and Set() methods are VERY frequently called, so doing as little as
    // possible with the data here is best, which actually aids in the readably of the code too.

    if ( _hasValue && GetType() == typeid( ValueType ) )
    {
        return &( (_Value<ValueType>*)_valueHolder )->value;
    }
    else
    {
        return nullptr;
    }
}

template <class ValueType>
void Signal::SetValue( ValueType const& newValue )
{
    if ( GetType() == typeid( ValueType ) )
    {
        ( (_Value<ValueType>*)_valueHolder )->value = newValue;
    }
    else
    {
        delete _valueHolder;
        _valueHolder = new _Value<ValueType>( newValue );
    }
    _hasValue = true;
}

}  // namespace DSPatch
