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
        return;
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
