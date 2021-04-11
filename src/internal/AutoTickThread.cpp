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

#include <internal/AutoTickThread.h>

#include <dspatch/Circuit.h>

#include <thread>

using namespace DSPatch::internal;

AutoTickThread::AutoTickThread()
{
}

AutoTickThread::~AutoTickThread()
{
    Stop();
}

DSPatch::Component::TickMode AutoTickThread::Mode()
{
    return _mode;
}

bool AutoTickThread::IsStopped() const
{
    return _stopped;
}

bool AutoTickThread::IsPaused() const
{
    return _pause;
}

void AutoTickThread::Start( DSPatch::Circuit* circuit, DSPatch::Component::TickMode mode )
{
    if ( !_stopped )
    {
        return;
    }

    _circuit = circuit;

    _mode = mode;
    _stop = false;
    _stopped = false;
    _pause = false;

    _thread = std::thread( &AutoTickThread::_Run, this );
}

void AutoTickThread::Stop()
{
    if ( _stopped )
    {
        return;
    }

    Pause();

    _stop = true;

    Resume();

    if ( _thread.joinable() )
    {
        _thread.join();
    }
}

void AutoTickThread::Pause()
{
    std::unique_lock<std::mutex> lock( _resumeMutex );

    if ( !_pause && !_stopped )
    {
        _pause = true;
        _pauseCondt.wait( lock );  // wait for resume
    }
}

void AutoTickThread::Resume()
{
    std::unique_lock<std::mutex> lock( _resumeMutex );

    if ( _pause )
    {
        _resumeCondt.notify_all();
        _pause = false;
    }
}

void AutoTickThread::_Run()
{
    if ( _circuit != nullptr )
    {
        while ( !_stop )
        {
            _circuit->Tick( _mode );

            if ( _pause )
            {
                std::unique_lock<std::mutex> lock( _resumeMutex );

                _pauseCondt.notify_all();

                _resumeCondt.wait( lock );  // wait for resume
            }
        }
    }

    _stopped = true;
}
