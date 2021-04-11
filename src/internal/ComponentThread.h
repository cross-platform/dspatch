/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2021, Marcus Tomlinson

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

#include <dspatch/Common.h>

#include <condition_variable>
#include <thread>
#include <functional>

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
