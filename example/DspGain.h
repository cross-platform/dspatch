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

#ifndef DSPGAIN_H
#define DSPGAIN_H

#include "../include/DSPatch.h"

//=================================================================================================

class DspGain : public DspComponent
{
public:
  DspGain()
  : _gain( 1.0 )
  {
    AddInput_();
    AddOutput_();
  }
  ~DspGain() {}

  void SetGain( float gain )
  {
    if( gain < 0.0 )
    {
      _gain = 0;
    }
    else
    {
      _gain = gain;
    }
  }

  float GetGain() const
  {
    return _gain;
  }

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs )
  {
    if( !inputs.GetValue( 0, _stream ) )
    {
      _stream.assign( _stream.size(), 0 );
    }

    for( unsigned long i = 0; i < _stream.size(); i++ )
    {
      _stream[i] *= _gain;
    }

    outputs.SetValue( 0, _stream );
  }

private:
  std::vector< float > _stream;
  float _gain;
};

//=================================================================================================

#endif // DSPGAIN_H
