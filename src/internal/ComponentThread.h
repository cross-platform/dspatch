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

#include <dspatch/Common.h>

#include <condition_variable>
#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for asynchronously ticking a single component

/**
...
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
    void Resume( std::function<void()> tickFunction );

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
    std::function<void()> _tickFunction;
};

}  // namespace internal
}  // namespace DSPatch
