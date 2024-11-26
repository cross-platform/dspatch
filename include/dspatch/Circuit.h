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

#include "Component.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <algorithm>
#include <condition_variable>
#include <thread>
#include <unordered_set>

namespace DSPatch
{

/// Workspace for adding and routing components

/**
Components can be added to a Circuit via the AddComponent() method, and routed to and from other components via the
ConnectOutToIn() method.

<b>NOTE:</b> Each component input can only accept a single "wire" at a time. When a wire is connected to an input that already has
a connected wire, that wire is replaced with the new one. One output, on the other hand, can be distributed to multiple inputs.

To boost performance in stream processing circuits, multi-buffering can be enabled via the SetBufferCount() method. A circuit's
buffer count can be adjusted at runtime.

<b>NOTE:</b> If none of the parallel branches in your circuit are time-consuming (⪆10μs), multi-buffering (or even zero buffering)
will almost always outperform multi-threading (via SetThreadCount()). The contention overhead caused by multiple threads
processing a single tick must be made negligible by time-consuming parallel components for any performance improvement to be seen.

The Circuit Tick() method runs through its internal array of components and calls each component's Tick() method. A circuit's
Tick() method can be called in a loop from the main application thread, or alternatively, by calling StartAutoTick(), a separate
thread will spawn, automatically calling Tick() continuously until PauseAutoTick() or StopAutoTick() is called.

The Circuit Optimize() method rearranges components such that they process in the most optimal order during Tick(). This
optimization will occur automatically during the first Tick() proceeding any connection / disconnection, however, if you'd like to
pre-order components before the next Tick() is processed, you can call Optimize() manually.
*/

class Circuit final
{
public:
    Circuit( const Circuit& ) = delete;
    Circuit& operator=( const Circuit& ) = delete;

    Circuit();
    ~Circuit();

    bool AddComponent( const Component::SPtr& component );

    bool RemoveComponent( const Component::SPtr& component );
    void RemoveAllComponents();

    int GetComponentCount() const;

    bool ConnectOutToIn( const Component::SPtr& fromComponent, int fromOutput, const Component::SPtr& toComponent, int toInput );

    bool DisconnectComponent( const Component::SPtr& component );
    void DisconnectAllComponents();

    void SetBufferCount( int bufferCount );
    int GetBufferCount() const;

    void SetThreadCount( int threadCount );
    int GetThreadCount() const;

    void Tick();
    void Sync();

    void StartAutoTick();
    void StopAutoTick();
    void PauseAutoTick();
    void ResumeAutoTick();

    void Optimize();

private:
    class AutoTickThread final
    {
    public:
        AutoTickThread( const AutoTickThread& ) = delete;
        AutoTickThread& operator=( const AutoTickThread& ) = delete;

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
            _pause = true;

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
                std::this_thread::yield();
            }
        }

