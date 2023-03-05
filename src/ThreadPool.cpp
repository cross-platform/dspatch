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
        void push( const std::function<void()>& job )
        {
            std::lock_guard<std::mutex> lock( mutex );
            queue.push( job );
            condt.notify_one();
        }

        std::function<void()> pop()
        {
            std::unique_lock<std::mutex> lock( mutex );
            while ( queue.empty() )
            {
                condt.wait( lock );
            }
            auto job = queue.front();
            queue.pop();
            return job;
        }

    private:
        std::queue<std::function<void()>> queue;
        std::mutex mutex;
        std::condition_variable condt;
    };

    ThreadPool( int bufferCount, int threadsPerBuffer )
        : bufferCount( bufferCount )
        , threadsPerBuffer( threadsPerBuffer )
    {
        jobs.resize( bufferCount );
        bufferThreads.resize( bufferCount );
        for ( int i = 0; i < bufferCount; ++i )
        {
            for ( int j = 0; j < threadsPerBuffer; ++j )
            {
                bufferThreads[i].push_back( std::thread( &ThreadPool::Run, this, i ) );
            }
        }
    }

    ~ThreadPool()
    {
        for ( int i = 0; i < bufferCount; ++i )
        {
            for ( int j = 0; j < threadsPerBuffer; ++j )
            {
                jobs[i].push( nullptr );
            }
            for ( int j = 0; j < threadsPerBuffer; ++j )
            {
                bufferThreads[i][j].join();
            }
        }
    }

    void AddJob( int bufferNo, const std::function<void()>& job )
    {
        jobs[bufferNo].push( job );
    }

    void Run( int bufferNo )
    {
        while ( true )
        {
            auto job = jobs[bufferNo].pop();
            if ( !job )
            {
                break;
            }
            job();
        }
    }

    const int bufferCount;
    const int threadsPerBuffer;
    std::vector<std::vector<std::thread>> bufferThreads;
    std::deque<JobQueue> jobs;
};

}  // namespace internal
}  // namespace DSPatch

ThreadPool::ThreadPool( int bufferCount, int threadsPerBuffer )
    : p( new internal::ThreadPool( bufferCount, threadsPerBuffer ) )
{
}

ThreadPool::~ThreadPool() = default;

void ThreadPool::AddJob( int bufferNo, const std::function<void()>& job )
{
    p->AddJob( bufferNo, job );
}
