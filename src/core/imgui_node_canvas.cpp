/*==============================================================================

    ImGuiExtension : NodeCanvas

    Copyright (c) 2020 Daan de Lange - http://daandelange.com/

    Mosaic is distributed under the MIT License. This gives everyone the
    freedoms to use Mosaic in any context: commercial or non-commercial,
    public or private, open or closed source.

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

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    DESCRIPTION
    This is an ImGui extension that allows drawing nodes on a canvas.
    No node engine nor logic is included; this is pure GUI stuff.

    INFO
    If you edit this file, I recommend working with the ImGui inspector ImGui::ShowMetricsWindow().
    Everything is vanilla ImGui using only core API calls.

    Tested with IMGUI 1.75 WIP
    https://github.com/ocornut/imgui/
    Inspired by several other node projects mentioned in https://github.com/ocornut/imgui/issues/306

    Made for Mocaic, but could be of use in any other project.
    https://mosaic.d3cod3.org/


==============================================================================*/

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_node_canvas.h"
#include <type_traits>

static inline ImVec2 ImAbs(const ImVec2& lhs) {
    return ImVec2(lhs.x > 0.0f ? lhs.x : std::abs(lhs.x), lhs.y > 0.0f ? lhs.y : std::abs(lhs.y));
}

bool ImGuiEx::NodeCanvas::Begin(const char* _id/*, const ImVec2& _size*/){

    // Verify orchestration of ImGui function calls
    IM_ASSERT(isDrawingCanvas == false); // Always call End(), even when Begin() returns true.
    IM_ASSERT(isDrawingNode == false); // forgot to EndNode()
    IM_ASSERT(canDrawNode == false); // forgot to End()

    // Spawn container window
    // You can set pos and size like any imgui window ( ImGui::setNextWindowPos )
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0)); // full frame
    bool ret = ImGui::Begin(_id, NULL,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoScrollbar
        //| ImGuiWindowFlags_NoMouseInputs
        );
    ImGui::PopStyleVar();

    // Allow tinier windows for nodes.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(IMGUI_EX_NODE_MIN_WIDTH, IMGUI_EX_NODE_MIN_HEIGHT) );

    // Store a common canvas draw buffer.
    canvasDrawList = ImGui::GetWindowDrawList();

    // todo : return early when invisible ? (Can this window be invisible ?)

    // Never focus to this window ?
    //ImGui::SetWindowFocus(false);

    // Not needed but could be later if ImGui takes over canvas scrolling.
    // Emit dummy widget matching bounds of the canvas to the window know the content size
//    ImGui::SetCursorScreenPos(canvasView.viewRect.Min);
//    ImGui::Dummy(canvasView.contentRect.GetSize());

    canDrawNode = ret;
    isDrawingCanvas = true;
    isDrawingNode = false;
    return ret;
}

void ImGuiEx::NodeCanvas::End(){
    // check calls
    IM_ASSERT(isDrawingCanvas == true); // // Begin() wasn't called
    IM_ASSERT(isDrawingNode == false); // Forgot to call EndNode()

    isAnyCanvasNodeHovered = !ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

    // reset cursor pos to canvas window
    ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());

# if __IMGUI_EX_NODECANVAS_DEBUG__
    // Visualise Canvas Window size
    ImGui::GetForegroundDrawList()->AddRect(ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax(), IM_COL32(255,255,0,255));
# endif

    // Close Canvas window
    ImGui::PopStyleVar(); // restore min win size
    ImGui::End();

    // Update state
    isDrawingCanvas = false;
    canDrawNode = false;
    canvasDrawList = nullptr;
}

