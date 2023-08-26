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
#include <components/FeedbackTester.h>
#include <components/Incrementer.h>
#include <components/NoOutputProbe.h>
#include <components/NullInputProbe.h>
#include <components/ParallelProbe.h>
#include <components/PassThrough.h>
#include <components/SerialProbe.h>
#include <components/SlowCounter.h>
#include <components/SlowPassThrough.h>
#include <components/SporadicCounter.h>
#include <components/ThreadingProbe.h>

#include <thread>

using namespace DSPatch;

// static double refEff;

// TEST_CASE( "SerialTest" )
// {
//     // Configure a circuit made up of a counter and 5 incrementers in series
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto inc_s1 = std::make_shared<Incrementer>( 1 );
//     auto inc_s2 = std::make_shared<Incrementer>( 2 );
//     auto inc_s3 = std::make_shared<Incrementer>( 3 );
//     auto inc_s4 = std::make_shared<Incrementer>( 4 );
//     auto inc_s5 = std::make_shared<Incrementer>( 5 );
//     auto probe = std::make_shared<SerialProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( inc_s1 );
//     circuit->AddComponent( inc_s2 );
//     circuit->AddComponent( inc_s3 );
//     circuit->AddComponent( inc_s4 );
//     circuit->AddComponent( inc_s5 );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, inc_s1, 0 );
//     circuit->ConnectOutToIn( inc_s1, 0, inc_s2, 0 );
//     circuit->ConnectOutToIn( inc_s2, 0, inc_s3, 0 );
//     circuit->ConnectOutToIn( inc_s3, 0, inc_s4, 0 );
//     circuit->ConnectOutToIn( inc_s4, 0, inc_s5, 0 );
//     circuit->ConnectOutToIn( inc_s5, 0, probe, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "ParallelTest" )
// {
//     // Configure a circuit made up of a counter and 5 incrementers in parallel
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto inc_p1 = std::make_shared<Incrementer>( 1 );
//     auto inc_p2 = std::make_shared<Incrementer>( 2 );
//     auto inc_p3 = std::make_shared<Incrementer>( 3 );
//     auto inc_p4 = std::make_shared<Incrementer>( 4 );
//     auto inc_p5 = std::make_shared<Incrementer>( 5 );
//     auto probe = std::make_shared<ParallelProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( inc_p1 );
//     circuit->AddComponent( inc_p2 );
//     circuit->AddComponent( inc_p3 );
//     circuit->AddComponent( inc_p4 );
//     circuit->AddComponent( inc_p5 );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, inc_p1, 0 );
//     circuit->ConnectOutToIn( counter, 0, inc_p2, 0 );
//     circuit->ConnectOutToIn( counter, 0, inc_p3, 0 );
//     circuit->ConnectOutToIn( counter, 0, inc_p4, 0 );
//     circuit->ConnectOutToIn( counter, 0, inc_p5, 0 );
//     circuit->ConnectOutToIn( inc_p1, 0, probe, 0 );
//     circuit->ConnectOutToIn( inc_p2, 0, probe, 1 );
//     circuit->ConnectOutToIn( inc_p3, 0, probe, 2 );
//     circuit->ConnectOutToIn( inc_p4, 0, probe, 3 );
//     circuit->ConnectOutToIn( inc_p5, 0, probe, 4 );

//     // Tick the circuit for 100ms with 3 threads
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 3 ) );
//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->StopAutoTick();
// }

// TEST_CASE( "BranchSyncTest" )
// {
//     // Configure a circuit made up of 3 parallel branches of 4, 2, and 1 component(s) respectively
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto inc_p1_s1 = std::make_shared<Incrementer>();
//     auto inc_p1_s2 = std::make_shared<Incrementer>();
//     auto inc_p1_s3 = std::make_shared<Incrementer>();
//     auto inc_p1_s4 = std::make_shared<Incrementer>();
//     auto inc_p2_s1 = std::make_shared<Incrementer>();
//     auto inc_p2_s2 = std::make_shared<Incrementer>();
//     auto inc_p3_s1 = std::make_shared<Incrementer>();
//     auto probe = std::make_shared<BranchSyncProbe>();

//     circuit->AddComponent( counter );

//     circuit->AddComponent( inc_p1_s1 );
//     circuit->AddComponent( inc_p1_s2 );
//     circuit->AddComponent( inc_p1_s3 );
//     circuit->AddComponent( inc_p1_s4 );

//     circuit->AddComponent( inc_p2_s1 );
//     circuit->AddComponent( inc_p2_s2 );

//     circuit->AddComponent( inc_p3_s1 );

//     circuit->AddComponent( probe );

//     // Wire branch 1
//     circuit->ConnectOutToIn( counter, 0, inc_p1_s1, 0 );
//     circuit->ConnectOutToIn( inc_p1_s1, 0, inc_p1_s2, 0 );
//     circuit->ConnectOutToIn( inc_p1_s2, 0, inc_p1_s3, 0 );
//     circuit->ConnectOutToIn( inc_p1_s3, 0, inc_p1_s4, 0 );
//     circuit->ConnectOutToIn( inc_p1_s4, 0, probe, 0 );

