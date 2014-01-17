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

#include "DspCircuitThread.h"
#include "DspComponent.h"

//=================================================================================================

DspCircuitThread::DspCircuitThread()
: _components( NULL ),
  _threadNo( 0 ),
  _stop( false ),
  _stopped( true ),
  _gotResume( false ),
  _gotSync( false ) {}

//-------------------------------------------------------------------------------------------------

DspCircuitThread::~DspCircuitThread()
{
  Stop();
}

//=================================================================================================

void DspCircuitThread::Initialise(  std::vector< DspComponent* >* components, unsigned short threadNo )
{
  _components = components;
  _threadNo = threadNo;
}

//-------------------------------------------------------------------------------------------------

void DspCircuitThread::Start( Priority priority )
{
  if( _stopped )
  {
    _stop = false;
    _stopped = false;
    _gotResume = false;
    _gotSync = true;
    DspThread::Start( priority );
  }
}

//-------------------------------------------------------------------------------------------------

void DspCircuitThread::Stop()
{
  _stop = true;

  while( _stopped != true )
  {
    _syncCondt.WakeAll();
    _resumeCondt.WakeAll();
    MsSleep( 1 );
  }

  DspThread::Stop();
}

//-------------------------------------------------------------------------------------------------

void DspCircuitThread::Sync()
{
  _resumeMutex.Lock();

  if( !_gotSync ) // if haven't already got sync
  {
    _syncCondt.Wait( _resumeMutex ); // wait for sync
  }

  _resumeMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspCircuitThread::Resume()
{
  _resumeMutex.Lock();

  _gotSync = false; // reset the sync flag

  _gotResume = true; // set the resume flag
  _resumeCondt.WakeAll();

  _resumeMutex.Unlock();
}

//=================================================================================================

void DspCircuitThread::_Run()
{
  if( _components != NULL )
  {
    while( !_stop )
    {
      _resumeMutex.Lock();

      _gotSync = true; // set the sync flag

      _syncCondt.WakeAll();

      if( !_gotResume ) // if haven't already got resume
      {
        _resumeCondt.Wait( _resumeMutex ); // wait for resume
      }
      _gotResume = false; // reset the resume flag

      _resumeMutex.Unlock();

      if( !_stop )
      {
        for( unsigned short i = 0; i < _components->size(); i++ )
        {
          ( *_components )[i]->_ThreadTick( _threadNo );
        }
        for( unsigned short i = 0; i < _components->size(); i++ )
        {
          ( *_components )[i]->_ThreadReset( _threadNo );
        }
      }
    }
  }

  _stopped = true;
}

//=================================================================================================
