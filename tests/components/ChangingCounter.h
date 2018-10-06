#pragma once

namespace DSPatch
{

class ChangingCounter : public Component
{
public:
    ChangingCounter()
        : _count( 0 )
    {
        srand( (unsigned int)time( nullptr ) );

        SetOutputCount_( 1 );
    }

protected:
    virtual void Process_( SignalBus const&, SignalBus& outputs ) override
    {
        if ( rand() % 2 == 1 )
        {
            int choice = rand() % 4;

            switch ( choice )
            {
                case 0:
                    outputs.SetValue( 0, _count++ );
                    break;
                case 1:
                    outputs.SetValue( 0, (float)_count++ );
                    break;
                case 2:
                    outputs.SetValue( 0, std::to_string( _count++ ) );
                    break;
                case 3:
                    outputs.SetValue( 0, std::vector<int>{_count, _count + 1, _count + 2} );
                    _count += 3;
                    break;
            }
        }
    }

private:
    int _count;
};

}  // namespace DSPatch
