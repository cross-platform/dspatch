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

#ifndef DSPGAIN_H
#define DSPGAIN_H

#include <DSPatch.h>

//=================================================================================================

class DspGain : public DspComponent
{
public:
  static std::string const pGain; // Float

  DspGain()
  {
    AddInput_();
    AddOutput_();

    AddParameter_( pGain, DspParameter( DspParameter::Float, 1, std::make_pair( 0, 2 ) ) );
  }
  ~DspGain() {}

  void SetGain( float gain )
  {
    SetParameter_( pGain, DspParameter( DspParameter::Float, gain ) );
  }

  float GetGain() const
  {
    return *GetParameter_( pGain )->GetFloat();
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
      _stream[i] *= GetGain();
    }

    outputs.SetValue( 0, _stream );
  }

  virtual bool ParameterUpdating_( std::string const& name, DspParameter const& param )
  {
    if( name == pGain )
    {
      SetGain( *param.GetFloat() );
      return true;
    }
    return false;
  }

private:
  std::vector< float > _stream;
};

std::string const DspGain::pGain = "gain";

//=================================================================================================

#endif // DSPGAIN_H