//     // Wire branch 2
//     circuit->ConnectOutToIn( counter, 0, inc_p2_s1, 0 );
//     circuit->ConnectOutToIn( inc_p2_s1, 0, inc_p2_s2, 0 );
//     circuit->ConnectOutToIn( inc_p2_s2, 0, probe, 1 );

//     // Wire branch 3
//     circuit->ConnectOutToIn( counter, 0, inc_p3_s1, 0 );
//     circuit->ConnectOutToIn( inc_p3_s1, 0, probe, 2 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "FeedbackTest" )
// {
//     // Configure a circuit made up of an adder that adds a counter to its own previous output
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto adder = std::make_shared<Adder>();
//     auto passthrough = std::make_shared<PassThrough>();
//     auto probe = std::make_shared<FeedbackProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( adder );
//     circuit->AddComponent( passthrough );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, adder, 0 );
//     circuit->ConnectOutToIn( adder, 0, passthrough, 0 );

//     circuit->ConnectOutToIn( passthrough, 0, adder, 1 );

//     circuit->ConnectOutToIn( adder, 0, probe, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "FeedbackTestNoCircuit" )
// {
//     auto counter = std::make_shared<Counter>();
//     auto adder = std::make_shared<Adder>();
//     auto passthrough = std::make_shared<PassThrough>();
//     auto probe = std::make_shared<FeedbackProbe>();

//     adder->ConnectInput( counter, 0, 0 );
//     passthrough->ConnectInput( adder, 0, 0 );

//     adder->ConnectInput( passthrough, 0, 1 );

//     probe->ConnectInput( adder, 0, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         counter->Tick();
//         adder->Tick();
//         passthrough->Tick();
//         probe->Tick();

//         counter->Reset();
//         adder->Reset();
//         passthrough->Reset();
//         probe->Reset();
//     }
// }

// TEST_CASE( "RefCountResetRegressionTest" )
// {
//     auto circuit = std::make_shared<Circuit>();
//     auto feedback = std::make_shared<FeedbackTester>( 2 );

//     circuit->AddComponent( feedback );
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2 ) );

//     feedback->ConnectInput( feedback, 0, 0 );
//     feedback->SetValidInputs( 1 );

//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->PauseAutoTick();

//     feedback->ConnectInput( feedback, 0, 1 );
//     feedback->ConnectInput( feedback, 0, 2 );
//     feedback->ConnectInput( feedback, 0, 3 );
//     feedback->SetValidInputs( 4 );

//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->PauseAutoTick();

//     feedback->ConnectInput( feedback, 0, 4 );
//     feedback->ConnectInput( feedback, 0, 5 );
//     feedback->ConnectInput( feedback, 0, 6 );
//     feedback->ConnectInput( feedback, 0, 7 );
//     feedback->ConnectInput( feedback, 0, 8 );
//     feedback->ConnectInput( feedback, 0, 9 );
//     feedback->SetValidInputs( 10 );

//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->StopAutoTick();
// }

// TEST_CASE( "NoOutputTest" )
// {
//     // Configure a circuit where the counter responds sporadically
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<SporadicCounter>();
//     auto probe = std::make_shared<NoOutputProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, probe, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "ChangingOutputTest" )
// {
//     // Configure a circuit whereby a component outputs varying types (int, float, string, vector)
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<ChangingCounter>();
//     auto probe = std::make_shared<ChangingProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, probe, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "ThreadPerformanceTest" )
// {
//     // Configure a circuit made up of 4 parallel counters, then adjust the thread count
//     auto circuit = std::make_shared<Circuit>();

//     auto counter1 = std::make_shared<SlowCounter>();
//     auto passthrough1 = std::make_shared<SlowPassThrough>();
//     auto counter2 = std::make_shared<SlowCounter>();
//     auto passthrough2 = std::make_shared<SlowPassThrough>();
//     auto counter3 = std::make_shared<SlowCounter>();
//     auto passthrough3 = std::make_shared<SlowPassThrough>();
//     auto counter4 = std::make_shared<SlowCounter>();
//     auto passthrough4 = std::make_shared<SlowPassThrough>();
//     auto probe = std::make_shared<ThreadingProbe>();

//     circuit->AddComponent( counter1 );
//     circuit->AddComponent( passthrough1 );
//     circuit->AddComponent( counter2 );
//     circuit->AddComponent( passthrough2 );
//     circuit->AddComponent( counter3 );
//     circuit->AddComponent( passthrough3 );
//     circuit->AddComponent( counter4 );
//     circuit->AddComponent( passthrough4 );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter1, 0, passthrough1, 0 );
//     circuit->ConnectOutToIn( passthrough1, 0, probe, 0 );
//     circuit->ConnectOutToIn( counter2, 0, passthrough2, 0 );
//     circuit->ConnectOutToIn( passthrough2, 0, probe, 1 );
//     circuit->ConnectOutToIn( counter3, 0, passthrough3, 0 );
//     circuit->ConnectOutToIn( passthrough3, 0, probe, 2 );
//     circuit->ConnectOutToIn( counter4, 0, passthrough4, 0 );
//     circuit->ConnectOutToIn( passthrough4, 0, probe, 3 );

