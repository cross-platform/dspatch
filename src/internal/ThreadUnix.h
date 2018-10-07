/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
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

#pragma once

namespace DSPatch
{
namespace internal
{

/// Cross-platform, object-oriented thread

/**
A class that is required to run actions in a parallel thread can be derived from Thread in order to
inherit multi-threading abilities. The Start() method initiates a parallel thread and executes the
protected virtual Run_() method in that thread. The derived class must override this Run_() method
with one that executes the required parallel actions. Upon construction, the priority for the
created thread may be selected from the public enumeration: Priority.
*/

class Thread
{
public:
    NONCOPYABLE( Thread );

    Thread()
        : _threadAttatched( false )
    {
    }

    virtual ~Thread()
    {
        Stop();
    }

    enum Priority
    {
        IdlePriority,

        LowestPriority,
        LowPriority,
        NormalPriority,
        HighPriority,
        HighestPriority,

        TimeCriticalPriority
    };

    virtual void Start( Priority priority = NormalPriority )
    {
        pthread_create( &_thread, nullptr, _ThreadFunc, this );
        _threadAttatched = true;

        _SetPriority( _thread, priority );
    }

    virtual void Stop()
    {
        if ( _threadAttatched )
        {
            pthread_detach( _thread );
            _threadAttatched = false;
        }
    }

protected:
    virtual void Run_() = 0;

private:
    static void* _ThreadFunc( void* pv )
    {
        ( reinterpret_cast<Thread*>( pv ) )->Run_();
        return nullptr;
    }

    static void _SetPriority( pthread_t threadID, Priority priority )
    {
        int policy;
        struct sched_param param;

        pthread_getschedparam( threadID, &policy, &param );

        policy = SCHED_FIFO;
        param.sched_priority = ( ( priority - IdlePriority ) * ( 99 - 1 ) / TimeCriticalPriority ) + 1;

        pthread_setschedparam( threadID, policy, &param );
    }

private:
    pthread_t _thread;
    bool _threadAttatched;
};

}  // namespace internal
}  // namespace DSPatch
