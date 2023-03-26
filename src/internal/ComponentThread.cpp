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

#include <internal/ComponentThread.h>

using namespace DSPatch::internal;

ComponentThread::ComponentThread() = default;

void ComponentThread::Setup( DSPatch::Component* component, int bufferNo, const DSPatch::ThreadPool::SPtr& threadPool )
{
    _bufferNo = bufferNo;
    _component = component;
    _threadPool = threadPool;
}

void ComponentThread::TickAsync()
{
    _gotDone = false;  // reset the sync flag
    _threadPool->AddJob( _bufferNo, this );
}

void ComponentThread::Wait()
{
    if ( _gotDone )  // if already got sync
    {
        return;
    }

    std::unique_lock<std::mutex> lock( _doneMutex );

    // cppcheck-suppress knownConditionTrueFalse
    if ( !_gotDone )  // if haven't already got sync
    {
        _doneCondt.wait( lock );  // wait for sync
    }
}

void ComponentThread::Run()
{
    _component->DoTick( _bufferNo );

    std::lock_guard<std::mutex> lock( _doneMutex );

    _gotDone = true;  // set the sync flag
    _doneCondt.notify_all();
}
