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

#ifndef DSPPLUGINLOADER_H
#define DSPPLUGINLOADER_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspComponent.h>
#include <dspatch/DspPlugin.h>

#include <map>
#include <string>

//=================================================================================================
/// TODO

/**
///! TODO
*/

class DLLEXPORT DspPluginLoader : public DspPlugin
{
public:
  DspPluginLoader( std::string const& pluginPath );
  ~DspPluginLoader();

  bool IsLoaded() const;

  std::map< std::string, DspParameter > GetCreateParams() const;
  DspComponent* Create( std::map< std::string, DspParameter >& params ) const;

private:
  typedef std::map< std::string, DspParameter >( *GetCreateParams_t )();
  typedef DspComponent*( *Create_t )( std::map< std::string, DspParameter > const& );

  void* _handle;
  GetCreateParams_t _getCreateParams;
  Create_t _create;
};

//=================================================================================================

#endif // DSPPLUGINLOADER_H
