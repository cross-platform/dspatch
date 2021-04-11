/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2021, Marcus Tomlinson

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

#include <internal/CircuitThread.h>

using namespace DSPatch::internal;

CircuitThread::CircuitThread()
{
}

CircuitThread::~CircuitThread()
{
    Stop();
}

void CircuitThread::Start( std::vector<DSPatch::Component::SPtr>* components, int threadNo )
{
    if ( !_stopped )
    {
        return;
    }

    _components = components;
    _threadNo = threadNo;

    _stop = false;
    _stopped = false;
    _gotResume = false;
    _gotSync = false;

    _thread = std::thread( &CircuitThread::_Run, this );

    Sync();
}

void CircuitThread::Stop()
{
    if ( _stopped )
    {
        return;
    }

    Sync();

    _stop = true;

    SyncAndResume( _mode );

    if ( _thread.joinable() )
    {
        _thread.join();
    }
}

void CircuitThread::Sync()
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

void CircuitThread::SyncAndResume( DSPatch::Component::TickMode mode )
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
    _gotSync = false;  // reset the sync flag

    _mode = mode;

    _gotResume = true;  // set the resume flag
    _resumeCondt.notify_all();
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
                _syncCondt.notify_all();

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
                    component->Tick( _mode, _threadNo );
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