//     // Calculate reference efficiency

//     SignalBus testBus;
//     auto measureRef = [&testBus]( std::shared_ptr<SlowCounter>& counter )
//     {
//         for ( int i = 0; i < 2000; ++i )
//         {
//             counter->Process_( testBus, testBus );
//         }
//     };

//     auto begin = std::chrono::high_resolution_clock::now();
//     auto t1 = std::thread( [&measureRef, &counter1]() { measureRef( counter1 ); } );
//     auto t2 = std::thread( [&measureRef, &counter2]() { measureRef( counter2 ); } );
//     auto t3 = std::thread( [&measureRef, &counter3]() { measureRef( counter3 ); } );
//     measureRef( counter4 );
//     t1.join();
//     t2.join();
//     t3.join();
//     auto end = std::chrono::high_resolution_clock::now();
//     refEff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     counter1->ResetCount();
//     counter2->ResetCount();
//     counter3->ResetCount();
//     counter4->ResetCount();

//     std::cout << "Reference Efficiency: " << refEff << "%" << std::endl;

//     // Tick the circuit with 1 thread
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 1 ) );

//     begin = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1000; ++i )
//     {
//         circuit->Tick();
//     }
//     end = std::chrono::high_resolution_clock::now();
//     auto eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     auto overhead = 100 - ( 100 * ( eff / refEff ) );
//     std::cout << "1x Buffer Efficiency (Series Mode): " << eff << "% (-" << overhead << "%)" << std::endl;
//     REQUIRE( eff >= refEff * 0.25 * 0.90 );

//     // Tick the circuit with 2 threads, and check that more ticks occurred
//     if ( std::thread::hardware_concurrency() < 2 )
//     {
//         return;
//     }
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2 ) );

//     begin = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1000; ++i )
//     {
//         circuit->Tick();
//     }
//     end = std::chrono::high_resolution_clock::now();
//     eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     overhead = 100 - ( 100 * ( eff / refEff ) );
//     std::cout << "2x Buffer Efficiency (Series Mode): " << eff << "% (-" << overhead << "%)" << std::endl;
//     REQUIRE( eff >= refEff * 0.5 * 0.90 );

//     // Tick the circuit with 3 threads, and check that more ticks occurred
//     if ( std::thread::hardware_concurrency() < 4 )
//     {
//         return;
//     }
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 3 ) );

//     begin = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1000; ++i )
//     {
//         circuit->Tick();
//     }
//     end = std::chrono::high_resolution_clock::now();
//     eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     overhead = 100 - ( 100 * ( eff / refEff ) );
//     std::cout << "3x Buffer Efficiency (Series Mode): " << eff << "% (-" << overhead << "%)" << std::endl;
//     REQUIRE( eff >= refEff * 0.75 * 0.90 );

//     // Tick the circuit with 4 threads, and check that more ticks occurred
//     if ( std::thread::hardware_concurrency() < 4 )
//     {
//         return;
//     }
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 4 ) );

//     begin = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1000; ++i )
//     {
//         circuit->Tick();
//     }
//     end = std::chrono::high_resolution_clock::now();
//     eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     overhead = 100 - ( 100 * ( eff / refEff ) );
//     std::cout << "4x Buffer Efficiency (Series Mode): " << eff << "% (-" << overhead << "%)" << std::endl;
//     REQUIRE( eff >= refEff * 0.90 );
// }

// TEST_CASE( "StopAutoTickRegressionTest" )
// {
//     auto circuit = std::make_shared<Circuit>();

//     auto counter1 = std::make_shared<SlowCounter>();
//     auto counter2 = std::make_shared<SlowCounter>();
//     auto counter3 = std::make_shared<SlowCounter>();
//     auto counter4 = std::make_shared<SlowCounter>();
//     auto probe = std::make_shared<ThreadingProbe>();

//     circuit->AddComponent( counter1 );
//     circuit->AddComponent( counter2 );
//     circuit->AddComponent( counter3 );
//     circuit->AddComponent( counter4 );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter1, 0, probe, 0 );
//     circuit->ConnectOutToIn( counter2, 0, probe, 1 );
//     circuit->ConnectOutToIn( counter3, 0, probe, 2 );
//     circuit->ConnectOutToIn( counter4, 0, probe, 3 );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( std::thread::hardware_concurrency() ) );

//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->StopAutoTick();
//     circuit->RemoveComponent( counter1 );
//     circuit->RemoveComponent( counter2 );
//     circuit->RemoveComponent( counter3 );
//     circuit->RemoveComponent( counter4 );
//     circuit->RemoveComponent( probe );
// }

// TEST_CASE( "ThreadAdjustmentTest" )
// {
//     // Configure a counter circuit, then adjust the thread count while it's running
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto probe = std::make_shared<ThreadingProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, probe, 0 );
//     circuit->ConnectOutToIn( counter, 0, probe, 1 );
//     circuit->ConnectOutToIn( counter, 0, probe, 2 );
//     circuit->ConnectOutToIn( counter, 0, probe, 3 );

