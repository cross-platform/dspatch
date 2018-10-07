/************************************************************************
DSPatch - C++ Flow-Based Programming Framework
Copyright (c) 2012-2018 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LGPLv3.txt included in the packaging of this
file. Please review the following information to ensure the GNU Lesser
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

#include <internal/CircuitThread.h>

#include <thread>

using namespace DSPatch::internal;

CircuitThread::CircuitThread()
    : _threadNo( 0 )
    , _stop( false )
    , _stopped( true )
    , _gotResume( false )
    , _gotSync( false )
{
}

CircuitThread::~CircuitThread()
{
    Stop();
}

void CircuitThread::Initialise( std::shared_ptr<std::vector<DSPatch::Component::SPtr>> const& components, int threadNo )
{
    _components = components;
    _threadNo = threadNo;
}

void CircuitThread::Start( Priority priority )
{
    if ( _stopped )
    {
        _stop = false;
        _stopped = false;
        _gotResume = false;
        _gotSync = true;
        Thread::Start( priority );
    }
}

void CircuitThread::Stop()
{
    _stop = true;

    while ( _stopped != true )
    {
        _syncCondt.notify_one();
        _resumeCondt.notify_one();
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    Thread::Stop();
}

void CircuitThread::Sync()
{
    std::unique_lock<std::mutex> lock( _resumeMutex );

    if ( !_gotSync )  // if haven't already got sync
    {
        _syncCondt.wait( lock );  // wait for sync
    }
}

void CircuitThread::Resume()
{
    _resumeMutex.lock();

    _gotSync = false;  // reset the sync flag

    _gotResume = true;  // set the resume flag
    _resumeCondt.notify_one();

    _resumeMutex.unlock();
}

void CircuitThread::Run_()
{
    if ( _components.lock() != nullptr )
    {
        while ( !_stop )
        {
            {
                std::unique_lock<std::mutex> lock( _resumeMutex );

                _gotSync = true;  // set the sync flag

                _syncCondt.notify_one();

                if ( !_gotResume )  // if haven't already got resume
                {
                    _resumeCondt.wait( lock );  // wait for resume
                }
                _gotResume = false;  // reset the resume flag
            }

            if ( !_stop )
            {
                for ( size_t i = 0; i < _components.lock()->size(); i++ )
                {
                    ( *_components.lock() )[i]->_ThreadTick( _threadNo );
                }
                for ( size_t i = 0; i < _components.lock()->size(); i++ )
                {
                    ( *_components.lock() )[i]->_ThreadReset( _threadNo );
                }
            }
        }
    }

    _stopped = true;
}
