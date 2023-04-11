#pragma once

namespace DSPatch
{

class SerialProbe : public Component
{
public:
    SerialProbe()
        : _count( 0 )
    {
        SetInputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus& inputs, SignalBus& ) override
    {
        auto in = inputs.GetValue<int>( 0 );
        REQUIRE( in );

        REQUIRE( *in == _count + 1 + 2 + 3 + 4 + 5 );

        ++_count;
    }

private:
    int _count;
};

}  // namespace DSPatch
