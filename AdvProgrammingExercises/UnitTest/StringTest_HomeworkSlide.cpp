#include "pch.h"
#include "CppUnitTest.h"
#include "../AdvProgrammingExercises/StringHash.h"
#include "../AdvProgrammingExercises/String.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    TEST_CLASS(StringHomeworkSlideTest)
    {
    public:
        TEST_METHOD(HomeworkSlide_UsageExample)
        {
            String a("hello");
            String b(a);
            String c = a + b;

            // a = "hello"
            // b = "hello"
            // c = "hellohello"
            Assert::AreEqual((unsigned long long) 5, a.length());
            Assert::AreEqual((unsigned long long)5, b.length());
            Assert::AreEqual((unsigned long long)10, c.length());

            Assert::IsTrue(c == String("hellohello"));

            Assert::AreEqual((unsigned long long)10, c.length());

            c.clear();
            Assert::IsTrue(c.isEmpty());
            Assert::AreEqual((unsigned long long) 0, c.getSize());
        }

        TEST_METHOD(HomeworkSlide_ExtraBonus_MoveEfficiency)
        {
            auto GetMeAString = []() -> String {
                return String("another string");
                };

            String a = GetMeAString();

            Assert::IsTrue(a == String("another string"));
            Assert::AreEqual((unsigned long long) 14, a.length());
        }
    };
}