void ImGuiEx::NodeCanvas::DrawFrameBorder(const bool& _drawOnForeground) const {
    // Only use between NodeCanvas::Begin() and End().
    IM_ASSERT(isDrawingCanvas == true);  // forgot to Begin();
    IM_ASSERT(canDrawNode == true); // Don't call when Begin() returned true !
    IM_ASSERT(isDrawingNode == false); // Don't call within BeginNode() !

    // visualise viewport with a small offset
    ImDrawList* layer = _drawOnForeground ? ImGui::GetForegroundDrawList() : canvasDrawList ;
    layer->AddRect(
        ImGui::GetCurrentWindow()->Rect().Min +ImGui::GetStyle().WindowPadding,
        ImGui::GetCurrentWindow()->Rect().Max-ImGui::GetStyle().WindowPadding,
        //IM_COL32(255, 255, 255, 64)
        ImGui::GetColorU32( ImGuiCol_HeaderActive )
        );
}


// always use EndNode() even if returns false. Like ImGui Windows.
bool ImGuiEx::NodeCanvas::BeginNode( const char* _id, ImVec2& _pos, ImVec2& _size, const int& _numLeftPins, const int& _numRightPins ){
    // Check callstack
    IM_ASSERT(isDrawingCanvas == true);  // forgot to End();
    IM_ASSERT(canDrawNode == true); // Don't call if Begin() returned false
    IM_ASSERT(isDrawingNode == false); // Finish your previous node before staring a new one !

    // Precalc some vars
    ImVec2 nodeScale = ImVec2(1,1)*canvasView.scale;
    curNodeData = NodeLayoutData(canvasView.translation + _pos*nodeScale, _size*nodeScale, canvasView.scale );
    isDrawingNode = true; // to allow End() call

    // Is the node out of sight on canvas ?
    bool isNodeVisible = ImGui::IsRectVisible( curNodeData.outerContentBox.Min, curNodeData.outerContentBox.Max );
    if( !isNodeVisible ){
        curNodeData.zoomName = ImGuiExNodeZoom_Invisible;
        return false;
    }

    // Calc zoom name
    {
        unsigned int curWidth = curNodeData.outerContentBox.GetSize().x;
        if( curWidth < IMGUI_EX_NODE_MIN_WIDTH_SMALL )
            curNodeData.zoomName = ImGuiExNodeZoom_Imploded;
        else if( curWidth < IMGUI_EX_NODE_MIN_WIDTH_NORMAL )
            curNodeData.zoomName = ImGuiExNodeZoom_Small;
        else if( curWidth < IMGUI_EX_NODE_MIN_WIDTH_LARGE )
            curNodeData.zoomName = ImGuiExNodeZoom_Normal;
        else
            curNodeData.zoomName = ImGuiExNodeZoom_Large;
    }
    curNodeData.viewName = ImGuiExNodeView_None;

    // Adapt the layout for pins
    int pinsWidth = 0;
    if(curNodeData.zoomName == ImGuiExNodeZoom_Imploded){
        // Behaviour: if there are any pins, show only pins. Else imploded still have small content.
        pinsWidth = 0;
        if(_numLeftPins > 0 || _numLeftPins > 0)
            pinsWidth = ImFloor(curNodeData.innerContentBox.GetSize().x);
        if(_numLeftPins > 0 && _numLeftPins > 0)
            pinsWidth *= .5f;
    }
    else{
        pinsWidth = (curNodeData.zoomName >= ImGuiExNodeZoom_Large) ? IMGUI_EX_NODE_PINS_WIDTH_LARGE : (curNodeData.zoomName >= ImGuiExNodeZoom_Normal) ? IMGUI_EX_NODE_PINS_WIDTH_NORMAL : IMGUI_EX_NODE_PINS_WIDTH_SMALL;
    }
    // Remove pins space from innerContentBox
    if( _numLeftPins > 0 ){ // Has left pins
        curNodeData.pinsFlags |= ImGuiExNodePinsFlags_Left;
        curNodeData.leftPins.region.Max.x += pinsWidth;
        curNodeData.innerContentBox.Min.x += pinsWidth;

        curNodeData.leftPins.numPins = _numLeftPins;
        curNodeData.leftPins.pinSpace = ImVec2(curNodeData.leftPins.region.GetSize().x, curNodeData.leftPins.region.GetSize().y / _numLeftPins);
        curNodeData.leftPins.curDrawPos = curNodeData.leftPins.region.Min;
    }
    if( _numRightPins > 0 ){ // Has right pins
        curNodeData.pinsFlags |= ImGuiExNodePinsFlags_Right;
        curNodeData.rightPins.region.Min.x -= pinsWidth;
        curNodeData.innerContentBox.Max.x -= pinsWidth;

        curNodeData.rightPins.numPins = _numRightPins;
        curNodeData.rightPins.pinSpace = ImVec2(curNodeData.rightPins.region.GetSize().x, curNodeData.rightPins.region.GetSize().y / _numRightPins);
        curNodeData.rightPins.curDrawPos = ImVec2(curNodeData.rightPins.region.Max.x, curNodeData.rightPins.region.Min.y);
    }
#if __IMGUI_EX_NODECANVAS_DEBUG__
    // For debugging, draw all parts of node
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    fg->AddRect(curNodeData.rightPins.region.Min, curNodeData.rightPins.region.Max, IM_COL32(255,255,255,200));
    fg->AddRect(curNodeData.leftPins.region.Min, curNodeData.leftPins.region.Max, IM_COL32(255,255,255,200));
    fg->AddRect(curNodeData.innerContentBox.Min, curNodeData.innerContentBox.Max, IM_COL32(255,255,255,200));
    fg->AddRect(curNodeData.innerContentBox.Min, curNodeData.innerContentBox.Max, IM_COL32(255,255,255,200));
    fg->AddRect(curNodeData.outerContentBox.Min, curNodeData.outerContentBox.Max, IM_COL32(255,255,255,200));
    fg->AddRect(curNodeData.outerContentBox.Min, ImVec2(curNodeData.outerContentBox.Max.x,curNodeData.outerContentBox.Min.y+IMGUI_EX_NODE_HEADER_HEIGHT), IM_COL32(255,255,255,200));
    fg->AddRect(ImVec2(curNodeData.outerContentBox.Min.x, curNodeData.outerContentBox.Max.y-IMGUI_EX_NODE_FOOTER_HEIGHT), curNodeData.outerContentBox.Max, IM_COL32(255,255,255,200));
#endif
    // Create node window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));//IMGUI_EX_NODE_CONTENT_PADDING,IMGUI_EX_NODE_CONTENT_PADDING));
    ImGui::SetNextWindowPos(curNodeData.outerContentBox.Min);
    ImGui::SetNextWindowSize(curNodeData.outerContentBox.GetSize());
    bool ret = ImGui::Begin(_id, NULL,
            ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBackground
            | ImGuiWindowFlags_NoDecoration
            //| ImGuiWindowFlags_AlwaysUseWindowPadding
            | ImGuiWindowFlags_NoFocusOnAppearing
            );
    // Save draw context
    nodeDrawList = ImGui::GetWindowDrawList();
    bool nodeMenuIsOpen = false;

    ImGui::PopStyleVar(1);

    // Node namespace
    ImGui::PushID(_id);

    // Add clipping Area to mask outer-node space. (wires will have to be drawn somewhere else)
    ImGui::PushClipRect( curNodeData.outerContentBox.Min, curNodeData.outerContentBox.Max, true);

    // Draw Outer region
    {
        // Node BG fill
        nodeDrawList->AddRectFilled( curNodeData.outerContentBox.Min, curNodeData.outerContentBox.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 999.f));//ImGui::GetColorU32(ImGuiCol_WindowBg) );

        // Draw HeaderBar BG
        nodeDrawList->AddRectFilled(
                    curNodeData.outerContentBox.Min,
                    curNodeData.outerContentBox.Min+ImVec2(curNodeData.outerContentBox.GetSize().x, IMGUI_EX_NODE_HEADER_HEIGHT),
                    ImGui::GetColorU32(ImGuiCol_Header)
                    );

        // Draw footer
        nodeDrawList->AddRectFilled(
                    ImVec2(curNodeData.outerContentBox.Min.x, curNodeData.outerContentBox.Max.y-IMGUI_EX_NODE_FOOTER_HEIGHT),
                    curNodeData.outerContentBox.Max,
                    ImGui::GetColorU32(ImGuiCol_Header)
                    );

        // Node border (surrounding)
        nodeDrawList->AddRect( curNodeData.outerContentBox.Min+ImVec2(1,1), curNodeData.outerContentBox.Max-ImVec2(1,1), ImGui::GetColorU32(ImGuiCol_Border) );

        // Header info
        ImGui::SetCursorScreenPos( curNodeData.outerContentBox.Min+ImVec2(5, (IMGUI_EX_NODE_HEADER_HEIGHT-ImGui::CalcTextSize("").y)*.5f) );//canvasView.translation+pos+ImVec2(5,4));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
        ImGui::Text("%s", _id); // title
        ImGui::PopStyleColor();

        // Enable drag on title
        unsigned int curTabsWidth = (curNodeData.zoomName > ImGuiExNodeZoom_Imploded) ? IMGUI_EX_NODE_HEADER_TOOLBAR_WIDTH : 0;
        ImGui::SetCursorScreenPos( curNodeData.outerContentBox.Min );
        ImGui::InvisibleButton( "headerGripBtn", ImVec2( curNodeData.outerContentBox.GetSize().x-curTabsWidth, IMGUI_EX_NODE_HEADER_HEIGHT )  );
        static ImVec2 mouseOffset(0,0);
        static bool isDraggingHeader = false;

        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
            if(!isDraggingHeader){
                mouseOffset = ImGui::GetMousePos()-curNodeData.outerContentBox.Min;
                isDraggingHeader = true;
            }

# if __IMGUI_EX_NODECANVAS_DEBUG__
            // Visualise mouse offset
            ImGui::GetForegroundDrawList()->AddLine(curNodeData.outerContentBox.Min,curNodeData.outerContentBox.Min+mouseOffset,ImGui::ColorConvertFloat4ToU32(ImVec4(0,1,0,1.f)));
# endif
            _pos = (ImGui::GetMousePos()-mouseOffset)*(1.f/canvasView.scale)-canvasView.translation*(1.f/canvasView.scale);// ImGui::GetMouseDragDelta(0);
        }
        else if(ImGui::IsItemDeactivated()){
            if(isDraggingHeader) isDraggingHeader = false;
        }

        // Tab bar and widget info
        if( curNodeData.zoomName > ImGuiExNodeZoom_Imploded ){

            // Tabs
            ImGui::SetCursorScreenPos(ImVec2(curNodeData.outerContentBox.Max.x-IMGUI_EX_NODE_HEADER_TOOLBAR_WIDTH, curNodeData.outerContentBox.Min.y + 3));
            if(ImGui::BeginTabBar("widgetTabs")){//, ImGuiTabBarFlags_FittingPolicyResizeDown)){

                static bool tabInfoOpen = true, tabVisualiseOpen = false, tabParamsOpen = false;
                ImGui::SetCursorScreenPos(ImVec2(curNodeData.outerContentBox.Max.x-IMGUI_EX_NODE_HEADER_TOOLBAR_WIDTH, ImGui::GetCursorScreenPos().y));
                ImGui::SameLine();
                if((tabInfoOpen = ImGui::BeginTabItem("i",NULL, ImGuiTabItemFlags_NoCloseButton))) ImGui::EndTabItem();
                //if(ImGui::BeginTabItem("i", &tabInfoOpen, ImGuiTabItemFlags_NoCloseButton | ImGuiTabItemFlags_NoCloseWithMiddleMouseButton )) ImGui::EndTabItem();
                if((tabVisualiseOpen = ImGui::BeginTabItem("v",NULL, ImGuiTabItemFlags_NoCloseButton)))ImGui::EndTabItem();
                if((tabParamsOpen = ImGui::BeginTabItem("p",NULL, ImGuiTabItemFlags_NoCloseButton)))ImGui::EndTabItem();

                // Set view state
                curNodeData.viewName = tabInfoOpen ? ImGuiExNodeView_Info : tabVisualiseOpen ? ImGuiExNodeView_Visualise : tabParamsOpen ? ImGuiExNodeView_Params : ImGuiExNodeView_None;

                // Node Menu Toggle
                ImGui::SameLine();
                ImGui::Button("#");
                if(ImGui::IsItemActivated()){
                    nodeMenuIsOpen = true;
                }

                ImGui::EndTabBar();

                // draw line over tab bar to fit 100% width
                nodeDrawList->AddLine(
                            curNodeData.outerContentBox.Min+ImVec2(0,IMGUI_EX_NODE_HEADER_HEIGHT - 1),
                            ImVec2( curNodeData.outerContentBox.Max.x, curNodeData.outerContentBox.Min.y + IMGUI_EX_NODE_HEADER_HEIGHT - 1),
                            ImGui::GetColorU32(ImGuiCol_TabHovered)
                            );
            }
        }

        // Draw footer resize handle
        ImGui::SetCursorScreenPos( curNodeData.outerContentBox.Max-ImVec2( IMGUI_EX_NODE_FOOTER_HANDLE_SIZE, IMGUI_EX_NODE_FOOTER_HANDLE_SIZE )  );
        ImGui::InvisibleButton( "footerGripBtn", ImVec2( IMGUI_EX_NODE_FOOTER_HANDLE_SIZE, IMGUI_EX_NODE_FOOTER_HANDLE_SIZE )  );
        static bool isDraggingFooter = false;
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
            if(!isDraggingFooter){
                isDraggingFooter=true;
            }
            // Constrained size
            _size = ImMax(
                (ImGui::GetMousePos()-curNodeData.outerContentBox.Min)*(1.f/canvasView.scale),
                ImVec2(IMGUI_EX_NODE_MIN_WIDTH,IMGUI_EX_NODE_MIN_HEIGHT)
            );
        }
        else if(ImGui::IsItemDeactivated()){
            if(isDraggingFooter) isDraggingFooter = false;
        }

        nodeDrawList->AddTriangleFilled(
                    curNodeData.outerContentBox.Max-ImVec2( 0, IMGUI_EX_NODE_FOOTER_HEIGHT*2 ),
                    curNodeData.outerContentBox.Max,
                    curNodeData.outerContentBox.Max-ImVec2( IMGUI_EX_NODE_FOOTER_HEIGHT*2, 0 ),
                    //ImGui::GetColorU32(ImGuiCol_ResizeGrip)
                    ImGui::GetColorU32(ImGui::IsItemActive()?ImGuiCol_ResizeGripActive:ImGui::IsItemHovered()?ImGuiCol_ResizeGripHovered:ImGuiCol_ResizeGrip )
                    );
    }

    // Return before drawing content, if scale is too small ?
    // todo. (This is already partially handled by BeginNodeContent();)
    if( false && curNodeData.zoomName < ImGuiExNodeZoom_Small ){
        // Fill empty space with color
        //canvasDrawList->AddRectFilled(); // todo

        // Todo: something with : window->SkipItems to prevent drawing to node ?
        return false;
    }

    // reset origin
    ImGui::SetCursorScreenPos( curNodeData.innerContentBox.Min );

    // The combination of a cliprect and columns allows us to set a clipping space for node widgets while reserving drawable space for pins, without having to add an extra window / childframe.
    ImGui::PushClipRect( curNodeData.leftPins.region.Min, curNodeData.rightPins.region.Max, true); // Inner space + Node Spaces
    ImGui::BeginColumns("innerNode", 3,
                        ImGuiColumnsFlags_NoBorder
                        | ImGuiColumnsFlags_NoResize
                        //| ImGuiColumnsFlags_NoPreserveWidths
                        | ImGuiColumnsFlags_NoForceWithinWindow // important so there's no weird auto adjustments.
                        );
    // Column layout
    ImGui::SetColumnOffset(0,0);
    ImGui::SetColumnOffset(1,curNodeData.leftPins.region.GetSize().x);
    ImGui::SetColumnOffset(2,curNodeData.innerContentBox.Max.x-curNodeData.leftPins.region.Min.x);

    // move to middle column where the user can draw
    ImGui::NextColumn();

    // Draw column BG, to mask overlapping nodes
    if( curNodeData.zoomName > ImGuiExNodeZoom_Imploded ){
        ImGui::GetWindowDrawList()->AddRectFilled(curNodeData.innerContentBox.Min, curNodeData.innerContentBox.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 999.f) );
        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING));// Padding top
    }

    // By default, try to draw full width
    ImGui::PushItemWidth(-1.0f); // Todo: doesn't seem have any effect...

    // Allow User to catch the menu and extend it
    if( nodeMenuIsOpen ){
         ImGui::OpenPopup("nodeMenu");
    }

    // Draw default menu items
    if(ImGui::BeginPopup("nodeMenu")){
        ImGui::MenuItem("Delete");
        ImGui::MenuItem("Copy");
        ImGui::MenuItem("Duplicate");
        ImGui::EndPopup();
    }

    return ret;
}

