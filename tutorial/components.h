#include <DSPatch.h>

#include <time.h>
#include <iostream>
#include <stdlib.h>

//=================================================================================================
// DspAnd:
// DspAnd has 2 inputs and 1 output.
// This component performs a logic AND on 2 boolean input values and outputs the result.

// 1. Derive component class from DspComponent
// ===========================================
class DspAnd : public DspComponent
{
public:
  // 2. Configure component IO buses
  // ===============================
  DspAnd()
  {
    // add 2 inputs
    AddInput_( "input1" );
    AddInput_( "input2" );

    // add 1 output
    AddOutput_( "output" );
  }

protected:
  // 3. Implement virtual Process_() method
  // ======================================
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs )
  {
    // create local stack variables to hold input values
    bool bool1 = false;
    bool bool2 = false;

    // get values from inputs bus ( GetValue() returns true if successful )
    if( inputs.GetValue( 0, bool1 ) && //OR inputs.GetValue( "input1", bool1 );
        inputs.GetValue( 1, bool2 ) )  //OR inputs.GetValue( "input2", bool2 );
    {
      // set output as the result of bool1 AND bool2
      outputs.SetValue( 0, bool1 && bool2 );	//OR outputs.SetValue( "output", bool1 && bool2 );
    }
  }
};

//=================================================================================================

// DspRandBool:
// DspRandBool has 1 output.
// This component generates a random boolean value then outputs the result.

class DspRandBool : public DspComponent
{
public:
  DspRandBool()
  {
    // add 1 output
    AddOutput_();

    // seed randomizer
    srand( ( unsigned int ) time( NULL ) );
  }

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs )
  {
    // set output as randomized true / false
    outputs.SetValue( 0, rand() % 2 == 0 );
  }
};

//=================================================================================================

// DspPrintBool:
// DspPrintBool has 1 input.
// This component receives a boolean value and outputs it to the console.

class DspPrintBool : public DspComponent
{
public:
  DspPrintBool()
  {
    // add 1 input
    AddInput_();
  }

protected:
  virtual void Process_( DspSignalBus& inputs, DspSignalBus& outputs )
  {
    // create a local stack variable to hold input value
    bool inputBool;

    // get boolean value from inputs bus
    if( inputs.GetValue( 0, inputBool ) )
    {
      // print "true" / "false" depending on boolean value received
      if( inputBool )
      {
        std::cout << "true" << '\n';
      }
      else
      {
        std::cout << "false" << '\n';
      }
    }
  }
};

//=================================================================================================
