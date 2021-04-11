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
