#pragma once

namespace DSPatch
{

class FeedbackTester : public Component
{
public:
    FeedbackTester( int bufferCount )
        : _bufferCount( bufferCount )
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
            if ( _counter >= _bufferCount )
            {
                REQUIRE( in != nullptr );
                REQUIRE( *in == _counter - ( _bufferCount - 1 ) );
            }
        }

        outputs.SetValue( 0, ++_counter );
    }

private:
    int _bufferCount = 0;
    int _inputs = 0;
    int _counter = 0;
};

}  // namespace DSPatch
