#pragma once

namespace DSPatch
{

class Adder : public Component
{
public:
    Adder()
        : Component( ProcessOrder::OutOfOrder )
    {
        SetInputCount_( 2 );
        SetOutputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override
    {
        auto in0 = inputs.GetValue<int>( 0 );
        auto in1 = inputs.GetValue<int>( 1 );

        outputs.SetValue( 0, ( in0 ? *in0 : 0 ) + ( in1 ? *in1 : 0 ) );
    }
};

}  // namespace DSPatch
