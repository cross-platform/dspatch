/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2023, Marcus Tomlinson

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

#pragma once

#include <dspatch/Common.h>

namespace DSPatch
{

inline unsigned int type_id_seq = 0;
template <typename T>
inline const unsigned int type_id = type_id_seq++;

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

    inline Signal();
    inline Signal( Signal&& );
    inline ~Signal();

    inline bool HasValue() const;

    template <typename ValueType>
    inline ValueType* GetValue() const;

    template <typename ValueType>
    inline void SetValue( const ValueType& newValue );

    template <typename ValueType>
    inline void MoveValue( ValueType&& newValue );

    inline void SetSignal( const Signal& fromSignal );
    inline void MoveSignal( Signal& fromSignal );

    inline void ClearValue();

    inline unsigned int GetType() const;

private:
    struct _ValueHolder
    {
        NONCOPYABLE( _ValueHolder );

        inline _ValueHolder() = default;
        virtual inline ~_ValueHolder() = default;

        virtual inline _ValueHolder* GetCopy() const = 0;
        virtual inline void SetValue( _ValueHolder* valueHolder ) = 0;
    };

    template <typename ValueType>
    struct _Value final : _ValueHolder
    {
        NONCOPYABLE( _Value );

        explicit inline _Value( const ValueType& value )
            : type( type_id<ValueType> )
            , value( value )
        {
        }

        virtual inline _ValueHolder* GetCopy() const override
        {
            return new _Value( value );
        }

        virtual inline void SetValue( _ValueHolder* valueHolder ) override
        {
            value = ( (_Value<ValueType>*)valueHolder )->value;
        }

        const unsigned int type;
        ValueType value;
    };

    _ValueHolder* _valueHolder = nullptr;
    bool _hasValue = false;
};

inline Signal::Signal() = default;

inline Signal::Signal( Signal&& )
{
}

inline Signal::~Signal()
{
    delete _valueHolder;
}

inline bool Signal::HasValue() const
{
    return _hasValue;
}

template <typename ValueType>
inline ValueType* Signal::GetValue() const
{
    // You might be thinking: Why the raw pointer return here?

    // This is mainly for performance, and partly for readability. Performance, because returning a
    // shared_ptr here means having to store the value as a shared_ptr in Signal::_Value too. This
    // adds yet another level of indirection to the value, as well as some reference counting
    // overhead. These Get() and Set() methods are VERY frequently called, so doing as little as
    // possible with the data here is best, which actually aids in the readably of the code too.

    // cppcheck-suppress cstyleCast
    if ( _hasValue && ( (_Value<nullptr_t>*)_valueHolder )->type == type_id<ValueType> )
    {
        return &( (_Value<ValueType>*)_valueHolder )->value;
    }
    else
    {
        return nullptr;
    }
}

template <typename ValueType>
inline void Signal::SetValue( const ValueType& newValue )
{
    // cppcheck-suppress cstyleCast
    if ( _valueHolder && ( (_Value<nullptr_t>*)_valueHolder )->type == type_id<ValueType> )
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

template <typename ValueType>
inline void Signal::MoveValue( ValueType&& newValue )
{
    // cppcheck-suppress cstyleCast
    if ( _valueHolder && ( (_Value<nullptr_t>*)_valueHolder )->type == type_id<ValueType> )
    {
        ( (_Value<ValueType>*)_valueHolder )->value = std::move( newValue );
    }
    else
    {
        delete _valueHolder;
        _valueHolder = new _Value<ValueType>( std::move( newValue ) );
    }
    _hasValue = true;
}

inline void Signal::SetSignal( const Signal& fromSignal )
{
    if ( fromSignal._hasValue )
    {
        // cppcheck-suppress cstyleCast
        if ( _valueHolder && ( (_Value<nullptr_t>*)_valueHolder )->type == ( (_Value<nullptr_t>*)fromSignal._valueHolder )->type )
        {
            _valueHolder->SetValue( fromSignal._valueHolder );
        }
        else
        {
            delete _valueHolder;
            _valueHolder = fromSignal._valueHolder->GetCopy();
        }

        _hasValue = true;
    }
}

inline void Signal::MoveSignal( Signal& fromSignal )
{
    if ( fromSignal._hasValue )
    {
        // You might be thinking: Why std::swap and not std::move here?

        // This is a really nifty little optimisation actually. When we move a signal value from an
        // output to an input (or vice-versa within a component) we move it's type_id along with
        // it. If you look at SetValue(), you'll see that type_id is really useful in determining
        // whether we have to delete and copy (re)construct our contained value, or can simply copy
        // assign. To avoid the former as much as possible, a swap is done between source and
        // target signals such that, between these two points, just two value holders need to be
        // constructed, and shared back and forth from then on.

        std::swap( fromSignal._valueHolder, _valueHolder );
        std::swap( fromSignal._hasValue, _hasValue );
    }
}

inline void Signal::ClearValue()
{
    _hasValue = false;
}

inline unsigned int Signal::GetType() const
{
    if ( _valueHolder )
    {
        // cppcheck-suppress cstyleCast
        return ( (_Value<nullptr_t>*)_valueHolder )->type;
    }
    else
    {
        return type_id<void>;
    }
}

}  // namespace DSPatch