//     // Tick the circuit for 100ms with 1 thread
//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     // Adjust the thread count while the circuit is running
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 0 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 4 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 3 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->StopAutoTick();
// }

// TEST_CASE( "WiringTest" )
// {
//     // Configure a counter circuit, then re-wire it while it's running
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<CircuitCounter>();
//     auto probe = std::make_shared<CircuitProbe>();

//     auto counter_id = circuit->AddComponent( counter );
//     auto probe_id = circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter_id, 0, probe, 0 );
//     circuit->ConnectOutToIn( probe, 0, counter_id, 0 );

//     // Tick the circuit for 100ms with 1 thread
//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     // Re-wire

//     auto pass_s1 = std::make_shared<PassThrough>();
//     circuit->AddComponent( pass_s1 );

//     circuit->ConnectOutToIn( pass_s1, 0, probe_id, 0 );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->ConnectOutToIn( counter, 0, pass_s1, 0 );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     // Disconnect a component

//     circuit->PauseAutoTick();
//     probe->DisconnectInput( 0 );
//     circuit->ResumeAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->DisconnectComponent( probe );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     // Wire in a new component

//     auto pass_s2 = std::make_shared<PassThrough>();
//     circuit->AddComponent( pass_s2 );

//     circuit->ConnectOutToIn( probe, 0, counter_id, 0 );
//     circuit->ConnectOutToIn( pass_s2, 0, probe, 0 );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->ConnectOutToIn( pass_s1, 0, pass_s2, 0 );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->StopAutoTick();
// }

// //=================================================================================================

// TEST_CASE( "SerialTest2" )
// {
//     // Configure a circuit made up of a counter and 5 incrementers in series
//     auto circuit = std::make_shared<Circuit>();
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );

//     auto counter = std::make_shared<Counter>();
//     auto inc_s1 = std::make_shared<Incrementer>( 1 );
//     auto inc_s2 = std::make_shared<Incrementer>( 2 );
//     auto inc_s3 = std::make_shared<Incrementer>( 3 );
//     auto inc_s4 = std::make_shared<Incrementer>( 4 );
//     auto inc_s5 = std::make_shared<Incrementer>( 5 );
//     auto probe = std::make_shared<SerialProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( inc_s1 );
//     circuit->AddComponent( inc_s2 );
//     circuit->AddComponent( inc_s3 );
//     circuit->AddComponent( inc_s4 );
//     circuit->AddComponent( inc_s5 );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, inc_s1, 0 );
//     circuit->ConnectOutToIn( inc_s1, 0, inc_s2, 0 );
//     circuit->ConnectOutToIn( inc_s2, 0, inc_s3, 0 );
//     circuit->ConnectOutToIn( inc_s3, 0, inc_s4, 0 );
//     circuit->ConnectOutToIn( inc_s4, 0, inc_s5, 0 );
//     circuit->ConnectOutToIn( inc_s5, 0, probe, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "ParallelTest2" )
// {
//     // Configure a circuit made up of a counter and 5 incrementers in parallel
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto inc_p1 = std::make_shared<Incrementer>( 1 );
//     auto inc_p2 = std::make_shared<Incrementer>( 2 );
//     auto inc_p3 = std::make_shared<Incrementer>( 3 );
//     auto inc_p4 = std::make_shared<Incrementer>( 4 );
//     auto inc_p5 = std::make_shared<Incrementer>( 5 );
//     auto probe = std::make_shared<ParallelProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( inc_p1 );
//     circuit->AddComponent( inc_p2 );
//     circuit->AddComponent( inc_p3 );
//     circuit->AddComponent( inc_p4 );
//     circuit->AddComponent( inc_p5 );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, inc_p1, 0 );
//     circuit->ConnectOutToIn( counter, 0, inc_p2, 0 );
//     circuit->ConnectOutToIn( counter, 0, inc_p3, 0 );
//     circuit->ConnectOutToIn( counter, 0, inc_p4, 0 );
//     circuit->ConnectOutToIn( counter, 0, inc_p5, 0 );
//     circuit->ConnectOutToIn( inc_p1, 0, probe, 0 );
//     circuit->ConnectOutToIn( inc_p2, 0, probe, 1 );
//     circuit->ConnectOutToIn( inc_p3, 0, probe, 2 );
//     circuit->ConnectOutToIn( inc_p4, 0, probe, 3 );
//     circuit->ConnectOutToIn( inc_p5, 0, probe, 4 );

//     // Tick the circuit for 100ms with 3 threads
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 3, 4 ) );
//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->StopAutoTick();
// }

// TEST_CASE( "BranchSyncTest2" )
// {
//     // Configure a circuit made up of 3 parallel branches of 4, 2, and 1 component(s) respectively
//     auto circuit = std::make_shared<Circuit>();
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );

