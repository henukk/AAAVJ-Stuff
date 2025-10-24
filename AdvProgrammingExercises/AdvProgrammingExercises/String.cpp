#include "String.h"
#include <cassert>

//CONFIG
bool OPTIMIZED_COMPARATOR = false;

//PRIVATE
String::String(char* init, const unsigned long long size) noexcept : chars(init), size(size), hash(StringHash()) {}

//PUBLIC
#pragma region CONSTRUCTORS & DESTRUCTORS
	String::String() noexcept : chars(nullptr), size(0), hash(StringHash()) {}
	String::String(const char* init) : size(0), hash(StringHash()) {
		assert(init != nullptr && "ASSERT FAIL: String constructor: init pointer is null\n");
		while (init[size] != '\0') {
			++size;
		}

		chars = new char[size + 1];
		for (unsigned long long i = 0; i < size; ++i) {
			chars[i] = init[i];
		}
		chars[size] = '\0';
	}
	String::String(const String& other) : chars(nullptr), size(other.size), hash(StringHash(other.hash)) {
		if (size == 0) {
			chars = nullptr;
		}
		else {
			chars = new char[size + 1];
			for (unsigned long long i = 0; i < size; ++i) {
				chars[i] = other[i];
			}
			chars[size] = '\0';
		}
	}
	String::String(String&& other) noexcept : chars(other.chars), size(other.size), hash(other.hash) {
		other.chars = nullptr;
		other.size = 0;
	}

	String::~String() noexcept {
		if (chars) delete[] chars;
	}
#pragma endregion

#pragma region GETTERS
	unsigned long long String::getSize() const noexcept {
		return size;
	}
	bool String::isEmpty() const noexcept {
		return size == 0;
	}
#pragma endregion

#pragma region SETTERS
#pragma endregion

#pragma region OPERATORS
	char& String::operator[](unsigned long long i) { //modificador de caracter -> por eso reset hash
		assert(chars != nullptr && "ASSERT FAIL: String::operator[] - chars is null");
		assert(i < size && "ASSERT FAIL: String::operator[] - index out of bounds");
		hash.reset();
		return chars[i];
	}
	char String::operator[](unsigned long long i) const noexcept { //consultor de caracter
		assert(chars != nullptr && "ASSERT FAIL: String::operator[] - chars is null");
		assert(i < size && "ASSERT FAIL: String::operator[] - index out of bounds");
		return chars[i];
	}
	bool String::operator==(const String& other) const noexcept {
		if (this == &other) {
			return true;
		}
		if (size != other.size) {
			return false;
		}
		if (size == 0) {
			return true;
		}

		assert(chars != nullptr && "String::operator== - left operand is null");
		assert(other.chars != nullptr && "String::operator== - right operand is null");

		if (OPTIMIZED_COMPARATOR) {
			if (!hash.getHashed()) {
				hash.generateHash(size, chars);
			}
			if (!other.hash.getHashed()) {
				other.hash.generateHash(other.size, other.chars);
			}

			return (hash == other.hash);
		}

		for (unsigned long long curr = 0; curr < size; ++curr) {
			if (chars[curr] != other.chars[curr]) {
				return false;
			}
		}
		return true;
	}

	String String::operator+(const String& other) const {
		//assert(chars != nullptr && "String::operator+ - left operand is null");
		//assert(other.chars != nullptr && "String::operator+ - right operand is null");
		unsigned long long newSize = size + other.size;
		char* newChars = new char[newSize + 1];

		for (unsigned long long i = 0; i < size; ++i)
			newChars[i] = chars[i];
		for (unsigned long long i = 0; i < other.size; ++i)
			newChars[i + size] = other[i];
		newChars[newSize] = '\0';

		return String(newChars, newSize);
	}

	String& String::operator=(const String& other) {
		if (this == &other) {
			return *this;
		}

		char* newChars = nullptr;
		if (other.size > 0 && other.chars != nullptr) {
			newChars = new char[other.size + 1];
			std::copy_n(other.chars, other.size, newChars);
			newChars[other.size] = '\0';
		}

		if (chars) {
			delete[] chars;
		}

		chars = newChars;
		size = other.size;
		hash = other.hash;

		return *this;
	}

	String& String::operator=(String&& other) noexcept {
		if (this == &other) {
			return *this;
		}

		if (chars) {
			delete[] chars;
			chars = nullptr;
		}

		chars = other.chars;
		size = other.size;
		hash = other.hash;

		other.chars = nullptr;
		other.size = 0;

		return *this;
	}
#pragma endregion

#pragma region FUNCTIONS
	unsigned long long String::length() const noexcept {
		return size;
	}
	void String::clear() noexcept {
		if (chars) delete[] chars;
		chars = nullptr;
		size = 0;
		hash.reset();
	}
#pragma endregion

#pragma region UTILS/HELPERS
	void String::print() const noexcept {
		if (chars == nullptr) {
			std::cout << "(null)\n";
		}
		else {
			std::cout << *this << '\n';
		}
	}
#pragma endregion