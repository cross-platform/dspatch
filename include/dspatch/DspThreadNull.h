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

#ifndef DSPTHREADNULL_H
#define DSPTHREADNULL_H

//=================================================================================================
/// Cross-platform, object-oriented thread

/**
An class that is required to run actions in a parallel thread can be derived from DspThread in
order to inherit multi-threading abilities. The Start() method initiates a parallel thread and
executes the private virtual _Run() method in that thread. The derived class must override this
_Run() method with one that executes the required parallel actions. Other threads may use the
static MsSleep() and SetPriority() methods without having to derive from, or create an instance of
DspThread. Priority for the created thread, or calling threads (via SetPriority()), may be selected
from the public enumeration: Priority.
*/

class DspThread
{
public:
  DspThread() {}

  virtual ~DspThread() {}

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

  virtual void Start( Priority priority ) {}
  virtual void Stop();
  static void SetPriority( Priority priority ) {}
  static void MsSleep( unsigned short milliseconds ) {}
};

//=================================================================================================
/// Cross-platform, object-oriented mutex

/**
DspMutex is a simple mutex that can lock a critical section of code for exclusive access by
the calling thread. Other threads attempting to acquire a lock while another has acquired it
will wait at the Lock() method call until the thread that owns the mutex calls Unlock().
*/

class DspMutex
{
public:
  DspMutex() {}

  virtual ~DspMutex() {}

  static void Lock() {}
  static void Unlock() {}
};

//=================================================================================================
/// Cross-platform, object-oriented conditional wait

/**
A wait condition works like an indefinite sleep. When a thread calls the Wait() function it
is put to sleep until it is woken by the WakeAll() function of the same DspWaitCondition object.
This is used to synchronize actions between threads.
*/

class DspWaitCondition
{
public:
  DspWaitCondition() {}

  virtual ~DspWaitCondition() {}

  static void Wait( DspMutex& mutex ) {}
  static void WakeAll() {}
};

//=================================================================================================

#endif // DSPTHREADNULL_H
