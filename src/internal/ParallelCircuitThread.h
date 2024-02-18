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
#undef WIN32_LEAN_AND_MEAN
#endif

#include <condition_variable>
#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for asynchronously ticking parallel circuit components

/**
A ParallelCircuitThread is responsible for ticking parallel components within a Circuit. Upon initialisation, a reference to the
parallel-ordered vector of circuit components must be provided for the thread _Run() method to loop through. Each
ParallelCircuitThread has a buffer number (bufferNo) and thread number (threadNo), which is also provided upon initialisation.
When creating multiple ParallelCircuitThreads, each thread must have their own unique bufferNo:threadNo combination, beginning at
0:0 and incrementing for every buffer thread added. The buffer number corresponds with the Component's buffer number when calling
its Tick() method in the ParallelCircuitThread's component loop, and the thread number is that buffer's thread index - i.e. a
circuit with x threads, will spawn x threads per buffer.

The Sync() method will block until the thread is ready to process. The Resume() method will then signal the ParallelCircuitThread
to tick its components once, after which the thread will wait until instructed to resume again. As each component is done
processing it hands over control to the next waiting CircuitThread, therefore, from an external control loop (I.e. Circuit's
Tick() method) we can simply loop through our array of ParallelCircuitThreads twice, calling Sync() on each, then Resume() on
each.
*/

class ParallelCircuitThread final
{
public:
    NONCOPYABLE( ParallelCircuitThread );

    inline ParallelCircuitThread() = default;

    // cppcheck-suppress missingMemberCopy
    inline ParallelCircuitThread( ParallelCircuitThread&& )
    {
    }

    inline ~ParallelCircuitThread()
    {
        Stop();
    }

    inline void Start( std::vector<DSPatch::Component*>* components, int bufferNo, int threadNo, int threadCount )
    {
        _components = components;
        _bufferNo = bufferNo;
        _threadNo = threadNo;
        _threadCount = threadCount;

        _stop = false;
        _gotSync = false;

        _thread = std::thread( &ParallelCircuitThread::_Run, this );
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
        std::unique_lock<std::mutex> lock( _syncMutex );

        if ( !_gotSync )  // if haven't already got sync
        {
            _syncCondt.wait( lock );  // wait for sync
        }
    }

    inline void Resume()
    {
        _gotSync = false;  // reset the sync flag
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
            auto& components = *_components;
            size_t componentsSize;

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
                    componentsSize = components.size();
                    for ( size_t i = _threadNo; i < componentsSize; i += _threadCount )
                    {
                        components[i]->TickParallel( _bufferNo );
                    }
                }
            }
        }
    }

    std::thread _thread;
    std::vector<DSPatch::Component*>* _components = nullptr;
    int _bufferNo = 0;
    int _threadNo = 0;
    int _threadCount = 0;
    bool _stop = false;
    bool _gotSync = false;
    std::mutex _syncMutex;
    std::condition_variable _resumeCondt, _syncCondt;
};

}  // namespace internal
}  // namespace DSPatch