//     auto counter = std::make_shared<Counter>();
//     auto inc_p1_s1 = std::make_shared<Incrementer>();
//     auto inc_p1_s2 = std::make_shared<Incrementer>();
//     auto inc_p1_s3 = std::make_shared<Incrementer>();
//     auto inc_p1_s4 = std::make_shared<Incrementer>();
//     auto inc_p2_s1 = std::make_shared<Incrementer>();
//     auto inc_p2_s2 = std::make_shared<Incrementer>();
//     auto inc_p3_s1 = std::make_shared<Incrementer>();
//     auto probe = std::make_shared<BranchSyncProbe>();

//     circuit->AddComponent( counter );

//     circuit->AddComponent( inc_p1_s1 );
//     circuit->AddComponent( inc_p1_s2 );
//     circuit->AddComponent( inc_p1_s3 );
//     circuit->AddComponent( inc_p1_s4 );

//     circuit->AddComponent( inc_p2_s1 );
//     circuit->AddComponent( inc_p2_s2 );

//     circuit->AddComponent( inc_p3_s1 );

//     circuit->AddComponent( probe );

//     // Wire branch 1
//     circuit->ConnectOutToIn( counter, 0, inc_p1_s1, 0 );
//     circuit->ConnectOutToIn( inc_p1_s1, 0, inc_p1_s2, 0 );
//     circuit->ConnectOutToIn( inc_p1_s2, 0, inc_p1_s3, 0 );
//     circuit->ConnectOutToIn( inc_p1_s3, 0, inc_p1_s4, 0 );
//     circuit->ConnectOutToIn( inc_p1_s4, 0, probe, 0 );

//     // Wire branch 2
//     circuit->ConnectOutToIn( counter, 0, inc_p2_s1, 0 );
//     circuit->ConnectOutToIn( inc_p2_s1, 0, inc_p2_s2, 0 );
//     circuit->ConnectOutToIn( inc_p2_s2, 0, probe, 1 );

//     // Wire branch 3
//     circuit->ConnectOutToIn( counter, 0, inc_p3_s1, 0 );
//     circuit->ConnectOutToIn( inc_p3_s1, 0, probe, 2 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "FeedbackTest2" )
// {
//     // Configure a circuit made up of an adder that adds a counter to its own previous output
//     auto circuit = std::make_shared<Circuit>();
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );

//     auto counter = std::make_shared<Counter>();
//     auto adder = std::make_shared<Adder>();
//     auto passthrough = std::make_shared<PassThrough>();
//     auto probe = std::make_shared<FeedbackProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( adder );
//     circuit->AddComponent( passthrough );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, adder, 0 );
//     circuit->ConnectOutToIn( adder, 0, passthrough, 0 );

//     circuit->ConnectOutToIn( passthrough, 0, adder, 1 );

//     circuit->ConnectOutToIn( adder, 0, probe, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "FeedbackTestNoCircuit2" )
// {
//     auto counter = std::make_shared<Counter>();
//     counter->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );
//     auto adder = std::make_shared<Adder>();
//     adder->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );
//     auto passthrough = std::make_shared<PassThrough>();
//     passthrough->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );
//     auto probe = std::make_shared<FeedbackProbe>();
//     probe->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );

//     adder->ConnectInput( counter, 0, 0 );
//     passthrough->ConnectInput( adder, 0, 0 );

//     adder->ConnectInput( passthrough, 0, 1 );

//     probe->ConnectInput( adder, 0, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         counter->Tick();
//         adder->Tick();
//         passthrough->Tick();
//         probe->Tick();

//         counter->Reset();
//         adder->Reset();
//         passthrough->Reset();
//         probe->Reset();
//     }
// }

// TEST_CASE( "RefCountResetRegressionTest2" )
// {
//     auto circuit = std::make_shared<Circuit>();
//     auto feedback = std::make_shared<FeedbackTester>( 2 );

//     circuit->AddComponent( feedback );
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2, 4 ) );

//     feedback->ConnectInput( feedback, 0, 0 );
//     feedback->SetValidInputs( 1 );

//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->PauseAutoTick();

//     feedback->ConnectInput( feedback, 0, 1 );
//     feedback->ConnectInput( feedback, 0, 2 );
//     feedback->ConnectInput( feedback, 0, 3 );
//     feedback->SetValidInputs( 4 );

//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->PauseAutoTick();

//     feedback->ConnectInput( feedback, 0, 4 );
//     feedback->ConnectInput( feedback, 0, 5 );
//     feedback->ConnectInput( feedback, 0, 6 );
//     feedback->ConnectInput( feedback, 0, 7 );
//     feedback->ConnectInput( feedback, 0, 8 );
//     feedback->ConnectInput( feedback, 0, 9 );
//     feedback->SetValidInputs( 10 );

//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->StopAutoTick();
// }

// TEST_CASE( "NoOutputTest2" )
// {
//     // Configure a circuit where the counter responds sporadically
//     auto circuit = std::make_shared<Circuit>();
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );

//     auto counter = std::make_shared<SporadicCounter>();
//     auto probe = std::make_shared<NoOutputProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, probe, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "ChangingOutputTest2" )
// {
//     // Configure a circuit whereby a component outputs varying types (int, float, string, vector)
//     auto circuit = std::make_shared<Circuit>();
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );

