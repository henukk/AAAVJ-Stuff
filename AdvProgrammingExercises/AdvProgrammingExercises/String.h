#pragma once
#include <iostream>
#include <cassert>


constexpr int UNDEFINED_HASH = -1;
class String {
private:
	char* chars;
	int size;
	int hash;

	constexpr inline String(char* init, int size, int hash) noexcept : chars(init), size(size), hash(hash) {}
public:
#pragma region CONSTRUCTORS & DESTRUCTORS
	constexpr inline String() noexcept : chars(nullptr), size(0), hash(UNDEFINED_HASH) {}
	inline String(const char* init) : size(0), hash(UNDEFINED_HASH) {
		assert(init != nullptr && "ASSERT FAIL: String constructor: init pointer is null\n");
		while (init[size] != '\0') {
			++size;
		}

		chars = new char[size + 1];
		for (int i = 0; i < size; ++i) {
			chars[i] = init[i];
		}
		chars[size] = '\0';
	}
	inline String(const String& other) : chars(nullptr), size(other.size), hash(other.hash) {
		if (size == 0) {
			chars = nullptr;
		}
		else {
			chars = new char[size + 1];
			for (int i = 0; i < size; ++i) {
				chars[i] = other[i];
			}
			chars[size] = '\0';
		}
	}
	inline String(String&& other) noexcept : chars(other.chars), size(other.size), hash(other.hash) {
		other.chars = nullptr;
		other.size = 0;
		other.hash = UNDEFINED_HASH;
	}

	inline ~String() noexcept {
		delete[] chars;
	}
#pragma endregion

#pragma region GETTERS
	constexpr inline int getSize() const noexcept {
		return size;
	}
	constexpr inline int getHash() const noexcept {
		if (hash == UNDEFINED_HASH) {
			;// hash = calculateHash();
		}
		return hash;
	}
	constexpr inline bool isEmpty() const noexcept {
		return size == 0;
	}
#pragma endregion

#pragma region SETTERS
#pragma endregion

#pragma region OPERATORS
	inline char& operator[](int i) { //modificador de carácter -> por eso reset hash
		assert(chars != nullptr && "ASSERT FAIL: String::operator[] - chars is null");
		assert(i >= 0 && i < size && "ASSERT FAIL: String::operator[] - index out of bounds");
		hash = UNDEFINED_HASH;
		return chars[i];
	}
	constexpr char operator[](int i) const noexcept { //consultor de carácter
		assert(chars != nullptr && "ASSERT FAIL: String::operator[] - chars is null");
		assert(i >= 0 && i < size && "ASSERT FAIL: String::operator[] - index out of bounds");
		return chars[i];
	}
	constexpr bool operator==(const String& other) const noexcept {
		assert(chars != nullptr && "String::operator== - left operand is null");			//en RELEASE no se que hará
		assert(other.chars != nullptr && "String::operator== - right operand is null");		//en RELEASE no se que hará
		if (!chars || !other.chars) return chars == other.chars;
		if (size != other.size) return false;
		if (getHash() != other.getHash()) return false;

		bool result = true;
		int curr = 0;
		while (result && curr < size) { //creo que prevé mejor los saltos la cpu así
			result = chars[curr] == other[curr];
			++curr;
		}
		return result;
	}
	inline String operator+(const String& other) const {
		//assert(chars != nullptr && "String::operator+ - left operand is null");
		//assert(other.chars != nullptr && "String::operator+ - right operand is null");
		int newSize = size + other.size;
		char* newChars = new char[newSize + 1];

		for (int i = 0; i < size; ++i)
			newChars[i] = chars[i];
		for (int i = 0; i < other.size; ++i)
			newChars[i + size] = other[i];
		newChars[newSize] = '\0';

		return String(newChars, newSize, UNDEFINED_HASH);
	}
	friend std::ostream& operator<<(std::ostream& os, const String& s) noexcept {
		if (!s.chars) return os;
		os << s.chars;
		return os;
	}
#pragma endregion

#pragma region FUNCTIONS
	constexpr inline int length() const noexcept {
		return size;
	}
	inline void clear() noexcept {
		assert(chars != nullptr && "String::clear() - already null or not initialized");
		delete[] chars;
		chars = nullptr;
		size = 0;
		hash = UNDEFINED_HASH;
	}
#pragma endregion

#pragma region UTILS/HELPERS
	static inline String GetMeAString() { return String("another string"); }
	inline void print() const noexcept {
		if (chars == nullptr) {
			std::cout << "(null)\n";
		}
		else {
			std::cout << *this << '\n';
		}
	}
#pragma endregion	
};