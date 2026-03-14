#pragma once
#include <thread>
#include <atomic>
#include <functional>

// Runs a background std::thread that pre-decodes N frames ahead of the
// current playhead, keeping the FrameCache warm during playback.
//
// This eliminates the one-frame decode latency during continuous playback.
// The prefetch distance (default 30 frames) covers ~1 second at 30fps.
class PrefetchService {
public:
    using RequestFrameFn = std::function<void(int frameNumber)>;

    explicit PrefetchService(RequestFrameFn requestFn, int lookahead = 30);
    ~PrefetchService();

    // Start/update prefetching from the given frame position.
    void setPosition(int currentFrame, int totalFrames);
    void stop();

private:
    void prefetchLoop();

    RequestFrameFn      requestFn_;
    int                 lookahead_;

    std::atomic<int>    currentFrame_{0};
    std::atomic<int>    totalFrames_{0};
    std::atomic<bool>   running_{false};
    std::atomic<bool>   dirty_{false};  // new position arrived

    std::thread         thread_;
    std::mutex          mutex_;
    std::condition_variable cv_;
};