//     auto counter = std::make_shared<ChangingCounter>();
//     auto probe = std::make_shared<ChangingProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, probe, 0 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//     }
// }

// TEST_CASE( "ThreadPerformanceTest2" )
// {
//     double cores = std::thread::hardware_concurrency();
//     cores = cores == 3.0 ? 2.0 : cores;
//     cores = cores > 4.0 ? 4.0 : cores;
//     auto effFrac = cores / 4.0;

//     // Configure a circuit made up of 4 parallel counters, then adjust the thread count
//     auto circuit = std::make_shared<Circuit>();

//     auto counter1 = std::make_shared<SlowCounter>();
//     auto passthrough1 = std::make_shared<SlowPassThrough>();
//     auto counter2 = std::make_shared<SlowCounter>();
//     auto passthrough2 = std::make_shared<SlowPassThrough>();
//     auto counter3 = std::make_shared<SlowCounter>();
//     auto passthrough3 = std::make_shared<SlowPassThrough>();
//     auto counter4 = std::make_shared<SlowCounter>();
//     auto passthrough4 = std::make_shared<SlowPassThrough>();
//     auto probe = std::make_shared<ThreadingProbe>();

//     circuit->AddComponent( counter1 );
//     circuit->AddComponent( passthrough1 );
//     circuit->AddComponent( counter2 );
//     circuit->AddComponent( passthrough2 );
//     circuit->AddComponent( counter3 );
//     circuit->AddComponent( passthrough3 );
//     circuit->AddComponent( counter4 );
//     circuit->AddComponent( passthrough4 );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter1, 0, passthrough1, 0 );
//     circuit->ConnectOutToIn( passthrough1, 0, probe, 0 );
//     circuit->ConnectOutToIn( counter2, 0, passthrough2, 0 );
//     circuit->ConnectOutToIn( passthrough2, 0, probe, 1 );
//     circuit->ConnectOutToIn( counter3, 0, passthrough3, 0 );
//     circuit->ConnectOutToIn( passthrough3, 0, probe, 2 );
//     circuit->ConnectOutToIn( counter4, 0, passthrough4, 0 );
//     circuit->ConnectOutToIn( passthrough4, 0, probe, 3 );

//     circuit->Sort();

//     // Tick the circuit with 1 thread
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );

//     auto begin = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1000; ++i )
//     {
//         circuit->Tick();
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     auto eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     auto overhead = 100 - ( 100 * ( eff / refEff ) );
//     std::cout << "1x Buffer Efficiency (Parallel Mode): " << eff << "% (-" << overhead << "%)" << std::endl;
//     REQUIRE( eff >= refEff * effFrac * 0.90 );

//     // Tick the circuit with 2 threads, and check that more ticks occurred
//     if ( std::thread::hardware_concurrency() < 2 )
//     {
//         return;
//     }
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2, 4 ) );

//     begin = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1000; ++i )
//     {
//         circuit->Tick();
//     }
//     end = std::chrono::high_resolution_clock::now();
//     eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     overhead = 100 - ( 100 * ( eff / refEff ) );
//     std::cout << "2x Buffer Efficiency (Parallel Mode): " << eff << "% (-" << overhead << "%)" << std::endl;
//     REQUIRE( eff >= refEff * effFrac * 0.98 );

//     // Tick the circuit with 3 threads, and check that more ticks occurred
//     if ( std::thread::hardware_concurrency() < 4 )
//     {
//         return;
//     }
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 3, 4 ) );

//     begin = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1000; ++i )
//     {
//         circuit->Tick();
//     }
//     end = std::chrono::high_resolution_clock::now();
//     eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     overhead = 100 - ( 100 * ( eff / refEff ) );
//     std::cout << "3x Buffer Efficiency (Parallel Mode): " << eff << "% (-" << overhead << "%)" << std::endl;
//     REQUIRE( eff >= refEff * effFrac * 0.98 );

//     // Tick the circuit with 4 threads, and check that more ticks occurred
//     if ( std::thread::hardware_concurrency() < 4 )
//     {
//         return;
//     }
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 4, 4 ) );

//     begin = std::chrono::high_resolution_clock::now();
//     for ( int i = 0; i < 1000; ++i )
//     {
//         circuit->Tick();
//     }
//     end = std::chrono::high_resolution_clock::now();
//     eff = 100000.0 / std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count();

//     overhead = 100 - ( 100 * ( eff / refEff ) );
//     std::cout << "4x Buffer Efficiency (Parallel Mode): " << eff << "% (-" << overhead << "%)" << std::endl;
//     REQUIRE( eff >= refEff * effFrac * 0.98 );
// }

// TEST_CASE( "StopAutoTickRegressionTest2" )
// {
//     auto circuit = std::make_shared<Circuit>();

//     auto counter1 = std::make_shared<SlowCounter>();
//     auto counter2 = std::make_shared<SlowCounter>();
//     auto counter3 = std::make_shared<SlowCounter>();
//     auto counter4 = std::make_shared<SlowCounter>();
//     auto probe = std::make_shared<ThreadingProbe>();

