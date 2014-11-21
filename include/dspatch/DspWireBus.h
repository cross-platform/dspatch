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

#ifndef DSPWIREBUS_H
#define DSPWIREBUS_H

//-------------------------------------------------------------------------------------------------

#include <vector>

#include <dspatch/DspWire.h>
#include <dspatch/DspThread.h>

class DspComponent;

//=================================================================================================
/// DspWire container

/**
A DspWireBus contains DspWires (see DspWire). Each component contains an input wire bus. Via
the Tick() method, a DspComponent uses it's input wire bus to retrieve it's input signals from
incoming linked components, as mapped out in each DspWire. The DspCircuit class has an additional 2
wire buses use to connect the circuit's IO signals to and from it's internal components.
*/

class DLLEXPORT DspWireBus
{
public:
  DspWireBus( bool isLinkedComponentReceivingSignals = false );
  virtual ~DspWireBus();

  bool AddWire( DspComponent* linkedComponent, unsigned short fromSignalIndex, unsigned short toSignalIndex );

  bool RemoveWire( unsigned short wireIndex );
  bool RemoveWire( DspComponent const* linkedComponent, unsigned short fromSignalIndex, unsigned short toSignalIndex );

  void RemoveAllWires();

  DspWire* GetWire( unsigned short wireIndex );

  unsigned short GetWireCount() const;

private:
  bool _isLinkedComponentReceivingSignals;
  std::vector< DspWire > _wires;
};

//=================================================================================================

#endif // DSPWIREBUS_H
