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

#ifndef DSPCOMPONENT_H
#define DSPCOMPONENT_H

//-------------------------------------------------------------------------------------------------

#include <dspatch/DspSignalBus.h>
#include <dspatch/DspWireBus.h>
#include <dspatch/DspComponentThread.h>

class DspCircuit;

//=================================================================================================
/// Abstract base class for all DSPatch components

/**
Classes derived from DspComponent can be added to an DspCircuit and routed to and from other
DspComponents. On construction, derived classes must configure the component's IO buses by calling
AddInput_() and AddOutput_() respectively. Derived classes must also implement the virtual method:
Process_(). The Process_() method is a callback from the DSPatch engine that occurs when a new set
of input signals is ready for processing. The Process_() method has 2 parameters: the input bus and
the output bus. This method's purpose is to pull its required inputs out of the input bus, process
these inputs, and populate the output bus with the results (see DspSignalBus).

In order for a component to do any work it must be ticked over. This is performed by repeatedly
calling the Tick() and Reset() methods. The Tick() method is responsible for acquiring the next set
of input signals from component input wires and populating the component's input bus. To insure
that these inputs are up-to-date, the dependent component first calls all of its input components'
Tick() methods -hence recursively called in all components going backward through the circuit (This
is what's classified as a "pull system"). The acquired input bus is then passed to the Process_()
method. The Reset() method then informs the component that the last circuit traversal has completed
and hence can execute the next Tick() request. A component's Tick() and Reset() methods can be
called in a loop from the main application thread, or alternatively, by calling StartAutoTick(), a
seperate thread will spawn, automatically calling Tick() and Reset() methods continuously (This is
most commonly used to tick over an instance of DspCircuit).
*/

class DLLEXPORT DspComponent
{
  friend class DspCircuit;
  friend class DspCircuitThread;

public:
  DspComponent();
  virtual ~DspComponent();

  void SetComponentName( std::string componentName );
  std::string GetComponentName() const;

  template< class FromOutputType, class ToInputType >
  bool ConnectInput( DspComponent* fromComponent, FromOutputType fromOutput, ToInputType toInput );

  template< class FromOutputType, class ToInputType >
  bool ConnectInput( DspComponent& fromComponent, FromOutputType fromOutput, ToInputType toInput );

  template< class FromOutputType, class ToInputType >
  void DisconnectInput( DspComponent* fromComponent, FromOutputType fromOutput, ToInputType toInput );

  template< class FromOutputType, class ToInputType >
  void DisconnectInput( DspComponent& fromComponent, FromOutputType fromOutput, ToInputType toInput );

  void DisconnectInput( unsigned short inputIndex );
  void DisconnectInput( std::string inputName );
  void DisconnectInput( DspComponent* inputComponent );
  void DisconnectInputs();

  unsigned short GetInputCount() const;
  unsigned short GetOutputCount() const;

  void Tick();
  void Reset();

  virtual void StartAutoTick();
  virtual void StopAutoTick();
  virtual void PauseAutoTick();
  virtual void ResumeAutoTick();

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs ) {};

  bool AddInput_( std::string inputName = "" );
  bool AddOutput_( std::string outputName = "" );

  void RemoveInputs_();
  void RemoveOutputs_();

private:
  void _SetParentCircuit( DspCircuit* parentCircuit );
  DspCircuit* _GetParentCircuit();

  bool _FindInput( std::string signalName, unsigned short& returnIndex ) const;
  bool _FindInput( unsigned short signalIndex, unsigned short& returnIndex ) const;
  bool _FindOutput( std::string signalName, unsigned short& returnIndex ) const;
  bool _FindOutput( unsigned short signalIndex, unsigned short& returnIndex ) const;

  void _SetBufferCount( unsigned short bufferCount );
  unsigned short _GetBufferCount() const;

  void _ThreadTick( unsigned short threadNo );
  void _ThreadReset( unsigned short threadNo );

  bool _SetInputSignal( unsigned short inputIndex, const DspSignal* newSignal );
  bool _SetInputSignal( unsigned short inputIndex, unsigned short threadIndex, const DspSignal* newSignal );
  DspSignal* _GetOutputSignal( unsigned short outputIndex );
  DspSignal* _GetOutputSignal( unsigned short outputIndex, unsigned short threadIndex );

  void _WaitForRelease( unsigned short threadNo );
  void _ReleaseThread( unsigned short threadNo );

private:
  DspCircuit* _parentCircuit;

  unsigned short _bufferCount;

  DspSignalBus _inputBus;
  DspSignalBus _outputBus;

  std::vector< DspSignalBus > _inputBuses;
  std::vector< DspSignalBus > _outputBuses;

  std::string _componentName;
  bool _isAutoTickRunning;
  bool _isAutoTickPaused;

  DspWireBus _inputWires;

  bool _hasTicked;

  DspComponentThread _componentThread;

  std::vector< bool* > _hasTickeds; // bool pointers ensure that parallel threads will only read from this vector
  std::vector< bool > _gotReleases; // bool pointers not used here as only 1 thread writes to this vector at a time
  std::vector< DspMutex > _releaseMutexes;
  std::vector< DspWaitCondition > _releaseCondts;
};

//=================================================================================================

template< class FromOutputType, class ToInputType >
bool DspComponent::ConnectInput( DspComponent* fromComponent, FromOutputType fromOutput, ToInputType toInput )
{
  unsigned short fromOutputIndex;
  unsigned short toInputIndex;

  if( !fromComponent->_outputBus.FindSignal( fromOutput, fromOutputIndex ) ||
      !_inputBus.FindSignal( toInput, toInputIndex ) )
  {
    return false;
  }

  PauseAutoTick();
  _inputWires.AddWire( fromComponent, fromOutputIndex, toInputIndex );
  ResumeAutoTick();

  return true;
}

//-------------------------------------------------------------------------------------------------

template< class FromOutputType, class ToInputType >
bool DspComponent::ConnectInput( DspComponent& fromComponent, FromOutputType fromOutput, ToInputType toInput )
{
  return ConnectInput( &fromComponent, fromOutput, toInput );
}

//-------------------------------------------------------------------------------------------------

template< class FromOutputType, class ToInputType >
void DspComponent::DisconnectInput( DspComponent* fromComponent, FromOutputType fromOutput, ToInputType toInput )
{
  unsigned short fromOutputIndex;
  unsigned short toInputIndex;

  if( !fromComponent->_outputBus.FindSignal( fromOutput, fromOutputIndex ) ||
      !_inputBus.FindSignal( toInput, toInputIndex ) )
  {
    return;
  }

  PauseAutoTick();
  _inputWires.RemoveWire( fromComponent, fromOutputIndex, toInputIndex );
  ResumeAutoTick();
}

//-------------------------------------------------------------------------------------------------

template< class FromOutputType, class ToInputType >
void DspComponent::DisconnectInput( DspComponent& fromComponent, FromOutputType fromOutput, ToInputType toInput )
{
  DisconnectInput( &fromComponent, fromOutput, toInput );
}

//=================================================================================================

#endif // DSPCOMPONENT_H
