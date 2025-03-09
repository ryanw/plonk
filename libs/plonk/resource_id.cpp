#include "include/plonk/resource_id.h"

std::atomic<uint64_t> NEXT_RESOURCE_ID(0);

ResourceId ResourceId::next() {
	auto next_id = NEXT_RESOURCE_ID.fetch_add(1, std::memory_order::relaxed);
	return ResourceId(next_id);
}
