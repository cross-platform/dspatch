#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <DSPatch.h>

#include <components/Adder.h>
#include <components/BranchSyncProbe.h>
#include <components/ChangingCounter.h>
#include <components/ChangingProbe.h>
#include <components/CircuitCounter.h>
#include <components/CircuitProbe.h>
#include <components/Counter.h>
#include <components/FeedbackProbe.h>
#include <components/Incrementer.h>
#include <components/NoOutputProbe.h>
#include <components/ParallelProbe.h>
#include <components/PassThrough.h>
#include <components/SerialProbe.h>
#include <components/SlowCounter.h>
#include <components/SporadicCounter.h>
#include <components/ThreadingProbe.h>

#include <thread>

using namespace DSPatch;

TEST_CASE( "SerialTest" )
{
    // Configure a circuit made up of a counter and 5 incrementers in series
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_s1 = std::make_shared<Incrementer>( 1 );
    auto inc_s2 = std::make_shared<Incrementer>( 2 );
    auto inc_s3 = std::make_shared<Incrementer>( 3 );
    auto inc_s4 = std::make_shared<Incrementer>( 4 );
    auto inc_s5 = std::make_shared<Incrementer>( 5 );
    auto probe = std::make_shared<SerialProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( inc_s1 );
    circuit->AddComponent( inc_s2 );
    circuit->AddComponent( inc_s3 );
    circuit->AddComponent( inc_s4 );
    circuit->AddComponent( inc_s5 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, inc_s1, 0 );
    circuit->ConnectOutToIn( inc_s1, 0, inc_s2, 0 );
    circuit->ConnectOutToIn( inc_s2, 0, inc_s3, 0 );
    circuit->ConnectOutToIn( inc_s3, 0, inc_s4, 0 );
    circuit->ConnectOutToIn( inc_s4, 0, inc_s5, 0 );
    circuit->ConnectOutToIn( inc_s5, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick();
        circuit->Reset();
    }
}

TEST_CASE( "ParallelTest" )
{
    // Configure a circuit made up of a counter and 5 incrementers in parallel
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_p1 = std::make_shared<Incrementer>( 1 );
    auto inc_p2 = std::make_shared<Incrementer>( 2 );
    auto inc_p3 = std::make_shared<Incrementer>( 3 );
    auto inc_p4 = std::make_shared<Incrementer>( 4 );
    auto inc_p5 = std::make_shared<Incrementer>( 5 );
    auto probe = std::make_shared<ParallelProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( inc_p1 );
    circuit->AddComponent( inc_p2 );
    circuit->AddComponent( inc_p3 );
    circuit->AddComponent( inc_p4 );
    circuit->AddComponent( inc_p5 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, inc_p1, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p2, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p3, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p4, 0 );
    circuit->ConnectOutToIn( counter, 0, inc_p5, 0 );
    circuit->ConnectOutToIn( inc_p1, 0, probe, 0 );
    circuit->ConnectOutToIn( inc_p2, 0, probe, 1 );
    circuit->ConnectOutToIn( inc_p3, 0, probe, 2 );
    circuit->ConnectOutToIn( inc_p4, 0, probe, 3 );
    circuit->ConnectOutToIn( inc_p5, 0, probe, 4 );

    // Tick the circuit for 1s with 3 threads
    circuit->SetThreadCount( 3 );
    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->StopAutoTick();
}

