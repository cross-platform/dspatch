/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2014 Marcus Tomlinson

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

#ifndef DSPPLUGIN_H
#define DSPPLUGIN_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspComponent.h>

#include <map>
#include <string>

//=================================================================================================
/// Abstract base class for DspComponent plugins

/**
A DspComponent can be packaged into a shared library (.so / .dylib / .dll) and dynamically loaded
from a DSPatch application at runtime. This allows DSPatch applications and components to be built
and released independently of each other, resulting in a highly flexible developer (and end-user)
experience. Now, in order for a DspComponent to be packaged into a plugin (shared library), the
plugin needs to expose some details about itself to the outside world. This is achieved via the
DspPlugin abstract base class.

A DspComponent plugin project must contain 2 classes: 1. A class derived from DspComponent that
implements the component logic, and 2. A class derived from DspPlugin that implements the component
construction logic.

Classes derived from DspPlugin must (at least) implement the pure virtual method: Create(). If the
DspComponent does not require any construction parameters, Create() should simply return a pointer
to a new instantiation of that DspComponent. If a DspComponent does require construction parameters
to be instantiated, the derived class must also implement the virtual GetCreateParams() method.
This method should return a map of parameter name-to-DspParameter pairs. The plugin host can then
assign values to this parameter map and return them via the Create() call.

Once both DspComponent and DspPlugin classes have been implemented, add the line
"EXPORT_DSPPLUGIN( MyPlugin )" anywhere after the declaration of your DspPlugin class, where
"MyPlugin" is the name of your DspPlugin class. Then compile the project into a shared library.

The plugin is now ready to be loaded into a DSPatch host application (see DspPluginLoader).
*/

class DLLEXPORT DspPlugin
{
public:
  virtual ~DspPlugin() {}
  virtual std::map< std::string, DspParameter > GetCreateParams() const;
  virtual DspComponent* Create( std::map< std::string, DspParameter >& params ) const = 0;
};

//=================================================================================================

#define EXPORT_DSPPLUGIN( Plugin )\
extern "C"\
{\
  DLLEXPORT std::map< std::string, DspParameter > GetCreateParams()\
  {\
    DspPlugin* plugin = new Plugin();\
    std::map< std::string, DspParameter > params  = plugin->GetCreateParams();\
    delete plugin;\
    return params;\
  }\
  DLLEXPORT DspComponent* Create( std::map< std::string, DspParameter >& params )\
  {\
    DspPlugin* plugin = new Plugin();\
    DspComponent* component = plugin->Create( params );\
    delete plugin;\
    return component;\
  }\
}

//=================================================================================================

#endif // DSPPLUGIN_H
