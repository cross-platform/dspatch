/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2014 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LGPLv3.txt included in the packaging of this
file. Please review the following information to ensure the GNU Lesser
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

#ifndef DSPRUNTYPE_H
#define DSPRUNTYPE_H

//-------------------------------------------------------------------------------------------------

#include <utility>
#include <typeinfo>

//=================================================================================================
/// Dynamically typed variable

/**
DspRunType holds a variable that can be dynamically typed at run-time (hence the name). The
DspRunType class makes use of an internal template class and public template methods to allow
users to get and set the contained variable as any type they wish. A DspRunType object also has the
ability to change type at any point during program execution. Built-in typecasting and error
checking (via the RunTypeCast() method) prevents critical runtime errors from occurring when signal
types are mismatched.
*/

class DspRunType
{
public:
  DspRunType()
  : _valueHolder( NULL ) {}

  template< typename ValueType >
  DspRunType( ValueType const& value )
  {
    _valueHolder = new _DspRtValue< ValueType >( value );
  }

  DspRunType( DspRunType const& other )
  {
    if( other._valueHolder != NULL )
    {
      _valueHolder = other._valueHolder->GetCopy();
    }
    else
    {
      _valueHolder = NULL;
    }
  }

  virtual ~DspRunType()
  {
    delete _valueHolder;
  }

public:
  DspRunType& MoveTo( DspRunType& rhs )
  {
    std::swap( _valueHolder, rhs._valueHolder );
    return *this;
  }

  void CopyFrom( DspRunType const& rhs )
  {
    if( _valueHolder != NULL && rhs._valueHolder != NULL &&
        _valueHolder->GetType() == rhs._valueHolder->GetType() )
    {
      _valueHolder->SetValue( rhs._valueHolder );
    }
    else
    {
      *this = rhs;
    }
  }

  template< typename ValueType >
  DspRunType& operator=( ValueType const& rhs )
  {
    if( typeid( rhs ) == GetType() )
    {
      ( ( _DspRtValue< ValueType >* ) _valueHolder )->value = rhs;
    }
    else
    {
      DspRunType( rhs ).MoveTo( *this );
    }
    return *this;
  }

  DspRunType& operator=( DspRunType rhs )
  {
    rhs.MoveTo( *this );
    return *this;
  }

public:
  bool IsEmpty() const
  {
    return !_valueHolder;
  }

  std::type_info const& GetType() const
  {
    if( _valueHolder != NULL )
    {
      return _valueHolder->GetType();
    }
    else
    {
      return typeid( void );
    }
  }

  template< typename ValueType >
  static ValueType* RunTypeCast( DspRunType* operand )
  {
    if( operand != NULL && operand->GetType() == typeid( ValueType ) )
    {
      return &static_cast< DspRunType::_DspRtValue< ValueType >* >( operand->_valueHolder )->value;
    }
    else
    {
      return NULL;
    }
  }

  template< typename ValueType >
  static inline ValueType const* RunTypeCast( DspRunType const* operand )
  {
    return RunTypeCast< ValueType >( const_cast< DspRunType* >( operand ) );
  }

private:
  class _DspRtValueHolder
  {
  public:
    virtual ~_DspRtValueHolder() {}

  public:
    virtual std::type_info const& GetType() const = 0;
    virtual _DspRtValueHolder* GetCopy() const = 0;
    virtual void SetValue( _DspRtValueHolder* valueHolder ) = 0;
  };

  template< typename ValueType >
  class _DspRtValue : public _DspRtValueHolder
  {
  public:
    _DspRtValue( ValueType const& value )
    : value( value ) {}

  public:
    virtual std::type_info const& GetType() const
    {
      return typeid( ValueType );
    }

    virtual _DspRtValueHolder* GetCopy() const
    {
      return new _DspRtValue( value );
    }

    void SetValue( _DspRtValueHolder* valueHolder )
    {
      value = ( ( _DspRtValue< ValueType >* ) valueHolder )->value;
    }

  public:
    ValueType value;

  private:
    _DspRtValue& operator=( _DspRtValue const& ); // disable copy-assignment
  };

private:
  _DspRtValueHolder* _valueHolder;
};

//=================================================================================================

#endif // DSPRUNTYPE_H
