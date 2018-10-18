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

#pragma once

#include <windows.h>

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
        : _threadHandle( nullptr )
    {
    }

    virtual ~Thread()
    {
        Stop();
    }

    enum Priority
    {
        IdlePriority = -15,

        LowestPriority = -2,
        LowPriority = -1,
        NormalPriority = 0,
        HighPriority = 1,
        HighestPriority = 2,

        TimeCriticalPriority = 15
    };

    virtual void Start( Priority priority = NormalPriority )
    {
        DWORD threadId;
        _threadHandle = CreateThread( nullptr, 0, _ThreadFunc, this, CREATE_SUSPENDED, &threadId );
        SetThreadPriority( _threadHandle, priority );
        ResumeThread( _threadHandle );
    }

    virtual void Stop()
    {
        CloseHandle( _threadHandle );
        _threadHandle = nullptr;
    }

protected:
    virtual void Run_() = 0;

private:
    static DWORD WINAPI _ThreadFunc( LPVOID pv )
    {
        ( reinterpret_cast<Thread*>( pv ) )->Run_();
        return 0;
    }

private:
    HANDLE _threadHandle;
};

}  // namespace internal
}  // namespace DSPatch
