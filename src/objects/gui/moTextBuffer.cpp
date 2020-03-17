#include "moTextBuffer.h"

moTextBuffer::moTextBuffer(){
    text = "Comment Block";
    cursorPosition = text.end();
}

string moTextBuffer::getText() {
    return text;
}
void moTextBuffer::setText(string t) {
    text = t;
    cursorPosition = text.end();
}

void moTextBuffer::insert(int key) {
    text.insert(cursorPosition, key);
    cursorPosition = text.end();
}

void moTextBuffer::insert(const string s) {
    text.insert(cursorPosition, s.begin(), s.end());
    cursorPosition = text.end();
}

void moTextBuffer::backspace() {
    text.pop_back();
    cursorPosition = text.end();
}


void moTextBuffer::clear() {
    text.clear();
    cursorPosition = text.end();
}


