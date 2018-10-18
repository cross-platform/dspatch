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

#include <internal/ComponentThread.h>

#include <thread>

using namespace DSPatch::internal;

ComponentThread::ComponentThread()
    : _stop( false )
    , _pause( false )
    , _stopped( true )
{
}

ComponentThread::~ComponentThread()
{
    Stop();
}

void ComponentThread::Initialise( DSPatch::Component::SPtr const& component )
{
    _component = component;
}

bool ComponentThread::IsInitialised() const
{
    return _component.lock() != nullptr;
}

bool ComponentThread::IsStopped() const
{
    return _stopped;
}

void ComponentThread::Start( Priority priority )
{
    if ( _stopped )
    {
        _stop = false;
        _stopped = false;
        _pause = false;
        Thread::Start( priority );
    }
}

void ComponentThread::Stop()
{
    _stop = true;

    while ( _stopped != true )
    {
        _pauseCondt.notify_one();
        _resumeCondt.notify_one();
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    Thread::Stop();
}

void ComponentThread::Pause()
{
    std::unique_lock<std::mutex> lock( _resumeMutex );

    _pause = true;
    _pauseCondt.wait( lock );  // wait for resume
    _pause = false;
}

void ComponentThread::Resume()
{
    _resumeMutex.lock();
    _resumeCondt.notify_one();
    _resumeMutex.unlock();
}

void ComponentThread::Run_()
{
    if ( _component.lock() != nullptr )
    {
        while ( !_stop )
        {
            _component.lock()->Tick();
            _component.lock()->Reset();

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
