/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2023, Marcus Tomlinson

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

#include <dspatch/ThreadPool.h>

#include <internal/ComponentThread.h>

#include <mutex>
#include <queue>
#include <thread>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class ThreadPool
{
public:
    class JobQueue final
    {
    public:
        void push( ComponentThread* job )
        {
            std::lock_guard<std::mutex> lock( mutex );
            queue.push( job );
            condt.notify_all();
        }

        ComponentThread* pop()
        {
            if ( !queue.empty() )
            {
                std::lock_guard<std::mutex> lock( mutex );
                auto job = queue.front();
                queue.pop();
                return job;
            }

            std::unique_lock<std::mutex> lock( mutex );
            // cppcheck-suppress knownConditionTrueFalse
            if ( queue.empty() )
            {
                condt.wait( lock );
            }
            // cppcheck-suppress containerOutOfBounds
            auto job = queue.front();
            // cppcheck-suppress containerOutOfBounds
            queue.pop();
            return job;
        }

    private:
        std::queue<ComponentThread*> queue;
        std::mutex mutex;
        std::condition_variable condt;
    };

    ThreadPool( int bufferCount, int threadsPerBuffer )
        : c_bufferCount( bufferCount < 0 ? 0 : bufferCount )
        , c_threadsPerBuffer( c_bufferCount == 0 || threadsPerBuffer <= 1 ? 0 : threadsPerBuffer )
    {
        if ( c_threadsPerBuffer == 0 )
        {
            return;
        }

        nextThread.resize( c_bufferCount );
        bufferThreads.resize( c_bufferCount );
        bufferQueues.resize( c_bufferCount );
        for ( int i = 0; i < c_bufferCount; ++i )
        {
            nextThread[i] = 0;
            bufferQueues[i].resize( c_threadsPerBuffer );
            for ( int j = 0; j < c_threadsPerBuffer; ++j )
            {
                bufferThreads[i].push_back( std::thread( &ThreadPool::Run, &bufferQueues[i][j] ) );
            }
        }
    }

    ~ThreadPool()
    {
        for ( int i = 0; i < c_bufferCount; ++i )
        {
            for ( int j = 0; j < c_threadsPerBuffer; ++j )
            {
                bufferQueues[i][j].push( nullptr );
                bufferThreads[i][j].join();
            }
        }
    }

    void AddJob( int bufferNo, ComponentThread* job )
    {
        bufferQueues[bufferNo][nextThread[bufferNo]].push( job );

        if ( ++nextThread[bufferNo] == c_threadsPerBuffer )
        {
            nextThread[bufferNo] = 0;
        }
    }

    static void Run( JobQueue* jobs )
    {
        while ( true )
        {
            auto job = jobs->pop();
            if ( !job )
            {
                break;
            }
            job->Run();
        }
    }

    const int c_bufferCount;
    const int c_threadsPerBuffer;

    std::vector<int> nextThread;
    std::vector<std::vector<std::thread>> bufferThreads;
    std::vector<std::deque<JobQueue>> bufferQueues;
};

}  // namespace internal
}  // namespace DSPatch

ThreadPool::ThreadPool( int bufferCount, int threadsPerBuffer )
    : p( new internal::ThreadPool( bufferCount, threadsPerBuffer ) )
{
}

ThreadPool::~ThreadPool()
{
    delete p;
}

int ThreadPool::GetBufferCount() const
{
    return p->c_bufferCount;
}

int ThreadPool::GetThreadsPerBuffer() const
{
    return p->c_threadsPerBuffer;
}

void ThreadPool::AddJob( int bufferNo, internal::ComponentThread* job )
{
    p->AddJob( bufferNo, job );
}
