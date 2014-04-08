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

#include <dspatch/DspSignal.h>

//=================================================================================================

DspSignal::DspSignal( std::string signalName )
: _signalName( signalName ),
  _valueAvailable( false ) {}

//-------------------------------------------------------------------------------------------------

DspSignal::~DspSignal() {}

//=================================================================================================

bool DspSignal::SetSignal( DspSignal const* newSignal )
{
  if( newSignal != NULL )
  {
    if( newSignal->_valueAvailable == false )
    {
      return false;
    }
    else
    {
      _signalValue.CopyFrom( newSignal->_signalValue );
      _valueAvailable = true;
      return true;
    }
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------------------------------------

void DspSignal::ClearValue()
{
  _valueAvailable = false;
}

//-------------------------------------------------------------------------------------------------

const std::type_info& DspSignal::GetSignalType() const
{
  return _signalValue.GetType();
}

//-------------------------------------------------------------------------------------------------

std::string DspSignal::GetSignalName() const
{
  return _signalName;
}

//=================================================================================================
