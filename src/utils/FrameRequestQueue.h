#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>

// Thread-safe "latest-only" queue for frame decode requests.
//
// The UI thread pushes frame numbers as the user scrubs the timeline.
// The decode thread calls popLatest() which:
//   1. Blocks until at least one request is available.
//   2. Drains the entire queue and returns ONLY the most recent request.
//
// This "latest-only" pattern prevents the decode thread from falling behind
// during rapid scrubbing — exactly the technique used in real-time sensor
// fusion pipelines where stale data is useless.
class FrameRequestQueue {
public:
    FrameRequestQueue() = default;

    // Push a new frame request. Called from the UI thread.
    void push(int frameNumber);

    // Block until a request arrives, then return the newest one.
    // All older requests are discarded. Called from the decode thread.
    // Returns -1 if the queue has been stopped.
    int popLatest();

    // Signal the decode thread to wake up and exit.
    void stop();

    bool isStopped() const { return stopped_.load(); }

private:
    std::deque<int>         queue_;
    std::mutex              mutex_;
    std::condition_variable cv_;
    std::atomic<bool>       stopped_{false};
};
