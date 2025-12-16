#pragma once

#include <vector>

class EditorPerformance
{
private:
    std::vector<float> fpsHistory;
    std::vector<float> msHistory;

    size_t current = 0;
    const size_t maxSamples = 600;

public:
    EditorPerformance();
    ~EditorPerformance();

    void draw(const char* title, bool* p_open = nullptr);
};
