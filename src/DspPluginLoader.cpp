/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2015 Marcus Tomlinson

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

#include <dspatch/DspPluginLoader.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

//=================================================================================================

DspPluginLoader::DspPluginLoader(std::string const& pluginPath)
    : _pluginPath(pluginPath)
    , _handle(NULL)
{
    _LoadPlugin(_pluginPath);
}

//-------------------------------------------------------------------------------------------------

DspPluginLoader::DspPluginLoader(DspPluginLoader const& other)
    : _pluginPath(other._pluginPath)
    , _handle(NULL)
{
    _LoadPlugin(_pluginPath);
}

//-------------------------------------------------------------------------------------------------

DspPluginLoader& DspPluginLoader::operator=(const DspPluginLoader& other)
{
    _pluginPath = other._pluginPath;
    _handle = NULL;
    _LoadPlugin(_pluginPath);
    return *this;
}

//-------------------------------------------------------------------------------------------------

DspPluginLoader::~DspPluginLoader()
{
    // close library
    if (_handle)
    {
#ifdef _WIN32
        FreeLibrary((HMODULE)_handle);
#else
        dlclose(_handle);
#endif
    }
}

//=================================================================================================

bool DspPluginLoader::IsLoaded() const
{
    return _handle ? true : false;
}

//-------------------------------------------------------------------------------------------------

std::map<std::string, DspParameter> DspPluginLoader::GetCreateParams() const
{
    if (_handle)
    {
        return _getCreateParams();
    }
    return std::map<std::string, DspParameter>();
}

//-------------------------------------------------------------------------------------------------

DspComponent* DspPluginLoader::Create(std::map<std::string, DspParameter>& params) const
{
    if (_handle)
    {
        return _create(params);
    }
    return NULL;
}

//=================================================================================================

DspPluginLoader::DspPluginLoader()
{
}

//-------------------------------------------------------------------------------------------------

void DspPluginLoader::_LoadPlugin(std::string const& pluginPath)
{
    // open library
    #ifdef _WIN32
        _handle = LoadLibrary(pluginPath.c_str());
    #else
        _handle = dlopen(pluginPath.c_str(), RTLD_NOW);
    #endif

        if (_handle)
        {
    // load symbols
    #ifdef _WIN32
            _getCreateParams = (GetCreateParams_t)GetProcAddress((HMODULE)_handle, "GetCreateParams");
            _create = (Create_t)GetProcAddress((HMODULE)_handle, "Create");
    #else
            _getCreateParams = (GetCreateParams_t)dlsym(_handle, "GetCreateParams");
            _create = (Create_t)dlsym(_handle, "Create");
    #endif

            if (!_getCreateParams || !_create)
            {
    #ifdef _WIN32
                FreeLibrary((HMODULE)_handle);
    #else
                dlclose(_handle);
    #endif

                _handle = NULL;
            }
        }
}

//=================================================================================================
