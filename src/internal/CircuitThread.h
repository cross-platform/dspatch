/************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2012-2019 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LICENSE included in the packaging of this file.
Please review the following information to ensure the GNU Lesser
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

#pragma once

#include <dspatch/Component.h>

#include <condition_variable>
#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for asynchronously ticking circuit components

/**
A CircuitThread is responsible for ticking and reseting all components within a Circuit. Upon
initialisation, a reference to the vector of circuit components must be provided for the thread
_Run() method to loop through. Each CircuitThread has a thread number (threadNo), which is also
provided upon initialisation. When creating multiple CircuitThreads, each thread must have their
own unique thread number, beginning at 0 and incrementing by 1 for every thread added. This thread
number corresponds with the Component's buffer number when calling it's Tick() and Reset() methods
in the CircuitThread's component loop. Hence, for every circuit thread created, each component's
buffer count within that circuit must be incremented to match.

The SyncAndResume() method causes the CircuitThread to tick and reset all circuit components once,
after which the thread will wait until instructed to resume again. As each component is done
processing it hands over control to the next waiting circuit thread, therefore, from an external
control loop (I.e. Circuit's Tick() method) we can simply loop through our array of CircuitThreads
calling SyncAndResume() on each. If a circuit thread is busy processing, a call to SyncAndResume()
will block momentarily until that thread is done processing.
*/

class CircuitThread final
{
public:
    NONCOPYABLE( CircuitThread );
    DEFINE_PTRS( CircuitThread );

    CircuitThread();
    ~CircuitThread();

    void Start( std::vector<DSPatch::Component::SPtr>* components, int threadNo );
    void Stop();
    void Sync();
    void SyncAndResume( DSPatch::Component::TickMode mode );

private:
    void _Run();

private:
    DSPatch::Component::TickMode _mode;
    std::thread _thread;
    std::vector<DSPatch::Component::SPtr>* _components = nullptr;
    int _threadNo = 0;
    bool _stop = false;
    bool _stopped = true;
    bool _gotResume = false;
    bool _gotSync = true;
    std::mutex _resumeMutex;
    std::condition_variable _resumeCondt, _syncCondt;
};

}  // namespace internal
}  // namespace DSPatch
