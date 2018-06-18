#pragma once

#include "ofMain.h"

#define DRAG_SIZE 5

class DraggableVertex : public ofVec3f{

public:
    DraggableVertex(float x=0, float y=0);

    void over(float x, float y);
    void drag(float x, float y);
    void move(float x, float y);
    void draw(float x, float y);

    bool bOver;
    ofRectangle r;

};
