/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2020 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "ofMain.h"
#include "ofxImGui.h"
#include "ImGuiFileBrowser.h"
#include "imgui_node_canvas.h"
#include "imgui_plot.h"


namespace ImGuiEx {

//--------------------------------------------------------------
inline void drawOFTexture(ofTexture* tex, float& _tw, float& _th, float& posX, float& posY, float& drawW, float& drawH ){
  if(tex->isAllocated()){
    if(tex->getWidth()/tex->getHeight() >= _tw/_th){
      if(tex->getWidth() > tex->getHeight()){   // horizontal texture
        drawW           = _tw;
        drawH           = (_tw/tex->getWidth())*tex->getHeight();
        posX            = 0;
        posY            = (_th-drawH)/2.0f;
      }else{ // vertical texture
        drawW           = (tex->getWidth()*_th)/tex->getHeight();
        drawH           = _th;
        posX            = (_tw-drawW)/2.0f;
        posY            = 0;
      }
    }else{ // always considered vertical texture
      drawW           = (tex->getWidth()*_th)/tex->getHeight();
      drawH           = _th;
      posX            = (_tw-drawW)/2.0f;
      posY            = 0;
    }

    ImVec2 cursor_pos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(posX,posY+IMGUI_EX_NODE_HEADER_HEIGHT));

    ofxImGui::AddImage(*tex, ofVec2f(drawW, drawH));

    ImGui::SetCursorPos(cursor_pos);

  }
}

//--------------------------------------------------------------
inline void drawOFTextureFromImGui(ofTexture* tex, float& _tw, float& _th, float& posX, float& posY, float& drawW, float& drawH , void (*drawTexture)(ofTexture*,float&,float&,float&,float&,float&,float&)){
    drawTexture(tex,_tw,_th,posX,posY,drawW,drawH);
}

//--------------------------------------------------------------
inline bool getFileDialog(imgui_addons::ImGuiFileBrowser& fileDialog, bool show, std::string text, imgui_addons::ImGuiFileBrowser::DialogMode mode, std::string valid_types, std::string nameProposal = "", float retinaScale = 1.0f){
    if(show) ImGui::OpenPopup(text.c_str());

    if(fileDialog.showFileDialog(text.c_str(), mode, ImVec2(700*retinaScale,380*retinaScale), valid_types.c_str(), nameProposal)){
        return true;
    }

    return false;
}

//--------------------------------------------------------------
inline void drawTimecode(ImDrawList* drawList, int seconds, std::string pre="", bool onDrawList=false, ImVec2 pos=ImVec2(0,0), float fontScale=1.0f){
    int _hours   = static_cast<int>(ceil(seconds)/3600);
    int _minutes = static_cast<int>(ceil(seconds)/60);
    int _seconds = static_cast<int>(round(seconds))%60;

    string _sh, _sm, _ss;

    if(_hours < 10){
        _sh = "0"+ofToString(_hours);
    }else{
        _sh = ofToString(_hours);
    }

    if(_minutes < 10){
        _sm = "0"+ofToString(_minutes);
    }else{
        _sm = ofToString(_minutes);
    }

    if(_seconds < 10){
        _ss = "0"+ofToString(_seconds);
    }else{
        _ss = ofToString(_seconds);
    }

    if(onDrawList){
        char temp[256];
#if defined(TARGET_WIN32)
        sprintf_s(temp,"%s %s:%s:%s", pre.c_str(), _sh.c_str(), _sm.c_str(), _ss.c_str());
#else
        sprintf(temp,"%s %s:%s:%s", pre.c_str(), _sh.c_str(), _sm.c_str(), _ss.c_str());
#endif

        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize()*fontScale, pos, IM_COL32_WHITE,temp, NULL, 0.0f);
    }else{
        ImGui::Text("%s %s:%s:%s", pre.c_str(), _sh.c_str(), _sm.c_str(), _ss.c_str());
    }

}

//--------------------------------------------------------------
inline void drawWaveform(ImDrawList* drawList, ImVec2 dim, float* data, int dataSize, float thickness, ImU32 color, float retinaScale=1.0f){
    // draw signal background
    drawList->AddRectFilled(ImGui::GetWindowPos(),ImGui::GetWindowPos()+dim,IM_COL32_BLACK);

    // draw signal plot
    ImGuiEx::PlotConfig conf;
    conf.values.ys = data;
    conf.values.count = dataSize;
    conf.values.color = color;
    conf.scale.min = -1;
    conf.scale.max = 1;
    conf.tooltip.show = false;
    conf.tooltip.format = "x=%.2f, y=%.2f";
    conf.grid_x.show = false;
    conf.grid_y.show = false;
    conf.frame_size = ImVec2(dim.x - (20*retinaScale), dim.y - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*retinaScale));
    conf.line_thickness = thickness;

    ImGuiEx::Plot("plot", conf);
}

//--------------------------------------------------------------
inline void plotValue(float value, float min, float max, ImU32 color=IM_COL32(255,255,255,255), float retinaScale=1.0f){
    ImGuiEx::PlotVarConfig conf;
    conf.value = value;
    conf.frame_size = ImVec2(ImGui::GetWindowSize().x - 20, ImGui::GetWindowSize().y - ((IMGUI_EX_NODE_HEADER_HEIGHT+IMGUI_EX_NODE_FOOTER_HEIGHT)*retinaScale));
    conf.scale.min = min;
    conf.scale.max = max;
    conf.buffer_size = 256;

    ImGuiEx::PlotVar("", conf,color);
}

//--------------------------------------------------------------
static void HelpMarker(const char* desc){
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()){
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

//--------------------------------------------------------------
static void ObjectInfo(const char* desc, const char* url, float retinaScale=1.0f){
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::InvisibleButton("empty",ImVec2(224*retinaScale,1));  // fix widget width
    if (ImGui::CollapsingHeader("INFO", ImGuiTreeNodeFlags_None)){
        ImGui::TextWrapped("%s",desc);
        ImGui::Spacing();
        if(ImGui::Button("Reference")){
            ofLaunchBrowser(url);
        }
    }
}

}
