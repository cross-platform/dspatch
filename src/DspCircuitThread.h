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

#ifndef DSPCIRCUITTHREAD_H
#define DSPCIRCUITTHREAD_H

//-------------------------------------------------------------------------------------------------

#include <vector>

#include "DspThread.h"

class DspComponent;

//=================================================================================================
/// Thread class for ticking and reseting circuit components

/**
A DspCircuitThread is responsible for ticking and reseting all components in a DspCircuit.
On construction, a reference to the vector of circuit components must be provided for the thread
_Run() method to loop through. Each DspCircuitThread has a thread number (threadNo), which also
can be provided on construction. When creating multiple DspCircuitThreads, each thread must have
their own unique thread number, beginning at 0 and incrementing by 1 for every thread added. This
thread number corresponds with the DspComponent's buffer number when calling it's ThreadTick() and
ThreadReset() methods in the DspCircuitThread's component loop. Hence, for every circuit thread
created, each component's buffer count within that circuit must be incremented to match.

The Resume() method causes the DspCircuitThread to tick and reset all circuit components once,
after which the thread will wait until instructed to resume again. As each component is done
processing it hands over control to the next waiting circuit thread, therefore, from an external
control loop (I.e. DspCircuit's Process_() method) we simply loop through our array of
DspCircuitThreads calling Resume() on each. If a circuit thread is busy processing, a call to
Resume() will block momentarily until processing is complete, then begin the next iteration
immediately upon unblocking the calling thread.

The Sync() method, when called, will block the calling thread until the circuit thread is done
processing. If the circuit thread is already awaiting the next Resume() request, this method will
return immediately.
*/

class DLLEXPORT DspCircuitThread : public DspThread
{
public:
  DspCircuitThread();
  ~DspCircuitThread();

  void Initialise(  std::vector< DspComponent* >* components, unsigned short threadNo );

  void Start( Priority priority = TimeCriticalPriority );
  void Stop();
  void Sync();
  void Resume();

private:
  std::vector< DspComponent* >* _components;
  unsigned short _threadNo;
  bool _stop;
  bool _stopped;
  bool _gotResume, _gotSync;
  DspMutex _resumeMutex;
  DspWaitCondition _resumeCondt, _syncCondt;

  virtual void _Run();
};

//=================================================================================================

#endif // DSPCIRCUITTHREAD_H
