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

#ifndef DSPCIRCUIT_H
#define DSPCIRCUIT_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspComponent.h>
#include <dspatch/DspWireBus.h>
#include <dspatch/DspCircuitThread.h>

//=================================================================================================
/// Workspace for adding and routing components

/**
DspComponents can be added to a DspCircuit and routed to and from other DspComponents. When a
DspComponent is added to a DspCircuit, a unique string ID is optional. This name can be used to
identify the component within the circuit's component collection. ConnectOutToIn and
DisconnectOutToIn provide a means of routing component outputs to other component inputs, while
ConnectInToIn / DisconnectInToIn and ConnectOutToOut / DisconnectOutToOut route the circuit's IO
signals to and from it's internal components.

*N.B. Each component input can only accept one wire at a time. When another wire is connected to an
input that already has a connected wire, that wire is replaced with the new one. One output, on the
other hand, can be distributed to multiple inputs.

For process intensive circuits, multi-threaded processing can be enabled via the SetThreadCount()
method. DspCircuit allows the user to specify the number of threads in which he/she requires the
circuit to process (0 threads: multi-threading disabled). A circuit's thread count can be adjusted
at runtime, allowing the user to increase / decrease the number of threads as required during
execution.

DspCircuit is derived from DspComponent and therefore inherits all DspComponent behavior. This
means that a DspCircuit can be added to, and routed within another DspCircuit as a component. This
also means a circuit object needs to be Tick()ed and Reset()ed as a component (see DspComponent).
The DspCircuit Process_() method simply runs through it's internal array of components and calls each
component's Tick() and Reset() methods.
*/

class DLLEXPORT DspCircuit : public DspComponent
{
public:
  DspCircuit( unsigned short threadCount = 0 );
  ~DspCircuit();

  virtual void PauseAutoTick();

  void SetThreadCount( unsigned short threadCount );
  unsigned short GetThreadCount() const;

  bool AddComponent( DspComponent* component, std::string const& componentName = "" );
  bool AddComponent( DspComponent& component, std::string const& componentName = "" );

  void RemoveComponent( DspComponent const* component );
  void RemoveComponent( DspComponent const& component );
  void RemoveComponent( std::string const& componentName );

  void RemoveAllComponents();

  template< class ComponentType >
  ComponentType* GetComponent( std::string const& componentName );

  unsigned short GetComponentCount() const;

  // component output to component input
  template< class FromComponentType, class FromOutputId, class ToComponentType, class ToInputId >
  bool ConnectOutToIn( FromComponentType& fromComponent, FromOutputId const& fromOutput, ToComponentType& toComponent, ToInputId const& toInput );

  // circuit input to component input
  template< class FromInputId, class ToComponentType, class ToInputId >
  bool ConnectInToIn( FromInputId const& fromInput, ToComponentType& toComponent, ToInputId const& toInput );

  // component output to circuit output
  template< class FromComponentType, class FromOutputId, class ToOutputId >
  bool ConnectOutToOut( FromComponentType& fromComponent, FromOutputId const& fromOutput, ToOutputId const& toOutput );

  // component output to component input
  template< class FromComponentType, class FromOutputId, class ToComponentType, class ToInputId >
  void DisconnectOutToIn( FromComponentType const& fromComponent, FromOutputId const& fromOutput, ToComponentType const& toComponent, ToInputId const& toInput );

  // circuit input to component input
  template< class FromInputId, class ToComponentType, class ToInputId >
  bool DisconnectInToIn( FromInputId const& fromInput, ToComponentType const& toComponent, ToInputId const& toInput );

  // component output to circuit output
  template< class FromComponentType, class FromOutputId, class ToOutputId >
  bool DisconnectOutToOut( FromComponentType const& fromComponent, FromOutputId const& fromOutput, ToOutputId const& toOutput );

  void DisconnectComponent( std::string const& component );

  bool AddInput( std::string const& inputName = "" );
  bool AddOutput( std::string const& outputName = "" );

  void RemoveInput();
  void RemoveOutput();

  void RemoveAllInputs();
  void RemoveAllOutputs();

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs );

private:
  std::vector< DspComponent* > _components;

  std::vector< DspCircuitThread > _circuitThreads;
  unsigned short _currentThreadIndex;

  DspWireBus _inToInWires;
  DspWireBus _outToOutWires;

  bool _FindComponent( DspComponent const* component, unsigned short& returnIndex ) const;
  bool _FindComponent( DspComponent const& component, unsigned short& returnIndex ) const;
  bool _FindComponent( std::string const& componentName, unsigned short& returnIndex ) const;
  bool _FindComponent( unsigned short componentIndex, unsigned short& returnIndex ) const;

  void _DisconnectComponent( unsigned short componentIndex );
  void _RemoveComponent( unsigned short componentIndex );
};

//=================================================================================================

template< class ComponentType >
ComponentType* DspCircuit::GetComponent( std::string const& componentName )
{
  unsigned short componentIndex;

  if( _FindComponent( componentName, componentIndex ) )
  {
    return static_cast< ComponentType* >( _components[componentIndex] );
  }
  else
  {
    return NULL;
  }
}

