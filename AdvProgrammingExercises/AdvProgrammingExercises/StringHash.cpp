	#include "StringHash.h"
	#include <cassert>

	StringHash::StringHash() noexcept : hash(0), hashed(false) {}
	StringHash::StringHash(const StringHash& other) noexcept : hash(other.hash), hashed(other.hashed) {}
	StringHash::~StringHash() noexcept {}

	bool StringHash::operator==(const StringHash& other) const noexcept {
		assert(!hashed && "ASSERT FAIL: StringHash::operator== - comparing without hashing is null");
		if (!hashed || !other.hashed) {
			return false;
		}
		return (hash == other.hash);
	}

	bool StringHash::getHashed() const noexcept {
		return hashed;
	}

	unsigned long long StringHash::getHash() const noexcept {
		return hash;
	}

	void StringHash::generateHash(const unsigned long long size, const char* chars) noexcept {
		hash = 0;
		unsigned long long p_pow = 1;
		for (unsigned long long i = 0; i < size; ++i) {
			hash = (hash + (chars[i] - 'a' + 1) * p_pow) % MAX_HASH;
			p_pow = (p_pow * p) % MAX_HASH;
		}
		hashed = true;
	}

	void StringHash::reset() noexcept {
		hash = 0;
		hashed = false;
	}
