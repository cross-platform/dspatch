#include "DSPatch.h"
#include "components.h"
#include <stdio.h>

//=================================================================================================

int main()
{
  // 1. Create a DspCircuit where we can route our components
  // ========================================================
  DspCircuit circuit;

  // 2. Create instances of the components needed for our circuit
  // ============================================================
  DspRandBool randBoolGen1;
  DspRandBool randBoolGen2;
  DspAnd logicAnd;
  DspPrintBool boolPrinter;

  // 3. Add component instances to circuit
  // =====================================
  circuit.AddComponent( randBoolGen1, "Bool Generator 1" );
  circuit.AddComponent( randBoolGen2, "Bool Generator 2" );
  circuit.AddComponent( logicAnd, "Logic AND" );
  circuit.AddComponent( boolPrinter, "Bool Printer" );

  // 4. Wire up the components inside the circuit
  // ============================================

  circuit.ConnectOutToIn( randBoolGen1, 0, logicAnd, 0 );
  //OR circuit.ConnectOutToIn( "Bool Generator 1", 0, "Logic AND", 0 );
  //OR circuit.ConnectOutToIn( "Bool Generator 1", 0, "Logic AND", "input1" );

  circuit.ConnectOutToIn( randBoolGen2, 0, logicAnd, 1 );
  //OR circuit.ConnectOutToIn( "Bool Generator 2", 0, "Logic AND", 1 );
  //OR circuit.ConnectOutToIn( "Bool Generator 2", 0, "Logic AND", "input2" );

  circuit.ConnectOutToIn( logicAnd, 0, boolPrinter, 0 );
  //OR circuit.ConnectOutToIn( "Logic AND", 0, "Bool Printer", 0 );
  //OR circuit.ConnectOutToIn( "Logic AND", "output", "Bool Printer", 0 );

  // 5. Tick the circuit
  // ===================

  // Circuit tick method 1: Manual
  for( unsigned short i = 0; i < 10; i++ )
  {
    circuit.Tick();
    circuit.Reset();
  }

  // Circuit tick method 2: Automatic
  std::cout << "Press any key to begin circuit auto-tick.";
  getchar();
  circuit.StartAutoTick();

  // Increase circuit thread count for higher performance
  getchar();
  circuit.SetThreadCount( 4 );

  // Press any key to quit
  getchar();

  // 6. Clean up
  // ===========
  DSPatch::Finalize();

  return 0;
}

//=================================================================================================
