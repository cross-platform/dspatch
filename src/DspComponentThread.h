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

#ifndef DSPCOMPONENTTHREAD_H
#define DSPCOMPONENTTHREAD_H

//-------------------------------------------------------------------------------------------------

#include "DspThread.h"

class DspComponent;

//=================================================================================================
/// Thread class for ticking and reseting a single component

/**
A DspComponentThread is responsible for ticking and reseting a single component continuously in
a separate free-running thread. On construction, a reference to the component must be provided for
the DspThread's _Run() method to use. Once Start() has been called, the thread will begin
repeatedly executing the _Run() method. On each thread iteration, DspComponentThread simply calls
the reference component's Tick() and Reset() methods. The Pause() method causes DspComponentThread
to wait until instructed to Resume() again.
*/

class DLLEXPORT DspComponentThread : public DspThread
{
public:
  DspComponentThread();
  ~DspComponentThread();

  void Initialise( DspComponent* component );
  bool IsStopped() const;

  void Start( Priority priority = TimeCriticalPriority );
  void Stop();
  void Pause();
  void Resume();

private:
  DspComponent* _component;
  bool _stop, _pause;
  bool _stopped;
  DspMutex _resumeMutex;
  DspWaitCondition _resumeCondt, _pauseCondt;

  virtual void _Run();
};

//=================================================================================================

#endif // DSPCOMPONENTTHREAD_H
