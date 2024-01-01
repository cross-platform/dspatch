/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2024, Marcus Tomlinson

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

namespace DSPatch
{

class ChangingCounter final : public Component
{
public:
    ChangingCounter()
        : _count( 0 )
    {
        srand( (unsigned int)time( nullptr ) );

        SetOutputCount_( 1 );
    }

protected:
    void Process_( SignalBus&, SignalBus& outputs ) override
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
                    outputs.SetValue( 0, std::vector<int>{ _count, _count + 1, _count + 2 } );
                    _count += 3;
                    break;
            }
        }
    }

private:
    int _count;
};

}  // namespace DSPatch
