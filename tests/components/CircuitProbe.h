#pragma once

namespace DSPatch
{

class CircuitProbe : public Component
{
public:
    CircuitProbe()
        : _count( 0 )
    {
        SetInputCount_( 1 );
        SetOutputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override
    {
        auto in = inputs.GetValue<int>( 0 );
        if ( in )
        {
            REQUIRE( *in == _count++ );

            // inform the counter that the circuit is closed
            outputs.SetValue( 0, true );
        }
    }

private:
    int _count;
};

}  // namespace DSPatch
