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

#include <DspAudioDevice.h>

#include <RtAudio.h>

#include <iostream>
#include <string.h>
#include <cstdlib>

//=================================================================================================

struct RtAudioMembers
{
  std::vector< RtAudio::DeviceInfo > deviceList;

  RtAudio audioStream;
  RtAudio::StreamParameters outputParams;
  RtAudio::StreamParameters inputParams;
};

//=================================================================================================

DspAudioDevice::DspAudioDevice()
: _rtAudio( new RtAudioMembers() ),

  _bufferSize( 512 ),
  _sampleRate( 44100 ),

  _deviceCount( 0 ),

  _gotWaitReady( false ),
  _gotSyncReady( true ),

  _streamStop( false ),
  _isStreaming( false ),

  _currentDevice( 0 )
{
  _outputChannels.resize( 20 );
  for( unsigned short i = 0; i < 20; i++ )
  {
    AddInput_();
  }

  AddInput_( "Sample Rate" );

  _inputChannels.resize( 20 );
  for( unsigned short i = 0; i < 20; i++ )
  {
    AddOutput_();
  }

  _deviceCount = _rtAudio->audioStream.getDeviceCount();
  _rtAudio->deviceList.resize( _deviceCount );

  for( short i = 0; i < _deviceCount; i++ )
  {
    _rtAudio->deviceList[i] = _rtAudio->audioStream.getDeviceInfo( i );
  }

  SetDevice( _rtAudio->audioStream.getDefaultOutputDevice() );

  SetBufferSize( _bufferSize );
  SetSampleRate( _sampleRate );
}

//-------------------------------------------------------------------------------------------------

DspAudioDevice::~DspAudioDevice()
{
  _StopStream();

  delete _rtAudio;
}

//-------------------------------------------------------------------------------------------------