TEST_CASE( "IntegratedParallelTest" )
{
    // Configure a circuit made up of a counter and 5 parallel incrementers within an integrated circuit
    auto circuit = std::make_shared<Circuit>();
    auto ic = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_p1 = std::make_shared<Incrementer>( 1 );
    auto inc_p2 = std::make_shared<Incrementer>( 2 );
    auto inc_p3 = std::make_shared<Incrementer>( 3 );
    auto inc_p4 = std::make_shared<Incrementer>( 4 );
    auto inc_p5 = std::make_shared<Incrementer>( 5 );
    auto probe = std::make_shared<ParallelProbe>();

    // Try to add corrupt components to the circuit
    REQUIRE( ic->AddComponent( ic ) == -1 );
    REQUIRE( ic->AddComponent( nullptr ) == -1 );

    // Try to add a component to the circuit twice
    REQUIRE( ic->AddComponent( inc_p1 ) == 0 );
    REQUIRE( ic->AddComponent( inc_p1 ) == 0 );

    // Move the inc_p1 component to the host circuit
    ic->RemoveComponent( inc_p1 );
    REQUIRE( circuit->AddComponent( inc_p1 ) == 0 );

    // Try to add a component to multiple circuits
    REQUIRE( ic->AddComponent( inc_p1 ) == -1 );
    circuit->RemoveAllComponents();
    REQUIRE( ic->AddComponent( inc_p1 ) == 0 );

    // Configure our integrated circuit
    // ================================

    ic->SetInputCount( 5 );
    ic->SetOutputCount( 5 );

    auto inc_p2_id = ic->AddComponent( inc_p2 );
    auto inc_p3_id = ic->AddComponent( inc_p3 );
    auto inc_p4_id = ic->AddComponent( inc_p4 );
    auto inc_p5_id = ic->AddComponent( inc_p5 );
    auto inc_p6_id = ic->AddComponent( std::make_shared<Incrementer>( 6 ) );

    // Purposely connect incorrectly then re-connect correctly
    ic->ConnectInToIn( 1, inc_p1, 0 );
    ic->ConnectInToIn( 0, inc_p1, 0 );

    ic->ConnectInToIn( 1, inc_p2_id, 0 );
    ic->ConnectInToIn( 2, inc_p3_id, 0 );
    ic->ConnectInToIn( 3, inc_p4, 0 );

    // Test one circuit input to multiple component inputs
    ic->ConnectInToIn( 4, inc_p6_id, 0 );
    ic->ConnectInToIn( 4, inc_p5, 0 );

    // Purposely connect incorrectly then re-connect correctly
    ic->ConnectOutToOut( inc_p2, 0, 0 );
    ic->ConnectOutToOut( inc_p1, 0, 0 );

    ic->ConnectOutToOut( inc_p2, 0, 1 );
    ic->ConnectOutToOut( inc_p3, 0, 2 );
    ic->ConnectOutToOut( inc_p4_id, 0, 3 );
    ic->ConnectOutToOut( inc_p5_id, 0, 4 );

    // Try connect a non-existant component id
    auto bad_id = -99;
    REQUIRE( ic->ConnectInToIn( 0, bad_id, 0 ) == false );

    // Configure our host circuit
    // ==========================

    circuit->AddComponent( counter );
    circuit->AddComponent( ic );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, ic, 0 );
    circuit->ConnectOutToIn( counter, 0, ic, 1 );
    circuit->ConnectOutToIn( counter, 0, ic, 2 );
    circuit->ConnectOutToIn( counter, 0, ic, 3 );
    circuit->ConnectOutToIn( counter, 0, ic, 4 );

    // Try connect a non-existant input / output
    REQUIRE( circuit->ConnectOutToIn( counter, 0, ic, 5 ) == false );
    REQUIRE( circuit->ConnectOutToIn( counter, 1, ic, 4 ) == false );

    circuit->ConnectOutToIn( ic, 0, probe, 0 );
    circuit->ConnectOutToIn( ic, 1, probe, 1 );
    circuit->ConnectOutToIn( ic, 2, probe, 2 );
    circuit->ConnectOutToIn( ic, 3, probe, 3 );
    circuit->ConnectOutToIn( ic, 4, probe, 4 );

    // Try connect an already connected wire
    REQUIRE( circuit->ConnectOutToIn( ic, 4, probe, 4 ) == true );

    // Tick the circuit for 1s with 3 threads
    circuit->SetThreadCount( 3 );
    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->StopAutoTick();

    // Check circuit component counts
    REQUIRE( ic->GetComponentCount() == 6 );
    REQUIRE( circuit->GetComponentCount() == 3 );
}

TEST_CASE( "BranchSyncTest" )
{
    // Configure a circuit made up of 3 parallel branches of 4, 2, and 1 component(s) respectively
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto inc_p1_s1 = std::make_shared<Incrementer>();
    auto inc_p1_s2 = std::make_shared<Incrementer>();
    auto inc_p1_s3 = std::make_shared<Incrementer>();
    auto inc_p1_s4 = std::make_shared<Incrementer>();
    auto inc_p2_s1 = std::make_shared<Incrementer>();
    auto inc_p2_s2 = std::make_shared<Incrementer>();
    auto inc_p3_s1 = std::make_shared<Incrementer>();
    auto probe = std::make_shared<BranchSyncProbe>();

    circuit->AddComponent( counter );

    circuit->AddComponent( inc_p1_s1 );
    circuit->AddComponent( inc_p1_s2 );
    circuit->AddComponent( inc_p1_s3 );
    circuit->AddComponent( inc_p1_s4 );

    circuit->AddComponent( inc_p2_s1 );
    circuit->AddComponent( inc_p2_s2 );

    circuit->AddComponent( inc_p3_s1 );

    circuit->AddComponent( probe );

    // Wire branch 1
    circuit->ConnectOutToIn( counter, 0, inc_p1_s1, 0 );
    circuit->ConnectOutToIn( inc_p1_s1, 0, inc_p1_s2, 0 );
    circuit->ConnectOutToIn( inc_p1_s2, 0, inc_p1_s3, 0 );
    circuit->ConnectOutToIn( inc_p1_s3, 0, inc_p1_s4, 0 );
    circuit->ConnectOutToIn( inc_p1_s4, 0, probe, 0 );

    // Wire branch 2
    circuit->ConnectOutToIn( counter, 0, inc_p2_s1, 0 );
    circuit->ConnectOutToIn( inc_p2_s1, 0, inc_p2_s2, 0 );
    circuit->ConnectOutToIn( inc_p2_s2, 0, probe, 1 );

    // Wire branch 3
    circuit->ConnectOutToIn( counter, 0, inc_p3_s1, 0 );
    circuit->ConnectOutToIn( inc_p3_s1, 0, probe, 2 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick();
        circuit->Reset();
    }
}

