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

#include <internal/CircuitThread.h>

using namespace DSPatch::internal;

CircuitThread::CircuitThread()
{
}

CircuitThread::~CircuitThread()
{
    Stop();
}

void CircuitThread::Initialise( std::vector<DSPatch::Component::SPtr>* components, int threadNo )
{
    _components = components;
    _threadNo = threadNo;
}

void CircuitThread::Start()
{
    if ( _stopped )
    {
        _stop = false;
        _stopped = false;
        _gotResume = false;
        _gotSync = true;

        _thread = std::thread( &CircuitThread::_Run, this );
    }
}

void CircuitThread::Stop()
{
    if ( !_stopped )
    {
        _stop = true;

        while ( !_stopped )
        {
            _syncCondt.notify_one();
            _resumeCondt.notify_one();
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        }

        if ( _thread.joinable() )
        {
            _thread.join();
        }
    }
}

void CircuitThread::Sync()
{
    std::unique_lock<std::mutex> lock( _resumeMutex );

    if ( !_gotSync )  // if haven't already got sync
    {
        _syncCondt.wait( lock );  // wait for sync
    }
}

void CircuitThread::SyncAndResume()
{
    std::unique_lock<std::mutex> lock( _resumeMutex );

    if ( !_gotSync )  // if haven't already got sync
    {
        _syncCondt.wait( lock );  // wait for sync
    }
    _gotSync = false;  // reset the sync flag

    _gotResume = true;  // set the resume flag
    _resumeCondt.notify_one();
}

void CircuitThread::_Run()
{
    if ( _components != nullptr )
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
                // You might be thinking: Can't we have each thread start on a different component?

                // Well no. Because threadNo == bufferNo, in order to maintain synchronisation
                // within the circuit, when a component wants to process its buffers in-order, it
                // requires that every other in-order component in the system has not only
                // processed its buffers in the same order, but has processed the same number of
                // buffers too.

                // E.g. 1,2,3 and 1,2,3. Not 1,2,3 and 2,3,1,2,3.

                for ( auto& component : *_components )
                {
                    component->Tick( _threadNo );
                }
                for ( auto& component : *_components )
                {
                    component->Reset( _threadNo );
                }
            }
        }
    }

    _stopped = true;
}