//-------------------------------------------------------------------------------------------------

template< class FromComponentType, class FromOutputId, class ToComponentType, class ToInputId >
bool DspCircuit::ConnectOutToIn( FromComponentType& fromComponent, FromOutputId const& fromOutput, ToComponentType& toComponent, ToInputId const& toInput )
{
  unsigned short fromComponentIndex;
  unsigned short toComponentIndex;

  // only interconnect components that have been added to this system
  if( !_FindComponent( fromComponent, fromComponentIndex ) ||
      !_FindComponent( toComponent, toComponentIndex ) )
  {
    return false;
  }

  PauseAutoTick();

  bool result = _components[toComponentIndex]->ConnectInput( _components[fromComponentIndex], fromOutput, toInput );

  ResumeAutoTick();

  return result;
}

//-------------------------------------------------------------------------------------------------

template< class FromInputId, class ToComponentType, class ToInputId >
bool DspCircuit::ConnectInToIn( FromInputId const& fromInput, ToComponentType& toComponent, ToInputId const& toInput )
{
  unsigned short fromInputIndex;
  unsigned short toComponentIndex;
  unsigned short toInputIndex;

  // only interconnect components that have been added to this system
  if( !_FindInput( fromInput, fromInputIndex ) ||
      !_FindComponent( toComponent, toComponentIndex ) ||
      !_components[toComponentIndex]->_FindInput( toInput, toInputIndex ) )
  {
    return false;
  }

  PauseAutoTick();

  bool result = _inToInWires.AddWire( _components[toComponentIndex], fromInputIndex, toInputIndex );

  ResumeAutoTick();

  return result;
}

//-------------------------------------------------------------------------------------------------

template< class FromComponentType, class FromOutputId, class ToOutputId >
bool DspCircuit::ConnectOutToOut( FromComponentType& fromComponent, FromOutputId const& fromOutput, ToOutputId const& toOutput )
{
  unsigned short fromComponentIndex;
  unsigned short fromOutputIndex;
  unsigned short toOutputIndex;

  // only interconnect components that have been added to this system
  if( !_FindComponent( fromComponent, fromComponentIndex ) ||
      !_components[fromComponentIndex]->_FindOutput( fromOutput, fromOutputIndex ) ||
      !_FindOutput( toOutput, toOutputIndex ) )
  {
    return false;
  }

  PauseAutoTick();

  bool result = _outToOutWires.AddWire( _components[fromComponentIndex], fromOutputIndex, toOutputIndex );

  ResumeAutoTick();

  return result;
}

//-------------------------------------------------------------------------------------------------

template< class FromComponentType, class FromOutputId, class ToComponentType, class ToInputId >
void DspCircuit::DisconnectOutToIn( FromComponentType const& fromComponent, FromOutputId const& fromOutput, ToComponentType const& toComponent, ToInputId const& toInput )
{
  unsigned short fromComponentIndex;
  unsigned short toComponentIndex;

  // only interconnect components that have been added to this system
  if( !_FindComponent( fromComponent, fromComponentIndex ) ||
      !_FindComponent( toComponent, toComponentIndex ) )
  {
    return;
  }

  PauseAutoTick();

  _components[toComponentIndex]->DisconnectInput( _components[fromComponentIndex], fromOutput, toInput );

  ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

template< class FromInputId, class ToComponentType, class ToInputId >
bool DspCircuit::DisconnectInToIn( FromInputId const& fromInput, ToComponentType const& toComponent, ToInputId const& toInput )
{
  unsigned short fromInputIndex;
  unsigned short toComponentIndex;
  unsigned short toInputIndex;

  // only interconnect components that have been added to this system
  if( !_FindInput( fromInput, fromInputIndex ) ||
      !_FindComponent( toComponent, toComponentIndex ) ||
      !_components[toComponentIndex]->_FindInput( toInput, toInputIndex ) )
  {
    return false;
  }

  PauseAutoTick();

  bool result = _inToInWires.RemoveWire( _components[toComponentIndex], fromInputIndex, toInputIndex );

  ResumeAutoTick();

  return result;
}

//-------------------------------------------------------------------------------------------------

template< class FromComponentType, class FromOutputId, class ToOutputId >
bool DspCircuit::DisconnectOutToOut( FromComponentType const& fromComponent, FromOutputId const& fromOutput, ToOutputId const& toOutput )
{
  unsigned short fromComponentIndex;
  unsigned short fromOutputIndex;
  unsigned short toOutputIndex;

  // only interconnect components that have been added to this system
  if( !_FindComponent( fromComponent, fromComponentIndex ) ||
      !_components[fromComponentIndex]->_FindOutput( fromOutput, fromOutputIndex ) ||
      !_FindOutput( toOutput, toOutputIndex ) )
  {
    return false;
  }

  PauseAutoTick();

  bool result = _outToOutWires.RemoveWire( _components[fromComponentIndex], fromOutputIndex, toOutputIndex );

  ResumeAutoTick();

  return result;
}

//=================================================================================================

#endif // DSPCIRCUIT_H
