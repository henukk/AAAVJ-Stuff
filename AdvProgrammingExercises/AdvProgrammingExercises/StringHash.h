#pragma once
class StringHash
{
private:
	constexpr static unsigned long long MAX_HASH = 1e9 + 9;
	const static int p = 31;
	unsigned long long hash;
	bool hashed;

public:
	StringHash() noexcept;
	StringHash(const StringHash& other) noexcept;
	~StringHash() noexcept;
	
	bool operator==(const StringHash& other) const noexcept;
	bool getHashed() const noexcept;
	unsigned long long getHash() const noexcept;

	void generateHash(const unsigned long long size, const char* chars) noexcept;
	void reset() noexcept;
};

