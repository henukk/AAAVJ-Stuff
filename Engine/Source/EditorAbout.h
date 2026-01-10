#pragma once

class EditorAbout {
private:
    bool& imOpened;
public:
    EditorAbout(bool& imOppened);
    ~EditorAbout();

    void draw(const char* title);
    void closeMyself();
};
