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
    Plugin( std::string const& pluginPath )
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

bool Plugin::IsLoaded() const
{
    return p->handle;
}

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
