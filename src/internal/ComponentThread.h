/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2018 Marcus Tomlinson

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

#pragma once

#include <dspatch/Common.h>
#include <dspatch/Component.h>

#include <internal/Thread.h>

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

class ComponentThread final : public Thread
{
public:
    NONCOPYABLE( ComponentThread );

    ComponentThread();
    virtual ~ComponentThread() override;

    void Initialise( std::shared_ptr<DSPatch::Component> const& component );

    bool IsInitialised() const;
    bool IsStopped() const;

    void Start( Priority priority = HighestPriority ) override;
    void Stop() override;
    void Pause();
    void Resume();

private:
    virtual void Run_() override;

private:
    std::weak_ptr<DSPatch::Component> _component;
    bool _stop, _pause;
    bool _stopped;
    std::mutex _resumeMutex;
    std::condition_variable _resumeCondt, _pauseCondt;
};

} // namespace internal
} // namespace DSPatch
