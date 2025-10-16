#include "pch.h"
#include "CppUnitTest.h"
#include "../AdvProgrammingExercises/String.h"
#include "../AdvProgrammingExercises/StringHash.h"
#include <chrono>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace std::chrono;

namespace UnitTest
{
    TEST_CLASS(StringTest_Comparison)
    {
    public:

        // 🧭 Función auxiliar para medir tiempo
        long long MeasureComparisonTime(size_t length, size_t repetitions, bool useHash)
        {
            // Configurar flag en tiempo de ejecución
            OPTIMIZED_COMPARATOR = useHash;

            std::string raw(length, 'a');
            String A(raw.c_str());
            String B(raw.c_str());

            auto start = high_resolution_clock::now();
            for (size_t i = 0; i < repetitions; ++i)
            {
                volatile bool result = (A == B);
            }
            auto end = high_resolution_clock::now();
            return duration_cast<milliseconds>(end - start).count();
        }

        TEST_METHOD(ComparePerformance_HashVsNoHash)
        {
            std::ostringstream oss;
            oss << "=== Performance Benchmark ===\n";

            struct TestCase { size_t len; size_t reps; };
            std::vector<TestCase> tests = {
                {100, 100000},
                {10000, 1000},
                {500000, 20}
            };

            for (auto& t : tests)
            {
                long long t_nohash = MeasureComparisonTime(t.len, t.reps, false);
                long long t_hash = MeasureComparisonTime(t.len, t.reps, true);

                double speedup = t_hash > 0 ? (double)t_nohash / t_hash : (double)t_nohash / 1.0;

                oss << "Length = " << t.len
                    << " | Reps = " << t.reps
                    << " | NoHash = " << t_nohash << " ms"
                    << " | Hash = " << t_hash << " ms"
                    << " | Speedup = " << fixed << setprecision(2) << speedup << "x\n";
            }

            oss << "=============================\n";

            Logger::WriteMessage(oss.str().c_str());

            // Puedes dejar un assert simbólico si quieres
            Assert::IsTrue(true);
        }
        TEST_METHOD(Compare_LongTexts_MultipleWords)
        {
            OPTIMIZED_COMPARATOR = true;

            std::string longTextA =
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                "Vestibulum in ligula tincidunt, ultrices urna vel, gravida erat. "
                "Phasellus facilisis, massa nec posuere posuere, justo risus posuere libero, "
                "id malesuada lorem justo non orci. Donec congue, ex ut cursus dignissim, "
                "nibh odio cursus felis, at scelerisque velit justo sed nulla. ";

            std::string longTextB = longTextA; // idéntico
            String A(longTextA.c_str());
            String B(longTextB.c_str());

            Assert::IsTrue(A == B);

            // Ahora modificamos un carácter al final
            longTextB.back() = 'X';
            String C(longTextB.c_str());
            Assert::IsFalse(A == C);

            Logger::WriteMessage(L"✅ Compare_LongTexts_MultipleWords passed.\n");
        }

        TEST_METHOD(Compare_HashCollision_Check)
        {
            OPTIMIZED_COMPARATOR = true;

            std::vector<std::string> words = {
                "apple", "banana", "grape", "orange", "pineapple", "watermelon",
                "kiwi", "mango", "strawberry", "blueberry", "blackberry", "peach",
                "pear", "plum", "cherry", "melon", "lemon", "lime"
            };

            for (size_t i = 0; i < words.size(); ++i)
            {
                for (size_t j = 0; j < words.size(); ++j)
                {
                    String A(words[i].c_str());
                    String B(words[j].c_str());
                    bool result = (A == B);
                    if (i == j)
                        Assert::IsTrue(result);
                    else
                        Assert::IsFalse(result);
                }
            }

            Logger::WriteMessage(L"✅ Compare_HashCollision_Check passed.\n");
        }

        TEST_METHOD(ComparePerformance_MixedConcatenatedTexts)
        {
            OPTIMIZED_COMPARATOR = true;

            // Crear textos de gran tamaño con palabras distintas
            std::ostringstream builderA, builderB;
            for (int i = 0; i < 5000; ++i)
            {
                builderA << "word" << i << " ";
                builderB << "word" << i << " ";
            }

            String A(builderA.str().c_str());
            String B(builderB.str().c_str());
            String C = A + String(" extra"); // C es más largo

            auto start = high_resolution_clock::now();
            bool eqAB = (A == B);
            bool eqAC = (A == C);
            auto end = high_resolution_clock::now();

            auto duration = duration_cast<milliseconds>(end - start).count();

            Assert::IsTrue(eqAB);
            Assert::IsFalse(eqAC);

            std::ostringstream msg;
            msg << "Mixed large text comparison completed in " << duration << " ms.\n";
            Logger::WriteMessage(msg.str().c_str());
        }

    };
}
