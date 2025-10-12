#include "pch.h"
#include "CppUnitTest.h"
#include "../AdvProgrammingExercises/Vector3.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    TEST_CLASS(Vector3Test_HomeworkSlide)
    {
    public:
        TEST_METHOD(ClassroomSlide_Task)
        {
            Vector3<float> a;            // (0,0,0)
            Vector3<float> b(1, 0, 1);   // (1,0,1)
            Vector3<float> c(b);         // copia de b
            Vector3<float> d = b + c;    // (2,0,2)

            Vector3<float> n = d.Normalize();
            Assert::AreEqual(0.7071f, n.getX(), 0.001f);
            Assert::AreEqual(0.0f, n.getY(), 0.001f);
            Assert::AreEqual(0.7071f, n.getZ(), 0.001f);

            float distance = d.distance_to(b);
            Assert::AreEqual(1.4142f, distance, 0.001f);

            float dot = d.dot_product(b);
            Assert::AreEqual(4.0f, dot, 0.0001f);

            Vector3<float> cross = d.cross_product(b);
            Assert::AreEqual(0.0f, cross.getX(), 0.0001f);
            Assert::AreEqual(0.0f, cross.getY(), 0.0001f);
            Assert::AreEqual(0.0f, cross.getZ(), 0.0001f);

            float angleRadians = d.angle_between(b);
            float angleDegrees = angleRadians * (180.0f / 3.14159265f);
            Assert::AreEqual(0.0f, angleDegrees, 0.1f);
        }
    };
}
