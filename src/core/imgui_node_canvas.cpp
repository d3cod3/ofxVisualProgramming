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
#include <bitset> // bitset::count
#include <algorithm>

// NodePin Connection types in ImGui namespace
#define IMGUI_PAYLOAD_TYPE_PIN_FLOAT     "PINF"    // float - VP_LINK_NUMERIC


struct BezierCurve
{
    // the curve control points
    ImVec2 p0, p1, p2, p3;
};

struct LinkBezierData
{
    BezierCurve bezier;
    int num_segments;
};


// ImGui helper func
static inline ImVec2 ImAbs(const ImVec2& lhs) {
    return ImVec2(lhs.x > 0.0f ? lhs.x : std::abs(lhs.x), lhs.y > 0.0f ? lhs.y : std::abs(lhs.y));
}

inline ImVec2 eval_bezier(float t, const BezierCurve& bezier){
    // B(t) = (1-t)**3 p0 + 3(1 - t)**2 t P1 + 3(1-t)t**2 P2 + t**3 P3
    return ImVec2(
                (1 - t) * (1 - t) * (1 - t) * bezier.p0.x +
                3 * (1 - t) * (1 - t) * t * bezier.p1.x +
                3 * (1 - t) * t * t * bezier.p2.x + t * t * t * bezier.p3.x,
                (1 - t) * (1 - t) * (1 - t) * bezier.p0.y +
                3 * (1 - t) * (1 - t) * t * bezier.p1.y +
                3 * (1 - t) * t * t * bezier.p2.y + t * t * t * bezier.p3.y);
}

// Divides the bezier curve into n segments. Evaluates the distance to each
// segment. Chooses the segment with the smallest distance, and repeats the
// algorithm on that segment, for the given number of iterations.
inline float get_closest_point_on_cubic_bezier(const int num_iterations, const int num_segments, const ImVec2& pos, const BezierCurve& bezier){
    assert(num_iterations > 0 && num_segments > 0);
    float tstart = 0.0f;
    float tend = 1.0f;
    float tbest = 0.5f;
    float best_distance = FLT_MAX;

    for (int i = 0; i < num_iterations; ++i)
    {
        // split the current t-range to segments
        const float dt = (tend - tstart) / num_segments;
        for (int s = 0; s < num_segments; ++s)
        {
            const float tmid = tstart + dt * (float(s) + 0.5f);
            const ImVec2 bt = eval_bezier(tmid, bezier);
            const ImVec2 dv = bt - pos;
            float cur_distance = ImLengthSqr(dv);
            if (cur_distance < best_distance)
            {
                best_distance = cur_distance;
                tbest = tmid;
            }
        }
        // shrink the current t-range to the best segment
        tstart = tbest - 0.5f * dt;
        tend = tbest + 0.5f * dt;
    }

    return tbest;
}

inline float get_distance_to_cubic_bezier(const ImVec2& pos, const BezierCurve& bezier){
    const int segments = 5;
    const float length = ImSqrt(ImLengthSqr(bezier.p3 - bezier.p2)) +
            ImSqrt(ImLengthSqr(bezier.p2 - bezier.p1)) +
            ImSqrt(ImLengthSqr(bezier.p1 - bezier.p0));
    const float iterations_per_length = 0.01f;
    const int iterations =
            int(ImClamp(length * iterations_per_length, 2.0f, 8.f));

    const float t =
            get_closest_point_on_cubic_bezier(iterations, segments, pos, bezier);
    const ImVec2 point_on_curve = eval_bezier(t, bezier);

    const ImVec2 to_curve = point_on_curve - pos;
    return ImSqrt(ImLengthSqr(to_curve));
}

inline ImRect get_containing_rect_for_bezier_curve(const BezierCurve& bezier){
    const ImVec2 min = ImVec2(
                ImMin(bezier.p0.x, bezier.p3.x), ImMin(bezier.p0.y, bezier.p3.y));
    const ImVec2 max = ImVec2(
                ImMax(bezier.p0.x, bezier.p3.x), ImMax(bezier.p0.y, bezier.p3.y));

    ImRect rect(min, max);
    rect.Add(bezier.p1);
    rect.Add(bezier.p2);

    return rect;
}

inline LinkBezierData get_link_renderable(ImVec2 start, ImVec2 end, const float line_segments_per_length){

    // function arguments assed by value, since we mutate them
    const ImVec2 delta = end - start;
    const float link_length = ImSqrt(ImLengthSqr(delta));
    const ImVec2 offset = ImVec2(0.25f * link_length, 0.f);
    LinkBezierData link_data;
    link_data.bezier.p0 = start;
    link_data.bezier.p1 = start + offset;
    link_data.bezier.p2 = end - offset;
    link_data.bezier.p3 = end;
    link_data.num_segments = ImMax(int(link_length * line_segments_per_length), 1);
    return link_data;
}

inline bool is_mouse_hovering_near_link(const BezierCurve& bezier){
    const ImVec2 mouse_pos = ImGui::GetIO().MousePos;

    // First, do a simple bounding box test against the box containing the link
    // to see whether calculating the distance to the link is worth doing.
    const ImRect link_rect = get_containing_rect_for_bezier_curve(bezier);

    if (link_rect.Contains(mouse_pos))
    {
        const float distance = get_distance_to_cubic_bezier(mouse_pos, bezier);
        if (distance < IMGUI_EX_NODE_HOVER_DISTANCE)
        {
            return true;
        }
    }

    return false;
}


