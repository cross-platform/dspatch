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

#include <dspatch/ThreadPool.h>

using namespace DSPatch::internal;

ComponentThread::ComponentThread() = default;

void ComponentThread::Sync()
{
    std::unique_lock<std::mutex> lock( _syncMutex );

    if ( !_gotSync )  // if haven't already got sync
    {
        _syncCondt.wait( lock );  // wait for sync
    }
}

void ComponentThread::Resume( int bufferNo, const std::function<void()>& tick )
{
    _gotSync = false;  // reset the sync flag
    _tick = tick;
    threadPool.AddJob( bufferNo, std::bind( &ComponentThread::_Run, this ) );
}

void ComponentThread::_Run()
{
    _tick();

    std::lock_guard<std::mutex> lock( _syncMutex );

    _gotSync = true;  // set the sync flag
    _syncCondt.notify_all();
}
