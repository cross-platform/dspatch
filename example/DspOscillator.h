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

#ifndef DSPOSCILLATOR_H
#define DSPOSCILLATOR_H

#include "../include/DSPatch.h"

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

#endif /* DSPOSCILLATOR_H */