bool ImGuiEx::NodeCanvas::Begin(const char* _id){

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
                            ImGuiWindowFlags_NoNav |
                            ImGuiWindowFlags_NoBringToFrontOnFocus |
                            ImGuiWindowFlags_NoScrollWithMouse |
                            ImGuiWindowFlags_NoScrollbar |
                            ImGuiWindowFlags_NoMouseInputs
                            );
    ImGui::PopStyleVar();

    // Allow tinier windows for nodes.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(IMGUI_EX_NODE_MIN_WIDTH*scaleFactor, IMGUI_EX_NODE_MIN_HEIGHT*scaleFactor) );

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

    isAnyCanvasNodeHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow); // not really needed anymore...

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
bool ImGuiEx::NodeCanvas::BeginNode( int nId, const char* _id, std::string name, ImVec2& _pos, ImVec2& _size, const int& _numLeftPins, const int& _numRightPins, const bool& canResize, const bool& isTextureNode ){
    // Check callstack
    IM_ASSERT(isDrawingCanvas == true);  // forgot to End();
    IM_ASSERT(canDrawNode == true); // Don't call if Begin() returned false
    IM_ASSERT(isDrawingNode == false); // Finish your previous node before staring a new one !

    // Precalc some vars
    ImVec2 nodeScale = ImVec2(1,1)*canvasView.scale;
    curNodeData = NodeLayoutData(canvasView.translation + _pos*nodeScale, _size*nodeScale, canvasView.scale, scaleFactor );
    isDrawingNode = true; // to allow End() call
    curNodeData.menuActions = ImGuiExNodeMenuActionFlags_None; // reset menu flags

    // Is the node out of sight on canvas ?
    bool isNodeVisible = ImGui::IsRectVisible( curNodeData.outerContentBox.Min, curNodeData.outerContentBox.Max );

    // Calc zoom name
    if( !isNodeVisible ){
        curNodeData.zoomName = ImGuiExNodeZoom_Invisible;
        //return false;
    }else{
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
    static int pinsWidth; pinsWidth = 0;
    /*if(curNodeData.zoomName == ImGuiExNodeZoom_Imploded){
        // Behaviour: if there are any pins, show only pins. Else imploded still have small content.
        pinsWidth = 0;
        if(_numLeftPins > 0 || _numLeftPins > 0)
            pinsWidth = ImFloor(curNodeData.innerContentBox.GetSize().x);
        if(_numLeftPins > 0 && _numLeftPins > 0)
            pinsWidth *= .5f;
    }
    else{
        pinsWidth = (curNodeData.zoomName >= ImGuiExNodeZoom_Large) ? IMGUI_EX_NODE_PINS_WIDTH_LARGE : (curNodeData.zoomName >= ImGuiExNodeZoom_Normal) ? IMGUI_EX_NODE_PINS_WIDTH_NORMAL : IMGUI_EX_NODE_PINS_WIDTH_SMALL;
    }*/



    // Remove pins space from innerContentBox
    //if( _numLeftPins > 0 ){ // Has left pins
        pinsWidth = IMGUI_EX_NODE_PINS_WIDTH_NORMAL*scaleFactor;
        curNodeData.pinsFlags |= ImGuiExNodePinsFlags_Left;
        curNodeData.leftPins.region.Max.x += pinsWidth;
        curNodeData.innerContentBox.Min.x += pinsWidth;

        curNodeData.leftPins.numPins = _numLeftPins;
        curNodeData.leftPins.pinSpace = ImVec2(curNodeData.leftPins.region.GetSize().x, curNodeData.leftPins.region.GetSize().y / _numLeftPins);
        curNodeData.leftPins.curDrawPos = curNodeData.leftPins.region.Min;
    //}
    //if( _numRightPins > 0 ){ // Has right pins
        pinsWidth = IMGUI_EX_NODE_PINS_WIDTH_SMALL*scaleFactor;
        curNodeData.pinsFlags |= ImGuiExNodePinsFlags_Right;
        curNodeData.rightPins.region.Min.x -= pinsWidth;
        curNodeData.innerContentBox.Max.x -= pinsWidth;

        curNodeData.rightPins.numPins = _numRightPins;
        curNodeData.rightPins.pinSpace = ImVec2(curNodeData.rightPins.region.GetSize().x, curNodeData.rightPins.region.GetSize().y / _numRightPins);
        curNodeData.rightPins.curDrawPos = ImVec2(curNodeData.rightPins.region.Max.x, curNodeData.rightPins.region.Min.y);
    //}

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
    // Return early now that everything has been calc'd.
    if( !isNodeVisible ){
        nodeDrawList = ImGui::GetWindowDrawList(); // So that nodes can still draw pins !
        return false;
    }
    // Create node window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));//IMGUI_EX_NODE_CONTENT_PADDING,IMGUI_EX_NODE_CONTENT_PADDING));
    ImGui::SetNextWindowPos(curNodeData.outerContentBox.Min);
    ImGui::SetNextWindowSize(curNodeData.outerContentBox.GetSize());
    //ImGui::SetNextWindowSize(ImVec2(curNodeData.outerContentBox.GetSize().x,0.0f));
    bool ret = ImGui::Begin(_id, NULL,
                            ImGuiWindowFlags_NoMove
                            | ImGuiWindowFlags_NoBackground
                            | ImGuiWindowFlags_NoDecoration
                            //| ImGuiWindowFlags_AlwaysAutoResize
                            //| ImGuiWindowFlags_AlwaysUseWindowPadding
                            | ImGuiWindowFlags_NoFocusOnAppearing
                            | ImGuiWindowFlags_NoDocking
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
        if(curNodeData.zoomName < ImGuiExNodeZoom_Small || !isTextureNode){
            nodeDrawList->AddRectFilled( curNodeData.outerContentBox.Min, curNodeData.outerContentBox.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 999.f));
        }else{
            nodeDrawList->AddRectFilled( curNodeData.outerContentBox.Min, curNodeData.outerContentBox.Max, IM_COL32(0,0,0,0));
            nodeDrawList->AddRectFilled( curNodeData.outerContentBox.Min, ImVec2(curNodeData.outerContentBox.Min.x+curNodeData.leftPins.region.GetSize().x,curNodeData.outerContentBox.Max.y), ImGui::GetColorU32(ImGuiCol_FrameBg, 999.f));
            nodeDrawList->AddRectFilled( curNodeData.rightPins.region.Min, curNodeData.outerContentBox.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 999.f));
        }

        // Draw HeaderBar BG
        nodeDrawList->AddRectFilled(
                    curNodeData.outerContentBox.Min,
                    curNodeData.outerContentBox.Min+ImVec2(curNodeData.outerContentBox.GetSize().x, IMGUI_EX_NODE_HEADER_HEIGHT*scaleFactor),
                    ImGui::GetColorU32(ImGuiCol_Header)
                    );

        // Draw footer
        nodeDrawList->AddRectFilled(
                    ImVec2(curNodeData.outerContentBox.Min.x, curNodeData.outerContentBox.Max.y-IMGUI_EX_NODE_FOOTER_HEIGHT*scaleFactor),
                    curNodeData.outerContentBox.Max,
                    ImGui::GetColorU32(ImGuiCol_Header)
                    );

        // Node border (surrounding)
        ImU32 _tempColor = IM_COL32(0,0,0,0);
        if (std::find(selected_nodes.begin(), selected_nodes.end(),nId)!=selected_nodes.end()){ // selected
            _tempColor = IM_COL32(255,0,0,255);
        }
        nodeDrawList->AddRect( curNodeData.outerContentBox.Min, curNodeData.outerContentBox.Max, _tempColor );

        // Header info
        ImGui::SetCursorScreenPos( curNodeData.outerContentBox.Min+ImVec2(5, ((IMGUI_EX_NODE_HEADER_HEIGHT*scaleFactor)-ImGui::CalcTextSize("").y)*.5f) );//canvasView.translation+pos+ImVec2(5,4));

        if (name.find('|') != std::string::npos ){
            if(nId == activeNode){
                std::string nodeName = name.substr(0, name.find('|')+1);
                std::string specialName = name.substr(name.find('|')+2);
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
                ImGui::Text("%s", nodeName.c_str()); // node name
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,220,240,255));
                ImGui::Text("%s", specialName.c_str()); // special name
            }else{
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,0,0,255));
                ImGui::Text("%s", name.c_str()); // title
            }
        }else{
            if(nId == activeNode){
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
            }else{
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,0,0,255));
            }
            ImGui::Text("%s", name.c_str()); // title
        }

        ImGui::PopStyleColor();

        // Enable drag on title
        //unsigned int curTabsWidth = (curNodeData.zoomName > ImGuiExNodeZoom_Imploded) ? IMGUI_EX_NODE_HEADER_TOOLBAR_WIDTH : 0;
        ImGui::SetCursorScreenPos( curNodeData.outerContentBox.Min );
        //ImGui::InvisibleButton( "headerGripBtn", ImVec2( curNodeData.outerContentBox.GetSize().x-IMGUI_EX_NODE_HEADER_HEIGHT*scaleFactor, IMGUI_EX_NODE_HEADER_HEIGHT*scaleFactor )  );
        ImGui::InvisibleButton( "headerGripBtn", ImMax(ImVec2( curNodeData.outerContentBox.GetSize().x-IMGUI_EX_NODE_HEADER_HEIGHT*scaleFactor, IMGUI_EX_NODE_HEADER_HEIGHT*scaleFactor ), ImVec2(1,1))  );
        static ImVec2 mouseOffset(0,0);
        static bool isDraggingHeader = false;

        if(ImGui::IsItemActive() && ImGui::IsItemClicked(ImGuiMouseButton_Left)){
            activeNode = nId;
            if(ImGui::GetIO().KeyShift && name != "audio device"){
                selected_nodes.push_back(nId);
            }
        }

        // deselect nodes on clicking on patch
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !isAnyCanvasNodeHovered){
            selected_nodes.clear();
        }

        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
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
            /*ImGui::SetCursorScreenPos(ImVec2(curNodeData.outerContentBox.Max.x-IMGUI_EX_NODE_HEADER_TOOLBAR_WIDTH, curNodeData.outerContentBox.Min.y + 2));
            if(ImGui::BeginTabBar("widgetTabs",ImGuiTabBarFlags_NoTooltip)){//, ImGuiTabBarFlags_FittingPolicyResizeDown)){

                static bool tabInfoOpen = true, tabVisualiseOpen = false, tabParamsOpen = false;
                ImGui::SetCursorScreenPos(ImVec2(curNodeData.outerContentBox.Max.x-IMGUI_EX_NODE_HEADER_TOOLBAR_WIDTH, ImGui::GetCursorScreenPos().y));
                ImGui::SameLine();
                //if(ImGui::BeginTabItem("i", &tabInfoOpen, ImGuiTabItemFlags_NoCloseButton | ImGuiTabItemFlags_NoCloseWithMiddleMouseButton )) ImGui::EndTabItem();
                if((tabVisualiseOpen = ImGui::BeginTabItem("v",NULL, ImGuiTabItemFlags_NoCloseButton)))ImGui::EndTabItem();
                if((tabParamsOpen = ImGui::BeginTabItem("p",NULL, ImGuiTabItemFlags_NoCloseButton)))ImGui::EndTabItem();
                if((tabInfoOpen = ImGui::BeginTabItem("i",NULL, ImGuiTabItemFlags_NoCloseButton))) ImGui::EndTabItem();

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
                            ImGui::GetColorU32(ImGuiCol_WindowBg)
                            );
            }*/

            // NO TABS DESIGN -- TESTING

            // Node Menu Toggle
            ImGui::SetCursorScreenPos(ImVec2(curNodeData.outerContentBox.Max.x-(18*scaleFactor), curNodeData.outerContentBox.Min.y + 1));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(4,2));
            ImGui::Button("#",ImVec2(IMGUI_EX_NODE_HEADER_HEIGHT*scaleFactor,(IMGUI_EX_NODE_HEADER_HEIGHT-1)*scaleFactor));
            if(ImGui::IsItemActivated()){
                nodeMenuIsOpen = true;
            }
            ImGui::PopStyleVar();

            curNodeData.viewName = ImGuiExNodeView_Visualise;

        }

        // Draw footer resize handle
        if(canResize){
            ImGui::SetCursorScreenPos( curNodeData.outerContentBox.Max-ImVec2( IMGUI_EX_NODE_FOOTER_HANDLE_SIZE*scaleFactor, IMGUI_EX_NODE_FOOTER_HANDLE_SIZE*scaleFactor )  );
            ImGui::InvisibleButton( "footerGripBtn", ImVec2( IMGUI_EX_NODE_FOOTER_HANDLE_SIZE*scaleFactor, IMGUI_EX_NODE_FOOTER_HANDLE_SIZE*scaleFactor )  );
            static bool isDraggingFooter = false;
            if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
                if(!isDraggingFooter){
                    isDraggingFooter=true;
                }
                // Constrained size
                _size = ImMax(
                            (ImGui::GetMousePos()-curNodeData.outerContentBox.Min)*(1.f/canvasView.scale),
                            ImVec2(IMGUI_EX_NODE_MIN_WIDTH*scaleFactor,IMGUI_EX_NODE_MIN_HEIGHT*scaleFactor)
                            );
            }
            else if(ImGui::IsItemDeactivated()){
                if(isDraggingFooter) isDraggingFooter = false;
            }

            nodeDrawList->AddTriangleFilled(
                        curNodeData.outerContentBox.Max-ImVec2( 0, IMGUI_EX_NODE_FOOTER_HEIGHT*2*scaleFactor ),
                        curNodeData.outerContentBox.Max,
                        curNodeData.outerContentBox.Max-ImVec2( IMGUI_EX_NODE_FOOTER_HEIGHT*2*scaleFactor, 0 ),
                        //ImGui::GetColorU32(ImGuiCol_ResizeGrip)
                        ImGui::GetColorU32(ImGui::IsItemActive()?ImGuiCol_ResizeGripActive:ImGui::IsItemHovered()?ImGuiCol_ResizeGripHovered:ImGuiCol_ResizeGrip )
                        );
        }

    }

    // reset origin
    ImGui::SetCursorScreenPos( curNodeData.innerContentBox.Min );

    // Return before drawing content, if scale is too small ?
    // todo. (This is already partially handled by BeginNodeContent();)
    if( curNodeData.zoomName <= ImGuiExNodeZoom_Imploded ){
        // Fill empty space with color
        //canvasDrawList->AddRectFilled(); // todo

        // Todo: something with : window->SkipItems to prevent drawing to node ?
        return false;
    }


    // The combination of a cliprect and columns allows us to set a clipping space for node widgets while reserving drawable space for pins, without having to add an extra window / childframe.
    ImGui::PushClipRect( curNodeData.leftPins.region.Min, curNodeData.rightPins.region.Max, true); // Inner space + Node Spaces
    ImGui::BeginColumns("innerNode", 3,
                        ImGuiColumnsFlags_NoBorder
                        | ImGuiColumnsFlags_NoResize
                        //| ImGuiOldColumnFlags_NoPreserveWidths
                        | ImGuiColumnsFlags_NoForceWithinWindow // important so there's no weird auto adjustments.
                        );
    // Column layout
    // Note: A column of 0 width will probably cause crashes
    ImGui::SetColumnOffset(0,0);
    ImGui::SetColumnOffset(1, std::max(curNodeData.leftPins.region.GetSize().x, 1.f));
    ImGui::SetColumnOffset(2, std::max(curNodeData.innerContentBox.Max.x-curNodeData.leftPins.region.Min.x, 2.f));

    // move to middle column where the user can draw
    ImGui::NextColumn();

    // Draw column BG, to mask overlapping nodes
    /*if( curNodeData.zoomName > ImGuiExNodeZoom_Imploded ){
        ImGui::GetWindowDrawList()->AddRectFilled(curNodeData.innerContentBox.Min, curNodeData.innerContentBox.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 999.f) );
        ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING));// Padding top
    }*/

    // By default, try to draw full width
    //ImGui::PushItemWidth(-1.0f); // Todo: doesn't seem have any effect...

    // Allow User to catch the menu and extend it
    if( nodeMenuIsOpen ){
        ImGui::OpenPopup(IMGUI_EX_NODE_MENU_ID);
    }

    // Draw default menu items
    if(ImGui::BeginPopup(IMGUI_EX_NODE_MENU_ID)){
        if(name != "audio device"){ // Todo: should not be checked on title... maybe: if(node->canBeRemoved())
            if(ImGui::MenuItem("Delete")) curNodeData.menuActions |= ImGuiExNodeMenuActionFlags_DeleteNode;
            //if(ImGui::MenuItem("Copy")) curNodeData.menuActions |= ImGuiExNodeMenuActionFlags_CopyNode;
            if(ImGui::MenuItem("Duplicate")) curNodeData.menuActions |= ImGuiExNodeMenuActionFlags_DuplicateNode;
        }
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
        /*if( true || curNodeData.zoomName > ImGuiExNodeZoom_Imploded ){
            //ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING)); // Padding bottom
            ImGui::EndColumns();
            ImGui::PopClipRect(); // Inner space + nodes
        }

        // Always pop these ()
        ImGui::PopClipRect();*/

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

ImGuiEx::NodeConnectData ImGuiEx::NodeCanvas::AddNodePin( const int nodeID, const int pinID, const char* _label, std::vector<ofxVPLinkData>& _linksData, std::string _type, bool _wireless, bool _connected, const ImU32& _color, const ImGuiExNodePinsFlags& _pinFlag ){

    /*if(_pinFlag == ImGuiExNodePinsFlags_Left){
        // set data
        inletPinsPositions[nodeID][pinID] = ImVec2(0,0);
    }else if(_pinFlag == ImGuiExNodePinsFlags_Right){
        // set data
        outletPinsPositions[nodeID][pinID] = ImVec2(0,0);
    }*/

    ImGuiEx::NodeConnectData connectData;
    connectData.connectType = -1;
    connectData.linkID = -1;
    connectData.fromObjectID = -1;
    connectData.fromOutletPinID = -1;
    connectData.toObjectID = -1;
    connectData.toInletPinID = -1;

    static int connectType = 0;
    static int fromObjectID = 0;
    static int outletPinID = 0;
    static int linkID = -1;

    // Check ImGui Callstack
    IM_ASSERT(nodeDrawList != NULL ); // Huh ?
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

    // Draw pins
    if(!_wireless){


        if(pinLayout.pinSpace.x > 1){ // Minimum width (with ±0 values ImGui crashes)
            if( _pinFlag==ImGuiExNodePinsFlags_Left ){
                ImGui::SetCursorScreenPos( pinLayout.curDrawPos  );
            }
            else {
                ImGui::SetCursorScreenPos( pinLayout.curDrawPos + ImVec2( -pinLayout.pinSpace.x, 0)  );
            }


# if __IMGUI_EX_NODECANVAS_DEBUG__
            ImGui::Button("##nodeBtn", pinLayout.pinSpace);
# else
            ImGui::InvisibleButton("nodeBtn", ImMax(ImVec2(pinLayout.pinSpace.x,pinLayout.pinSpace.y*.8f), ImVec2(1,1)));
# endif

            IM_ASSERT( _label != NULL );

            std::string _str_label(_label);
            std::string _gui_label(_label);
            _str_label += std::to_string(pinID);
            _str_label += std::to_string(_pinFlag);

            // update node active pin
            if(ImGui::IsItemClicked()){ // || ImGui::IsItemHovered()
                activePin = _str_label;
                activePinType = _type;
            }


            // Let this pin be draggable
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers|ImGuiDragDropFlags_SourceNoPreviewTooltip)){
                // draw dragging link creation
                if( activePin == _str_label ){ // draw link from active pin only ( last clicked )

                    if(_pinFlag==ImGuiExNodePinsFlags_Right){ // drag new links ( only from outlets )

                        fromObjectID = nodeID;
                        outletPinID = pinID;

                        connectType = 1;

                        // PAYLOAD
                        static int tmpNum=777;
                        ImGui::SetDragDropPayload(IMGUI_PAYLOAD_TYPE_PIN_FLOAT, &tmpNum, sizeof(int)); // Set payload to carry the index of our item (could be anything)

                        // add connecting link
                        auto connectingColor = ImGui::ColorConvertU32ToFloat4(_color);
                        connectingColor.w = 0.4f;
                        const LinkBezierData link_data = get_link_renderable(pinLayout.curDrawPos + ImVec2( IMGUI_EX_NODE_PIN_WIDTH*scaleFactor * -.5f, pinLayout.pinSpace.y * .5f),ImGui::GetMousePos(),IMGUI_EX_NODE_LINK_LINE_SEGMENTS_PER_LENGTH);
                        canvasDrawList->AddBezierCurve(link_data.bezier.p0,link_data.bezier.p1,link_data.bezier.p2,link_data.bezier.p3,ImGui::ColorConvertFloat4ToU32(connectingColor),IMGUI_EX_NODE_LINK_THICKNESS,link_data.num_segments);

                        // add link info
                        std::string _temp = _type+" "+_gui_label;
                        ImVec2 tempPos = ImGui::GetMousePos() - (ImGui::GetMousePos() - (pinLayout.curDrawPos + ImVec2( IMGUI_EX_NODE_PIN_WIDTH*scaleFactor * -.5f, pinLayout.pinSpace.y * .5f)))/2.0f - ImGui::CalcTextSize(_temp.c_str())/2.0f;
                        canvasDrawList->AddRectFilled(tempPos + ImVec2(-IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,-IMGUI_EX_NODE_PIN_WIDTH*scaleFactor),tempPos + ImGui::CalcTextSize(_temp.c_str()) + ImVec2(IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,IMGUI_EX_NODE_PIN_WIDTH*scaleFactor),IM_COL32(40,40,40,180) );
                        canvasDrawList->AddText( tempPos , _color, _temp.c_str() );
                    }
                    else if(_pinFlag==ImGuiExNodePinsFlags_Left && _connected && !_linksData.empty()){ // change previously created links ( only from inlets )

                        fromObjectID = _linksData.at(0)._fromObjectID;
                        outletPinID = _linksData.at(0)._fromPinID;

                        connectType = 2;
                        linkID = _linksData.at(0)._linkID;

                        // PAYLOAD
                        static int tmpNum=777;
                        ImGui::SetDragDropPayload(IMGUI_PAYLOAD_TYPE_PIN_FLOAT, &tmpNum, sizeof(int)); // Set payload to carry the index of our item (could be anything)

                        // retrieve previously connected link
                        auto connectingColor = ImGui::ColorConvertU32ToFloat4(_color);
                        connectingColor.w = 0.4f;
                        const LinkBezierData link_data = get_link_renderable(canvasView.translation+(_linksData.at(0)._toPinPosition*canvasView.scale),ImGui::GetMousePos(),IMGUI_EX_NODE_LINK_LINE_SEGMENTS_PER_LENGTH);
                        canvasDrawList->AddBezierCurve(link_data.bezier.p0,link_data.bezier.p1,link_data.bezier.p2,link_data.bezier.p3,ImGui::ColorConvertFloat4ToU32(connectingColor),IMGUI_EX_NODE_LINK_THICKNESS,link_data.num_segments);

                        // add link info
                        std::string _temp = _type+" "+_linksData.at(0)._linkLabel;
                        ImVec2 tempPos = ImGui::GetMousePos() - (ImGui::GetMousePos() - (canvasView.translation+(_linksData.at(0)._toPinPosition*canvasView.scale)))/2.0f - ImGui::CalcTextSize(_temp.c_str())/2.0f;
                        canvasDrawList->AddRectFilled(tempPos + ImVec2(-IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,-IMGUI_EX_NODE_PIN_WIDTH*scaleFactor),tempPos + ImGui::CalcTextSize(_temp.c_str()) + ImVec2(IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,IMGUI_EX_NODE_PIN_WIDTH*scaleFactor),IM_COL32(40,40,40,180) );
                        canvasDrawList->AddText( tempPos , _color, _temp.c_str() );

                    }

                }

                ImGui::EndDragDropSource();
            }

            // Accept other pins dropping on this
            if (ImGui::BeginDragDropTarget()){

                if(activePinType == _type){

                    static int tmpAccept = 0;
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_PIN_FLOAT,ImGuiDragDropFlags_AcceptNoDrawDefaultRect)){ // ImGuiDragDropFlags_AcceptNoDrawDefaultRect

                        if(_pinFlag==ImGuiExNodePinsFlags_Left){ // only on inlets
                            memcpy((float*)&tmpAccept, payload->Data, sizeof(int));

                            //std::cout << "ACCEPTED = " << tmpAccept << " from object " << fromObjectID << ", from OUTLET PIN " << outletPinID << " to object ID " << nodeID << " to INLET PIN " << pinID << std::endl;

                            // connect/reconnect
                            connectData.connectType = connectType;
                            connectData.linkID = linkID;
                            connectData.fromObjectID = fromObjectID;
                            connectData.fromOutletPinID = outletPinID;
                            connectData.toObjectID = nodeID;
                            connectData.toInletPinID = pinID;

                            // reset selected links
                            for(unsigned int i=0;i<_linksData.size();i++){
                                std::vector<int>::iterator it = std::find(selected_links.begin(), selected_links.end(),_linksData.at(i)._linkID);
                                if (it!=selected_links.end()){
                                    selected_links.erase(it);
                                }
                            }

                            // reset deactivated links
                            for(unsigned int i=0;i<_linksData.size();i++){
                                std::vector<int>::iterator it = std::find(deactivated_links.begin(), deactivated_links.end(),_linksData.at(i)._linkID);
                                if (it!=deactivated_links.end()){
                                    deactivated_links.erase(it);
                                }
                            }

                        }

                        activePin = "";
                        activePinType = "";
                        connectType = 0;

                    }

                }else{
                    if(ImGui::GetIO().MouseReleased[0]){
                        activePinType = "";
                        connectType = 0;
                    }
                }

                ImGui::EndDragDropTarget();


            }

        }

    }

    // Set position of pin so user can draw it.
    //float pinSpace = (ImGui::IsItemHovered()) ? IMGUI_EX_NODE_PIN_WIDTH_HOVERED : IMGUI_EX_NODE_PIN_WIDTH;
    float pinSpace = IMGUI_EX_NODE_PIN_WIDTH*scaleFactor;

    if(!_wireless){
        // Left side (INLETS)
        if( _pinFlag==ImGuiExNodePinsFlags_Left ){
            // Update pin position
            inletPinsPositions[nodeID][pinID] = pinLayout.curDrawPos + ImVec2( IMGUI_EX_NODE_PIN_WIDTH*scaleFactor * .5f, pinLayout.pinSpace.y * .5f) + ImVec2(IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,0);

            // Draw pin
            if( curNodeData.zoomName != ImGuiExNodeZoom_Invisible ){
                nodeDrawList->AddCircleFilled(inletPinsPositions[nodeID][pinID], pinSpace * .5f, _color, 6);

                // Interactivity
                if(ImGui::GetMousePos().x > inletPinsPositions[nodeID][pinID].x-(pinLayout.pinSpace.x*.5f) && ImGui::GetMousePos().x < inletPinsPositions[nodeID][pinID].x+(pinLayout.pinSpace.x*.5f) && ImGui::GetMousePos().y > inletPinsPositions[nodeID][pinID].y-(pinLayout.pinSpace.y*.5f) && ImGui::GetMousePos().y < inletPinsPositions[nodeID][pinID].y+(pinLayout.pinSpace.y*.5f)){
                    if(activePinType == _type || activePinType == ""){
                        nodeDrawList->AddCircle(inletPinsPositions[nodeID][pinID],pinSpace * 0.9f, _color, 6);
                        ImVec2 tempPos = inletPinsPositions[nodeID][pinID] - ImVec2(pinSpace * .5f + (IMGUI_EX_NODE_PIN_WIDTH + 6)*scaleFactor,ImGui::GetTextLineHeight()*-.4f) - ImGui::CalcTextSize(_label);
                        canvasDrawList->AddRectFilled(tempPos + ImVec2(-IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,-IMGUI_EX_NODE_PIN_WIDTH*scaleFactor),tempPos + ImGui::CalcTextSize(_label) + ImVec2(IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,IMGUI_EX_NODE_PIN_WIDTH*scaleFactor),IM_COL32(40,40,40,180) );
                        canvasDrawList->AddText( tempPos, _color, _label);
                    }

                }

                // Draw Connected Appearance
                if(_connected){
                    nodeDrawList->AddCircle(inletPinsPositions[nodeID][pinID],pinSpace * 0.9f, _color, 6);
                }

            }

        }

        // right side (OUTLETS)
        else if( _pinFlag==ImGuiExNodePinsFlags_Right ){
            // Update pin position
            outletPinsPositions[nodeID][pinID] = pinLayout.curDrawPos + ImVec2( IMGUI_EX_NODE_PIN_WIDTH*scaleFactor * -.5f, pinLayout.pinSpace.y * .5f);

            // draw links (OUTLETS to INLETS ONLY)
            for(unsigned int i=0;i<_linksData.size();i++){
                const LinkBezierData link_data = get_link_renderable(outletPinsPositions[nodeID][pinID], canvasView.translation+(_linksData.at(i)._toPinPosition*canvasView.scale), IMGUI_EX_NODE_LINK_LINE_SEGMENTS_PER_LENGTH);

                const bool is_hovered = is_mouse_hovering_near_link(link_data.bezier);

                if(ImGui::IsMouseClicked(0) && !isAnyCanvasNodeHovered){
                    if (is_hovered && !ImGui::GetIO().KeyShift){
                        if (std::find(selected_links.begin(), selected_links.end(),_linksData.at(i)._linkID)==selected_links.end()){
                            selected_links.push_back(_linksData.at(i)._linkID);
                        }
                    }else if(!is_hovered && !ImGui::GetIO().KeyShift){
                        std::vector<int>::iterator it = std::find(selected_links.begin(), selected_links.end(),_linksData.at(i)._linkID);
                        if (it!=selected_links.end()){
                            selected_links.erase(it);
                        }
                    }else if(is_hovered && ImGui::GetIO().KeyShift){
                        // deactivate if activated
                        if (std::find(deactivated_links.begin(), deactivated_links.end(),_linksData.at(i)._linkID)==deactivated_links.end()){
                            deactivated_links.push_back(_linksData.at(i)._linkID);
                        }else{ // else the opposite
                            std::vector<int>::iterator it = std::find(deactivated_links.begin(), deactivated_links.end(),_linksData.at(i)._linkID);
                            if (it!=deactivated_links.end()){
                                deactivated_links.erase(it);
                            }
                        }
                    }
                }


                static ImU32 _tempColor;
                _tempColor = _color;

                if (std::find(deactivated_links.begin(), deactivated_links.end(),_linksData.at(i)._linkID)!=deactivated_links.end()){ // disabled
                    _tempColor = IM_COL32(255,255,255,70);
                }

                if (std::find(selected_links.begin(), selected_links.end(),_linksData.at(i)._linkID)!=selected_links.end()){ // selected
                    _tempColor = IM_COL32(255,0,0,255);
                }

                canvasDrawList->AddBezierCurve(link_data.bezier.p0, link_data.bezier.p1, link_data.bezier.p2, link_data.bezier.p3, _tempColor, IMGUI_EX_NODE_LINK_THICKNESS, link_data.num_segments);
            }

            // Draw pin
            if( curNodeData.zoomName != ImGuiExNodeZoom_Invisible ){
                nodeDrawList->AddCircleFilled(outletPinsPositions[nodeID][pinID], pinSpace * .5f, _color, 6);

                // draw labels
                if(ImGui::GetMousePos().x > outletPinsPositions[nodeID][pinID].x-IMGUI_EX_NODE_PIN_WIDTH_HOVERED && ImGui::GetMousePos().x < outletPinsPositions[nodeID][pinID].x+IMGUI_EX_NODE_PIN_WIDTH_HOVERED && ImGui::GetMousePos().y > outletPinsPositions[nodeID][pinID].y-IMGUI_EX_NODE_PIN_WIDTH_HOVERED && ImGui::GetMousePos().y < outletPinsPositions[nodeID][pinID].y+IMGUI_EX_NODE_PIN_WIDTH_HOVERED){
                    if(connectType == 0){
                        nodeDrawList->AddCircle(outletPinsPositions[nodeID][pinID],pinSpace * 0.9f, _color, 6);
                        ImVec2 tempPos = outletPinsPositions[nodeID][pinID] + ImVec2(pinSpace * .5f + 6,ImGui::GetTextLineHeight()*-.5f);
                        canvasDrawList->AddRectFilled(tempPos + ImVec2(-IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,-IMGUI_EX_NODE_PIN_WIDTH*scaleFactor),tempPos + ImGui::CalcTextSize(_label) + ImVec2(IMGUI_EX_NODE_PIN_WIDTH*scaleFactor,IMGUI_EX_NODE_PIN_WIDTH*scaleFactor),IM_COL32(40,40,40,180) );
                        canvasDrawList->AddText( tempPos, _color, _label);
                    }
                }
            }

            // Draw Connected Appearance
            if(_connected){
                nodeDrawList->AddCircle(outletPinsPositions[nodeID][pinID],pinSpace * 0.9f, _color, 6);
            }


        }
    }

    pinLayout.curDrawPos += ImVec2(0,pinLayout.pinSpace.y);

    // Move back to center column
    if( _pinFlag==ImGuiExNodePinsFlags_Right ) ImGui::NextColumn(); // left column
    ImGui::NextColumn(); // center column

    // Restore cursor
    ImGui::SetCursorScreenPos(cursorBackup);

    // reset if drag&drop on self
    if(ImGui::GetIO().MouseReleased[0] && ImGui::IsItemHovered() && connectData.toObjectID == -1){
        activePinType = "";
        connectType = 0;
    }

    // remove link if drag from connected inlet and drop on canvas
    if(ImGui::GetIO().MouseReleased[0] && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemFocused() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && connectType == 2){
        connectData.connectType = 3;
        connectData.linkID = linkID;

        activePinType = "";
        connectType = 0;
    }

    return connectData;
}

