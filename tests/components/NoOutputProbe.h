#pragma once

namespace DSPatch
{

class NoOutputProbe : public Component
{
public:
    NoOutputProbe()
        : _count( 0 )
    {
        SetInputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& ) override
    {
        auto in = inputs.GetValue<int>( 0 );

        if ( in )
        {
            REQUIRE( *in == _count++ );
        }
    }

private:
    int _count;
};

}  // namespace DSPatch
