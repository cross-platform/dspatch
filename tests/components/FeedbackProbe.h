#pragma once

namespace DSPatch
{

class FeedbackProbe : public Component
{
public:
    FeedbackProbe()
        : _adder_in( 0 )
        , _adder_out( 0 )
    {
        SetInputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& ) override
    {
        auto in = inputs.GetValue<int>( 0 );
        REQUIRE( in != nullptr );

        // The Adder component adds a counter input to its previous output
        _adder_out = _adder_in + _adder_out;

        REQUIRE( *in == _adder_out );

        ++_adder_in;
    }

private:
    int _adder_in;
    int _adder_out;
};

}  // namespace DSPatch
