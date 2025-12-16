#pragma once

class EditorAbout {
private:
    bool& imOpened;
public:
    EditorAbout(bool& imOppened);
    ~EditorAbout();

    void draw(const char* title, bool* p_open = nullptr);
    void closeMyself();
};
