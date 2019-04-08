#pragma once

namespace DSPatch
{

class NullInputProbe : public Component
{
public:
    NullInputProbe()
    {
        SetInputCount_( 2 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& ) override
    {
        auto in0 = inputs.GetValue<int>( 0 );
        REQUIRE( in0 == nullptr );

        auto in1 = inputs.GetValue<int>( 1 );
        REQUIRE( in1 == nullptr );
    }
};

}  // namespace DSPatch
