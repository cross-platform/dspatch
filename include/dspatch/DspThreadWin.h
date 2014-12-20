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

#ifndef DSPTHREADWIN_H
#define DSPTHREADWIN_H

//-------------------------------------------------------------------------------------------------

#include <windows.h>

//=================================================================================================

class DspThread
{
public:
  DspThread()
  : _threadHandle( NULL ) {}

  DspThread( DspThread const& )
  : _threadHandle( NULL ) {}

  virtual ~DspThread()
  {
    Stop();
  }

  enum Priority
  {
    IdlePriority = -15,

    LowestPriority = -2,
    LowPriority = -1,
    NormalPriority = 0,
    HighPriority = 1,
    HighestPriority = 2,

    TimeCriticalPriority = 15
  };

  virtual void Start( Priority priority = NormalPriority )
  {
    DWORD threadId;
    _threadHandle = CreateThread( NULL, 0, _ThreadFunc, this, CREATE_SUSPENDED, &threadId );
    SetThreadPriority( _threadHandle, priority );
    ResumeThread( _threadHandle );
  }

  virtual void Stop()
  {
    CloseHandle( _threadHandle );
    _threadHandle = NULL;
  }

  static void SetPriority( Priority priority )
  {
    SetThreadPriority( GetCurrentThread(), priority );
  }

  static void MsSleep( unsigned short milliseconds )
  {
    Sleep( milliseconds );
  }

private:
  static DWORD WINAPI _ThreadFunc( LPVOID pv )
  {
    ( reinterpret_cast<DspThread*>( pv ) )->_Run();
    return 0;
  }

  virtual void _Run() = 0;

private:
  HANDLE _threadHandle;
};

//=================================================================================================

class DspMutex
{
public:
  DspMutex()
  {
    InitializeCriticalSection( &_cs );
  }

  DspMutex( DspMutex const& )
  {
    InitializeCriticalSection( &_cs );
  }

  virtual ~DspMutex()
  {
    DeleteCriticalSection( &_cs );
  }

  void Lock()
  {
    EnterCriticalSection( &_cs );
  }

  void Unlock()
  {
    LeaveCriticalSection( &_cs );
  }

private:
  CRITICAL_SECTION _cs;
};

//=================================================================================================

class DspWaitCondition
{
public:
  DspWaitCondition()
  {
    _hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
  }

  DspWaitCondition( DspWaitCondition const& )
  {
    _hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
  }

  virtual ~DspWaitCondition()
  {
    CloseHandle( _hEvent );
  }

  void Wait( DspMutex& mutex )
  {
    ResetEvent( _hEvent );

    mutex.Unlock();

    WaitForSingleObject( _hEvent, INFINITE );

    mutex.Lock();
  }

  void WakeAll()
  {
    SetEvent( _hEvent );
  }

private:
  HANDLE _hEvent;
};

//=================================================================================================

#endif // DSPTHREADWIN_H
