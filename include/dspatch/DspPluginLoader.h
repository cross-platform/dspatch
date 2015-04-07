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

#ifndef DSPPLUGINLOADER_H
#define DSPPLUGINLOADER_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspPlugin.h>

//=================================================================================================
/// Plugin loader for DspPlugin host applications

/**
A DspComponent packaged into a shared library (.so / .dylib / .dll) and exported via the DspPlugin
class can be dynamically loaded from a DSPatch application at runtime using a DspPluginLoader. A
DspPluginLoader represents exactly one DspPlugin in a host application.

A DspPluginLoader should be constructed with the absolute path of the plugin (shared library) to be
loaded. Once instantiated you should check that the plugin was successfully loaded by calling
IsLoaded(). Thereafter, the contained DspComponent can be instantiated via the GetCreateParams()
and Create() methods accordingly (For more detail on the structure of a plugin, see DspPlugin).
*/

class DLLEXPORT DspPluginLoader : public DspPlugin
{
public:
    DspPluginLoader(std::string const& pluginPath);
    DspPluginLoader(DspPluginLoader const& other);
    DspPluginLoader& operator=(const DspPluginLoader& other);
    ~DspPluginLoader();

    bool IsLoaded() const;

    std::map<std::string, DspParameter> GetCreateParams() const;
    DspComponent* Create(std::map<std::string, DspParameter>& params) const;

private:
    DspPluginLoader();
    void _LoadPlugin(std::string const& pluginPath);

private:
    typedef std::map<std::string, DspParameter>(*GetCreateParams_t)();
    typedef DspComponent* (*Create_t)(std::map<std::string, DspParameter>&);

    std::string _pluginPath;
    void* _handle;
    GetCreateParams_t _getCreateParams;
    Create_t _create;
};

//=================================================================================================

#endif  // DSPPLUGINLOADER_H
