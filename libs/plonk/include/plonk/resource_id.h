#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <variant>

class ResourceId {
public:
	using Variant = std::variant<std::monostate, uint64_t, std::string>;

	static ResourceId next();

	ResourceId() : value(std::monostate()) {}
	ResourceId(uint64_t id) : value(id) {}
	ResourceId(std::string id) : value(id) {}

	bool is_none() const { return std::holds_alternative<std::monostate>(value); }
	bool is_int() const { return std::holds_alternative<uint64_t>(value); }
	bool is_string() const { return std::holds_alternative<std::string>(value); }

	bool operator==(const ResourceId &other) const {
		if (is_none() && other.is_none()) {
			return true;
		}

		if (is_int()) {
			if (!other.is_int()) {
				return false;
			}
			return std::get<uint64_t>(value) == std::get<uint64_t>(other.value);
		}

		if (is_string()) {
			if (!other.is_string()) {
				return false;
			}
			return std::get<std::string>(value) == std::get<std::string>(other.value);
		}

		return false;
	}

private:
	friend std::hash<ResourceId>;
	Variant value;
};

namespace std {
template <> struct hash<ResourceId> {
	size_t operator()(const ResourceId &id) const {
		if (id.is_int()) {
			return std::hash<uint64_t>()(std::get<uint64_t>(id.value));
		}
		if (id.is_string()) {
			return std::hash<std::string>()(std::get<std::string>(id.value));
		}
		return 0;
	}
};
} // namespace std
