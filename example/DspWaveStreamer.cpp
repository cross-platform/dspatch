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

#include <DspWaveStreamer.h>

#include <fstream>
#include <iostream>
#include <string.h>

//=================================================================================================

std::string const DspWaveStreamer::pFilePath = "filePath";
std::string const DspWaveStreamer::pPlay = "play";
std::string const DspWaveStreamer::pPause = "pause";
std::string const DspWaveStreamer::pStop = "stop";
std::string const DspWaveStreamer::pIsPlaying = "isPlaying";

//=================================================================================================

DspWaveStreamer::DspWaveStreamer()
: _waveData( 0 ),
  _bufferSize( 256 ),
  _sampleIndex( 0 ),
  _shortToFloatCoeff( 1.0f / 32767.0f )
{
  _waveFormat.Clear();

  _leftChannel.resize( _bufferSize );
  _rightChannel.resize( _bufferSize );

  AddOutput_();
  AddOutput_();
  AddOutput_( "Sample Rate" );

  AddParameter_( pFilePath, DspParameter( DspParameter::FilePath, "" ) );
  AddParameter_( pPlay, DspParameter( DspParameter::Trigger ) );
  AddParameter_( pPause, DspParameter( DspParameter::Trigger ) );
  AddParameter_( pStop, DspParameter( DspParameter::Trigger ) );
  AddParameter_( pIsPlaying, DspParameter( DspParameter::Bool, false ) );
}

//-------------------------------------------------------------------------------------------------

DspWaveStreamer::~DspWaveStreamer() {}

//=================================================================================================

bool DspWaveStreamer::LoadFile( char const* filePath )
{
  bool wasPlaying = IsPlaying();
  Stop();

  if( filePath == NULL )
    return false;

  std::ifstream inFile( filePath, std::ios::binary | std::ios::in );
  if( inFile.bad() )
    return false;

  unsigned long dwFileSize = 0, dwChunkSize = 0;
  char dwChunkId[5];
  char dwExtra[5];

  dwChunkId[4] = 0;
  dwExtra[4] = 0;

  //look for 'RIFF' chunk identifier
  inFile.seekg( 0, std::ios::beg );
  inFile.read( dwChunkId, 4 );
  if( strcmp( dwChunkId, "RIFF" ) )
  {
    std::cerr << "'" << filePath << "' not found.\n";
    inFile.close();
    return false;
  }
  inFile.seekg( 4, std::ios::beg ); //get file size
  inFile.read( reinterpret_cast<char*>( &dwFileSize ), 4 );
  if( dwFileSize <= 16 )
  {
    inFile.close();
    return false;
  }
  inFile.seekg( 8, std::ios::beg ); //get file format
  inFile.read( dwExtra, 4 );
  if( strcmp( dwExtra, "WAVE" ) )
  {
    inFile.close();
    return false;
  }

  //look for 'fmt ' chunk id
  bool bFilledFormat = false;
  for( unsigned long i = 12; i < dwFileSize; )
  {
    inFile.seekg( i, std::ios::beg );
    inFile.read( dwChunkId, 4 );
    inFile.seekg( i + 4, std::ios::beg );
    inFile.read( reinterpret_cast<char*>( &dwChunkSize ), 4 );
    if( !strcmp( dwChunkId, "fmt " ) )
    {
      inFile.seekg( i + 8, std::ios::beg );

      _waveFormat.Clear();
      inFile.read( reinterpret_cast<char*>( &_waveFormat.format ), 2 );
      inFile.read( reinterpret_cast<char*>( &_waveFormat.channelCount ), 2 );
      inFile.read( reinterpret_cast<char*>( &_waveFormat.sampleRate ), 4 );
      inFile.read( reinterpret_cast<char*>( &_waveFormat.byteRate ), 4 );
      inFile.read( reinterpret_cast<char*>( &_waveFormat.frameSize ), 2 );
      inFile.read( reinterpret_cast<char*>( &_waveFormat.bitDepth ), 2 );
      inFile.read( reinterpret_cast<char*>( &_waveFormat.extraDataSize ), 2 );

      bFilledFormat = true;
      break;
    }
    dwChunkSize += 8; //add offsets of the chunk id, and chunk size data entries
    dwChunkSize += 1;
    dwChunkSize &= 0xfffffffe; //guarantees WORD padding alignment
    i += dwChunkSize;
  }
  if( !bFilledFormat )
  {
    inFile.close();
    return false;
  }

  //look for 'data' chunk id
  bool bFilledData = false;
  for( unsigned long i = 12; i < dwFileSize; )
  {
    inFile.seekg( i, std::ios::beg );
    inFile.read( dwChunkId, 4 );
    inFile.seekg( i + 4, std::ios::beg );
    inFile.read( reinterpret_cast<char*>( &dwChunkSize ), 4 );
    if( !strcmp( dwChunkId, "data" ) )
    {
      _waveData.resize( dwChunkSize / 2 );
      inFile.seekg( i + 8, std::ios::beg );
      inFile.read( reinterpret_cast<char*>( &_waveData[0] ), dwChunkSize );
      bFilledData = true;
      break;
    }
    dwChunkSize += 8; //add offsets of the chunk id, and chunk size data entries
    dwChunkSize += 1;
    dwChunkSize &= 0xfffffffe; //guarantees WORD padding alignment
    i += dwChunkSize;
  }
  if( !bFilledData )
  {
    inFile.close();
    return false;
  }

  inFile.close();

  if( wasPlaying )
  {
    Play();
  }

  SetParameter_( pFilePath, DspParameter( DspParameter::FilePath, filePath ) );
  return true;
}

