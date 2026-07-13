#pragma once

#include <cstddef>

const size_t kPageDirectoryCount = 64;

void SetupIdentityPageTable();

uintptr_t MakeIdentityPageTable();

void EditPageTable(uintptr_t *page_table, uintptr_t address_before, uintptr_t address_after, uint64_t range);

uintptr_t FindPhysicalAddress(uintptr_t virtual_address, uintptr_t CR3);
