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

#include <memory>

#define NONCOPYABLE( classname )            \
    classname( classname const& ) = delete; \
    classname& operator=( classname const& ) = delete

#define DEFINE_PTRS( classname )                    \
    using SPtr = std::shared_ptr<classname>;        \
    using SCPtr = std::shared_ptr<classname const>; \
    using UPtr = std::unique_ptr<classname>;        \
    using UCPtr = std::unique_ptr<classname const>

#define EXPORT_PLUGIN( classname, ... )          \
    extern "C"                                   \
    {                                            \
        DLLEXPORT Component* Create()            \
        {                                        \
            return new classname( __VA_ARGS__ ); \
        }                                        \
    }

#ifdef _WIN32

#define DLLEXPORT __declspec( dllexport )

#pragma warning( disable : 4251 )  // disable class needs to have dll-interface warning

#else

#define DLLEXPORT

#endif