    private:
        inline void _Run()
        {
            if ( _circuit )
            {
                while ( true )
                {
                    _circuit->Tick();

                    if ( _pause )
                    {
                        if ( _stop )
                        {
                            break;
                        }

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

    class CircuitThread final
    {
    public:
        CircuitThread( const CircuitThread& ) = delete;
        CircuitThread& operator=( const CircuitThread& ) = delete;

        inline CircuitThread() = default;

        // cppcheck-suppress missingMemberCopy
        inline CircuitThread( CircuitThread&& )
        {
        }

        inline ~CircuitThread()
        {
            Stop();
        }

        inline void Start( std::vector<DSPatch::Component*>* components, int bufferNo )
        {
            _components = components;
            _bufferNo = bufferNo;

            _stop = false;
            _gotSync = false;

            _thread = std::thread( &CircuitThread::_Run, this );
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
            std::this_thread::yield();
        }

        inline void SyncAndResume()
        {
            Sync();
            Resume();
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
                while ( true )
                {
                    {
                        std::unique_lock<std::mutex> lock( _syncMutex );

                        _gotSync = true;  // set the sync flag
                        _syncCondt.notify_all();
                        _resumeCondt.wait( lock );  // wait for resume
                    }

                    if ( _stop )
                    {
                        break;
                    }

                    // You might be thinking: Can't we have each thread start on a different component?

                    // Well no. In order to maintain synchronisation within the circuit, when a component
                    // wants to process its buffers in-order, it requires that every other in-order
                    // component in the system has not only processed its buffers in the same order, but
                    // has processed the same number of buffers too.

                    // E.g. 1,2,3 and 1,2,3. Not 1,2,3 and 2,3,1,2,3.

                    for ( auto component : *_components )
                    {
                        component->Tick( _bufferNo );
                    }
                }
            }
        }

        std::thread _thread;
        std::vector<DSPatch::Component*>* _components = nullptr;
        int _bufferNo = 0;
        bool _stop = false;
        bool _gotSync = false;
        std::mutex _syncMutex;
        std::condition_variable _resumeCondt, _syncCondt;
    };

    class CircuitThreadParallel final
    {
    public:
        CircuitThreadParallel( const CircuitThreadParallel& ) = delete;
        CircuitThreadParallel& operator=( const CircuitThreadParallel& ) = delete;

        inline CircuitThreadParallel() = default;

        // cppcheck-suppress missingMemberCopy
        inline CircuitThreadParallel( CircuitThreadParallel&& )
        {
        }

        inline ~CircuitThreadParallel()
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

            _thread = std::thread( &CircuitThreadParallel::_Run, this );
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
            std::this_thread::yield();
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
                while ( true )
                {
                    {
                        std::unique_lock<std::mutex> lock( _syncMutex );

                        _gotSync = true;  // set the sync flag
                        _syncCondt.notify_all();
                        _resumeCondt.wait( lock );  // wait for resume
                    }

                    if ( _stop )
                    {
                        break;
                    }

                    for ( auto it = _components->begin() + _threadNo; it < _components->end(); it += _threadCount )
                    {
                        ( *it )->TickParallel( _bufferNo );
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

    void _Optimize();

    int _bufferCount = 0;
    int _threadCount = 0;
    int _currentBuffer = 0;

    AutoTickThread _autoTickThread;

    std::unordered_set<DSPatch::Component::SPtr> _componentsSet;

    std::vector<DSPatch::Component*> _components;
    std::vector<DSPatch::Component*> _componentsParallel;

    std::vector<CircuitThread> _circuitThreads;
    std::vector<std::vector<CircuitThreadParallel>> _circuitThreadsParallel;

    bool _circuitDirty = false;
};

inline Circuit::Circuit() = default;

inline Circuit::~Circuit()
{
    StopAutoTick();
    DisconnectAllComponents();
}

inline bool Circuit::AddComponent( const Component::SPtr& component )
{
    if ( !component || _componentsSet.find( component ) != _componentsSet.end() )
    {
        return false;
    }

    // components within the circuit need to have as many buffers as there are threads in the circuit
    component->SetBufferCount( _bufferCount, _currentBuffer );

    PauseAutoTick();
    _components.emplace_back( component.get() );
    _componentsParallel.emplace_back( component.get() );
    ResumeAutoTick();

    _componentsSet.emplace( component );

    return true;
}

inline bool Circuit::RemoveComponent( const Component::SPtr& component )
{
    if ( _componentsSet.find( component ) == _componentsSet.end() )
    {
        return false;
    }

    auto findFn = [&component]( auto comp ) { return comp == component.get(); };

    if ( auto it = std::find_if( _components.begin(), _components.end(), findFn ); it != _components.end() )
    {
        PauseAutoTick();

        DisconnectComponent( component );

        _components.erase( it );

        ResumeAutoTick();

        _componentsSet.erase( component );

        return true;
    }

    return false;
}

// cppcheck-suppress unusedFunction
inline void Circuit::RemoveAllComponents()
{
    PauseAutoTick();

    DisconnectAllComponents();

    _components.clear();
    _componentsParallel.clear();

    ResumeAutoTick();

    _componentsSet.clear();
}

inline int Circuit::GetComponentCount() const
{
    return (int)_components.size();
}

inline bool Circuit::ConnectOutToIn( const Component::SPtr& fromComponent,
                                     int fromOutput,
                                     const Component::SPtr& toComponent,
                                     int toInput )
{
    if ( _componentsSet.find( fromComponent ) == _componentsSet.end() ||
         _componentsSet.find( toComponent ) == _componentsSet.end() )
    {
        return false;
    }

    PauseAutoTick();

    bool result = toComponent->ConnectInput( fromComponent, fromOutput, toInput );

    _circuitDirty = result;

    ResumeAutoTick();

    return result;
}

inline bool Circuit::DisconnectComponent( const Component::SPtr& component )
{
    if ( _componentsSet.find( component ) == _componentsSet.end() )
    {
        return false;
    }

    PauseAutoTick();

    component->DisconnectAllInputs();

    // remove any connections this component has to other components
    for ( auto comp : _components )
    {
        comp->DisconnectInput( component );
    }

    _circuitDirty = true;

    ResumeAutoTick();

    return true;
}

inline void Circuit::DisconnectAllComponents()
{
    PauseAutoTick();

    for ( auto component : _components )
    {
        component->DisconnectAllInputs();
    }

    ResumeAutoTick();
}

inline void Circuit::SetBufferCount( int bufferCount )
{
    PauseAutoTick();

    _bufferCount = bufferCount;

    // stop all threads
    for ( auto& circuitThread : _circuitThreads )
    {
        circuitThread.Stop();
    }

    // resize thread array
    if ( _threadCount != 0 )
    {
        _circuitThreads.resize( 0 );
        SetThreadCount( _threadCount );
    }
    else
    {
        _circuitThreads.resize( _bufferCount );

        // initialise and start all threads
        for ( int i = 0; i < _bufferCount; ++i )
        {
            _circuitThreads[i].Start( &_components, i );
        }
    }

    if ( _currentBuffer >= _bufferCount )
    {
        _currentBuffer = 0;
    }

    // set all components to the new buffer count
    for ( auto component : _components )
    {
        component->SetBufferCount( _bufferCount, _currentBuffer );
    }

    ResumeAutoTick();
}

inline int Circuit::GetBufferCount() const
{
    return _bufferCount;
}

inline void Circuit::SetThreadCount( int threadCount )
{
    PauseAutoTick();

    _threadCount = threadCount;

    // stop all threads
    for ( auto& circuitThreads : _circuitThreadsParallel )
    {
        for ( auto& circuitThread : circuitThreads )
        {
            circuitThread.Stop();
        }
    }

    // resize thread array
    if ( _threadCount == 0 )
    {
        _circuitThreadsParallel.resize( 0 );
        SetBufferCount( _bufferCount );
    }
    else
    {
        _circuitThreadsParallel.resize( _bufferCount == 0 ? 1 : _bufferCount );
        for ( auto& circuitThread : _circuitThreadsParallel )
        {
            circuitThread.resize( _threadCount );
        }

        // initialise and start all threads
        int i = 0;
        for ( auto& circuitThreads : _circuitThreadsParallel )
        {
            int j = 0;
            for ( auto& circuitThread : circuitThreads )
            {
                circuitThread.Start( &_componentsParallel, i, j++, _threadCount );
            }
            ++i;
        }
    }

    ResumeAutoTick();
}

// cppcheck-suppress unusedFunction
inline int Circuit::GetThreadCount() const
{
    return _threadCount;
}

inline void Circuit::Tick()
{
    if ( _circuitDirty )
    {
        _Optimize();
    }

    // process in multiple threads if this circuit has threads
    // =======================================================
    if ( _threadCount != 0 )
    {
        auto& circuitThreads = _circuitThreadsParallel[_currentBuffer];

        for ( auto& circuitThread : circuitThreads )
        {
            circuitThread.Sync();
        }
        for ( auto& circuitThread : circuitThreads )
        {
            circuitThread.Resume();
        }
    }
    // process in a single thread if this circuit has no threads
    // =========================================================
    else if ( _bufferCount == 0 )
    {
        // tick all internal components
        for ( auto component : _components )
        {
            component->Tick( 0 );
        }

        return;
    }
    else
    {
        _circuitThreads[_currentBuffer].SyncAndResume();  // sync and resume thread x
    }

    if ( _bufferCount != 0 && ++_currentBuffer == _bufferCount )
    {
        _currentBuffer = 0;
    }
}

inline void Circuit::Sync()
{
    // sync all threads
    for ( auto& circuitThread : _circuitThreads )
    {
        circuitThread.Sync();
    }
    for ( auto& circuitThreads : _circuitThreadsParallel )
    {
        for ( auto& circuitThread : circuitThreads )
        {
            circuitThread.Sync();
        }
    }
}

inline void Circuit::StartAutoTick()
{
    _autoTickThread.Start( this );
}

inline void Circuit::StopAutoTick()
{
    _autoTickThread.Stop();
    Sync();
}

inline void Circuit::PauseAutoTick()
{
    _autoTickThread.Pause();
    Sync();
}

inline void Circuit::ResumeAutoTick()
{
    _autoTickThread.Resume();
}

inline void Circuit::Optimize()
{
    if ( _circuitDirty )
    {
        PauseAutoTick();
        _Optimize();
        ResumeAutoTick();
    }
}

inline void Circuit::_Optimize()
{
    // scan for optimal series order -> update _components
    {
        std::vector<DSPatch::Component*> orderedComponents;
        orderedComponents.reserve( _components.size() );

        for ( auto component : _components )
        {
            component->Scan( orderedComponents );
        }
        for ( auto component : _components )
        {
            component->EndScan();
        }

        _components = std::move( orderedComponents );
    }

    // scan for optimal parallel order -> update _componentsParallel
    std::vector<std::vector<DSPatch::Component*>> componentsMap;
    componentsMap.reserve( _components.size() );

    int scanPosition;
    for ( auto component : _components )
    {
        component->ScanParallel( componentsMap, scanPosition );
    }
    for ( auto component : _components )
    {
        component->EndScan();
    }

    _componentsParallel.clear();
    _componentsParallel.reserve( _components.size() );
    for ( auto& componentsMapEntry : componentsMap )
    {
        _componentsParallel.insert( _componentsParallel.end(), componentsMapEntry.begin(), componentsMapEntry.end() );
    }

    // clear _circuitDirty flag
    _circuitDirty = false;
}

}  // namespace DSPatch
