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

#ifndef DSPSIGNALBUS_H
#define DSPSIGNALBUS_H

//-------------------------------------------------------------------------------------------------

#include "DspSignal.h"

//=================================================================================================
/// DspSignal container

/**
A DspSignalBus contains DspSignals (see DspSignal). Via the Process_() method, a DspComponent
receives signals into it's "inputs" DspSignalBus and provides signals to it's "outputs"
DspSignalBus. Although DspSignals can be acquired from a DspSignalBus, the DspSignalBus class
provides public getters and setters for manipulating it's internal DspSignal values directly,
abstracting the need to retrieve and interface with the contained DspSignal objects. Likewise,
signals can be added and removed from a DspSignalBus via public methods.
*/

class DLLEXPORT DspSignalBus
{
public:
  virtual ~DspSignalBus();

  bool AddSignal( std::string signalName = "" );

  bool SetSignal( unsigned short signalIndex, const DspSignal* newSignal );
  bool SetSignal( std::string signalName, const DspSignal* newSignal );

  DspSignal* GetSignal( unsigned short signalIndex );
  DspSignal* GetSignal( std::string signalName );

  bool FindSignal( std::string signalName, unsigned short& returnIndex ) const;
  bool FindSignal( unsigned short signalIndex, unsigned short& returnIndex ) const;

  unsigned short GetSignalCount() const;

  void RemoveAllSignals();

  template< class ValueType >
  bool SetValue( unsigned short signalIndex, const ValueType& newValue );

  template< class ValueType >
  bool SetValue( std::string signalName, const ValueType& newValue );

  template< class ValueType >
  bool GetValue( unsigned short signalIndex, ValueType& returnValue ) const;

  template< class ValueType >
  bool GetValue( std::string signalName, ValueType& returnValue ) const;

  void ClearValue( unsigned short signalIndex );
  void ClearValue( std::string signalName );

  void ClearAllValues();

private:
  std::vector< DspSignal > _signals;
};

//=================================================================================================

template< class ValueType >
bool DspSignalBus::SetValue( unsigned short signalIndex, const ValueType& newValue )
{
  if( signalIndex < _signals.size() )
  {
    return _signals[signalIndex].SetValue( newValue );
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------------------------------------

template< class ValueType >
bool DspSignalBus::SetValue( std::string signalName, const ValueType& newValue )
{
  unsigned short signalIndex;

  if( FindSignal( signalName, signalIndex ) )
  {
    return _signals[signalIndex].SetValue( newValue );
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------------------------------------

template< class ValueType >
bool DspSignalBus::GetValue( unsigned short signalIndex, ValueType& returnValue ) const
{
  if( signalIndex < _signals.size() )
  {
    return _signals[signalIndex].GetValue( returnValue );
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------------------------------------

template< class ValueType >
bool DspSignalBus::GetValue( std::string signalName, ValueType& returnValue ) const
{
  unsigned short signalIndex;

  if( FindSignal( signalName, signalIndex ) )
  {
    return _signals[signalIndex].GetValue( returnValue );
  }
  else
  {
    return false;
  }
}

//=================================================================================================

#endif // DSPSIGNALBUS_H