//     circuit->AddComponent( counter1 );
//     circuit->AddComponent( counter2 );
//     circuit->AddComponent( counter3 );
//     circuit->AddComponent( counter4 );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter1, 0, probe, 0 );
//     circuit->ConnectOutToIn( counter2, 0, probe, 1 );
//     circuit->ConnectOutToIn( counter3, 0, probe, 2 );
//     circuit->ConnectOutToIn( counter4, 0, probe, 3 );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( std::thread::hardware_concurrency(), 4 ) );

//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->StopAutoTick();
//     circuit->RemoveComponent( counter1 );
//     circuit->RemoveComponent( counter2 );
//     circuit->RemoveComponent( counter3 );
//     circuit->RemoveComponent( counter4 );
//     circuit->RemoveComponent( probe );
// }

// TEST_CASE( "ThreadAdjustmentTest2" )
// {
//     // Configure a counter circuit, then adjust the thread count while it's running
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto probe = std::make_shared<ThreadingProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, probe, 0 );
//     circuit->ConnectOutToIn( counter, 0, probe, 1 );
//     circuit->ConnectOutToIn( counter, 0, probe, 2 );
//     circuit->ConnectOutToIn( counter, 0, probe, 3 );

//     // Tick the circuit for 100ms with 1 thread
//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     // Adjust the thread count while the circuit is running
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2, 4 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 0, 4 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 4, 4 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2, 4 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 3, 4 ) );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->StopAutoTick();
// }

// TEST_CASE( "WiringTest2" )
// {
//     // Configure a counter circuit, then re-wire it while it's running
//     auto circuit = std::make_shared<Circuit>();
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 1, 4 ) );

//     auto counter = std::make_shared<CircuitCounter>();
//     auto probe = std::make_shared<CircuitProbe>();

//     auto counter_id = circuit->AddComponent( counter );
//     auto probe_id = circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter_id, 0, probe, 0 );
//     circuit->ConnectOutToIn( probe, 0, counter_id, 0 );

//     // Tick the circuit for 100ms with 1 thread
//     circuit->StartAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     // Re-wire

//     auto pass_s1 = std::make_shared<PassThrough>();
//     circuit->AddComponent( pass_s1 );

//     circuit->ConnectOutToIn( pass_s1, 0, probe_id, 0 );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->ConnectOutToIn( counter, 0, pass_s1, 0 );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     // Disconnect a component

//     circuit->PauseAutoTick();
//     probe->DisconnectInput( 0 );
//     circuit->ResumeAutoTick();
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
//     circuit->DisconnectComponent( probe );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     // Wire in a new component

//     auto pass_s2 = std::make_shared<PassThrough>();
//     circuit->AddComponent( pass_s2 );

//     circuit->ConnectOutToIn( probe, 0, counter_id, 0 );
//     circuit->ConnectOutToIn( pass_s2, 0, probe, 0 );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->ConnectOutToIn( pass_s1, 0, pass_s2, 0 );
//     std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

//     circuit->StopAutoTick();
// }

// //=================================================================================================

// TEST_CASE( "ThreadStopRegressionTest" )
// {
//     // Configure a circuit made up of 3 parallel branches of 4, 2, and 1 component(s) respectively
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto inc_p1_s1 = std::make_shared<Incrementer>();
//     auto inc_p1_s2 = std::make_shared<Incrementer>();
//     auto inc_p1_s3 = std::make_shared<Incrementer>();
//     auto inc_p1_s4 = std::make_shared<Incrementer>();
//     auto inc_p2_s1 = std::make_shared<Incrementer>();
//     auto inc_p2_s2 = std::make_shared<Incrementer>();
//     auto inc_p3_s1 = std::make_shared<Incrementer>();
//     auto probe = std::make_shared<BranchSyncProbe>();

//     circuit->AddComponent( counter );

//     circuit->AddComponent( inc_p1_s1 );
//     circuit->AddComponent( inc_p1_s2 );
//     circuit->AddComponent( inc_p1_s3 );
//     circuit->AddComponent( inc_p1_s4 );

//     circuit->AddComponent( inc_p2_s1 );
//     circuit->AddComponent( inc_p2_s2 );

//     circuit->AddComponent( inc_p3_s1 );

//     circuit->AddComponent( probe );

//     // Wire branch 1
//     circuit->ConnectOutToIn( counter, 0, inc_p1_s1, 0 );
//     circuit->ConnectOutToIn( inc_p1_s1, 0, inc_p1_s2, 0 );
//     circuit->ConnectOutToIn( inc_p1_s2, 0, inc_p1_s3, 0 );
//     circuit->ConnectOutToIn( inc_p1_s3, 0, inc_p1_s4, 0 );
//     circuit->ConnectOutToIn( inc_p1_s4, 0, probe, 0 );

//     // Wire branch 2
//     circuit->ConnectOutToIn( counter, 0, inc_p2_s1, 0 );
//     circuit->ConnectOutToIn( inc_p2_s1, 0, inc_p2_s2, 0 );
//     circuit->ConnectOutToIn( inc_p2_s2, 0, probe, 1 );

