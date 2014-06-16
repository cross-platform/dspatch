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

#ifndef DSPWAVESTREAMER_H
#define DSPWAVESTREAMER_H

#include <DSPatch.h>

//=================================================================================================

class DspWaveStreamer : public DspComponent
{
public:
  static std::string const pFilePath; // FilePath
  static std::string const pPlay; // Trigger
  static std::string const pPause; // Trigger
  static std::string const pStop; // Trigger
  static std::string const pIsPlaying; // Bool

  DspWaveStreamer();
  ~DspWaveStreamer();

  bool LoadFile( char const* filePath );
  void Play();
  void Pause();
  void Stop();

  bool IsPlaying() const;

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs );
  virtual bool ParameterUpdating_( std::string const& name, DspParameter const& param );

private:
  struct WaveFormat
  {
    void Clear()
    {
      format = 0;
      channelCount = 0;
      sampleRate = 0;
      byteRate = 0;
      frameSize = 0;
      bitDepth = 0;
      extraDataSize = 0;
    }

    unsigned short format;        // Integer identifier of the format
    unsigned short channelCount;  // Number of audio channels
    unsigned long sampleRate;     // Audio sample rate
    unsigned long byteRate;       // Bytes per second (possibly approximate)
    unsigned short frameSize;     // Size in bytes of a sample block (all channels)
    unsigned short bitDepth;      // Size in bits of a single per-channel sample
    unsigned short extraDataSize; // Bytes of extra data appended to this struct
  };

  WaveFormat _waveFormat;
  std::vector< short > _waveData;
  unsigned long _bufferSize;
  unsigned long _sampleIndex;
  float _shortToFloatCoeff;
  DspMutex _busyMutex;

  std::vector< float > _leftChannel;
  std::vector< float > _rightChannel;
};

//=================================================================================================

#endif // DSPWAVESTREAMER_H
