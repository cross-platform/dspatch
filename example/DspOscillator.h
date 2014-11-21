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

//=================================================================================================

class DspOscillator : public DspComponent
{
public:
  static std::string const pBufferSize; // Int
  static std::string const pSampleRate; // Int
  static std::string const pAmplitude; // Float
  static std::string const pFrequency; // Float

  DspOscillator( float startFreq = 1000.0, float startAmpl = 1.0 );
  ~DspOscillator();

  void SetBufferSize( int bufferSize );
  void SetSampleRate( int sampleRate );
  void SetAmpl( float ampl );
  void SetFreq( float freq );

  int GetBufferSize() const;
  int GetSampleRate() const;
  float GetAmpl() const;
  float GetFreq() const;

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs );
  virtual bool ParameterUpdating_( std::string const& name, DspParameter const& param );

private:
  std::vector< float > _signalLookup;
  std::vector< float > _signal;

  unsigned long _lastPos;
  unsigned long _lookupLength;

  DspMutex _processMutex;

  void _BuildLookup();
};

//=================================================================================================

#endif /* DSPOSCILLATOR_H */
