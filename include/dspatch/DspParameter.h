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

#ifndef DSPPARAMETER_H
#define DSPPARAMETER_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspThread.h>
#include <string>
#include <vector>

//=================================================================================================
/// Value container used to hold non-transient component IO

/**
DspParameters are similar to DspSignals in that they provide a generic means of inputting and
outputting data to and from DspComponents. However, unlike signals, parameters are non-transient
inputs and outputs (such as enable/disable, bias, offset, etc.) that allow for a component's
behaviour to be manipulated via direct set/get methods. The parameter type (ParamType) must be
specified on construction of a DspParameter. Any type mismatches on subsequent set/get calls will
immediatly return false.

NOTE: Abstracting component parameters behind this generic DspParameter container allows a
component to be entirely controllable via the DspComponent base class.
*/

class DLLEXPORT DspParameter
{
public:
  enum ParamType
  {
    Null,
    Bool,
    Int,
    Float,
    String,
    FilePath, // this is essentially just a string, but helps when determining an appropriate user input method
    Trigger, // this type has no value, SetParam(triggerParam) simply represents a trigger. E.g. a button press
    List // this type acts as a vector (available items), an int (index selected), and a string (item selected)
  };

  DspParameter( ParamType const& type, bool isInputParam = true );

  ParamType const Type() const;
  bool const IsInputParam() const;

  bool GetBool( bool& returnValue ) const;
  bool GetInt( int& returnValue ) const;
  bool GetIntRange( int& minValue, int& maxValue ) const;
  bool GetFloat( float& returnValue ) const;
  bool GetFloatRange( float& minValue, float& maxValue ) const;
  bool GetString( std::string& returnValue ) const;
  bool GetList( std::vector< std::string >& returnValue ) const;

  bool SetBool( bool const& value );
  bool SetInt( int const& value );
  bool SetIntRange( int const& minValue, int const& maxValue );
  bool SetFloat( float const& value );
  bool SetFloatRange( float const& minValue, float const& maxValue );
  bool SetString( std::string const& value );
  bool SetList( std::vector< std::string > const& value );
  bool SetParam( DspParameter const& param );

private:
  const ParamType _type;
  const bool _isInputParam;
  bool _isSet;
  bool _isRangeSet;

  union
  {
    bool boolValue;

    struct
    {
      int min;
      int max;
      int current;
    } intValue;

    struct
    {
      float min;
      float max;
      float current;
    } floatValue;
  } _value;

  std::string _stringValue;
  std::vector< std::string > _listValue;
};

//=================================================================================================

#endif // DSPPARAMETER_H
