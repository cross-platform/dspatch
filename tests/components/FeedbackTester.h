#pragma once

namespace DSPatch
{

class FeedbackTester : public Component
{
public:
    FeedbackTester()
    {
        SetInputCount_( 10 );
        SetOutputCount_( 1 );
    }

    void SetValidInputs( int count )
    {
        _inputs = count;
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override
    {
        for ( int i = 0; i < _inputs; ++i )
        {
            auto in = inputs.GetValue<int>( i );
            REQUIRE( in != nullptr );
            REQUIRE( *in == _counter );
        }

        outputs.SetValue( 0, ++_counter );
    }

private:
    int _inputs = 0;
    int _counter = 0;
};

}  // namespace DSPatch
