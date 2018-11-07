#pragma once

#include <DSPatch.h>

#include <cstdlib>
#include <ctime>
#include <iostream>

namespace DSPatch
{

// And:
// And has 2 inputs and 1 output.
// This component performs a logic AND on 2 boolean input values and outputs the result.

// 1. Derive And class from Component
// ==================================
class And final : public Component
{
public:
    // 2. Configure component IO buses
    // ===============================
    And()
    {
        // add 2 inputs
        SetInputCount_( 2 );

        // add 1 output
        SetOutputCount_( 1 );
    }

protected:
    // 3. Implement virtual Process_() method
    // ======================================
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override
    {
        // create some local pointers to hold our input values
        auto bool1 = inputs.GetValue<bool>( 0 );
        auto bool2 = inputs.GetValue<bool>( 1 );

        // check first that our component has received valid inputs
        if ( bool1 && bool2 )
        {
            // set the output as the result of bool1 AND bool2
            outputs.SetValue( 0, *bool1 && *bool2 );
        }
    }
};

// RandBool:
// RandBool has 1 output.
// This component generates a random boolean value then outputs the result.

class RandBool final : public Component
{
public:
    RandBool()
    {
        // add 1 output
        SetOutputCount_( 1 );

        // seed randomizer
        srand( static_cast<unsigned int>( time( nullptr ) ) );
    }

protected:
    virtual void Process_( SignalBus const&, SignalBus& outputs ) override
    {
        // set output as randomized true / false
        outputs.SetValue( 0, rand() % 2 == 0 );
    }
};

// PrintBool:
// PrintBool has 1 input.
// This component receives a boolean value and outputs it to the console.

class PrintBool final : public Component
{
public:
    PrintBool()
    {
        // add 1 input
        SetInputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& ) override
    {
        // create a local stack variable to hold input value
        auto inputBool = inputs.GetValue<bool>( 0 );

        // get boolean value from inputs bus
        if ( inputBool )
        {
            // print "true" / "false" depending on boolean value received
            if ( *inputBool )
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

}  // namespace DSPatch
