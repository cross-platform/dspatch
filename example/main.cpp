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

#include "../include/DSPatch.h"

#include "DspWaveStreamer.h"
#include "DspGain.h"
#include "DspAudioDevice.h"
#include "DspOscillator.h"
#include "DspAdder.h"

#include <stdio.h>

//=================================================================================================
// This is a simple program that streams an wave out of an audio device,
// then overlays a 1KHz oscillator when a key is pressed.

int main()
{
  // 1. Stream Wave
  // ==============

  // create a circuit 
  DspCircuit circuit;

  // declare components to be added to the circuit
  DspWaveStreamer waveStreamer;
  DspAudioDevice audioDevice;
  DspGain gainLeft;
  DspGain gainRight;

  // set circuit thread count to 2
  circuit.SetThreadCount( 2 );

  //// start separate thread to tick the circuit continuously ("auto-tick")
  circuit.StartAutoTick();

  // add new components to the circuit (these methods return pointers to the new components)
  circuit.AddComponent( waveStreamer );
  circuit.AddComponent( audioDevice );
  circuit.AddComponent( gainLeft );
  circuit.AddComponent( gainRight );

  // DspWaveStreamer has an output signal named "Sample Rate" that streams the current wave's sample rate
  // DspAudioDevice's "Sample Rate" input receives a sample rate value and updates the audio stream accordingly 
  circuit.ConnectOutToIn( waveStreamer, "Sample Rate", audioDevice, "Sample Rate" ); // sample rate sync

  // connect component output signals to respective component input signals
  circuit.ConnectOutToIn( waveStreamer, 0, gainLeft, 0 );   // wave left channel into gain left
  circuit.ConnectOutToIn( waveStreamer, 1, gainRight, 0 );  // wave right channel into gain right
  circuit.ConnectOutToIn( gainLeft, 0, audioDevice, 0 );    // gain left into audio device left channel
  circuit.ConnectOutToIn( gainRight, 0, audioDevice, 1 );   // gain right into audio device right channel

  // set the gain of components gainLeft and gainRight (wave left and right channels)
  gainLeft.SetGain( 0.75 );
  gainRight.SetGain( 0.75 );

  // load a wave into the wave streamer and start playing the track
  waveStreamer.LoadFile( "../Tchaikovski-Swan-Lake-Scene.wav" );
  waveStreamer.Play();

  // wait for key press
  getchar();

  // 2. Overlay oscillator
  // =====================

  // A component input pin can only receive one signal at a time so an adders are required to combine the signals

  // declare components to be added to the circuit
  DspOscillator oscillator( 1000.0f, 0.1f );
  DspAdder adder1;
  DspAdder adder2;

  // add new components to the circuit
  circuit.AddComponent( oscillator );
  circuit.AddComponent( adder1 );
  circuit.AddComponent( adder2 );

  // DspOscillator's "Sample Rate" / "Buffer Size" input values are used to re-build its wave table accordingly
  circuit.ConnectOutToIn( waveStreamer, "Sample Rate", oscillator, "Sample Rate" ); // sample rate sync
  circuit.ConnectOutToIn( waveStreamer, 0, oscillator, "Buffer Size" );             // buffer size sync

  // connect component output signals to respective component input signals
  circuit.ConnectOutToIn( gainLeft, 0, adder1, 0 );     // wave left channel into adder1 ch0
  circuit.ConnectOutToIn( oscillator, 0, adder1, 1 );   // oscillator output into adder1 ch1
  circuit.ConnectOutToIn( adder1, 0, audioDevice, 0 );  // adder1 output into audio device left channel

  circuit.ConnectOutToIn( gainRight, 0, adder2, 0 );    // wave right channel into adder2 ch0
  circuit.ConnectOutToIn( oscillator, 0, adder2, 1 );   // oscillator output into adder2 ch1
  circuit.ConnectOutToIn( adder2, 0, audioDevice, 1 );  // adder2 output into audio device right channel

  // wait for key press
  getchar();

  // clean up DSPatch
  DSPatch::Finalize();

  return 0;
}

//=================================================================================================