//-------------------------------------------------------------------------------------------------

void DspWaveStreamer::Play()
{
  SetParameter_( pIsPlaying, DspParameter( DspParameter::Bool, true ) );
}

//-------------------------------------------------------------------------------------------------

void DspWaveStreamer::Pause()
{
  SetParameter_( pIsPlaying, DspParameter( DspParameter::Bool, false ) );
}

//-------------------------------------------------------------------------------------------------

void DspWaveStreamer::Stop()
{
  _busyMutex.Lock();

  _sampleIndex = 0;
  SetParameter_( pIsPlaying, DspParameter( DspParameter::Bool, false ) );

  _busyMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

bool DspWaveStreamer::IsPlaying() const
{
  return *GetParameter_( pIsPlaying )->GetBool();
}

//=================================================================================================

void DspWaveStreamer::Process_( DspSignalBus& inputs, DspSignalBus& outputs )
{
  if( IsPlaying() && _waveData.size() > 0 )
  {
    _busyMutex.Lock();

    unsigned short index = 0;
    for( unsigned short i = 0; i < _bufferSize * 2; i += 2 )
    {
      _leftChannel[index++] = ( float ) _waveData[_sampleIndex + i] * _shortToFloatCoeff;
    }

    index = 0;
    for( unsigned short i = 1; i < _bufferSize * 2; i += 2 )
    {
      _rightChannel[index++] = ( float ) _waveData[_sampleIndex + i] * _shortToFloatCoeff;
    }

    _sampleIndex += _bufferSize * 2;
    _sampleIndex %= _waveData.size() - _bufferSize * 2;

    _busyMutex.Unlock();

    outputs.SetValue( 0, _leftChannel );
    outputs.SetValue( 1, _rightChannel );
    outputs.SetValue( "Sample Rate", _waveFormat.sampleRate );
  }
  else
  {
    outputs.ClearValue( 0 );
    outputs.ClearValue( 1 );
  }
}

//-------------------------------------------------------------------------------------------------

bool DspWaveStreamer::ParameterUpdating_( std::string const& name, DspParameter const& param )
{
  if( name == pFilePath )
  {
    return LoadFile( param.GetString()->c_str() );
  }
  else if( name == pPlay )
  {
    Play();
    return true;
  }
  else if( name == pPause )
  {
    Pause();
    return true;
  }
  else if( name == pStop )
  {
    Stop();
    return true;
  }

  return false;
}

//=================================================================================================
