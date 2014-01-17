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

#ifndef DSPSIGNAL_H
#define DSPSIGNAL_H

//-------------------------------------------------------------------------------------------------

#include <string>
#include <vector>

#include "DspRunType.h"
#include "DspThread.h"

//=================================================================================================
/// Value container used to carry data between components

/**
DspComponents process and transfer data between each other in the form of "signals" via
interconnecting wires. The DspSignal class holds a single value that can be dynamically typed at
runtime. Furthermore, a DspSignal has the ability to change it's data type at any point during
program execution. This is designed such that a signal bus can hold any number of different typed
variables, as well as to allow for a variable to dynamically change it's type when needed -this can
be useful for inputs that accept a number of different data types (E.g. Varying sample size in an
audio buffer: array of byte / int / float).
*/

class DLLEXPORT DspSignal
{
public:
  DspSignal( std::string signalName = "" );

  virtual ~DspSignal();

  template< class ValueType >
  bool SetValue( const ValueType& newValue );

  template< class ValueType >
  bool GetValue( ValueType& returnValue ) const;

  bool SetSignal( const DspSignal* newSignal );

  void ClearValue();

  const std::type_info& GetSignalType() const;

  std::string GetSignalName() const;

private:
  DspRunType _signalValue;
  std::string _signalName;
  bool _valueAvailable;
};

//=================================================================================================

template< class ValueType >
bool DspSignal::SetValue( const ValueType& newValue )
{
  _signalValue = newValue;
  _valueAvailable = true;
  return true;
}

//-------------------------------------------------------------------------------------------------

template< class ValueType >
bool DspSignal::GetValue( ValueType& returnValue ) const
{
  if( _valueAvailable )
  {
    const ValueType* returnValuePtr = DspRunType::RunTypeCast< ValueType >( &_signalValue );
    if( returnValuePtr != NULL )
    {
      returnValue = *returnValuePtr;
      return true;
    }
    else
    {
      return false; // incorrect type matching
    }
  }
  else
  {
    return false; // no value available
  }
}

//=================================================================================================

#endif // DSPSIGNAL_H
