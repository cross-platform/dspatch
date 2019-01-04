#include <DSPatch.h>
#include <components.h>

using namespace DSPatch;

int main()
{
    // 1. Create a circuit where we can route our components
    // =====================================================
    auto circuit = std::make_shared<Circuit>();

    // 2. Create instances of the components needed for our circuit
    // ============================================================
    auto randBoolGen1 = std::make_shared<RandBool>();
    auto randBoolGen2 = std::make_shared<RandBool>();
    auto logicAnd = std::make_shared<And>();
    auto boolPrinter = std::make_shared<PrintBool>();

    // 3. Add component instances to circuit
    // =====================================
    circuit->AddComponent( randBoolGen1 );
    circuit->AddComponent( randBoolGen2 );
    circuit->AddComponent( logicAnd );
    circuit->AddComponent( boolPrinter );

    // 4. Wire up the components inside the circuit
    // ============================================
    circuit->ConnectOutToIn( randBoolGen1, 0, logicAnd, 0 );
    circuit->ConnectOutToIn( randBoolGen2, 0, logicAnd, 1 );
    circuit->ConnectOutToIn( logicAnd, 0, boolPrinter, 0 );

    // 5. Tick the circuit
    // ===================

    // Circuit tick method 1: Manual
    for ( int i = 0; i < 10; ++i )
    {
        circuit->Tick();
    }

    // Circuit tick method 2: Automatic
    std::cout << "Press any key to begin circuit auto-tick.";
    getchar();
    circuit->StartAutoTick();

    // Increase circuit buffer count for higher performance
    getchar();
    circuit->SetBufferCount( 4 );

    // Press any key to quit
    getchar();
    return 0;
}