void ImGuiEx::NodeCanvas::EndNode() {
    // Verify ImGui call stack
    IM_ASSERT(isDrawingCanvas == true); // Call ImGuiEx::NodeCanvas::Begin() first !
    IM_ASSERT(isDrawingNode == true); // Call ImGuiEx::NodeCanvas::BeginNode() first !
    IM_ASSERT(isDrawingMenu == false); // You didn't finish your menu ! EndNodeMenu()
    IM_ASSERT(isDrawingContent == false); // Check EndNodeContent(); calls

    if(curNodeData.zoomName > ImGuiExNodeZoom_Invisible ){

        // Only pop these if content is drawn
        if( true || curNodeData.zoomName > ImGuiExNodeZoom_Imploded ){
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING)); // Padding bottom
            ImGui::EndColumns();
            ImGui::PopClipRect(); // Inner space + nodes
        }

        // Always pop these ()
        ImGui::PopClipRect();

        //ImGui::EndGroup();
        ImGui::PopID();
        ImGui::End();
    }

    // manage state
    isDrawingNode = false;
    nodeDrawList = nullptr;
}

void ImGuiEx::NodeCanvas::SetTransform(const ImVec2& _origin, float _scale){
    // Verify orchestration of ImGui function calls
    IM_ASSERT(isDrawingCanvas == false); // Please set the view before NodeCanvas::Begin()

    canvasView.setTransform( _origin, _scale );
}

