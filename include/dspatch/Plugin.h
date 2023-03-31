/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2023, Marcus Tomlinson

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

#include <dspatch/Component.h>

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
Thereafter, the contained component type can be instantiated (mutiple times) via the Create()
method.
*/

class DLLEXPORT Plugin final
{
public:
    NONCOPYABLE( Plugin );

    explicit Plugin( const std::string& pluginPath );
    ~Plugin();

    bool IsLoaded() const;

    Component::SPtr Create() const;

private:
    internal::Plugin* p;
};

}  // namespace DSPatch
