/************************************************************************
DSPatch - The C++ Flow-Based Programming Framework
Copyright (c) 2012-2018 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LICENSE included in the packaging of this file.
Please review the following information to ensure the GNU Lesser
General Public License version 3.0 requirements will be met:
http://www.gnu.org/copyleft/lgpl.html.

Other Usage
Alternatively, this file may be used in accordance with the terms and
conditions contained in a signed written agreement between you and
Marcus Tomlinson.

DSPatch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
************************************************************************/

#include <internal/AutoTickThread.h>

#include <dspatch/Circuit.h>

#include <thread>

#ifdef _WIN32
#include <windows.h>

static void MaximiseThreadPriority( std::thread::native_handle_type const& handle )
{
    SetThreadPriority( handle, THREAD_PRIORITY_TIME_CRITICAL );
}
#else
static void MaximiseThreadPriority( std::thread::native_handle_type const& handle )
{
    struct sched_param params;
    params.sched_priority = 99;
    pthread_setschedparam( handle, SCHED_FIFO, &params );
}
#endif

using namespace DSPatch::internal;

AutoTickThread::AutoTickThread()
    : _circuit( nullptr )
    , _stop( false )
    , _pause( false )
    , _stopped( true )
{
}

AutoTickThread::~AutoTickThread()
{
    Stop();
}

void AutoTickThread::Initialise( DSPatch::Circuit* circuit )
{
    _circuit = circuit;
}

bool AutoTickThread::IsInitialised() const
{
    return _circuit != nullptr;
}

bool AutoTickThread::IsStopped() const
{
    return _stopped;
}

void AutoTickThread::Start()
{
    if ( _stopped )
    {
        _stop = false;
        _stopped = false;
        _pause = false;

        _thread = std::thread( &AutoTickThread::_Run, this );
        MaximiseThreadPriority( _thread.native_handle() );
    }
}

void AutoTickThread::Stop()
{
    if ( !_stopped )
    {
        _stop = true;

        while ( _stopped != true )
        {
            _pauseCondt.notify_one();
            _resumeCondt.notify_one();
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        }

        if ( _thread.joinable() )
        {
            _thread.join();
        }
    }
}

void AutoTickThread::Pause()
{
    std::unique_lock<std::mutex> lock( _resumeMutex );

    if ( !_pause )
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
        _resumeCondt.notify_one();
        _pause = false;
    }
}

void AutoTickThread::_Run()
{
    if ( _circuit != nullptr )
    {
        while ( !_stop )
        {
            _circuit->Tick();

            if ( _pause )
            {
                std::unique_lock<std::mutex> lock( _resumeMutex );

                _pauseCondt.notify_one();

                _resumeCondt.wait( lock );  // wait for resume
            }
        }
    }

    _stopped = true;
}
