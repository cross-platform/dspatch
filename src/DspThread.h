/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2013 Marcus Tomlinson

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

#ifdef _WIN32

  #define DLLEXPORT __declspec(dllexport)
  #include "DspThreadWin.h"

  #pragma warning(disable:4251) // disable class needs to have dll-interface warning
  #pragma warning(disable:4275) // disable non dll-interface class used as base warning

#elif DSP_NOTHREADS

  #define DLLEXPORT
  #include "DspThreadNull.h"

#else

  #define DLLEXPORT
  #include "DspThreadUnix.h"

#endif
