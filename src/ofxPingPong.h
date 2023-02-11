class ofxPingPong {
public:
    void allocate( int _width, int _height, int _internalformat = GL_RGBA){
        // Allocate
        for(int i = 0; i < 2; i++)
            FBOs[i].allocate(_width,_height, _internalformat );

        // Clean
        clear();

        // Set everything to 0
        flag = 0;
        swap();
        flag = 0;
    }

    void swap(){
        src = &(FBOs[(flag)%2]);
        dst = &(FBOs[++(flag)%2]);
    }

    void clear(){
        for(int i = 0; i < 2; i++){
            FBOs[i].begin();
            ofClear(0,0);
            FBOs[i].end();
        }
    }

    ofFbo& operator[]( int n ){ return FBOs[n];}

    ofFbo   *src;       // Source       ->  Ping
    ofFbo   *dst;       // Destination  ->  Pong

private:
    ofFbo   FBOs[2];    // Real addresses of ping/pong FBO´s
    int     flag;       // Integer for making a quick swap
};
