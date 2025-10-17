#include "poorcraft/memory/MemoryTracker.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/platform/Platform.h"

namespace PoorCraft {

MemoryTracker& MemoryTracker::getInstance() {
    static MemoryTracker instance;
    return instance;
}

void MemoryTracker::recordAllocation(void* ptr, size_t size, const char* file, int line) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    AllocationInfo info;
    info.size = size;
    info.file = file ? file : "unknown";
    info.line = line;
    info.timestamp = poorcraft::Platform::get_time();
    
    m_Allocations[ptr] = info;
    m_TotalAllocated += size;
    
    if (m_TotalAllocated > m_PeakMemoryUsage) {
        m_PeakMemoryUsage = m_TotalAllocated;
    }
    
    PC_TRACE("[MemoryTracker] Allocated " + std::to_string(size) + " bytes at " + info.file + ":" + std::to_string(line));
}

void MemoryTracker::recordDeallocation(void* ptr) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Allocations.find(ptr);
    if (it != m_Allocations.end()) {
        m_TotalAllocated -= it->second.size;
        PC_TRACE("[MemoryTracker] Deallocated " + std::to_string(it->second.size) + " bytes");
        m_Allocations.erase(it);
    }
}

size_t MemoryTracker::getTotalAllocated() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_TotalAllocated;
}

size_t MemoryTracker::getAllocationCount() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Allocations.size();
}

size_t MemoryTracker::getPeakMemoryUsage() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_PeakMemoryUsage;
}

void MemoryTracker::dumpAllocations() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    PC_INFO("[MemoryTracker] === Memory Allocation Dump ===");
    PC_INFO("[MemoryTracker] Total allocations: " + std::to_string(m_Allocations.size()));
    PC_INFO("[MemoryTracker] Total memory: " + std::to_string(m_TotalAllocated) + " bytes");
    PC_INFO("[MemoryTracker] Peak memory: " + std::to_string(m_PeakMemoryUsage) + " bytes");
    
    for (const auto& [ptr, info] : m_Allocations) {
        double timestampSeconds = std::chrono::duration<double>(info.timestamp.time_since_epoch()).count();
        PC_INFO("[MemoryTracker]   " + std::to_string(info.size) + " bytes at " +
            info.file + ":" + std::to_string(info.line) +
            " (t=" + std::to_string(timestampSeconds) + "s)");
    }
    
    PC_INFO("[MemoryTracker] === End Dump ===");
}

void MemoryTracker::reset() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    m_Allocations.clear();
    m_TotalAllocated = 0;
    m_PeakMemoryUsage = 0;
    
    PC_INFO("[MemoryTracker] Memory tracking reset");
}

} // namespace PoorCraft
