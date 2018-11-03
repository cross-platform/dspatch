/************************************************************************
DSPatch - The C++ Flow-Based Programming Framework
Copyright (c) 2012-2018 Marcus Tomlinson

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

#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for ticking and reseting a single component

/**
A ComponentThread is responsible for ticking and reseting a single component continuously in a
free-running thread. Upon initialisation, a reference to the component must be provided for the
Thread's Run_() method to use. Once Start() has been called, the thread will begin repeatedly
executing the Run_() method. On each thread iteration, ComponentThread simply calls the reference
component's Tick() and Reset() methods.

The Pause() method causes ComponentThread to wait until instructed to Resume() again.
*/

class AutoTickThread final
{
public:
    NONCOPYABLE( AutoTickThread );
    DEFINE_PTRS( AutoTickThread );

    AutoTickThread();
    virtual ~AutoTickThread();

    void Initialise( DSPatch::Circuit* circuit );

    bool IsInitialised() const;
    bool IsStopped() const;

    void Start();
    void Stop();
    void Pause();
    void Resume();

private:
    virtual void _Run();

private:
    std::thread _thread;
    DSPatch::Circuit* _circuit;
    bool _stop, _pause;
    bool _stopped;
    std::mutex _resumeMutex;
    std::condition_variable _resumeCondt, _pauseCondt;
};

}  // namespace internal
}  // namespace DSPatch
