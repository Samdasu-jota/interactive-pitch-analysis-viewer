#include "FrameCache.h"

FrameCache::FrameCache(int capacity)
    : capacity_(capacity) {}

void FrameCache::insert(int frameNumber, const QImage& image) {
    QWriteLocker locker(&lock_);

    auto it = cache_.find(frameNumber);
    if (it != cache_.end()) {
        // Update existing entry and move to front
        it->second.first = image;
        lruOrder_.erase(it->second.second);
        lruOrder_.push_front(frameNumber);
        it->second.second = lruOrder_.begin();
        return;
    }

    // Evict if at capacity
    if (static_cast<int>(cache_.size()) >= capacity_) {
        evictLRU();
    }

    lruOrder_.push_front(frameNumber);
    cache_[frameNumber] = {image, lruOrder_.begin()};
}

QImage FrameCache::get(int frameNumber) const {
    QWriteLocker locker(&lock_);  // write lock because we update LRU order on access

    auto it = cache_.find(frameNumber);
    if (it == cache_.end()) return {};

    // Move to front (most recently used)
    lruOrder_.erase(it->second.second);
    lruOrder_.push_front(frameNumber);
    it->second.second = lruOrder_.begin();

    return it->second.first;
}

bool FrameCache::contains(int frameNumber) const {
    QReadLocker locker(&lock_);
    return cache_.find(frameNumber) != cache_.end();
}

void FrameCache::clear() {
    QWriteLocker locker(&lock_);
    cache_.clear();
    lruOrder_.clear();
}

int FrameCache::size() const {
    QReadLocker locker(&lock_);
    return static_cast<int>(cache_.size());
}

void FrameCache::evictLRU() {
    // Must be called under write lock
    if (lruOrder_.empty()) return;
    int lruFrame = lruOrder_.back();
    lruOrder_.pop_back();
    cache_.erase(lruFrame);
}
