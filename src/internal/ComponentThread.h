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

#include <dspatch/Common.h>

#include <condition_variable>
#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for asynchronously ticking a single circuit component

/**
A ComponentThread's primary purpose is to tick parallel circuit components in parallel.

Upon Start(), an internal thread will spawn and wait for the first call to Resume() before
executing the tick method provided. A call to Sync() will then block until the thread has completed
execution of the tick method. At this point, the thread will wait until instructed to resume again.
*/

class ComponentThread final
{
public:
    NONCOPYABLE( ComponentThread );
    DEFINE_PTRS( ComponentThread );

    ComponentThread();
    ~ComponentThread();

    void Start();
    void Stop();
    void Sync();
    void Resume( std::function<void()> const& tick );

private:
    void _Run();

private:
    std::thread _thread;
    bool _stop = false;
    bool _stopped = true;
    bool _gotResume = false;
    bool _gotSync = true;
    std::mutex _resumeMutex;
    std::condition_variable _resumeCondt, _syncCondt;
    std::function<void()> _tick;
};

}  // namespace internal
}  // namespace DSPatch
