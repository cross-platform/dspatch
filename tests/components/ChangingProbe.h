#pragma once

namespace DSPatch
{

class ChangingProbe : public Component
{
public:
    ChangingProbe()
        : _count( 0 )
    {
        SetInputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& ) override
    {
        if ( auto integer = inputs.GetValue<int>( 0 ) )
        {
            REQUIRE( *integer == _count++ );
        }
        else if ( auto floating = inputs.GetValue<float>( 0 ) )
        {
            REQUIRE( *floating == (float)_count++ );
        }
        else if ( auto string = inputs.GetValue<std::string>( 0 ) )
        {
            REQUIRE( *string == std::to_string( _count++ ) );
        }
        else if ( auto vector = inputs.GetValue<std::vector<int>>( 0 ) )
        {
            REQUIRE( ( *vector )[0] == _count++ );
            REQUIRE( ( *vector )[1] == _count++ );
            REQUIRE( ( *vector )[2] == _count++ );
        }
    }

private:
    int _count;
};

}  // namespace DSPatch
