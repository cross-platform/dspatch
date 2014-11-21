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

#ifndef DSPTHREADUNIX_H
#define DSPTHREADUNIX_H

//-------------------------------------------------------------------------------------------------

#include <pthread.h>
#include <unistd.h>

//=================================================================================================

class DspThread
{
public:
  DspThread()
  : _threadAttatched( false ) {}

  virtual ~DspThread()
  {
    Stop();
  }

  enum Priority
  {
    IdlePriority,

    LowestPriority,
    LowPriority,
    NormalPriority,
    HighPriority,
    HighestPriority,

    TimeCriticalPriority
  };

  virtual void Start( Priority priority = NormalPriority )
  {
    pthread_create( &_thread, NULL, _ThreadFunc, this );
    _threadAttatched = true;

    _SetPriority( _thread, priority );
  }

  virtual void Stop()
  {
    if( _threadAttatched )
    {
      pthread_detach( _thread );
      _threadAttatched = false;
    }
  }

  static void SetPriority( Priority priority )
  {
    _SetPriority( pthread_self(), priority );
  }

  static void MsSleep( unsigned short milliseconds )
  {
    usleep( ( unsigned int ) milliseconds );
  }

private:
  static void* _ThreadFunc( void* pv )
  {
    ( reinterpret_cast<DspThread*>( pv ) )->_Run();
    return NULL;
  }

  virtual void _Run() = 0;

  static void _SetPriority( pthread_t threadID, Priority priority )
  {
    int policy;
    struct sched_param param;

    pthread_getschedparam( threadID, &policy, &param );

    policy = SCHED_FIFO;
    param.sched_priority = ( ( priority - IdlePriority ) * ( 99 - 1 ) / TimeCriticalPriority ) + 1;

    pthread_setschedparam( threadID, policy, &param );
  }

private:
  pthread_t _thread;
  bool _threadAttatched;
};

//=================================================================================================

class DspMutex
{
public:
  DspMutex()
  {
    pthread_mutex_init( &_mutex, NULL );
  }

  virtual ~DspMutex()
  {
    pthread_mutex_destroy( &_mutex );
  }

  void Lock()
  {
    pthread_mutex_lock( &_mutex );
  }

  void Unlock()
  {
    pthread_mutex_unlock( &_mutex );
  }

private:
  friend class DspWaitCondition;

  pthread_mutex_t _mutex;
};

//=================================================================================================

class DspWaitCondition
{
public:
  DspWaitCondition()
  {
    pthread_cond_init( &_cond, NULL );
  }

  virtual ~DspWaitCondition()
  {
    pthread_cond_destroy( &_cond );
  }

  void Wait( DspMutex& mutex )
  {
    pthread_cond_wait( &_cond, &( mutex._mutex ) );
  }

  void WakeAll()
  {
    pthread_cond_broadcast( &_cond );
  }

private:
  pthread_cond_t _cond;
};

//=================================================================================================

#endif // DSPTHREADUNIX_H
