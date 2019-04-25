/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2019 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

    ofxVisualProgramming is distributed under the MIT License.
    This gives everyone the freedoms to use ofxVisualProgramming in any context:
    commercial or non-commercial, public or private, open or closed source.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

    See https://github.com/d3cod3/ofxVisualProgramming for documentation

==============================================================================*/

#pragma once

#include "PatchObject.h"

#include "ofxFFmpegRecorder.h"


// from https://github.com/satoruhiga/ofxFastFboReader by Satoru Higa <https://github.com/satoruhiga>
class ofxFastFboReader
{
public:

    ofxFastFboReader(int _num_buffers=3)
    {
         pboIds = NULL;
         async= true;
         index = 0;
         nextIndex = 0;
         num_bytes = 0;
         num_buffers = _num_buffers;
    }

    ~ofxFastFboReader()
    {
        if (pboIds != NULL)
        {
            glDeleteBuffers(num_buffers, pboIds);
            delete [] pboIds;
            pboIds = NULL;
        }
    }

    bool readToPixels(ofFbo &fbo, ofPixelsRef pix, ofImageType type)
    {
        genPBOs();

        int channels;
        int glType;

        if (type == OF_IMAGE_COLOR)
        {
            channels = 3;
            glType = GL_RGB;
        }
        else if (type == OF_IMAGE_COLOR_ALPHA)
        {
            channels = 4;
            glType = GL_RGBA;
        }
        else if (type == OF_IMAGE_GRAYSCALE)
        {
            channels = 1;
            glType = GL_LUMINANCE;
        }
        else
        {
            return false;
        }

        const int width = fbo.getWidth();
        const int height = fbo.getHeight();

        if (async)
        {
            index = (index + 1) % num_buffers;
            nextIndex = (index + 1) % num_buffers;
        }
        else
        {
            index = nextIndex = 0;
        }

        size_t nb = width * height * channels;

        if (nb != num_bytes)
        {
            num_bytes = nb;
            setupPBOs(num_bytes);
        }

        glReadBuffer(GL_FRONT);

        fbo.bind();

        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[index]);
        glReadPixels(0, 0, width, height, glType, GL_UNSIGNED_BYTE, NULL);

        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[nextIndex]);
        unsigned char* mem = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

        if (mem)
        {
            pix.setFromPixels(mem, width, height, channels);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        fbo.unbind();

        return mem != NULL;
    }

    bool getAsync() { return async; }
    void setAsync(bool v) { async = v; }

protected:

    int num_buffers;

    GLuint *pboIds;
    int index, nextIndex;
    size_t num_bytes;
    bool async;

    ofxFastFboReader(const ofxFastFboReader&);
    ofxFastFboReader& operator=(const ofxFastFboReader&);

    void genPBOs()
    {
        if (!pboIds)
        {
            pboIds = new GLuint[num_buffers];
            glGenBuffers(num_buffers, pboIds);
        }
    }

    void setupPBOs(int num_bytes)
    {
        for (int i = 0; i < num_buffers; i++)
        {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[i]);
            glBufferData(GL_PIXEL_PACK_BUFFER, num_bytes, NULL, GL_STREAM_READ);
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

};


class VideoExporter : public PatchObject {

public:

    VideoExporter();

    void            newObject();
    void            setupObjectContent(shared_ptr<ofAppGLFWWindow> &mainWindow);
    void            updateObjectContent(map<int,PatchObject*> &patchObjects, ofxThreadedFileDialog &fd);
    void            drawObjectContent(ofxFontStash *font);
    void            removeObjectContent();

    void            mouseMovedObjectContent(ofVec3f _m);
    void            dragGUIObject(ofVec3f _m);
    void            fileDialogResponse(ofxThreadedFileDialogResponse &response);

    void            onToggleEvent(ofxDatGuiToggleEvent e);
    void            onTextInputEvent(ofxDatGuiTextInputEvent e);

    ofxFFmpegRecorder   recorder;
    ofxFastFboReader    reader;
    ofFbo               captureFbo;
    ofPixels            capturePix;

    bool                needToGrab;

    float               posX, posY, drawW, drawH;
    bool                isNewObject;

    bool                exportVideoFlag;
    bool                videoSaved;

    ofxDatGui*          gui;
    ofxDatGuiHeader*    header;
    ofxDatGuiToggle*    recButton;

};