bool ImGuiEx::NodeCanvas::BeginNodeMenu(){
    // Check ImGui Callstack
    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
    IM_ASSERT(isDrawingMenu == false); // Forgot to call EndNodeMenu !

    // returns true if menu is open
    bool ret = ImGui::BeginPopup(IMGUI_EX_NODE_MENU_ID);
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

    if(curNodeData.zoomName > ImGuiExNodeZoom_Invisible ){

        // Only pop these if content is drawn
        if( true || curNodeData.zoomName > ImGuiExNodeZoom_Imploded ){
            //ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(-1,IMGUI_EX_NODE_CONTENT_PADDING*scaleFactor)); // Padding bottom
            ImGui::EndColumns();
            ImGui::PopClipRect(); // Inner space + nodes
        }

        // Always pop these ()
        ImGui::PopClipRect();

    }
}

bool ImGuiEx::NodeCanvas::doNodeMenuAction( const ImGuiExNodeMenuActionFlags& _menuItem ){
    IM_ASSERT(isDrawingCanvas == true); // Please Call between Begin() and End()
    IM_ASSERT(isDrawingNode == true); // Please Call between BeginNode() and EndNode()
    IM_ASSERT(!(_menuItem & ImGuiExNodeMenuActionFlags_None) ); // Don't you fool me !
    IM_ASSERT( std::bitset<16>(_menuItem).count() ==1 ); // Max 1 flag at a time !

    return (curNodeData.menuActions & _menuItem);
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
