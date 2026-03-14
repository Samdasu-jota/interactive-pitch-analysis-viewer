#pragma once
#include <QImage>
#include <QReadWriteLock>
#include <list>
#include <unordered_map>

// Thread-safe LRU cache mapping frame numbers to decoded QImages.
//
// Thread safety:
//   insert() acquires a write lock.
//   get() / contains() acquire a read lock.
//   This allows concurrent reads from the UI thread while the decode thread writes.
//
// Capacity: configurable (default 60 frames ≈ 2 seconds at 30fps).
class FrameCache {
public:
    explicit FrameCache(int capacity = 60);

    void   insert(int frameNumber, const QImage& image);
    QImage get(int frameNumber) const;
    bool   contains(int frameNumber) const;
    void   clear();
    int    size() const;

private:
    void evictLRU();  // called under write lock

    const int capacity_;

    // LRU list: front = most recently used, back = least recently used
    mutable std::list<int>                         lruOrder_;
    mutable std::unordered_map<int, std::pair<
        QImage,
        std::list<int>::iterator>>                 cache_;

    mutable QReadWriteLock lock_;
};