bool DspAudioDevice::SetDevice( short deviceIndex )
{
  if( deviceIndex >= 0 && deviceIndex < ( _deviceCount ) )
  {
    _currentDevice = deviceIndex;

    _StopStream();

    _rtAudio->inputParams.nChannels = _rtAudio->deviceList[deviceIndex].inputChannels;
    _rtAudio->inputParams.deviceId = deviceIndex;

    _rtAudio->outputParams.nChannels = _rtAudio->deviceList[deviceIndex].outputChannels;
    _rtAudio->outputParams.deviceId = deviceIndex;

    _StartStream();

    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------------------

std::string DspAudioDevice::GetDeviceName( short deviceIndex )
{
  if( deviceIndex >= 0 && deviceIndex < ( _deviceCount ) )
  {
    return _rtAudio->deviceList[deviceIndex].name;
  }

  return "";
}

//-------------------------------------------------------------------------------------------------

unsigned short DspAudioDevice::GetDeviceInputCount( short deviceIndex )
{
  return _rtAudio->deviceList[deviceIndex].inputChannels;
}

//-------------------------------------------------------------------------------------------------

unsigned short DspAudioDevice::GetDeviceOutputCount( short deviceIndex )
{
  return _rtAudio->deviceList[deviceIndex].outputChannels;
}

//-------------------------------------------------------------------------------------------------

unsigned short DspAudioDevice::GetCurrentDevice()
{
  return _currentDevice;
}

//-------------------------------------------------------------------------------------------------

unsigned short DspAudioDevice::GetDeviceCount()
{
  return _deviceCount;
}

//-------------------------------------------------------------------------------------------------

bool DspAudioDevice::IsStreaming()
{
  return _isStreaming;
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::SetBufferSize( unsigned long bufferSize )
{
  _StopStream();

  _bufferSize = bufferSize;

  for( unsigned short i = 0; i < _inputChannels.size(); i++ )
  {
    _inputChannels[i].resize( _bufferSize );
  }

  _StartStream();
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::SetSampleRate( unsigned long sampleRate )
{
  _StopStream();

  _sampleRate = sampleRate;

  _StartStream();
}

//-------------------------------------------------------------------------------------------------

unsigned long DspAudioDevice::GetSampleRate()
{
  return _sampleRate;
}

//=================================================================================================

void DspAudioDevice::Process_( DspSignalBus& inputs, DspSignalBus& outputs )
{
  // Wait until the sound card is ready for the next set of buffers
  // ==============================================================
  _syncMutex.Lock();
  if( !_gotSyncReady )              // if haven't already got the release
    _syncCondt.Wait( _syncMutex );  // wait for sync
  _gotSyncReady = false;            // reset the release flag
  _syncMutex.Unlock();

  // Synchronise sample rate with the "Sample Rate" input feed
  // =========================================================
  unsigned long sampleRate;
  if( inputs.GetValue( "Sample Rate", sampleRate ) )
  {
    if( sampleRate != GetSampleRate() )
    {
      SetSampleRate( sampleRate );
    }
  }

  // Synchronise buffer size with the size of incoming buffers
  // =========================================================
  if( inputs.GetValue( 0, _outputChannels[0] ) )
  {
    if( _bufferSize != _outputChannels[0].size() &&
        _outputChannels[0].size() != 0 )
    {
      SetBufferSize( _outputChannels[0].size() );
    }
  }

  // Retrieve incoming component buffers for the sound card to output
  // ================================================================
  for( unsigned short i = 0; i < _outputChannels.size(); i++ )
  {
    if( !inputs.GetValue( i, _outputChannels[i] ) )
    {
      _outputChannels[i].assign( _outputChannels[i].size(), 0 );
    }
  }

  // Retrieve incoming sound card buffers for the component to output
  // ================================================================
  for( unsigned short i = 0; i < _inputChannels.size(); i++ )
  {
    outputs.SetValue( i, _inputChannels[i] );
  }

  // Inform the sound card that buffers are now ready
  // ================================================
  _buffersMutex.Lock();
  _gotWaitReady = true; // set release flag
  _waitCondt.WakeAll(); // release sync
  _buffersMutex.Unlock();
}

//=================================================================================================

void DspAudioDevice::_WaitForBuffer()
{
  _buffersMutex.Lock();
  if( !_gotWaitReady )										//if haven't already got the release
    _waitCondt.Wait( _buffersMutex );		  //wait for sync
  _gotWaitReady = false;									//reset the release flag
  _buffersMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::_SyncBuffer()
{
  _syncMutex.Lock();
  _gotSyncReady = true;								//set release flag
  _syncCondt.WakeAll();								//release sync
  _syncMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::_StopStream()
{
  _streamStop = true;

  for( unsigned short i = 0; i < 250; i++ )
  {
    _waitCondt.WakeAll();

    if( _isStreaming )
    {
      DspThread::MsSleep( 1 );
    }
    else
    {
      break;
    }
  }

  if( _rtAudio->audioStream.isStreamRunning() )
  {
    _rtAudio->audioStream.stopStream();
  }

  if( _rtAudio->audioStream.isStreamOpen() )
  {
    _rtAudio->audioStream.closeStream();
  }

  _syncCondt.WakeAll();
}

//-------------------------------------------------------------------------------------------------

void DspAudioDevice::_StartStream()
{
  RtAudio::StreamParameters* inputParams = NULL;
  RtAudio::StreamParameters* outputParams = NULL;

  if( _rtAudio->inputParams.nChannels != 0 )
  {
    inputParams = &_rtAudio->inputParams;
  }

  if( _rtAudio->outputParams.nChannels != 0 )
  {
    outputParams = &_rtAudio->outputParams;
  }

  RtAudio::StreamOptions options;
  options.flags |= RTAUDIO_SCHEDULE_REALTIME;
  options.flags |= RTAUDIO_NONINTERLEAVED;

  _rtAudio->audioStream.openStream( outputParams,
                                    inputParams,
                                    RTAUDIO_FLOAT32,
                                    _sampleRate,
                                    ( unsigned int* ) &_bufferSize,
                                    &_StaticCallback,
                                    this,
                                    &options );

  _rtAudio->audioStream.startStream();

  _streamStop = false;
  _isStreaming = true;
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::_StaticCallback( void* outputBuffer,
                                     void* inputBuffer,
                                     unsigned int nBufferFrames,
                                     double streamTime,
                                     unsigned int status,
                                     void* userData )
{
  return ( ( DspAudioDevice* ) userData )->_DynamicCallback( inputBuffer, outputBuffer );
}

//-------------------------------------------------------------------------------------------------

int DspAudioDevice::_DynamicCallback( void* inputBuffer, void* outputBuffer )
{
  _WaitForBuffer();

  if( !_streamStop )
  {
    float* floatOutput = ( float* ) outputBuffer;
    float* floatInput = ( float* ) inputBuffer;

    if( outputBuffer != NULL )
    {
      for( unsigned long i = 0; i < _outputChannels.size(); i++ )
      {
        if( _rtAudio->deviceList[_currentDevice].outputChannels >= ( i + 1 ) )
        {
          for( unsigned long j = 0; j < _outputChannels[i].size(); j++ )
          {
            *floatOutput++ = _outputChannels[i][j];
          }
        }
      }
    }

    if( inputBuffer != NULL )
    {
      for( unsigned long i = 0; i < _inputChannels.size(); i++ )
      {
        if( _rtAudio->deviceList[_currentDevice].inputChannels >= ( i + 1 ) )
        {
          for( unsigned long j = 0; j < _inputChannels[i].size(); j++ )
          {
            _inputChannels[i][j] = *floatInput++;
          }
        }
      }
    }
  }
  else
  {
    _isStreaming = false;
  }

  _SyncBuffer();

  return 0;
}

//=================================================================================================
