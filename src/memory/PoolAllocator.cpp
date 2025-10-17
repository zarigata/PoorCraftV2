#include "poorcraft/memory/PoolAllocator.h"
#include "poorcraft/core/Logger.h"

namespace PoorCraft {

PoolAllocator::PoolAllocator(size_t elementSize, size_t capacity)
    : m_ElementSize(elementSize)
    , m_Capacity(capacity) {
    
    // Allocate memory block
    m_Memory.resize(elementSize * capacity);
    
    // Initialize free list
    m_FreeList.reserve(capacity);
    for (size_t i = 0; i < capacity; ++i) {
        void* ptr = m_Memory.data() + (i * elementSize);
        m_FreeList.push_back(ptr);
    }
    
    Logger::getInstance().log(LogLevel::INFO, "PoolAllocator", 
        "Created pool: " + std::to_string(capacity) + " elements of " + 
        std::to_string(elementSize) + " bytes each (" + 
        std::to_string(capacity * elementSize) + " bytes total)");
}

PoolAllocator::~PoolAllocator() {
    Logger::getInstance().log(LogLevel::INFO, "PoolAllocator", 
        "Destroyed pool (used: " + std::to_string(m_UsedCount) + "/" + 
        std::to_string(m_Capacity) + ")");
}

void* PoolAllocator::allocate() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    if (m_FreeList.empty()) {
        Logger::getInstance().log(LogLevel::WARN, "PoolAllocator", 
            "Pool exhausted! Cannot allocate more elements.");
        return nullptr;
    }
    
    void* ptr = m_FreeList.back();
    m_FreeList.pop_back();
    m_UsedCount++;
    
    return ptr;
}

void PoolAllocator::deallocate(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    // Verify pointer is within pool bounds
    uint8_t* bytePtr = static_cast<uint8_t*>(ptr);
    uint8_t* poolStart = m_Memory.data();
    uint8_t* poolEnd = poolStart + (m_ElementSize * m_Capacity);
    
    if (bytePtr < poolStart || bytePtr >= poolEnd) {
        Logger::getInstance().log(LogLevel::ERROR, "PoolAllocator", 
            "Attempted to deallocate pointer outside pool bounds!");
        return;
    }
    
    // Verify alignment
    size_t offset = bytePtr - poolStart;
    if (offset % m_ElementSize != 0) {
        Logger::getInstance().log(LogLevel::ERROR, "PoolAllocator", 
            "Attempted to deallocate misaligned pointer!");
        return;
    }
    
    m_FreeList.push_back(ptr);
    m_UsedCount--;
}

void PoolAllocator::reset() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    m_FreeList.clear();
    m_UsedCount = 0;
    
    // Rebuild free list
    for (size_t i = 0; i < m_Capacity; ++i) {
        void* ptr = m_Memory.data() + (i * m_ElementSize);
        m_FreeList.push_back(ptr);
    }
    
    Logger::getInstance().log(LogLevel::INFO, "PoolAllocator", "Pool reset");
}

} // namespace PoorCraft
