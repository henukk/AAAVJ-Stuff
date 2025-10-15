#include "pch.h"
#include "CppUnitTest.h"
#include "../AdvProgrammingExercises/String.h"
#define NDEBUG
#undef NDEBUG

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    TEST_CLASS(StringTest)
    {
    public:
        // ---------------------------------------------------------------
        // 🧱 Constructors & Basic Properties
        // ---------------------------------------------------------------
        TEST_METHOD(DefaultConstructor_ShouldInitEmpty)
        {
            String s;
            Assert::AreEqual(true, s.isEmpty());
            Assert::AreEqual((unsigned long long) 0, s.getSize());
        }

        TEST_METHOD(Constructor_FromCStr_ShouldCopyContent)
        {
            String s("hello");
            Assert::AreEqual((unsigned long long) 5, s.getSize());
            Assert::AreEqual(false, s.isEmpty());
            Assert::AreEqual('h', s[0]);
            Assert::AreEqual('o', s[4]);
        }

        TEST_METHOD(CopyConstructor_ShouldDuplicateContent)
        {
            String a("world");
            String b(a);

            Assert::AreEqual(a.getSize(), b.getSize());
            for (int i = 0; i < a.getSize(); ++i)
                Assert::AreEqual(a[i], b[i]);
        }

        TEST_METHOD(MoveConstructor_ShouldTransferOwnership)
        {
            String a("move");
            unsigned long long sizeA = a.getSize();

            String b(std::move(a));
            Assert::AreEqual(sizeA, b.getSize());
            Assert::AreEqual(true, a.isEmpty());  // el movido queda vacío
        }

        // ---------------------------------------------------------------
        // 🧮 Operators
        // ---------------------------------------------------------------
        TEST_METHOD(OperatorIndex_ShouldAccessCharacters)
        {
            String s("abc");
            Assert::AreEqual('a', s[0]);
            Assert::AreEqual('b', s[1]);
            Assert::AreEqual('c', s[2]);
        }

        TEST_METHOD(OperatorPlus_ShouldConcatenateStrings)
        {
            String a("foo");
            String b("bar");
            String c = a + b;

            Assert::AreEqual((unsigned long long) 6, c.getSize());
            Assert::AreEqual('f', c[0]);
            Assert::AreEqual('b', c[3]);
            Assert::AreEqual('r', c[5]);
        }

        TEST_METHOD(OperatorEquals_ShouldReturnTrueForSameContent)
        {
            String a("test");
            String b("test");
            Assert::IsTrue(a == b);
        }

        TEST_METHOD(OperatorEquals_ShouldReturnFalseForDifferentContent)
        {
            String a("test");
            String b("best");
            Assert::IsFalse(a == b);
        }

        // ---------------------------------------------------------------
        // 🔄 Clear & Length
        // ---------------------------------------------------------------
        TEST_METHOD(Clear_ShouldReleaseMemoryAndResetState)
        {
            String s("temporary");
            s.clear();

            Assert::AreEqual(true, s.isEmpty());
            Assert::AreEqual((unsigned long long) 0, s.getSize());
        }

        TEST_METHOD(Length_ShouldMatchSize)
        {
            String s("length");
            Assert::AreEqual((unsigned long long) 6, s.length());
        }

        // ---------------------------------------------------------------
        // 🧰 Utility
        // ---------------------------------------------------------------
        TEST_METHOD(GetMeAString_ShouldReturnExpectedValue)
        {
            String s = String::GetMeAString();
            Assert::IsTrue(s == String("another string"));
        }

        // ---------------------------------------------------------------
        // ⚠️ Edge Cases
        // ---------------------------------------------------------------
        TEST_METHOD(Concatenation_WithEmptyStrings_ShouldBehaveCorrectly)
        {
            String a;
            String b("data");
            String c = a + b;
            Assert::IsTrue(c == b);
        }

        TEST_METHOD(CopyOfEmptyString_ShouldRemainEmpty)
        {
            String a;
            String b(a);
            Assert::IsTrue(b.isEmpty());
        }

        TEST_METHOD(Comparison_WithEmptyStrings_ShouldBeTrue)
        {
            String a;
            String b;
            Assert::IsTrue(a == b);
        }
    };
}
