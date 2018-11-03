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

namespace DSPatch
{

/// Dynamically typed variable

/**
Signal holds a variable that can be dynamically typed at run-time (hence the name). The Signal
class makes use of an internal template class and public template methods to allow users to get and
set the contained variable as any type they wish. A Signal object also has the ability to change
type at any point during program execution. Built-in typecasting and error checking (via the
SignalCast() method) prevents critical runtime errors from occurring when signal types are
mismatched.
*/

class DLLEXPORT Signal final
{
public:
    NONCOPYABLE( Signal );
    DEFINE_PTRS( Signal );

    Signal()
    {
    }

    virtual ~Signal()
    {
        delete _valueHolder;
    }

    bool HasValue() const
    {
        return _hasValue;
    }

    template <class ValueType>
    ValueType* GetValue()
    {
        if ( _hasValue && GetType() == typeid( ValueType ) )
        {
            return &static_cast<Signal::_RtValue<ValueType>*>( _valueHolder )->value;
        }
        else
        {
            return nullptr;
        }
    }

    template <class ValueType>
    void SetValue( ValueType const& newValue )
    {
        if ( GetType() == typeid( ValueType ) )
        {
            ( (_RtValue<ValueType>*)_valueHolder )->value = newValue;
        }
        else
        {
            delete _valueHolder;
            _valueHolder = new _RtValue<ValueType>( newValue );
        }
        _hasValue = true;
    }

    bool CopySignal( Signal::SPtr const& newSignal )
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

    bool MoveSignal( Signal::SPtr const& newSignal )
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

    void ClearValue()
    {
        _hasValue = false;
    }

    std::type_info const& GetType() const
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

private:
    class _RtValueHolder
    {
    public:
        NONCOPYABLE( _RtValueHolder );

        _RtValueHolder()
        {
        }

        virtual ~_RtValueHolder()
        {
        }

        virtual std::type_info const& GetType() const = 0;
        virtual _RtValueHolder* GetCopy() const = 0;
        virtual void SetValue( _RtValueHolder* valueHolder ) = 0;
    };

    template <class ValueType>
    class _RtValue final : public _RtValueHolder
    {
    public:
        NONCOPYABLE( _RtValue );

        _RtValue( ValueType const& value )
            : value( value )
            , type( typeid( ValueType ) )
        {
        }

        virtual std::type_info const& GetType() const override
        {
            return type;
        }

        virtual _RtValueHolder* GetCopy() const override
        {
            return new _RtValue( value );
        }

        virtual void SetValue( _RtValueHolder* valueHolder ) override
        {
            value = ( (_RtValue<ValueType>*)valueHolder )->value;
        }

        ValueType value;
        std::type_info const& type;
    };

    _RtValueHolder* _valueHolder = nullptr;
    bool _hasValue = false;
};

}  // namespace DSPatch
