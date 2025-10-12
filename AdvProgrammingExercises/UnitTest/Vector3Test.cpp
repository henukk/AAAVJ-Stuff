#include "pch.h"
#include "CppUnitTest.h"
#define NDEBUG
#include "../AdvProgrammingExercises/Vector3.h"
#undef NDEBUG

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    TEST_CLASS(Vector3Test)
    {
    public:
        // ---------------------------------------------------------------
        // 🧱 Constructors & Getters
        // ---------------------------------------------------------------
        TEST_METHOD(DefaultConstructor_ShouldInitToZero)
        {
            Vector3<float> v;
            Assert::AreEqual(0.0f, v.getX(), 0.0001f);
            Assert::AreEqual(0.0f, v.getY(), 0.0001f);
            Assert::AreEqual(0.0f, v.getZ(), 0.0001f);
        }

        TEST_METHOD(Constructor_WithParameters_ShouldAssignValues)
        {
            Vector3<float> v(1.0f, 2.0f, 3.0f);
            Assert::AreEqual(1.0f, v.getX(), 0.0001f);
            Assert::AreEqual(2.0f, v.getY(), 0.0001f);
            Assert::AreEqual(3.0f, v.getZ(), 0.0001f);
        }

        TEST_METHOD(CopyConstructor_ShouldDuplicateValues)
        {
            Vector3<float> original(1.0f, 2.0f, 3.0f);
            Vector3<float> copy(original);

            Assert::AreEqual(original.getX(), copy.getX(), 0.0001f);
            Assert::AreEqual(original.getY(), copy.getY(), 0.0001f);
            Assert::AreEqual(original.getZ(), copy.getZ(), 0.0001f);
        }

        // ---------------------------------------------------------------
        // ➕ ➖ Operators
        // ---------------------------------------------------------------
        TEST_METHOD(OperatorPlus_ShouldAddComponents)
        {
            Vector3<float> a(1, 2, 3);
            Vector3<float> b(4, 5, 6);
            Vector3<float> result = a + b;

            Assert::AreEqual(5.0f, result.getX(), 0.0001f);
            Assert::AreEqual(7.0f, result.getY(), 0.0001f);
            Assert::AreEqual(9.0f, result.getZ(), 0.0001f);
        }

        TEST_METHOD(OperatorMinus_ShouldSubtractComponents)
        {
            Vector3<float> a(5, 7, 9);
            Vector3<float> b(1, 2, 3);
            Vector3<float> result = a - b;

            Assert::AreEqual(4.0f, result.getX(), 0.0001f);
            Assert::AreEqual(5.0f, result.getY(), 0.0001f);
            Assert::AreEqual(6.0f, result.getZ(), 0.0001f);
        }

        // ---------------------------------------------------------------
        // 📏 Magnitude & Normalize
        // ---------------------------------------------------------------
        TEST_METHOD(Magnitude_ShouldReturnLength)
        {
            Vector3<float> v(3, 4, 0);
            Assert::AreEqual(5.0f, v.Magnitude(), 0.0001f);
        }

        TEST_METHOD(Normalize_ShouldReturnUnitVector)
        {
            Vector3<float> v(3, 4, 0);
            Vector3<float> n = v.Normalize();

            Assert::AreEqual(0.6f, n.getX(), 0.0001f);
            Assert::AreEqual(0.8f, n.getY(), 0.0001f);
            Assert::AreEqual(0.0f, n.getZ(), 0.0001f);
        }

        TEST_METHOD(Normalize_ZeroVector_ShouldReturnZeroVector)
        {
            Vector3<float> v(0, 0, 0);
            Vector3<float> n = v.Normalize();

            Assert::AreEqual(0.0f, n.getX(), 0.0001f);
            Assert::AreEqual(0.0f, n.getY(), 0.0001f);
            Assert::AreEqual(0.0f, n.getZ(), 0.0001f);
        }

        // ---------------------------------------------------------------
        // 📐 Distance & Dot Product
        // ---------------------------------------------------------------
        TEST_METHOD(Distance_ShouldMatchPythagoras)
        {
            Vector3<float> a(0, 0, 0);
            Vector3<float> b(3, 4, 0);
            Assert::AreEqual(5.0f, a.distance_to(b), 0.0001f);
        }

        TEST_METHOD(DotProduct_ShouldBeCommutative)
        {
            Vector3<float> a(1, 2, 3);
            Vector3<float> b(4, -5, 6);
            float ab = a.dot_product(b);
            float ba = b.dot_product(a);

            Assert::AreEqual(ab, ba, 0.0001f);
        }

        TEST_METHOD(DotProduct_ShouldReturnCorrectValue)
        {
            Vector3<float> a(1, 2, 3);
            Vector3<float> b(4, -5, 6);
            float expected = 1 * 4 + 2 * -5 + 3 * 6; // 12
            Assert::AreEqual(expected, a.dot_product(b), 0.0001f);
        }

        // ---------------------------------------------------------------
        // 🔄 Cross Product
        // ---------------------------------------------------------------
        TEST_METHOD(CrossProduct_ShouldBePerpendicular)
        {
            Vector3<float> a(1, 0, 0);
            Vector3<float> b(0, 1, 0);
            Vector3<float> c = a.cross_product(b);

            // El resultado debe ser (0, 0, 1)
            Assert::AreEqual(0.0f, c.getX(), 0.0001f);
            Assert::AreEqual(0.0f, c.getY(), 0.0001f);
            Assert::AreEqual(1.0f, c.getZ(), 0.0001f);

            // Además, debe ser perpendicular
            Assert::AreEqual(0.0f, a.dot_product(c), 0.0001f);
            Assert::AreEqual(0.0f, b.dot_product(c), 0.0001f);
        }

        // ---------------------------------------------------------------
        // 🔺 Angle Between
        // ---------------------------------------------------------------
        TEST_METHOD(AngleBetween_ParallelVectors_ShouldBeZero)
        {
            Vector3<float> a(1, 0, 0);
            Vector3<float> b(2, 0, 0);
            Assert::AreEqual(0.0f, a.angle_between(b), 0.0001f);
        }

        TEST_METHOD(AngleBetween_PerpendicularVectors_ShouldBeNinety)
        {
            Vector3<float> a(1, 0, 0);
            Vector3<float> b(0, 1, 0);
            float degrees = a.angle_between(b) * (180.0f / 3.14159265f);
            Assert::AreEqual(90.0f, degrees, 0.1f);
        }

        TEST_METHOD(AngleBetween_OppositeVectors_ShouldBe180)
        {
            Vector3<float> a(1, 0, 0);
            Vector3<float> b(-1, 0, 0);
            float degrees = a.angle_between(b) * (180.0f / 3.14159265f);
            Assert::AreEqual(180.0f, degrees, 0.1f);
        }
    };
}
