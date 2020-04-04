#pragma once

#include "ofMain.h"

class moTextBuffer {

public:
    moTextBuffer();

    void insert(int i);
    void insert(string s);
    void backspace();
    void clear();

    string getText();
    void setText(string s);

    string text;
    string::iterator cursorPosition;

};
