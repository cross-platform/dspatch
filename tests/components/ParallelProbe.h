#pragma once

namespace DSPatch
{

class ParallelProbe : public Component
{
public:
    ParallelProbe()
        : _count( 0 )
    {
        SetInputCount_( 5 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& ) override
    {
        auto in0 = inputs.GetValue<int>( 0 );
        REQUIRE( in0 != nullptr );
        auto in1 = inputs.GetValue<int>( 1 );
        REQUIRE( in1 != nullptr );
        auto in2 = inputs.GetValue<int>( 2 );
        REQUIRE( in2 != nullptr );
        auto in3 = inputs.GetValue<int>( 3 );
        REQUIRE( in3 != nullptr );
        auto in4 = inputs.GetValue<int>( 4 );
        REQUIRE( in4 != nullptr );

        REQUIRE( *in0 == _count + 1 );
        REQUIRE( *in1 == _count + 2 );
        REQUIRE( *in2 == _count + 3 );
        REQUIRE( *in3 == _count + 4 );
        REQUIRE( *in4 == _count + 5 );

        ++_count;
    }

private:
    int _count;
};

}  // namespace DSPatch
