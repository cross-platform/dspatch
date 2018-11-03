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
