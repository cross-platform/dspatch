/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2023, Marcus Tomlinson

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
#include <dspatch/Component.h>
#include <dspatch/ThreadPool.h>

#include <condition_variable>
#include <functional>
#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for asynchronously ticking a single circuit component

/**
A ComponentThread's primary purpose is to tick parallel circuit components in parallel.

TickAsync() adds a tick call to the provided thread pool. Wait() will block until that thread pool
has completed execution of the tick call. Until this point, Done() will return false.
*/

class ComponentThread final
{
public:
    NONCOPYABLE( ComponentThread );

    ComponentThread();

    void Setup( DSPatch::Component* component, int bufferNo, const DSPatch::ThreadPool::SPtr& threadPool );

    void TickAsync();
    void Wait();

    void Run();

private:
    DSPatch::ThreadPool::SPtr _threadPool;
    int _bufferNo = 0;
    DSPatch::Component* _component = nullptr;
    bool _gotDone = true;
    std::mutex _doneMutex;
    std::condition_variable _doneCondt;
};

}  // namespace internal
}  // namespace DSPatch
