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

#include <dspatch/Component.h>

#include <map>

namespace DSPatch
{

namespace internal
{
class Plugin;
}

/// Component plugin loader

/**
A component, packaged into a shared library (.so / .dylib / .dll) and exported via the
EXPORT_PLUGIN macro, can be dynamically loaded into a host application using the Plugin class. Each
Plugin object represents one Component class.

A Plugin should be constructed with the absolute path of the plugin (shared library) to be loaded.
Once instantiated you should check that the plugin was successfully loaded by calling IsLoaded().
Thereafter, the contained component can be instantiated (mutiple times) via the Create() method.
*/

class DLLEXPORT Plugin final
{
public:
    DEFINE_PTRS( Plugin );
    NONCOPYABLE( Plugin );

    Plugin( std::string const& pluginPath );
    ~Plugin();

    bool IsLoaded() const;

    Component::SPtr Create() const;

private:
    std::unique_ptr<internal::Plugin> p;
};

}  // namespace DSPatch