//     // Wire branch 3
//     circuit->ConnectOutToIn( counter, 0, inc_p3_s1, 0 );
//     circuit->ConnectOutToIn( inc_p3_s1, 0, probe, 2 );

//     // Tick the circuit 100 times
//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//         circuit->Tick();
//     }

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 30, 4 ) );

//     for ( int i = 0; i < 100; ++i )
//     {
//         circuit->Tick();
//         circuit->Tick();
//     }
// }

// TEST_CASE( "DisconnectComponentRegressionTest" )
// {
//     auto circuit = std::make_shared<Circuit>();

//     auto counter = std::make_shared<Counter>();
//     auto probe = std::make_shared<NullInputProbe>();

//     circuit->AddComponent( counter );
//     circuit->AddComponent( probe );

//     circuit->ConnectOutToIn( counter, 0, probe, 0 );
//     circuit->ConnectOutToIn( counter, 0, probe, 1 );

//     circuit->DisconnectComponent( counter );

//     circuit->Tick();
// }

// TEST_CASE( "AutoTickOnCircuitDestructRegressionTest" )
// {
//     auto circuit = std::make_shared<Circuit>();
//     auto counter = std::make_shared<Counter>();
//     circuit->AddComponent( counter );
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 3 ) );

//     REQUIRE( counter->Count() == 0 );
//     circuit->Tick();
//     circuit->Tick();
//     circuit->Tick();
//     circuit->Tick();

//     circuit = nullptr;

//     REQUIRE( counter->Count() == 4 );
// }

// TEST_CASE( "AutoTickOnBuffersUpdateRegressionTest" )
// {
//     auto circuit = std::make_shared<Circuit>();
//     auto counter = std::make_shared<Counter>();
//     circuit->AddComponent( counter );
//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 3 ) );

//     REQUIRE( counter->Count() == 0 );
//     circuit->Tick();
//     circuit->Tick();
//     circuit->Tick();
//     circuit->Tick();

//     circuit->SetThreadPool( std::make_shared<ThreadPool>( 2 ) );

//     REQUIRE( counter->Count() == 4 );
// }

TEST_CASE( "TenThousandComponents" )
{
    auto circuit = std::make_shared<Circuit>();

    auto source = std::make_shared<Counter>();
    auto dest = std::make_shared<ThreadingProbe>( 500 );
    circuit->AddComponent( source );
    circuit->AddComponent( dest );

    for ( int i = 0; i < 500; i++ )
    {
        Component::SPtr last = source;
        for ( int j = 0; j < 20; j++ )
        {
            auto passthrough = std::make_shared<PassThrough>();
            circuit->AddComponent( passthrough );
            circuit->ConnectOutToIn( last, 0, passthrough, 0 );
            last = passthrough;
        }
        circuit->ConnectOutToIn( last, 0, dest, i );
    }

    // circuit->Sort();

    {
        circuit->SetThreadPool( std::make_shared<ThreadPool>( 1 ) );

        auto begin = std::chrono::high_resolution_clock::now();

        int iterationCount = 1000;

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "10000 components tick time (1x buffers, 1x threads per buffer): " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetThreadPool( std::make_shared<ThreadPool>( 2 ) );

        auto begin = std::chrono::high_resolution_clock::now();

        int iterationCount = 1000;

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "10000 components tick time (2x buffers, 1x threads per buffer): " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetThreadPool( std::make_shared<ThreadPool>( 3 ) );

        auto begin = std::chrono::high_resolution_clock::now();

        int iterationCount = 1000;

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "10000 components tick time (3x buffers, 1x threads per buffer): " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetThreadPool( std::make_shared<ThreadPool>( 4 ) );

        auto begin = std::chrono::high_resolution_clock::now();

        int iterationCount = 1000;

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "10000 components tick time (4x buffers, 1x threads per buffer): " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetThreadPool( std::make_shared<ThreadPool>( 1, 2 ) );

        auto begin = std::chrono::high_resolution_clock::now();

        int iterationCount = 1000;

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "10000 components tick time (1x buffers, 2x threads per buffer): " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetThreadPool( std::make_shared<ThreadPool>( 2, 2 ) );

        auto begin = std::chrono::high_resolution_clock::now();

        int iterationCount = 1000;

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "10000 components tick time (2x buffers, 2x threads per buffer): " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetThreadPool( std::make_shared<ThreadPool>( 3, 2 ) );

        auto begin = std::chrono::high_resolution_clock::now();

        int iterationCount = 1000;

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "10000 components tick time (3x buffers, 2x threads per buffer): " << diff_ms / iterationCount << "ms\n";
    }
    {
        circuit->SetThreadPool( std::make_shared<ThreadPool>( 4, 2 ) );

        auto begin = std::chrono::high_resolution_clock::now();

        int iterationCount = 1000;

        for ( int i = 0; i < iterationCount; i++ )
        {
            circuit->Tick();
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto diff_ms = std::chrono::duration_cast<std::chrono::microseconds>( end - begin ).count() / 1000.0;

        std::cout << "10000 components tick time (4x buffers, 2x threads per buffer): " << diff_ms / iterationCount << "ms\n";
    }
}
