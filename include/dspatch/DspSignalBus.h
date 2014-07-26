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

#ifndef DSPSIGNALBUS_H
#define DSPSIGNALBUS_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspSignal.h>

//=================================================================================================
/// DspSignal container

/**
A DspSignalBus contains DspSignals (see DspSignal). Via the Process_() method, a DspComponent
receives signals into it's "inputs" DspSignalBus and provides signals to it's "outputs"
DspSignalBus. Although DspSignals can be acquired from a DspSignalBus, the DspSignalBus class
provides public getters and setters for manipulating it's internal DspSignal values directly,
abstracting the need to retrieve and interface with the contained DspSignals themself.
*/

class DLLEXPORT DspSignalBus
{
public:
  virtual ~DspSignalBus();

  bool SetSignal( unsigned short signalIndex, DspSignal const* newSignal );
  bool SetSignal( std::string const& signalName, DspSignal const* newSignal );

  DspSignal* GetSignal( unsigned short signalIndex );
  DspSignal* GetSignal( std::string const& signalName );

  bool FindSignal( std::string const& signalName, unsigned short& returnIndex ) const;
  bool FindSignal( unsigned short signalIndex, unsigned short& returnIndex ) const;

  unsigned short GetSignalCount() const;

  template< class ValueType >
  bool SetValue( unsigned short signalIndex, ValueType const& newValue );

  template< class ValueType >
  bool SetValue( std::string const& signalName, ValueType const& newValue );

  template< class ValueType >
  bool GetValue( unsigned short signalIndex, ValueType& returnValue ) const;

  template< class ValueType >
  bool GetValue( std::string const& signalName, ValueType& returnValue ) const;

  template< class ValueType >
  ValueType const* GetValue( unsigned short signalIndex ) const;

  template< class ValueType >
  ValueType const* GetValue( std::string const& signalName ) const;

  void ClearValue( unsigned short signalIndex );
  void ClearValue( std::string const& signalName );

  void ClearAllValues();

private:
  friend class DspComponent;

  bool _AddSignal( std::string const& signalName = "" );

  bool _RemoveSignal();
  void _RemoveAllSignals();

private:
  std::vector< DspSignal > _signals;
};

//=================================================================================================

template< class ValueType >
bool DspSignalBus::SetValue( unsigned short signalIndex, ValueType const& newValue )
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
bool DspSignalBus::SetValue( std::string const& signalName, ValueType const& newValue )
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
bool DspSignalBus::GetValue( std::string const& signalName, ValueType& returnValue ) const
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

//-------------------------------------------------------------------------------------------------

template< class ValueType >
ValueType const* DspSignalBus::GetValue( unsigned short signalIndex ) const
{
    if( signalIndex < _signals.size() )
    {
      return _signals[signalIndex].GetValue< ValueType >();
    }
    else
    {
      return NULL;
    }
}

//-------------------------------------------------------------------------------------------------

template< class ValueType >
ValueType const* DspSignalBus::GetValue( std::string const& signalName ) const
{
    unsigned short signalIndex;

    if( FindSignal( signalName, signalIndex ) )
    {
      return _signals[signalIndex].GetValue< ValueType >();
    }
    else
    {
      return NULL;
    }
}

//=================================================================================================

#endif // DSPSIGNALBUS_H
