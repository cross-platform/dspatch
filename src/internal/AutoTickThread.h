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

#include <dspatch/Circuit.h>
#include <dspatch/Common.h>

#include <condition_variable>
#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for auto-ticking a circuit

/**
An AutoTickThread is responsible for ticking a circuit continuously in a free-running thread. Upon
initialisation, a reference to the circuit must be provided for the thread's _Run() method to use.
Once Start() has been called, the thread will begin, repeatedly calling the circuit's Tick()
method until instructed to Pause() or Stop().
*/

class AutoTickThread final
{
public:
    NONCOPYABLE( AutoTickThread );
    DEFINE_PTRS( AutoTickThread );

    AutoTickThread();
    ~AutoTickThread();

    DSPatch::Component::TickMode Mode();

    bool IsStopped() const;
    bool IsPaused() const;

    void Start( DSPatch::Circuit* circuit, DSPatch::Component::TickMode mode );
    void Stop();
    void Pause();
    void Resume();

private:
    void _Run();

private:
    DSPatch::Component::TickMode _mode;
    std::thread _thread;
    DSPatch::Circuit* _circuit = nullptr;
    bool _stop = false;
    bool _pause = false;
    bool _stopped = true;
    std::mutex _resumeMutex;
    std::condition_variable _resumeCondt, _pauseCondt;
};

}  // namespace internal
}  // namespace DSPatch
