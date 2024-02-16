/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2024, Marcus Tomlinson

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

#include <dspatch/Circuit.h>

#include <condition_variable>
#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for auto-ticking a circuit

/**
An AutoTickThread is responsible for ticking a circuit continuously in a free-running thread. Upon initialisation, a reference to
the circuit must be provided for the thread's _Run() method to use. Once Start() has been called, the thread will begin,
repeatedly calling the circuit's Tick() method until instructed to Pause() or Stop().
*/

class AutoTickThread final
{
public:
    NONCOPYABLE( AutoTickThread );

    inline AutoTickThread() = default;

    inline ~AutoTickThread()
    {
        Stop();
    }

    inline void Start( DSPatch::Circuit* circuit )
    {
        if ( !_stopped )
        {
            Resume();
            return;
        }

        _circuit = circuit;

        _stop = false;
        _stopped = false;
        _pause = false;

        _thread = std::thread( &AutoTickThread::_Run, this );
    }

    inline void Stop()
    {
        _stop = true;

        if ( _thread.joinable() )
        {
            _thread.join();
        }
    }

    inline void Pause()
    {
        if ( !_stopped && ++pauseCount == 1 )
        {
            std::unique_lock<std::mutex> lock( _resumeMutex );
            _pause = true;
            _pauseCondt.wait( lock );  // wait for pause
        }
    }

    inline void Resume()
    {
        if ( _pause && --pauseCount == 0 )
        {
            _pause = false;
            _resumeCondt.notify_all();
        }
    }

private:
    inline void _Run()
    {
        if ( _circuit )
        {
            while ( !_stop )
            {
                _circuit->Tick();

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

    std::thread _thread;
    DSPatch::Circuit* _circuit = nullptr;
    int pauseCount = 0;
    bool _stop = false;
    bool _pause = false;
    bool _stopped = true;
    std::mutex _resumeMutex;
    std::condition_variable _resumeCondt, _pauseCondt;
};

}  // namespace internal
}  // namespace DSPatch