// Pin functions
//void ImGuiEx::NodeCanvas::BeginNodePins(  const int& _numPins, const ImGuiExNodePinsFlags& _pinFlags ){
//    // Check ImGui Callstack
//    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
//    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
//    //IM_ASSERT(isDrawingPin == false); // Forgot to call EndNodePins() ?

//    // Modify layout and init draw data
//    int pinsWidth = IMGUI_EX_NODE_PINS_WIDTH_SMALL; // todo: adapt to layout here
//    if( _pinFlags==ImGuiExNodePinsFlags_Left ){
//        curNodeData.leftPins.region.Max.x += pinsWidth;
//        curNodeData.innerContentBox.Min.x += pinsWidth;

//        curNodeData.leftPins.numPins = _numPins;
//        curNodeData.leftPins.pinSpace = (curNodeData.leftPins.region.GetSize() / ImVec2(_numPins, _numPins));
//        curNodeData.leftPins.curDrawPos = curNodeData.leftPins.region.Min;
//    }
//    else if( _pinFlags==ImGuiExNodePinsFlags_Right ){
//        curNodeData.rightPins.region.Min.x -= pinsWidth;
//        curNodeData.innerContentBox.Max.x -= pinsWidth;

//        curNodeData.rightPins.numPins = _numPins;
//        curNodeData.rightPins.pinSpace = (curNodeData.rightPins.region.GetSize() / ImVec2(_numPins, _numPins));
//        curNodeData.rightPins.curDrawPos = curNodeData.rightPins.region.Min;
//    }

