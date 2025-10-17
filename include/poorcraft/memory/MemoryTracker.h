#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <cstddef>
#include <chrono>

namespace PoorCraft {

struct AllocationInfo {
    size_t size;
    std::string file;
    int line;
    std::chrono::high_resolution_clock::time_point timestamp;
};

class MemoryTracker {
public:
    static MemoryTracker& getInstance();

    // Record allocations
    void recordAllocation(void* ptr, size_t size, const char* file, int line);
    void recordDeallocation(void* ptr);

    // Statistics
    size_t getTotalAllocated() const;
    size_t getAllocationCount() const;
    size_t getPeakMemoryUsage() const;

    // Debugging
    void dumpAllocations() const;
    void reset();

    // Non-copyable, non-movable
    MemoryTracker(const MemoryTracker&) = delete;
    MemoryTracker& operator=(const MemoryTracker&) = delete;
    MemoryTracker(MemoryTracker&&) = delete;
    MemoryTracker& operator=(MemoryTracker&&) = delete;

private:
    MemoryTracker() = default;
    ~MemoryTracker() = default;

    mutable std::mutex m_Mutex;
    std::unordered_map<void*, AllocationInfo> m_Allocations;
    size_t m_TotalAllocated = 0;
    size_t m_PeakMemoryUsage = 0;
};

} // namespace PoorCraft
