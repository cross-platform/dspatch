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

#include <dspatch/DspParameter.h>

//=================================================================================================

DspParameter::DspParameter()
  : _type( Null ),
    _isInputParam( true ),
    _isSet( false ),
    _isRangeSet( false ) {}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter( bool isInputParam, ParamType const& type )
  : _type( type ),
    _isInputParam( isInputParam ),
    _isSet( false ),
    _isRangeSet( false ) {}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter( bool isInputParam, ParamType const& type, float const& initValue, float const& minValue, float const& maxValue )
  : _type( type ),
    _isInputParam( isInputParam ),
    _isSet( false ),
    _isRangeSet( false )
{
  if( type == Bool )
  {
    if( !SetBool( initValue ) )
    {
      _type = Null;
    }
  }
  if( type == Int )
  {
    if( !SetIntRange( minValue, maxValue ) || !SetInt( initValue ) )
    {
      _type = Null;
    }
  }
  else if( type == Float )
  {
    if( !SetFloatRange( minValue, maxValue ) || !SetFloat( initValue ) )
    {
      _type = Null;
    }
  }
}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter( bool isInputParam, ParamType const& type, std::string const& initValue )
  : _type( type ),
    _isInputParam( isInputParam ),
    _isSet( false ),
    _isRangeSet( false )
{
  if( !SetString( initValue ) )
  {
    _type = Null;
  }
}

//-------------------------------------------------------------------------------------------------

DspParameter::DspParameter( bool isInputParam, ParamType const& type, std::vector< std::string > const& initValue )
  : _type( type ),
    _isInputParam( isInputParam ),
    _isSet( false ),
    _isRangeSet( false )
{
  if( !SetList( initValue ) )
  {
    _type = Null;
  }
}

//=================================================================================================

DspParameter::ParamType DspParameter::Type() const
{
  return _type;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::IsInputParam() const
{
  return _isInputParam;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::GetBool( bool& returnValue ) const
{
  if( !_isSet )
  {
    return false;
  }

  if( _type == Bool )
  {
    returnValue = _value.boolValue;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::GetInt( int& returnValue ) const
{
  if( !_isSet )
  {
    return false;
  }

  if( _type == Int || _type == List )
  {
    returnValue = _value.intValue.current;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::GetIntRange( int& minValue, int& maxValue ) const
{
  if( !_isRangeSet )
  {
    return false;
  }

  if( _type == Int || _type == List )
  {
    minValue = _value.intValue.min;
    maxValue = _value.intValue.max;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::GetFloat( float& returnValue ) const
{
  if( !_isSet )
  {
    return false;
  }

  if( _type == Float )
  {
    returnValue = _value.floatValue.current;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::GetFloatRange( float& minValue, float& maxValue ) const
{
  if( !_isRangeSet )
  {
    return false;
  }

  if( _type == Float )
  {
    minValue = _value.floatValue.min;
    maxValue = _value.floatValue.max;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::GetString( std::string& returnValue ) const
{
  if( !_isSet )
  {
    return false;
  }

  if( _type == String || _type == FilePath )
  {
    returnValue = _stringValue;
    return true;
  }
  else if( _type == List )
  {
    returnValue = _listValue[_value.intValue.current];
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::GetList( std::vector< std::string >& returnValue ) const
{
  if( !_isSet )
  {
    return false;
  }

  if( _type == List )
  {
    returnValue = _listValue;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetBool( bool const& value )
{
  if( _type == Bool )
  {
    _value.boolValue = value;
    _isSet = true;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetInt( int const& value )
{
  if( _type == Int || _type == List )
  {
    if( _isRangeSet )
    {
      _value.intValue.current = value < _value.intValue.min ? _value.intValue.min : value;
      _value.intValue.current = value > _value.intValue.max ? _value.intValue.max : value;
    }
    else
    {
      _value.intValue.current = value;
    }
    _isSet = true;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetIntRange( int const& minValue, int const& maxValue )
{
  if( minValue == maxValue && minValue == -1 )
  {
    _isRangeSet = false;
    return true;
  }

  if( _type == Int )
  {
    _value.intValue.min = minValue;
    _value.intValue.max = maxValue;

    _value.intValue.current = _value.intValue.current < minValue ?
          minValue : _value.intValue.current;

    _value.intValue.current = _value.intValue.current > maxValue ?
          maxValue : _value.intValue.current;

    _isRangeSet = true;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetFloat( float const& value )
{
  if( _type == Float )
  {
    if( _isRangeSet )
    {
      _value.floatValue.current = value < _value.floatValue.min ? _value.floatValue.min : value;
      _value.floatValue.current = value > _value.floatValue.max ? _value.floatValue.max : value;
    }
    else
    {
      _value.floatValue.current = value;
    }
    _isSet = true;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetFloatRange( float const& minValue, float const& maxValue )
{
  if( minValue == maxValue && minValue == -1 )
  {
    _isRangeSet = false;
    return true;
  }

  if( _type == Float )
  {
    _value.floatValue.min = minValue;
    _value.floatValue.max = maxValue;

    _value.floatValue.current = _value.floatValue.current < minValue ?
          minValue : _value.floatValue.current;

    _value.floatValue.current = _value.floatValue.current > maxValue ?
          maxValue : _value.floatValue.current;

    _isRangeSet = true;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetString( std::string const& value )
{
  if( _type == String || _type == FilePath )
  {
    _stringValue = value;
    _isSet = true;
    return true;
  }
  else if( _type == List )
  {
    for( unsigned short i = 0; i < _listValue.size(); ++i )
    {
      if( _listValue[i] == value )
      {
        _value.intValue.current = i;
        _isSet = true;
        return true;
      }
    }
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetList( std::vector< std::string > const& value )
{
  if( _type == List )
  {
    _listValue = value;
    _value.intValue.min = 0;
    _value.intValue.max = value.size() - 1;
    _value.intValue.current = 0;
    _isSet = true;
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

bool DspParameter::SetParam( DspParameter const& param )
{
  if( param.Type() == Bool )
  {
    bool value;
    if( param.GetBool( value ) )
    {
      _type = param.Type();
      _isInputParam = param.IsInputParam();
      SetBool( value );
      return true;
    }
  }
  else if( param.Type() == Int )
  {
    int minValue, maxValue, value;
    if( param.GetIntRange( minValue, maxValue ) && param.GetInt( value ) )
    {
      _type = param.Type();
      _isInputParam = param.IsInputParam();
      SetIntRange( minValue, maxValue );
      SetInt( value );
      return true;
    }
  }
  else if( param.Type() == Float )
  {
    float minValue, maxValue, value;
    if( param.GetFloatRange( minValue, maxValue ) && param.GetFloat( value ) )
    {
      _type = param.Type();
      _isInputParam = param.IsInputParam();
      SetFloatRange( minValue, maxValue );
      SetFloat( value );
      return true;
    }
  }
  else if( param.Type() == String || param.Type() == FilePath )
  {
    std::string value;
    if( param.GetString( value ) )
    {
      _type = param.Type();
      _isInputParam = param.IsInputParam();
      SetString( value );
      return true;
    }
  }
  else if( param.Type() == Trigger )
  {
    _type = param.Type();
    _isInputParam = param.IsInputParam();
    return true;
  }
  else if( param.Type() == List )
  {
    std::vector< std::string > value;
    if( param.GetList( value ) )
    {
      _type = param.Type();
      _isInputParam = param.IsInputParam();
      SetList( value );
      return true;
    }
  }

  return false;
}

//=================================================================================================