//    // todo
//    // Push clip rect to endure not to draw over content

//    // Set pins draw state
//    //curPinsNum = _numPins;
//    //isDrawingPin = true;
//    //curPinFlags = _pinFlags;
//}
//void ImGuiEx::NodeCanvas::EndNodePins(){
//    // Check ImGui Callstack
//    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
//    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
//    //IM_ASSERT(isDrawingPin == true); // Forgot to call BeginNodePins() ?

//    // Set pins state
//    //curPinsNum = 0;
//    //isDrawingPin = false;
//    //curPinFlags = ImGuiExNodePinsFlags_Left; // resets
//}

void ImGuiEx::NodeCanvas::AddNodePin( const char* _label, ImVec2& _pinPosition, const ImU32& _color, const ImGuiExNodePinsFlags& _pinFlag ){
    // Check ImGui Callstack
    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
    IM_ASSERT(_pinFlag == ImGuiExNodePinsFlags_Left || _pinFlag == ImGuiExNodePinsFlags_Right); // Only left / right pins can be created
    IM_ASSERT(curNodeData.pinsFlags & _pinFlag); // Node was not created with correct flags

    // Choose a side
    PinLayout& pinLayout = (_pinFlag==ImGuiExNodePinsFlags_Left) ? curNodeData.leftPins : curNodeData.rightPins;
    // Backup cursor
    ImVec2 cursorBackup = ImGui::GetCursorScreenPos();

    // Move to column
    ImGui::NextColumn(); // right col
    if( _pinFlag==ImGuiExNodePinsFlags_Left ) ImGui::NextColumn(); // left column


    // Hover interaction
    if(pinLayout.pinSpace.x > 0){
        if( _pinFlag==ImGuiExNodePinsFlags_Left ){
            ImGui::SetCursorScreenPos( pinLayout.curDrawPos  );
        }
        else {
            ImGui::SetCursorScreenPos( pinLayout.curDrawPos + ImVec2( -pinLayout.pinSpace.x, 0)  );
        }
# if __IMGUI_EX_NODECANVAS_DEBUG__
        ImGui::Button("##nodeBtn", pinLayout.pinSpace);
# else
        ImGui::InvisibleButton("nodeBtn", ImMax(pinLayout.pinSpace, ImVec2(1,1)));
# endif
    }

    // Set position of pin so user can draw it.
    float pinSpace = (ImGui::IsItemHovered()) ? IMGUI_EX_NODE_PIN_WIDTH_HOVERED : IMGUI_EX_NODE_PIN_WIDTH;

    // Draw pin
    if( _pinFlag==ImGuiExNodePinsFlags_Left ){ // Left side
        _pinPosition = pinLayout.curDrawPos + ImVec2( IMGUI_EX_NODE_PIN_WIDTH * .5f, pinLayout.pinSpace.y * .5f);
        nodeDrawList->AddCircleFilled(_pinPosition, pinSpace * .5f, _color, 6);
        nodeDrawList->AddText( _pinPosition + ImVec2(pinSpace * .5f + 2,ImGui::GetTextLineHeight()*-.5f), _color, _label);
    }
    else { // right side
        _pinPosition = pinLayout.curDrawPos + ImVec2( IMGUI_EX_NODE_PIN_WIDTH * -.5f, pinLayout.pinSpace.y * .5f);
        nodeDrawList->AddCircleFilled(_pinPosition, pinSpace * .5f, _color, 6);
        ImVec2 labelSize = ImGui::CalcTextSize(_label);
        nodeDrawList->AddText( _pinPosition + ImVec2(pinSpace * -.5f - 2 -labelSize.x, labelSize.y*-.5f ), _color, _label);
    }
    pinLayout.curDrawPos += ImVec2(0,pinLayout.pinSpace.y);

    // Move back to center column
    if( _pinFlag==ImGuiExNodePinsFlags_Right ) ImGui::NextColumn(); // left column
    ImGui::NextColumn(); // center column

    // Restore cursor
    ImGui::SetCursorScreenPos(cursorBackup);
}

