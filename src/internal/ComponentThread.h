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

#include <dspatch/Component.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <condition_variable>
#include <thread>

namespace DSPatch
{
namespace internal
{

class ComponentThread final
{
public:
    NONCOPYABLE( ComponentThread );

    inline ComponentThread() = default;

    // cppcheck-suppress missingMemberCopy
    inline ComponentThread( ComponentThread&& )
    {
    }

    inline ~ComponentThread()
    {
        Stop();
    }

    inline void Start( std::vector<DSPatch::Component*>* components, int threadNo, int threadCount )
    {
        _components = components;
        _threadNo = threadNo;
        _threadCount = threadCount;

        _stop = false;
        _gotSync = false;

        _thread = std::thread( &ComponentThread::_Run, this );

        Sync();
    }

    inline void Stop()
    {
        _stop = true;

        Resume();

        if ( _thread.joinable() )
        {
            _thread.join();
        }
    }

    inline void Sync()
    {
        if ( _gotSync )
        {
            return;
        }

        std::unique_lock<std::mutex> lock( _syncMutex );

        // cppcheck-suppress knownConditionTrueFalse
        if ( !_gotSync )  // if haven't already got sync
        {
            _syncCondt.wait( lock );  // wait for sync
        }
    }

    inline void Resume()
    {
        _gotSync = false;  // reset the sync flag

        std::lock_guard<std::mutex> lock( _syncMutex );

        _resumeCondt.notify_all();
    }

private:
    inline void _Run()
    {
#ifdef _WIN32
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
#else
        sched_param sch_params;
        sch_params.sched_priority = sched_get_priority_max( SCHED_RR );
        pthread_setschedparam( pthread_self(), SCHED_RR, &sch_params );
#endif

        if ( _components )
        {
            while ( !_stop )
            {
                {
                    std::unique_lock<std::mutex> lock( _syncMutex );

                    _gotSync = true;  // set the sync flag
                    _syncCondt.notify_all();
                    _resumeCondt.wait( lock );  // wait for resume
                }

                // cppcheck-suppress knownConditionTrueFalse
                if ( !_stop )
                {
                    for ( size_t i = _threadNo; i < _components->size(); i += _threadCount )
                    {
                        ( *_components )[i]->Tick();
                    }
                }
            }
        }
    }

    std::thread _thread;
    std::vector<DSPatch::Component*>* _components = nullptr;
    int _threadNo = 0;
    int _threadCount = 0;
    bool _stop = false;
    bool _gotSync = true;
    std::mutex _syncMutex;
    std::condition_variable _resumeCondt, _syncCondt;
};

}  // namespace internal
}  // namespace DSPatch
