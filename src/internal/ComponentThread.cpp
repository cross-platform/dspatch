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

#include <internal/ComponentThread.h>

using namespace DSPatch::internal;

ComponentThread::ComponentThread()
{
}

ComponentThread::~ComponentThread()
{
    Stop();
}

void ComponentThread::Start()
{
    if ( !_stopped )
    {
        return;
    }

    _stop = false;
    _stopped = false;
    _gotResume = false;
    _gotSync = false;

    _thread = std::thread( &ComponentThread::_Run, this );

    Sync();
}

void ComponentThread::Stop()
{
    if ( _stopped )
    {
        return;
    }

    Sync();

    _stop = true;

    Resume( _tick );

    if ( _thread.joinable() )
    {
        _thread.join();
    }
}

void ComponentThread::Sync()
{
    if ( _stopped )
    {
        return;
    }

    std::unique_lock<std::mutex> lock( _resumeMutex );

    if ( !_gotSync )  // if haven't already got sync
    {
        _syncCondt.wait( lock );  // wait for sync
    }
}

void ComponentThread::Resume( std::function<void()> const& tick )
{
    if ( _stopped )
    {
        Start();
    }

    std::unique_lock<std::mutex> lock( _resumeMutex );

    _gotSync = false;  // reset the sync flag

    _tick = tick;

    _gotResume = true;  // set the resume flag
    _resumeCondt.notify_all();
}

void ComponentThread::_Run()
{
    while ( !_stop )
    {
        {
            std::unique_lock<std::mutex> lock( _resumeMutex );

            _gotSync = true;  // set the sync flag
            _syncCondt.notify_all();

            if ( !_gotResume )  // if haven't already got resume
            {
                _resumeCondt.wait( lock );  // wait for resume
            }
            _gotResume = false;  // reset the resume flag
        }

        if ( !_stop )
        {
            _tick();
        }
    }

    _stopped = true;
}
