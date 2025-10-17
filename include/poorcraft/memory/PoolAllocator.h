#pragma once

#include <vector>
#include <mutex>
#include <cstddef>
#include <cstdint>
#include <new>

namespace PoorCraft {

class PoolAllocator {
public:
    PoolAllocator(size_t elementSize, size_t capacity);
    ~PoolAllocator();

    // Allocation
    void* allocate();
    void deallocate(void* ptr);
    void reset();

    // Getters
    size_t getElementSize() const { return m_ElementSize; }
    size_t getCapacity() const { return m_Capacity; }
    size_t getUsedCount() const { return m_UsedCount; }
    size_t getFreeCount() const { return m_Capacity - m_UsedCount; }

    // Non-copyable, non-movable
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    PoolAllocator& operator=(PoolAllocator&&) = delete;

private:
    size_t m_ElementSize;
    size_t m_Capacity;
    size_t m_UsedCount = 0;
    
    std::vector<uint8_t> m_Memory;
    std::vector<void*> m_FreeList;
    
    mutable std::mutex m_Mutex;
};

// Template version for type-safe allocation
template<typename T>
class TypedPoolAllocator {
public:
    explicit TypedPoolAllocator(size_t capacity)
        : m_Pool(sizeof(T), capacity) {}

    template<typename... Args>
    T* construct(Args&&... args) {
        void* ptr = m_Pool.allocate();
        if (!ptr) return nullptr;
        
        return new (ptr) T(std::forward<Args>(args)...);
    }

    void destroy(T* ptr) {
        if (!ptr) return;
        
        ptr->~T();
        m_Pool.deallocate(ptr);
    }

    void reset() {
        m_Pool.reset();
    }

    size_t getCapacity() const { return m_Pool.getCapacity(); }
    size_t getUsedCount() const { return m_Pool.getUsedCount(); }
    size_t getFreeCount() const { return m_Pool.getFreeCount(); }

private:
    PoolAllocator m_Pool;
};

} // namespace PoorCraft
