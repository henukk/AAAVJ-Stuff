#pragma once
#include <iostream>

class StringHash;

constexpr bool OPTIMIZED_COMPARATOR = false;
class String {
private:
	char* chars;
	unsigned long long size;
	StringHash* hash;

	String(char* init, const unsigned long long size) noexcept;
public:
#pragma region CONSTRUCTORS & DESTRUCTORS
	String() noexcept;
	String(const char* init);
	String(const String& other);
	String(String&& other) noexcept;

	~String() noexcept;
#pragma endregion

#pragma region GETTERS
	unsigned long long getSize() const noexcept;
	bool isEmpty() const noexcept;
#pragma endregion

#pragma region SETTERS
#pragma endregion

#pragma region OPERATORS
	char& operator[](unsigned long long i);
	char operator[](unsigned long long i) const noexcept;
	bool operator==(const String& other) const noexcept;
	String operator+(const String& other) const;
	friend std::ostream& operator<<(std::ostream& os, const String& s) noexcept {
		if (!s.chars) return os;
		os << s.chars;
		return os;
	}
#pragma endregion

#pragma region FUNCTIONS
	unsigned long long length() const noexcept;
	void clear() noexcept;
#pragma endregion

#pragma region UTILS/HELPERS
	static String GetMeAString() { return String("another string"); }
	void print() const noexcept;
#pragma endregion	
};