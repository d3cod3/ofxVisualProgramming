#include "DraggableVertex.h"

//--------------------------------------------------------------
DraggableVertex::DraggableVertex(float x, float y){
    r.setFromCenter(x,y,DRAG_SIZE,DRAG_SIZE);
    this->x=x;
    this->y=y;
    this->z=0;
    bOver = false;
}

//--------------------------------------------------------------
void DraggableVertex::over(float x, float y){
    bOver = (r.inside(x, y));
}

void DraggableVertex::move(float x, float y){
    r.setFromCenter(this->x, this->y, DRAG_SIZE,DRAG_SIZE);
    this->x=x;
    this->y=y;
}

//--------------------------------------------------------------
void DraggableVertex::drag(float x, float y){
    if (bOver) {
        r.setFromCenter(this->x, this->y, DRAG_SIZE,DRAG_SIZE);
        this->x=x;
        this->y=y;
    }
}

//--------------------------------------------------------------
void DraggableVertex::draw(float x, float y){
    ofPushMatrix();
    ofTranslate(x, y);
    ofRotateZDeg(30);
    ofFill();
    ofDrawCircle(0,0,3);
    ofPopMatrix();
}
