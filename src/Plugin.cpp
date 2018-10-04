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

#include <dspatch/Plugin.h>

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
            : pluginPath( pluginPath )
        {
            LoadPlugin( pluginPath );
        }

        void LoadPlugin( std::string const& pluginPath );

        typedef DSPatch::Component::SPtr ( *Create_t )();

        std::string pluginPath;
        void* handle = nullptr;
        Create_t create;
    };
}
}

Plugin::Plugin( std::string const& pluginPath )
    : p( new internal::Plugin( pluginPath ) )
{
}

Plugin::Plugin( Plugin const& other )
    : p( new internal::Plugin( other.p->pluginPath ) )
{
}

Plugin& Plugin::operator=( const Plugin& other )
{
    p->pluginPath = other.p->pluginPath;
    p->handle = other.p->handle;
    p->create = other.p->create;
    return *this;
}

Plugin::~Plugin()
{
    // close library
    if ( p->handle )
    {
#ifdef _WIN32
        FreeLibrary( ( HMODULE )p->handle );
#else
        dlclose( p->handle );
#endif
    }
}

bool Plugin::IsLoaded() const
{
    return p->handle ? true : false;
}

Component::SPtr Plugin::Create() const
{
    if ( p->handle )
    {
        return p->create();
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
        create = ( Create_t )GetProcAddress( ( HMODULE )handle, "Create" );
#else
        create = ( Create_t )dlsym( handle, "Create" );
#endif

        if ( !create )
        {
#ifdef _WIN32
            FreeLibrary( ( HMODULE )handle );
#else
            dlclose( handle );
#endif

            handle = nullptr;
        }
    }
}