bool ImGuiEx::NodeCanvas::BeginNodeMenu(){
    // Check ImGui Callstack
    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
    IM_ASSERT(isDrawingMenu == false); // Forgot to call EndNodeMenu !

    // returns true if menu is open
    bool ret = ImGui::BeginPopup("nodeMenu");
    if( ret ){
        isDrawingMenu = true;

        // Add separator
        ImGui::Separator();
    }

    return ret;
}
void ImGuiEx::NodeCanvas::EndNodeMenu(){
    // Check ImGui Callstack
    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
    IM_ASSERT(isDrawingMenu == true); // Only call if BeginNodeMenu returned true

    isDrawingMenu = false;
    ImGui::EndPopup();
}

// For drawing content to the node
bool ImGuiEx::NodeCanvas::BeginNodeContent( const ImGuiExNodeView& _renderingView ){
    // Check ImGui Callstack
    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
    IM_ASSERT(isDrawingMenu == false); // Forgot to call EndNodeMenu() !
    IM_ASSERT(isDrawingContent == false);  // Forgot to call EndNodeContent() !

    // Dont allow drawing in invisible nodes
    bool ret = false;
    if( curNodeData.zoomName == ImGuiExNodeZoom_Invisible ){
        ret = false;
    }
    // User can draw to imploded when there are no pins...
    else if( curNodeData.zoomName == ImGuiExNodeZoom_Imploded ){

        if(curNodeData.pinsFlags == ImGuiExNodePinsFlags_None){ // No pins
            ret = true;
        }
    }
    // Zoom level accepts content
    else if( curNodeData.zoomName > ImGuiExNodeZoom_Imploded ){
        // If asked view includes current view, for custom rendering.
        ret = ( _renderingView & curNodeData.viewName );
    }
    // Any other case = no drawing !
    else {
        ret = false;
    }

    isDrawingContent = ret;
    return ret;
}
void ImGuiEx::NodeCanvas::EndNodeContent(){
    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
    IM_ASSERT(isDrawingMenu == false); // Huh ?
    IM_ASSERT(isDrawingContent == true); // // Only call when BeginNodeContent() returned true !

    isDrawingContent = false;
}


//void ImGuiEx::NodeCanvas::pushNodeWorkRect(){
//    ImGuiWindow* win = ImGui::GetCurrentWindow();
//    canvasWorkRectBackup.Min = win->WorkRect.Min;
//    canvasWorkRectBackup.Max = win->WorkRect.Max;
//    win->WorkRect.Max = curNodeData.innerContentBox.Max - ImVec2(IMGUI_EX_NODE_CONTENT_PADDING,IMGUI_EX_NODE_CONTENT_PADDING);
//    win->WorkRect.Min = curNodeData.innerContentBox.Min + ImVec2(IMGUI_EX_NODE_CONTENT_PADDING,IMGUI_EX_NODE_CONTENT_PADDING);
//}

//void ImGuiEx::NodeCanvas::popNodeWorkRect(){
//    ImGuiWindow* win = ImGui::GetCurrentWindow();
//    win->WorkRect.Max = curNodeData.innerContentBox.Max;
//    win->WorkRect.Min = curNodeData.innerContentBox.Min;
//}
