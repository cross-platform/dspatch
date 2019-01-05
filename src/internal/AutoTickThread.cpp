/************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2012-2019 Marcus Tomlinson

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
