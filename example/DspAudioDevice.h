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

#ifndef DSPAUDIODEVICE_H
#define DSPAUDIODEVICE_H

#include <DSPatch.h>

struct RtAudioMembers;

//-------------------------------------------------------------------------------------------------

class DspAudioDevice : public DspComponent
{
public:
  DspAudioDevice();
  ~DspAudioDevice();

  bool SetDevice( short deviceIndex );

  std::string GetDeviceName( short deviceIndex );
  unsigned short GetDeviceInputCount( short deviceIndex );
  unsigned short GetDeviceOutputCount( short deviceIndex );
  unsigned short GetCurrentDevice();
  unsigned short GetDeviceCount();

  bool IsStreaming();

  void SetBufferSize( unsigned long bufferSize );
  void SetSampleRate( unsigned long sampleRate );
  unsigned long GetSampleRate();

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs );

private:
  std::vector< std::vector< float > > _outputChannels;
  std::vector< std::vector< float > > _inputChannels;

  RtAudioMembers* _rtAudio;

  unsigned long _bufferSize;
  unsigned long _sampleRate;

  unsigned short _deviceCount;

  DspMutex _buffersMutex;
  DspMutex _syncMutex;
  DspWaitCondition _waitCondt;
  DspWaitCondition _syncCondt;
  bool _gotWaitReady;
  bool _gotSyncReady;

  bool _streamStop;
  bool _isStreaming;

  unsigned short _currentDevice;

  void _WaitForBuffer();
  void _SyncBuffer();

  void _StopStream();
  void _StartStream();

  static int _StaticCallback( void* outputBuffer,
                              void* inputBuffer,
                              unsigned int nBufferFrames,
                              double streamTime,
                              unsigned int status,
                              void* userData );

  int _DynamicCallback( void* inputBuffer, void* outputBuffer );
};

//-------------------------------------------------------------------------------------------------

#endif // DSPAUDIODEVICE_H