TEST_CASE( "FeedbackTest" )
{
    // Configure a circuit made up of an adder that adds a counter to its own previous output
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<Counter>();
    auto adder = std::make_shared<Adder>();
    auto probe = std::make_shared<FeedbackProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( adder );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, adder, 0 );
    circuit->ConnectOutToIn( adder, 0, adder, 1 );

    circuit->ConnectOutToIn( adder, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick();
        circuit->Reset();
    }
}

TEST_CASE( "NoOutputTest" )
{
    // Configure a circuit where the counter responds sporadically
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<SporadicCounter>();
    auto probe = std::make_shared<NoOutputProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick();
        circuit->Reset();
    }
}

TEST_CASE( "ChangingOutputTest" )
{
    // Configure a circuit whereby a component outputs varying types (int, float, string, vector)
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<ChangingCounter>();
    auto probe = std::make_shared<ChangingProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );

    // Tick the circuit 100 times
    for ( int i = 0; i < 100; ++i )
    {
        circuit->Tick();
        circuit->Reset();
    }
}

TEST_CASE( "ThreadPerformanceTest" )
{
    // Configure a circuit made up of 3 parallel counters, then adjust the thread count
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<SlowCounter>();
    auto counter2 = std::make_shared<SlowCounter>();
    auto counter3 = std::make_shared<SlowCounter>();
    auto probe = std::make_shared<ThreadingProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( counter2 );
    circuit->AddComponent( counter3 );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );
    circuit->ConnectOutToIn( counter2, 0, probe, 1 );
    circuit->ConnectOutToIn( counter3, 0, probe, 2 );

    // Tick the circuit for 1s with 1 thread
    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    int count = probe->GetCount();

    // Tick the circuit for 1s with 2 threads, and check that more iterations occurred
    circuit->SetThreadCount( 2 );

    counter->Reset();
    counter2->Reset();
    counter3->Reset();
    probe->Reset();

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    REQUIRE( count < probe->GetCount() );

    count = probe->GetCount();

    // Tick the circuit for 1s with 3 threads, and check that more iterations occurred
    circuit->SetThreadCount( 3 );

    counter->Reset();
    counter2->Reset();
    counter3->Reset();
    probe->Reset();

    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    circuit->PauseAutoTick();

    REQUIRE( count < probe->GetCount() );
}

TEST_CASE( "ThreadAdjustmentTest" )
{
    // Configure a counter circuit, then adjust the thread count while it's running
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<SlowCounter>();
    auto probe = std::make_shared<ThreadingProbe>();

    circuit->AddComponent( counter );
    circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter, 0, probe, 0 );
    circuit->ConnectOutToIn( counter, 0, probe, 1 );
    circuit->ConnectOutToIn( counter, 0, probe, 2 );

    // Tick the circuit for 0.1s with 1 thread
    circuit->StartAutoTick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    // Adjust the thread count while the circuit is running
    circuit->SetThreadCount( 2 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetThreadCount( 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetThreadCount( 4 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetThreadCount( 2 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->SetThreadCount( 3 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

    circuit->StopAutoTick();

    REQUIRE( circuit->GetThreadCount() == 3 );
}

TEST_CASE( "WiringTest" )
{
    // Configure a counter circuit, then re-wire it while it's running
    auto circuit = std::make_shared<Circuit>();

    auto counter = std::make_shared<CircuitCounter>();
    auto probe = std::make_shared<CircuitProbe>();

    auto counter_id = circuit->AddComponent( counter );
    auto probe_id = circuit->AddComponent( probe );

    circuit->ConnectOutToIn( counter_id, 0, probe, 0 );
    circuit->ConnectOutToIn( probe, 0, counter_id, 0 );

    // Tick the circuit for 0.5s with 1 thread
    probe->StartAutoTick();  // can start an auto-tick from any circuit component
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    // Re-wire

    auto pass_s1 = std::make_shared<PassThrough>();
    circuit->AddComponent( pass_s1 );

    circuit->ConnectOutToIn( pass_s1, 0, probe_id, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    circuit->ConnectOutToIn( counter, 0, pass_s1, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    // Disconnect a component

    probe->DisconnectInput( 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    circuit->DisconnectComponent( probe );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    // Wire in a new component

    auto pass_s2 = std::make_shared<PassThrough>();
    circuit->AddComponent( pass_s2 );

    circuit->ConnectOutToIn( probe, 0, counter_id, 0 );
    circuit->ConnectOutToIn( pass_s2, 0, probe, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    circuit->ConnectOutToIn( pass_s1, 0, pass_s2, 0 );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    pass_s2->StopAutoTick();  // can stop an auto-tick from any circuit component
}
