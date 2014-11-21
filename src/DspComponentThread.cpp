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

#include <dspatch/DspComponentThread.h>
#include <dspatch/DspComponent.h>

//=================================================================================================

DspComponentThread::DspComponentThread()
: _component( NULL ),
  _stop( false ),
  _pause( false ),
  _stopped( true ) {}

//-------------------------------------------------------------------------------------------------

DspComponentThread::~DspComponentThread()
{
  Stop();
}

//=================================================================================================

void DspComponentThread::Initialise( DspComponent* component )
{
  _component = component;
}

//-------------------------------------------------------------------------------------------------

bool DspComponentThread::IsStopped() const
{
  return _stopped;
}

//-------------------------------------------------------------------------------------------------

void DspComponentThread::Start( Priority priority )
{
  if( _stopped )
  {
    _stop = false;
    _stopped = false;
    _pause = false;
    DspThread::Start( priority );
  }
}

//-------------------------------------------------------------------------------------------------

void DspComponentThread::Stop()
{
  _stop = true;

  while( _stopped != true )
  {
    _pauseCondt.WakeAll();
    _resumeCondt.WakeAll();
    MsSleep( 1 );
  }

  DspThread::Stop();
}

//-------------------------------------------------------------------------------------------------

void DspComponentThread::Pause()
{
  _resumeMutex.Lock();

  _pause = true;
  _pauseCondt.Wait( _resumeMutex ); // wait for resume
  _pause = false;

  _resumeMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspComponentThread::Resume()
{
  _resumeMutex.Lock();
  _resumeCondt.WakeAll();
  _resumeMutex.Unlock();
}

//=================================================================================================

void DspComponentThread::_Run()
{
  if( _component != NULL )
  {
    while( !_stop )
    {
      _component->Tick();
      _component->Reset();

      if( _pause )
      {
        _resumeMutex.Lock();

        _pauseCondt.WakeAll();

        _resumeCondt.Wait( _resumeMutex ); // wait for resume

        _resumeMutex.Unlock();
      }
    }
  }

  _stopped = true;
}

//=================================================================================================
