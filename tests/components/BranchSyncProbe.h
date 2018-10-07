#pragma once

namespace DSPatch
{

class BranchSyncProbe : public Component
{
public:
    BranchSyncProbe()
        : _count( 0 )
    {
        SetInputCount_( 3 );
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

        REQUIRE( *in0 == 4 + _count );
        REQUIRE( *in1 == 2 + _count );
        REQUIRE( *in2 == 1 + _count++ );
    }

private:
    int _count;
};

}  // namespace DSPatch
