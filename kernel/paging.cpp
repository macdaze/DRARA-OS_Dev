#include <array>

#include "paging.hpp"
#include "logger.hpp"
#include "asmfunc.h"

namespace {
    const uint64_t kPageSize4K = 4096;
    const uint64_t kPageSize2M = 512 * kPageSize4K;
    const uint64_t kPageSize1G = 512 * kPageSize2M;

    alignas(kPageSize4K) std::array<uint64_t, 5512> pml4_table;
    alignas(kPageSize4K) std::array<uint64_t, 512> pdp_table;
    alignas(kPageSize4K) std::array<std::array<uint64_t, 512>, kPageDirectoryCount> page_directory;
}

void SetupIdentityPageTable() {
    Log(kDebug, "SetupIdentityPageTable is starting!\n");
    pml4_table[0] = reinterpret_cast<uint64_t>(&pdp_table[0]) | 0x003;
    for (int i_pdpt = 0; i_pdpt < page_directory.size(); ++i_pdpt) {
        pdp_table[i_pdpt] = reinterpret_cast<uint64_t>(&page_directory[i_pdpt]) | 0x003;
        for (int i_pd = 0; i_pd < 512; ++i_pd) {
            page_directory[i_pdpt][i_pd] = i_pdpt * kPageSize1G + i_pd * kPageSize2M | 0x083;
        }
    }
    Log(kDebug, "Setup Page Table is finished!\n");
    SetCR3(reinterpret_cast<uint64_t>(&pml4_table[0]));
    Log(kDebug, "SetCR3 is finished!\n");
}

uintptr_t MakeIdentityPageTable() {
    Log(kDebug, "SetupIdentityPageTable is starting!\n");
    pml4_table[0] = reinterpret_cast<uint64_t>(&pdp_table[0]) | 0x003;
    for (int i_pdpt = 0; i_pdpt < page_directory.size(); ++i_pdpt) {
        pdp_table[i_pdpt] = reinterpret_cast<uint64_t>(&page_directory[i_pdpt]) | 0x003;
        for (int i_pd = 0; i_pd < 512; ++i_pd) {
            page_directory[i_pdpt][i_pd] = i_pdpt * kPageSize1G + i_pd * kPageSize2M | 0x083;
        }
    }
    return reinterpret_cast<uintptr_t>(&pml4_table[0]);
}

void EditPageTable(uintptr_t *page_table, uintptr_t address_before, uintptr_t address_after, uint64_t range) {
    uintptr_t *PDP_T = reinterpret_cast<uintptr_t *>((page_table[((address_after >> 38) & 0x0000000000000fffull)]) & 0x000ffffffffff000ull);
    uintptr_t *PD = reinterpret_cast<uintptr_t *>((PDP_T[((address_after >> 29) & 0x0000007FC0000000ull)]) & 0x000ffffffffff000ull);
    uintptr_t *PT = reinterpret_cast<uintptr_t *>((PD[((address_after >> 20) & 0x00000000001FF000ull)]) & 0x000ffffffffff000ull);
    unsigned int number_of_pt = range / 12;
    if (number_of_pt == 0) {
        number_of_pt = 1;
    }
    for (int i = 0; i <= number_of_pt; ++i) {
        PT[i] = address_before + (0xfff * i);
    }
}

uintptr_t FindPhysicalAddress(uintptr_t virtual_address, uintptr_t CR3) {
    uintptr_t *PDP_T = reinterpret_cast<uintptr_t *>((reinterpret_cast<uintptr_t *>(CR3)[((virtual_address >> 38) & 0x0000000000000fffull)]) & 0x000ffffffffff000ull);
    uintptr_t *PD = reinterpret_cast<uintptr_t *>((PDP_T[((virtual_address >> 29) & 0x0000007FC0000000ull)]) & 0x000ffffffffff000ull);
    uintptr_t PT = static_cast<uintptr_t>((PD[((virtual_address >> 20) & 0x00000000001FF000ull)]) & 0x000ffffffffff000ull);
    uintptr_t PhysicalAddress = PT + (virtual_address & 0x00000000000007FFull);
    return PhysicalAddress;

}