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

#include <dspatch/Plugin.h>

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class Plugin
{
public:
    explicit Plugin( std::string const& pluginPath )
    {
        LoadPlugin( pluginPath );
    }

    void LoadPlugin( std::string const& pluginPath );

    typedef DSPatch::Component* ( *Create_t )();

    void* handle = nullptr;
    Create_t create = nullptr;
};

}  // namespace internal
}  // namespace DSPatch

Plugin::Plugin( std::string const& pluginPath )
    : p( new internal::Plugin( pluginPath ) )
{
}

Plugin::~Plugin()
{
    // close library
    if ( p->handle )
    {
#ifdef _WIN32
        FreeLibrary( (HMODULE)p->handle );
#else
        dlclose( p->handle );
#endif
    }
}

// cppcheck-suppress unusedFunction
bool Plugin::IsLoaded() const
{
    return p->handle;
}

// cppcheck-suppress unusedFunction
Component::SPtr Plugin::Create() const
{
    if ( p->handle )
    {
        return Component::SPtr( p->create() );
    }
    return nullptr;
}

void internal::Plugin::LoadPlugin( std::string const& pluginPath )
{
    // open library
#ifdef _WIN32
    handle = LoadLibrary( pluginPath.c_str() );
#else
    handle = dlopen( pluginPath.c_str(), RTLD_NOW );
#endif

    if ( handle )
    {
        // load symbols
#ifdef _WIN32
        create = (Create_t)GetProcAddress( (HMODULE)handle, "Create" );
#else
        create = (Create_t)dlsym( handle, "Create" );
#endif

        if ( !create )
        {
#ifdef _WIN32
            FreeLibrary( (HMODULE)handle );
#else
            dlclose( handle );
#endif

            handle = nullptr;
        }
    }
}
