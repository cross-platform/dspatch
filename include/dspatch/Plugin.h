/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2024, Marcus Tomlinson

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

#include "Component.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#define DLLEXPORT __declspec( dllexport )
#else
#include <dlfcn.h>
#define DLLEXPORT
#endif

#include <string>

#define EXPORT_PLUGIN( classname, ... )          \
    extern "C"                                   \
    {                                            \
        DLLEXPORT DSPatch::Component* Create()   \
        {                                        \
            return new classname( __VA_ARGS__ ); \
        }                                        \
    }

namespace DSPatch
{

/// Component plugin loader

/**
A component, packaged into a shared library (.so / .dylib / .dll) and exported via the EXPORT_PLUGIN macro, can be dynamically
loaded into a host application using the Plugin class. Each Plugin object represents one Component class. (<a
href="https://github.com/cross-platform/dspatchables/tree/master/Components"><b>Example component plugins</b></a>)

A Plugin should be constructed with the absolute path of the plugin (shared library) to be loaded. Once instantiated you should
check that the plugin was successfully loaded by calling IsLoaded(). Thereafter, the contained component type can be instantiated
(mutiple times) via the Create() method.
*/

class Plugin final
{
public:
    Plugin( const Plugin& ) = delete;
    Plugin& operator=( const Plugin& ) = delete;

    explicit Plugin( const std::string& pluginPath );
    ~Plugin();

    bool IsLoaded() const;

    Component::SPtr Create() const;

private:
    typedef DSPatch::Component* ( *Create_t )();

    void* _handle = nullptr;
    Create_t _create = nullptr;
};

inline Plugin::Plugin( const std::string& pluginPath )
{
    // open library
#ifdef _WIN32
    _handle = LoadLibrary( pluginPath.c_str() );
#else
    _handle = dlopen( pluginPath.c_str(), RTLD_NOW );
#endif

    if ( _handle )
    {
        // load symbols
#ifdef _WIN32
        _create = (Create_t)GetProcAddress( (HMODULE)_handle, "Create" );
#else
        _create = (Create_t)dlsym( _handle, "Create" );
#endif

        if ( !_create )
        {
#ifdef _WIN32
            FreeLibrary( (HMODULE)_handle );
#else
            dlclose( _handle );
#endif

            _handle = nullptr;
        }
    }
}

inline Plugin::~Plugin()
{
    // close library
    if ( _handle )
    {
#ifdef _WIN32
        FreeLibrary( (HMODULE)_handle );
#else
        dlclose( _handle );
#endif
    }
}

// cppcheck-suppress unusedFunction
inline bool Plugin::IsLoaded() const
{
    return _handle;
}

// cppcheck-suppress unusedFunction
inline Component::SPtr Plugin::Create() const
{
    if ( _handle )
    {
        return Component::SPtr( _create() );
    }
    return nullptr;
}

}  // namespace DSPatch
