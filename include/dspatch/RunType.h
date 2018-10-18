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

namespace DSPatch
{

/// Dynamically typed variable

/**
RunType holds a variable that can be dynamically typed at run-time (hence the name). The RunType
class makes use of an internal template class and public template methods to allow users to get and
set the contained variable as any type they wish. A RunType object also has the ability to change
type at any point during program execution. Built-in typecasting and error checking (via the
RunTypeCast() method) prevents critical runtime errors from occurring when signal types are
mismatched.
*/

class RunType final
{
public:
    RunType()
        : _valueHolder( nullptr )
    {
    }

    template <class ValueType>
    RunType( ValueType const& value )
    {
        _valueHolder = new _RtValue<ValueType>( value );
    }

    RunType( RunType const& other )
    {
        if ( other._valueHolder != nullptr )
        {
            _valueHolder = other._valueHolder->GetCopy();
        }
        else
        {
            _valueHolder = nullptr;
        }
    }

    virtual ~RunType()
    {
        delete _valueHolder;
    }

public:
    RunType& MoveTo( RunType& rhs )
    {
        std::swap( _valueHolder, rhs._valueHolder );
        return *this;
    }

    void CopyFrom( RunType const& rhs )
    {
        if ( _valueHolder != nullptr && rhs._valueHolder != nullptr && _valueHolder->GetType() == rhs._valueHolder->GetType() )
        {
            _valueHolder->SetValue( rhs._valueHolder );
        }
        else
        {
            *this = rhs;
        }
    }

    template <class ValueType>
    RunType& operator=( ValueType const& rhs )
    {
        if ( typeid( ValueType ) == GetType() )
        {
            ( (_RtValue<ValueType>*)_valueHolder )->_value = rhs;
        }
        else
        {
            RunType( rhs ).MoveTo( *this );
        }
        return *this;
    }

    RunType& operator=( RunType rhs )
    {
        rhs.MoveTo( *this );
        return *this;
    }

public:
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

    template <class ValueType>
    static ValueType* RunTypeCast( RunType* operand )
    {
        if ( operand != nullptr && operand->GetType() == typeid( ValueType ) )
        {
            return &static_cast<RunType::_RtValue<ValueType>*>( operand->_valueHolder )->_value;
        }
        else
        {
            return nullptr;
        }
    }

private:
    class _RtValueHolder
    {
    public:
        virtual ~_RtValueHolder()
        {
        }

    public:
        virtual std::type_info const& GetType() const = 0;
        virtual _RtValueHolder* GetCopy() const = 0;
        virtual void SetValue( _RtValueHolder* valueHolder ) = 0;
    };

    template <class ValueType>
    class _RtValue : public _RtValueHolder
    {
    public:
        _RtValue( ValueType const& value )
            : _value( value )
            , _type( typeid( ValueType ) )
        {
        }

    public:
        virtual std::type_info const& GetType() const
        {
            return _type;
        }

        virtual _RtValueHolder* GetCopy() const
        {
            return new _RtValue( _value );
        }

        void SetValue( _RtValueHolder* valueHolder )
        {
            _value = ( (_RtValue<ValueType>*)valueHolder )->_value;
        }

    public:
        ValueType _value;
        std::type_info const& _type;

    private:
        _RtValue& operator=( _RtValue const& );  // disable copy-assignment
    };

private:
    _RtValueHolder* _valueHolder;
};

}  // namespace DSPatch
