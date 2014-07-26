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

#ifndef DSPOSCILLATOR_H
#define DSPOSCILLATOR_H

#include <DSPatch.h>
#include <dspatch/DspPlugin.h>

#include <iostream>

//=================================================================================================

class DspOscillator : public DspComponent
{
public:
  DspOscillator( float startFreq = 1000.0, float startAmpl = 1.0 );
  ~DspOscillator();

  void SetBufferSize( unsigned long bufferSize );
  void SetSampleRate( unsigned long sampleRate );

  void SetAmpl( float ampl );
  void SetFreq( float freq );

  unsigned long GetBufferSize() const
  {
    return _bufferSize;
  }

  unsigned long GetSampleRate() const
  {
    return _sampleRate;
  }

  float GetAmpl() const
  {
    return _ampl;
  }

  float GetFreq() const
  {
    return _freq;
  }

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs );

private:
  float _freq;
  float _ampl;

  std::vector< float > _signalLookup;
  std::vector< float > _signal;

  unsigned long _lastPos;
  unsigned long _lookupLength;
  unsigned long _bufferSize;
  unsigned long _sampleRate;

  DspMutex _processMutex;

  void _BuildLookup();
};

//=================================================================================================

class DspOscillatorPlugin : public DspPlugin
{
  std::map< std::string, DspParameter > GetCreateParams() const
  {
    std::map< std::string, DspParameter > params;
    params.insert( std::make_pair( "startFreq", DspParameter( DspParameter::Float ) ) );
    params.insert( std::make_pair( "startAmpl", DspParameter( DspParameter::Float ) ) );
    return params;
  }

  DspComponent* Create( std::map< std::string, DspParameter > const& params ) const
  {
    float startFreq;
    float startAmpl;
    bool gotStartFreq = params.at( "startFreq" ).GetFloat( startFreq );
    bool gotStartAmpl = params.at( "startAmpl" ).GetFloat( startAmpl );

    if( !gotStartFreq && !gotStartAmpl)
    {
      return new DspOscillator();
    }
    else if( gotStartFreq && !gotStartAmpl)
    {
      return new DspOscillator( startFreq );
    }
    else if( gotStartFreq && gotStartAmpl)
    {
      return new DspOscillator( startFreq, startAmpl );
    }
  }
};

EXPORT_DSPPLUGIN( DspOscillatorPlugin )

//=================================================================================================

#endif /* DSPOSCILLATOR_H */
