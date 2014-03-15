/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2013 Marcus Tomlinson

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
/// TODO

/**
///!TODO
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

  DspParameter( std::string const& name, ParamType const& type, bool isInputParam = true );

  std::string const Name() const;
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
  const std::string _name;
  const ParamType _type;
  const bool _isInputParam;

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
