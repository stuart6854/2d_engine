#include "memory.hpp"

namespace app::core
{
    static AllocationMetrics s_AllocationMetrics{};

    auto GetAllocationMetrics() -> const AllocationMetrics&
    {
        return s_AllocationMetrics;
    }
}

void* operator new(app::sizet size)
{
    app::core::s_AllocationMetrics.TotalAllocated += size;
    return malloc(size);
}

void operator delete(void* memory, app::sizet size)
{
    app::core::s_AllocationMetrics.TotalFreed += size;
    free(memory);
}