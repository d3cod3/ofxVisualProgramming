/*==============================================================================

    ofxVisualProgramming: A visual programming patching environment for OF

    Copyright (c) 2018 Emanuele Mazza aka n3m3da <emanuelemazza@d3cod3.org>

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

#include "ofxVisualProgramming.h"
#include "imgui_internal.h"

#ifdef MOSAIC_ENABLE_PROFILING
#include "Tracy.hpp"
#endif

//--------------------------------------------------------------
ofxVisualProgramming::ofxVisualProgramming(){

    mainWindow = dynamic_pointer_cast<ofAppGLFWWindow>(ofGetCurrentWindow());

    // Profiler
    profilerActive          = false;

    //  Event listeners
    ofAddListener(ofEvents().mouseMoved, this, &ofxVisualProgramming::mouseMoved);
    ofAddListener(ofEvents().mouseDragged, this, &ofxVisualProgramming::mouseDragged);
    ofAddListener(ofEvents().mousePressed, this, &ofxVisualProgramming::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxVisualProgramming::mouseReleased);
    ofAddListener(ofEvents().mouseScrolled, this, &ofxVisualProgramming::mouseScrolled);
    ofAddListener(ofEvents().keyPressed, this, &ofxVisualProgramming::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxVisualProgramming::keyReleased);

    // System
    engine                  = new pdsp::Engine();

    font                    = new ofTrueTypeFont();
    fontSize                = 8;
    isRetina                = false;
    scaleFactor             = 1;

    actualObjectID          = 0;
    lastAddedObjectID       = -1;
    bLoadingNewObject       = false;
    bLoadingNewPatch        = false;
    bPopulatingObjectsMap   = false;
    clearingObjectsMap      = false;

    livePatchingObiID       = -1;

    currentPatchFile        = "empty_patch.xml";
    currentPatchFolderPath  = ofToDataPath("temp/");


    currentSubpatch         = "root";
    newSubpatchName         = "";

    vector<SubpatchConnection> rootBranch;
    subpatchesMap[currentSubpatch] = rootBranch;

    loadPatchTime           = ofGetElapsedTimeMillis();
    resetTime               = ofGetElapsedTimeMillis();
    deferredLoadTime        = ofGetElapsedTimeMillis();
    wait                    = 1000;
    deferredLoad            = false;

    alphabet                = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXZY";
    newFileCounter          = 0;

    audioSampleRate         = 44100;
    audioGUISRIndex         = 0;
    audioNumBuffers         = 4;
    audioGUIINIndex         = -1;
    audioGUIOUTIndex        = -1;
    audioGUIINChannels      = 0;
    audioGUIOUTChannels     = 0;
    audioDevicesBS          = {"64","128","256","512","1024","2048"};
    audioGUIBSIndex         = 4;
    audioBufferSize         = ofToInt(audioDevicesBS[audioGUIBSIndex]);

    bpm                     = 120;
    isInputDeviceAvailable  = false;
    isOutputDeviceAvailable = false;
    dspON                   = false;
    audioINDev              = 0;
    audioOUTDev             = 0;

    profilerActive          = false;
    inspectorActive         = false;
    navigationActive        = false;
    isCanvasVisible         = false;
    isCanvasActive          = true;
    isOverProfiler          = false;
    isOverInspector         = false;
    isOverSubpatchNavigator = false;

    inited                  = false;

}

//--------------------------------------------------------------
ofxVisualProgramming::~ofxVisualProgramming(){
    delete font;
    font = nullptr;
}

//--------------------------------------------------------------
void ofxVisualProgramming::setRetina(bool retina, float retinaScale){
    isRetina = retina;
    scaleFactor = retinaScale;
    fontSize = static_cast<int>(floor(ofMap(scaleFactor,1,6,MIN_OF_GUI_FONT_SIZE,MAX_OF_GUI_FONT_SIZE)));
}

//--------------------------------------------------------------
void ofxVisualProgramming::setup(ofxImGui::Gui* _guiRef, string release){

    // Load resources
    font->load(MAIN_FONT,fontSize);

    // Initialise GUI
    if( _guiRef == nullptr ){
        ofxVPGui = new ofxImGui::Gui();
        ofxVPGui->setup();
        string tmpstr = "Automatically setting up a new ImGui instance. If your app has its own one, pass it's reference in setup();";
        ofLogNotice("ofxVP","%s",tmpstr.c_str());
    }
    else {
        ofxVPGui = _guiRef;
        // Dummy call to IO which will crash if _guiRef is not initialised.
        ImGui::GetIO();
    }

    nodeCanvas.setContext(ImGui::GetCurrentContext());
    nodeCanvas.setRetina(isRetina,scaleFactor);

    profiler.setIsRetina(isRetina);

    // create failsafe window for always maintaining reference to shared context
    setupFailsafeWindow();

    // RESET TEMP FOLDER
    resetTempFolder();

    // Load external plugins objects
    plugins_kernel.add_server(PatchObject::server_name(), PatchObject::version);
    // list plugin directory
    ofDirectory pluginsDir;
#ifdef TARGET_LINUX
    pluginsDir.allowExt("so");
#elif defined(TARGET_OSX)
    pluginsDir.allowExt("bundle");
#elif defined(TARGET_WIN32)
    pluginsDir.allowExt("dll");
#endif

    bool hasPlugins = false;
    try{
        pluginsDir.listDir(ofToDataPath(PLUGINS_FOLDER));
        pluginsDir.sort();
        hasPlugins = pluginsDir.size() > 0;
    }
    catch(std::exception& e){
        // This can throw with OS file permission problems.
        ofLog(OF_LOG_WARNING,"Loading plugins: Can't load the plugins folder `%s`. Error: %s", pluginsDir.getOriginalDirectory().c_str(), e.what());
    }

    // load all plugins
    if(hasPlugins){
        for(unsigned int i = 0; i < pluginsDir.size(); i++){
            ofLog(OF_LOG_NOTICE,"Loading plugin: %s",pluginsDir.getFile(i).getFileName().c_str());
            string tmpPlugPath = pluginsDir.getFile(i).getAbsolutePath();
            plugins_kernel.load_plugin(tmpPlugPath);
        }
    }

    // Create new empty file patch
    newPatch(release);

}

//--------------------------------------------------------------
void ofxVisualProgramming::setupFailsafeWindow(){
    ofGLFWWindowSettings settings;
#if defined(OFXVP_GL_VERSION_MAJOR) && defined(OFXVP_GL_VERSION_MINOR)
    settings.setGLVersion(OFXVP_GL_VERSION_MAJOR,OFXVP_GL_VERSION_MINOR);
#else
    settings.setGLVersion(3,2);
#endif
    settings.shareContextWith = mainWindow;
    settings.decorated = true;
    settings.resizable = false;
    settings.visible = false;
    settings.setPosition(ofDefaultVec2(0,0));
    settings.setSize(10,10);

    failsafeWindow = dynamic_pointer_cast<ofAppGLFWWindow>(ofCreateWindow(settings));
    failsafeWindow->setVerticalSync(false);
    failsafeWindow->setWindowPosition(0,0);

    glfwSetWindowCloseCallback(failsafeWindow->getGLFWWindow(),GL_FALSE);
}

//--------------------------------------------------------------
void ofxVisualProgramming::update(){

#ifdef MOSAIC_ENABLE_PROFILING
    ZoneScopedN("ofxVisualProgramming::Update()");
#endif

    // canvas init
    if(!inited){
        inited = true;
    }

    // Clear map from deleted objects
    if(!bPopulatingObjectsMap){
        clearObjectsMap();
    }

    // deferred patch objects load for sharing context objects ( instant load cause visualization errors in some cases )
    if(deferredLoad && ofGetElapsedTimeMillis()-deferredLoadTime > wait){
        deferredLoad = false;
        loadPatchSharedContextObjects();
    }

    // update patch objects
    if(!bLoadingNewPatch && !patchObjects.empty()){

        std::lock_guard<std::mutex> lck(vp_mutex);

        // left to right computing order
        leftToRightIndexOrder.clear();
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                leftToRightIndexOrder.push_back(make_pair(static_cast<int>(floor(it->second->getPos().x)),it->second->getId()));
            }
        }
        // sort the vector by it's pair first value (object X position)
        sort(leftToRightIndexOrder.begin(),leftToRightIndexOrder.end());

        ImGuiEx::ProfilerTask *pt = new ImGuiEx::ProfilerTask[leftToRightIndexOrder.size()];

        for(unsigned int i=0;i<leftToRightIndexOrder.size();i++){
            string tmpon = patchObjects[leftToRightIndexOrder[i].second]->getName()+ofToString(patchObjects[leftToRightIndexOrder[i].second]->getId())+"_update";

            pt[i].color = profiler.cpuGraph.colors[static_cast<unsigned int>(i%16)];
            pt[i].startTime = ofGetElapsedTimef();
            pt[i].name = tmpon;

            patchObjects[leftToRightIndexOrder[i].second]->update(patchObjects,*engine);
            patchObjects[leftToRightIndexOrder[i].second]->updateWirelessLinks(patchObjects);

            pt[i].endTime = ofGetElapsedTimef();

            // update scripts objects files map
            std::ifstream testPath(patchObjects[leftToRightIndexOrder[i].second]->getFilepath());
            if(testPath){ // file exists
                ofFile tempsofp(patchObjects[leftToRightIndexOrder[i].second]->getFilepath());
                string fileExt = ofToUpper(tempsofp.getExtension());
                if(fileExt == "LUA" || fileExt == "SH"){
                    map<string,string>::iterator sofpIT = scriptsObjectsFilesPaths.find(tempsofp.getFileName());
                    if (sofpIT == scriptsObjectsFilesPaths.end()){
                        // not found, insert it
                        scriptsObjectsFilesPaths.insert( pair<string,string>(tempsofp.getFileName(),tempsofp.getAbsolutePath()) );
                    }
                }else if(fileExt == "FRAG"){
                    map<string,string>::iterator sofpIT = scriptsObjectsFilesPaths.find(tempsofp.getFileName());
                    if (sofpIT == scriptsObjectsFilesPaths.end()){
                        // not found, insert FRAG
                        scriptsObjectsFilesPaths.insert( pair<string,string>(tempsofp.getFileName(),tempsofp.getAbsolutePath()) );
                        // insert VERT
                        string fsName = tempsofp.getFileName();
                        string vsName = tempsofp.getEnclosingDirectory()+tempsofp.getFileName().substr(0,fsName.find_last_of('.'))+".vert";
                        ofFile newVertGLSLFile (vsName);
                        scriptsObjectsFilesPaths.insert( pair<string,string>(newVertGLSLFile.getFileName(),newVertGLSLFile.getAbsolutePath()) );
                    }
                }
            }

        }

        profiler.cpuGraph.LoadFrameData(pt,leftToRightIndexOrder.size());
        isOverProfiler = profiler.isMouseOver;

        if(patchObjects[lastAddedObjectID] != nullptr){
            nextObjectPosition = (patchObjects[lastAddedObjectID]->getPos()/ofPoint(scaleFactor,scaleFactor)) + (ofPoint(patchObjects[lastAddedObjectID]->getObjectWidth()+40,40)/ofPoint(scaleFactor,scaleFactor));

        }

        updateSubpatchNavigation();
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::updateRetina(float scale){
    scaleFactor = scale;
}

//--------------------------------------------------------------
void ofxVisualProgramming::updateCanvasViewport(){
    canvasViewport.set(0,20,ofGetWindowWidth(),ofGetWindowHeight()-20);
}

//--------------------------------------------------------------
void ofxVisualProgramming::updateSubpatchNavigation(){
    for(map<string,vector<SubpatchConnection>>::iterator it = subpatchesMap.begin(); it != subpatchesMap.end(); it++ ){
        for(int z=0;z<it->second.size();z++){
            if(it->second.at(z).objID != -1){
                it->second.at(z).name = patchObjects[it->second.at(z).objID]->wirelessName;
                it->second.at(z).type = patchObjects[it->second.at(z).objID]->wirelessType;
            }
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::draw(){

#ifdef MOSAIC_ENABLE_PROFILING
    ZoneScopedN("ofxVisualProgramming::Draw()");
#endif

    if(bLoadingNewPatch) return;
    if(bLoadingNewObject) return;

    // LIVE PATCHING SESSION
    drawLivePatchingSession();

    ofPushView();
    ofPushStyle();
    ofPushMatrix();

    // DEBUG
#ifdef OFXVP_DEBUG
    ofSetColor(0,255,255,236);
    ofNoFill();
    ofDrawRectangle( canvasViewport.x, canvasViewport.y, canvasViewport.width, canvasViewport.height);
    ofFill();
    ofDrawCircle(ofGetMouseX(), ofGetMouseY(), 6);
#endif

    ofEnableAlphaBlending();
    ofSetCurveResolution(50);
    ofSetColor(255);
    ofSetLineWidth(1);

    livePatchingObiID = -1;

    ofxVPGui->begin();

    // DEBUG
#ifdef OFXVP_DEBUG
    ImGui::ShowMetricsWindow();
#endif

    // Try to begin ImGui Canvas.
    // Should always return true, except if window is minimised or somehow not rendered.
    ImGui::SetNextWindowPos(ImVec2(canvasViewport.getTopLeft().x,canvasViewport.getTopLeft().y), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2(canvasViewport.width, canvasViewport.height), ImGuiCond_Always );
    isCanvasVisible = nodeCanvas.Begin("ofxVPNodeCanvas" );

    // Render objects.
    if(!bLoadingNewPatch && !patchObjects.empty()){
        pt = new ImGuiEx::ProfilerTask[leftToRightIndexOrder.size()];
        for(unsigned int i=0;i<leftToRightIndexOrder.size();i++){

            if(patchObjects[leftToRightIndexOrder[i].second]->subpatchName == currentSubpatch){

                string tmpon = patchObjects[leftToRightIndexOrder[i].second]->getName()+ofToString(patchObjects[leftToRightIndexOrder[i].second]->getId())+"_draw";

                pt[i].color = profiler.gpuGraph.colors[static_cast<unsigned int>(i%16)];
                pt[i].startTime = ofGetElapsedTimef();
                pt[i].name = tmpon;

                // LivePatchingObject hack, should not be handled by mosaic.
                if(patchObjects[leftToRightIndexOrder[i].second]->getName() == "live patching"){
                    livePatchingObiID = patchObjects[leftToRightIndexOrder[i].second]->getId();
                }

                // Draw
                patchObjects[leftToRightIndexOrder[i].second]->draw(font);
                if(isCanvasVisible){
                    patchObjects[leftToRightIndexOrder[i].second]->drawImGuiNode(nodeCanvas,patchObjects);
                }

                pt[i].endTime = ofGetElapsedTimef();
            }

        }

        profiler.gpuGraph.LoadFrameData(pt,leftToRightIndexOrder.size());

    }

    nodeCanvas.UpdateCanvasRect();
    nodeCanvas.setCanvasActive(isCanvasActive);
    if(isCanvasActive){
        nodeCanvas.UpdateCanvasScrollZoom();
        if(!nodeCanvas.isAnyNodeFocused()){
            nodeCanvas.UserInteraction();
            nodeCanvas.DrawSelecting();
            nodeCanvas.UpdateNodesFlags();
        }
        //nodeCanvas.CanvasPopupMenu();
    }else{
        nodeCanvas.setCanvasViewToDefault();
    }
    nodeCanvas.UpdateCanvasGrid(ImGui::GetWindowDrawList());


    // Close canvas
    nodeCanvas.End();


}

//--------------------------------------------------------------
void ofxVisualProgramming::closeDrawMainMenu(){
    // We're done drawing to IMGUI
    ofxVPGui->end();

    // Draw Bottom Bar
    ofSetColor(0,0,0,60);
    ofDrawRectangle(0,ofGetHeight() - (20*scaleFactor),ofGetWidth(),(20*scaleFactor));

    ofDisableAlphaBlending();

    ofPopMatrix();
    ofPopStyle();
    ofPopView();

    // Graphical Context
    ofxVPGui->draw();

}

//--------------------------------------------------------------
void ofxVisualProgramming::drawInspector(){

    ImGui::SetNextWindowSize(ImVec2(ofGetWindowWidth()/4,ofGetWindowHeight()/2), ImGuiCond_Appearing );
    //ImGui::SetNextWindowPos(ImVec2(ofGetWindowWidth()-200,26*scaleFactor), ImGuiCond_Appearing);

    ImGui::Begin(ICON_FA_ADJUST "  Inspector", &inspectorActive, ImGuiWindowFlags_NoCollapse);

    isOverInspector = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) || ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

    // if object id exists
    if(patchObjects.find(nodeCanvas.getActiveNode()) != patchObjects.end()){

        ImGui::Text("%s",patchObjects[nodeCanvas.getActiveNode()]->getDisplayName().c_str());
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("id: %s",ofToString(nodeCanvas.getActiveNode()).c_str());
        ImGui::Spacing();
        ImGui::Spacing();

        patchObjects[nodeCanvas.getActiveNode()]->drawImGuiNodeConfig();

    }

    ImGui::End();
}

//--------------------------------------------------------------
void ofxVisualProgramming::drawLivePatchingSession(){
    if(weAlreadyHaveObject("live patching") && livePatchingObiID != -1 && currentSubpatch == "root" && !patchObjects.empty() && patchObjects.find(livePatchingObiID) != patchObjects.end()){
        if(patchObjects[livePatchingObiID]->inletsConnected[0] && ofxVP_CAST_PIN_PTR<ofTexture>(patchObjects[livePatchingObiID]->_inletParams[0])->isAllocated()){
            float lpDrawW, lpDrawH, lpPosX, lpPosY;
            if(ofxVP_CAST_PIN_PTR<ofTexture>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth() >= ofxVP_CAST_PIN_PTR<ofTexture>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight()){   // horizontal texture
                lpDrawW           = ofGetWidth();
                lpDrawH           = (ofGetWidth()/ofxVP_CAST_PIN_PTR<ofTexture>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth())*ofxVP_CAST_PIN_PTR<ofTexture>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight();
                lpPosX            = 0;
                lpPosY            = (ofGetHeight()-lpDrawH)/2.0f;
            }else{ // vertical texture
                lpDrawW           = (ofxVP_CAST_PIN_PTR<ofTexture>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth()*ofGetHeight())/ofxVP_CAST_PIN_PTR<ofTexture>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight();
                lpDrawH           = ofGetHeight();
                lpPosX            = (ofGetWidth()-lpDrawW)/2.0f;
                lpPosY            = 0;
            }
            ofSetColor(255,*ofxVP_CAST_PIN_PTR<float>(patchObjects[livePatchingObiID]->_outletParams[0]));
            ofxVP_CAST_PIN_PTR<ofTexture>(patchObjects[livePatchingObiID]->_inletParams[0])->draw(lpPosX,lpPosY,lpDrawW,lpDrawH);
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::drawSubpatchNavigation(){
    static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    static ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
    static int selection_mask = (1 << 0); // start selection on root
    int node_clicked = -1;
    int i = 0;

    //ImGui::SetNextWindowSize(ImVec2(200*scaleFactor,400*scaleFactor), ImGuiCond_Appearing );

    ImGui::Begin(ICON_FA_NETWORK_WIRED "  Patch Navigator", &navigationActive, ImGuiWindowFlags_NoCollapse);

    isOverSubpatchNavigator = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) || ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

    ImGui::Spacing();
    if(ImGui::Button(ICON_FA_PLUS_CIRCLE " Add Subpatch",ImVec2(-1,26*scaleFactor))){
        newSubpatchName = "";
        ImGui::OpenPopup("Add Subpatch");
    }
    if(ImGui::BeginPopup("Add Subpatch")){

        ImGui::Text("Name: ");
        ImGui::SameLine();
        if(ImGui::InputText("##NewSubpatchNameInput", &newSubpatchName,ImGuiInputTextFlags_EnterReturnsTrue)){
            if(newSubpatchName != ""){
                // create subpatch
                currentSubpatch         = newSubpatchName;
                vector<SubpatchConnection> subpatchBranch;
                subpatchesMap[currentSubpatch] = subpatchBranch;
                selection_mask = (1 << getSubpatchIndex(currentSubpatch));
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")){
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Create")){
            if(newSubpatchName != ""){
                // create subpatch
                currentSubpatch         = newSubpatchName;
                vector<SubpatchConnection> subpatchBranch;
                subpatchesMap[currentSubpatch] = subpatchBranch;
                selection_mask = (1 << getSubpatchIndex(currentSubpatch));
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();

    }
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    for(map<string,vector<SubpatchConnection>>::iterator it = subpatchesMap.begin(); it != subpatchesMap.end(); it++ ){

        ImGuiTreeNodeFlags node_flags = base_flags;
        const bool is_selected = (selection_mask & (1 << i)) != 0;
        if (is_selected) node_flags |= ImGuiTreeNodeFlags_Selected;

        string rName = "[ ";
        rName += it->first;
        rName += " ]";
        bool node_open;
        if(it->first == "root") {
            node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, rName.c_str(), i);
        }else{
            node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, it->first.c_str(), i);
        }

        // drop object id to subpatch name to change object subpatch
        if (ImGui::BeginDragDropTarget()){
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SUBPATCH_NAME")){
                int move_from = *(const int*)payload->Data;
                std::string move_to = it->first;

                // disconnect object links ( in and out)
                disconnectObject(move_from);
                // switch object subpatch
                patchObjects[move_from]->setSubpatch(move_to);
                // add new subpatches map reference for wireless objects ( sender and receiver )
                if(patchObjects[move_from]->getName() == "sender"){
                    SubpatchConnection _t;
                    _t.objID = move_from;
                    _t.inOut = 1;
                    subpatchesMap[move_to].push_back(_t);
                }else if(patchObjects[move_from]->getName() == "receiver"){
                    SubpatchConnection _t;
                    _t.objID = move_from;
                    _t.inOut = 0;
                    subpatchesMap[move_to].push_back(_t);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()){
            node_clicked = i;
            currentSubpatch = it->first.c_str();
        }

        if (node_open){
            for(int z=0;z<it->second.size();z++){
                if(it->second.at(z).objID != -1){
                    switch (it->second.at(z).type) {
                        case VP_LINK_NUMERIC:
                            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.00f,1.00f,1.00f,1.00f));
                            break;
                        case VP_LINK_STRING:
                            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(200.0f/255.0f,180.0f/255.0f,1.00f,1.00f));
                            break;
                        case VP_LINK_ARRAY:
                            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(120.0f/255.0f,180.0f/255.0f,120.0f/255.0f,1.00f));
                            break;
                        case VP_LINK_TEXTURE:
                            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(120.0f/255.0f,1.00f,1.00f,1.00f));
                            break;
                        case VP_LINK_AUDIO:
                            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.00f,1.00f,120.0f/255.0f,1.00f));
                            break;
                        default:
                            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.5f,0.5f,0.5f,1.00f));
                            break;
                    }
                    // draw gray if not active yet
                    if(it->second.at(z).name == ""){
                        ImGui::PopStyleColor(1);
                        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.5f,0.5f,0.5f,1.00f));
                    }
                    if(it->second.at(z).inOut == 0){ // receiver
                        ImGui::TreeNodeEx((void*)(intptr_t)z, leaf_flags, "Receiving\t<--\t%s", it->second.at(z).name.c_str());
                    }else if(it->second.at(z).inOut == 1){ // sender
                        ImGui::TreeNodeEx((void*)(intptr_t)z, leaf_flags, "Sending \t -->\t%s", it->second.at(z).name.c_str());
                    }
                    ImGui::PopStyleColor(1);
                }
            }
            ImGui::TreePop();
        }

        if (node_clicked != -1){
            // Update selection state
            selection_mask = (1 << node_clicked);   // Click to single-select
        }

        i++;
    }

    ImGui::End();
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetTempFolder(){
    ofDirectory dir;

    if(dir.doesDirectoryExist(ofToDataPath("temp/",true),true)){
        dir.removeDirectory(ofToDataPath("temp/",true),true,true);
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::cleanPatchDataFolder(){
    ofDirectory dir;
    // get patch data folder
    dir.listDir(currentPatchFolderPath+"data/");


    for(size_t i = 0; i < dir.size(); i++){
        if(dir.getFile(i).exists()){
            if(dir.getFile(i).isFile()){
                map<string,string>::iterator sofpIT = scriptsObjectsFilesPaths.find(dir.getFile(i).getFileName());
                if (sofpIT == scriptsObjectsFilesPaths.end()){
                    // not found in patch scripts map, remove it from patch data folder
                    //ofLog(OF_LOG_NOTICE,"%s",dir.getFile(i).getAbsolutePath().c_str());
                    string fileExt = ofToUpper(dir.getFile(i).getExtension());
                    if(fileExt == "SH" || fileExt == "FRAG"){
                        dir.getFile(i).remove();
                    }
                    // remove if filename is empty
                    string tfn = dir.getFile(i).getFileName();
                    if(dir.getFile(i).getFileName().substr(0,tfn.find_last_of('.')) == "empty"){
                        dir.getFile(i).remove();
                    }
                    // remove alone .vert files
                    if(fileExt == "VERT"){
                        string vsName = dir.getFile(i).getFileName();
                        string fsName = dir.getFile(i).getFileName().substr(0,vsName.find_last_of('.'))+".frag";
                        map<string,string>::iterator sofpIT2 = scriptsObjectsFilesPaths.find(fsName);
                        if (sofpIT2 == scriptsObjectsFilesPaths.end()){
                            // related fragment shader not found in patch scripts map, remove it from patch data folder
                            dir.getFile(i).remove();
                        }
                    }
                }
            }
        }
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::reloadFont(){
    fontSize = static_cast<int>(floor(ofMap(scaleFactor,1,6,8,28)));
    font->load(MAIN_FONT,fontSize);
}

//--------------------------------------------------------------
void ofxVisualProgramming::exit(){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            it->second->removeObjectContent();
        }
    }

    if(dspON){
        deactivateDSP();
    }

#ifndef TARGET_WIN32
    cleanPatchDataFolder();

    resetTempFolder();
#endif

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseMoved(ofMouseEventArgs &e){
    unusedArgs(e);
}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseDragged(ofMouseEventArgs &e){

    if(ImGui::IsAnyItemActive() || nodeCanvas.isAnyNodeHovered() || ImGui::IsAnyItemHovered() )
        return;

}

//--------------------------------------------------------------
void ofxVisualProgramming::mousePressed(ofMouseEventArgs &e){

    if(ImGui::IsAnyItemActive() || nodeCanvas.isAnyNodeHovered() || ImGui::IsAnyItemHovered() )
        return;

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseReleased(ofMouseEventArgs &e){

    if(ImGui::IsAnyItemActive() || nodeCanvas.isAnyNodeHovered() || ImGui::IsAnyItemHovered() )
        return;

    setPatchVariable("canvasTranslationX",nodeCanvas.GetCanvasTranslation().x);
    setPatchVariable("canvasTranslationY",nodeCanvas.GetCanvasTranslation().y);

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseScrolled(ofMouseEventArgs &e){

    if(ImGui::IsAnyItemActive() || nodeCanvas.isAnyNodeHovered() || ImGui::IsAnyItemHovered())
        return;

}

//--------------------------------------------------------------
void ofxVisualProgramming::keyPressed(ofKeyEventArgs &e){

    if(ImGui::IsAnyItemActive())
        return;

    for(unsigned int i=0;i<leftToRightIndexOrder.size();i++){
        patchObjects[leftToRightIndexOrder[i].second]->keyPressed(e,patchObjects);
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::keyReleased(ofKeyEventArgs &e){

    if(ImGui::IsAnyItemActive())
        return;

    for(unsigned int i=0;i<leftToRightIndexOrder.size();i++){
        patchObjects[leftToRightIndexOrder[i].second]->keyReleased(e,patchObjects);
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::audioProcess(float *input, int bufferSize, int nChannels){

    if(bLoadingNewPatch) return;
    if(bLoadingNewObject) return;

    if(audioSampleRate != 0 && dspON){

        std::lock_guard<std::mutex> lck(vp_mutex);

        if(audioGUIINChannels > 0){
            inputBuffer.copyFrom(input, bufferSize, nChannels, audioSampleRate);

            // compute audio input
            if(!inputBuffer.getBuffer().empty()){
                for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                    if(it->second != nullptr){
                        if(!it->second->getWillErase()){
                            it->second->audioIn(inputBuffer);
                        }
                    }
                }

                lastInputBuffer = inputBuffer;
            }

        }
        if(audioGUIOUTChannels > 0){
            // compute audio output
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(it->second != nullptr){
                    if(!it->second->getWillErase()){
                        it->second->audioOut(emptyBuffer);
                    }
                }
            }
        }

    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::addObject(string name,ofVec2f pos,std::string fp){

    // check if object exists
    bool exists = isObjectInLibrary(name);

    if(!exists && name != "audio device"){
        return;
    }

    // discard special unique objects
    if(name == "live patching" && weAlreadyHaveObject(name)){
        ofLog(OF_LOG_WARNING,"%s object is a single instance type of object, a Mosaic patch can't contain more than one.",name.c_str());
        return;
    }

    bLoadingNewObject       = true;

    shared_ptr<PatchObject> tempObj = selectObject(name);

    // selectObject can return nullptr !
    if( tempObj.get() == nullptr ){
        ofLogWarning("ofxVisualProgramming::addObject") << "The requested object « " << name << " » is not available !"
    #ifdef OFXVP_BUILD_WITH_MINIMAL_OBJECTS
            << "\n(note: ofxVisualProgramming is compiling with OFXVP_BUILD_WITH_MINIMAL_OBJECTS enabled.)";
#else
            ;
#endif
        bLoadingNewObject = false;
        return;
    }

    tempObj->newObject();
    tempObj->setPatchfile(currentPatchFile);
    tempObj->setup(mainWindow);
    tempObj->setupDSP(*engine);
    tempObj->setIsRetina(isRetina,scaleFactor);
    tempObj->move(static_cast<int>(pos.x),static_cast<int>(pos.y));
    tempObj->setSubpatch(currentSubpatch);
    if(fp != "none"){
        tempObj->setFilepath(fp);
    }
    ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
    ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
    ofAddListener(tempObj->reconnectOutletsEvent ,this,&ofxVisualProgramming::reconnectObjectOutlets);
    ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);

    actualObjectID++;

    bool saved = tempObj->saveConfig(false);

    if(saved){
        patchObjects[tempObj->getId()] = tempObj;
        lastAddedObjectID = tempObj->getId();

        // store in a map all the wireless links objects reference, for the subpatch navigation window
        if(name == "sender"){
            SubpatchConnection _t;
            _t.objID = tempObj->getId();
            _t.inOut = 1;
            subpatchesMap[currentSubpatch].push_back(_t);
        }else if(name == "receiver"){
            SubpatchConnection _t;
            _t.objID = tempObj->getId();
            _t.inOut = 0;
            subpatchesMap[currentSubpatch].push_back(_t);
        }

        nodeCanvas.setActiveNode(lastAddedObjectID);
        nodeCanvas.addNodeToMap(lastAddedObjectID,name);

        //nodeCanvas.debugNodeMap();

        if(patchObjects[lastAddedObjectID] != nullptr){
            nextObjectPosition = (patchObjects[lastAddedObjectID]->getPos()/ofPoint(scaleFactor,scaleFactor)) + (ofPoint(patchObjects[lastAddedObjectID]->getObjectWidth()+40,40)/ofPoint(scaleFactor,scaleFactor));

        }
    }

    bLoadingNewObject       = false;

}

//--------------------------------------------------------------
shared_ptr<PatchObject> ofxVisualProgramming::getLastAddedObject(){
    if(lastAddedObjectID != -1 && patchObjects.count(lastAddedObjectID) > 0){
        return patchObjects[lastAddedObjectID];
    }else{
        return nullptr;
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::resetObject(int &id){
    if ((id != -1) && (patchObjects[id] != nullptr)){

        ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        if (XML.loadFile(currentPatchFile)){
#else
        if (XML.load(currentPatchFile)){
#endif

            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                if(it->second != nullptr){
                    vector<shared_ptr<PatchLink>> tempBuffer;
                    for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                        if(it->second->outPut[j]->toObjectID == id){
                            if(it->second->outPut[j]->toInletID < patchObjects[id]->getNumInlets()){
                                tempBuffer.push_back(it->second->outPut[j]);
                                if(it->second->outPut[j]->type == VP_LINK_AUDIO){
                                    // reconnect dsp link
                                    patchObjects[it->first]->pdspOut[it->second->outPut[j]->fromOutletID] >> patchObjects[id]->pdspIn[it->second->outPut[j]->toInletID];
                                }
                            }
                        }else{
                            tempBuffer.push_back(it->second->outPut[j]);
                        }
                    }
                    it->second->outPut = tempBuffer;
                }
            }

            int totalObjects = XML.getNumTags("object");

            // remove links to the resetted object
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) != id){
                        if(XML.pushTag("outlets")){
                            int totalLinks = XML.getNumTags("link");
                            for(int l=0;l<totalLinks;l++){
                                if(XML.pushTag("link",l)){
                                    int totalTo = XML.getNumTags("to");
                                    for(int t=0;t<totalTo;t++){
                                        if(XML.pushTag("to",t)){
                                            bool delLink = false;
                                            if(XML.getValue("id", -1) == id && XML.getValue("inlet", -1) >= patchObjects[id]->getNumInlets()){
                                                delLink = true;
                                            }
                                            XML.popTag();
                                            if(delLink){
                                                XML.removeTag("to",t);
                                            }
                                        }
                                    }
                                    XML.popTag();
                                }
                            }
                            XML.popTag();
                        }
                    }
                    XML.popTag();
                }
            }

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            XML.saveFile();
#else
            XML.save();
#endif

        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetObject(int id){
    if ((id != -1) && (patchObjects[id] != nullptr)){
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                vector<shared_ptr<PatchLink>> tempBuffer;
                for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                    if(it->second->outPut[j]->toObjectID != id){
                        tempBuffer.push_back(it->second->outPut[j]);
                    }else{
                        it->second->outPut[j]->isDisabled = true;
                        patchObjects[it->second->outPut[j]->toObjectID]->inletsConnected[it->second->outPut[j]->toInletID] = false;
                    }
                }
                it->second->outPut = tempBuffer;
            }
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::reconnectObjectOutlets(int &id){
    if ((id != -1) && (patchObjects[id] != nullptr)){

        ofxXmlSettings XML;
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        if (XML.loadFile(currentPatchFile)){
#else
        if (XML.load(currentPatchFile)){
#endif
            int totalObjects = XML.getNumTags("object");

            // relink object outlets from XML
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == id){
                        int fromID = XML.getValue("id", -1);
                        if (XML.pushTag("outlets")){
                            int totalOutlets = XML.getNumTags("link");
                            for(int j=0;j<totalOutlets;j++){
                                if (XML.pushTag("link",j)){
                                    int linkType = XML.getValue("type", 0);
                                    int totalLinks = XML.getNumTags("to");
                                    for(int z=0;z<totalLinks;z++){
                                        if(XML.pushTag("to",z)){
                                            int toObjectID = XML.getValue("id", 0);
                                            int toInletID = XML.getValue("inlet", 0);

                                            // fix loading patches with non-existent objects (older OFXVP versions)
                                            if(isObjectIDInPatchMap(toObjectID)){
                                                if(connect(fromID,j,toObjectID,toInletID,linkType)){
                                                    //ofLog(OF_LOG_NOTICE,"Connected object %s, outlet %i TO object %s, inlet %i",patchObjects[fromID]->getName().c_str(),z,patchObjects[toObjectID]->getName().c_str(),toInletID);
                                                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                                }
                                            }

                                            XML.popTag();
                                        }
                                    }
                                    XML.popTag();
                                }
                            }

                            XML.popTag();
                        }
                        XML.popTag();
                    }
                    XML.popTag();
                }
            }

        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::clearObjectsMap(){
    if(ofGetElapsedTimeMillis()-resetTime > wait){
        resetTime = ofGetElapsedTimeMillis();
        eraseIndexes.clear();
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                if(it->second->getWillErase()){
                    eraseIndexes.push_back(it->first);
                }
            }
        }
        for(int x=0;x<static_cast<int>(eraseIndexes.size());x++){

            if(!clearingObjectsMap){
                for(int p=0;p<static_cast<int>(patchObjects.at(eraseIndexes.at(x))->outPut.size());p++){
                    patchObjects[patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toObjectID]->inletsConnected.at(patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toInletID) = false;
                }
            }

            // remove scripts objects filepath reference from scripts objects files map
            std::ifstream testPath(patchObjects.at(eraseIndexes.at(x))->getFilepath());
            if(testPath){ // file exists
                ofFile tempsofp(patchObjects.at(eraseIndexes.at(x))->getFilepath());
                string fileExt = ofToUpper(tempsofp.getExtension());
                if(fileExt == "LUA" || fileExt == "SH"){
                    map<string,string>::iterator sofpIT = scriptsObjectsFilesPaths.find(tempsofp.getFileName());
                    if (sofpIT != scriptsObjectsFilesPaths.end()){
                        // found it, remove it
                        scriptsObjectsFilesPaths.erase(sofpIT);
                    }
                }else if(fileExt == "FRAG"){
                    // remove .frag
                    map<string,string>::iterator sofpIT = scriptsObjectsFilesPaths.find(tempsofp.getFileName());
                    if (sofpIT != scriptsObjectsFilesPaths.end()){
                        // found it, remove it
                        scriptsObjectsFilesPaths.erase(sofpIT);
                    }
                    // remove .vert
                    string pf_fsName = tempsofp.getFileName();
                    string pf_vsName = tempsofp.getEnclosingDirectory()+tempsofp.getFileName().substr(0,pf_fsName.find_last_of('.'))+".vert";
                    ofFile tempVert(pf_vsName);
                    map<string,string>::iterator sofpITV = scriptsObjectsFilesPaths.find(tempVert.getFileName());
                    if (sofpITV != scriptsObjectsFilesPaths.end()){
                        // found it, remove it
                        scriptsObjectsFilesPaths.erase(sofpITV);
                    }
                }
            }

            patchObjects.at(eraseIndexes.at(x))->removeObjectContent(true);
            patchObjects.erase(eraseIndexes.at(x));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if(clearingObjectsMap){
            clearingObjectsMap = false;
            openPatch(currentPatchFile);
        }

    }
}

//--------------------------------------------------------------
bool ofxVisualProgramming::isObjectInLibrary(string name){
    bool exists = false;
    for(ofxVPObjects::factory::objectRegistry::iterator it = ofxVPObjects::factory::getObjectRegistry().begin(); it != ofxVPObjects::factory::getObjectRegistry().end(); it++ ){
        if(it->first == name){
            exists = true;
            break;
        }
    }

    return exists;
}

//--------------------------------------------------------------
bool ofxVisualProgramming::isObjectIDInPatchMap(int id){
    bool exists = false;

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            if(it->first == id){
                exists = true;
                break;
            }
        }
    }

    return exists;
}

//--------------------------------------------------------------
string ofxVisualProgramming::getObjectNameFromID(int id){
    string name = "";
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            if(it->first == id){
                name = it->second->getName();
            }
        }
    }

    return name;
}

//--------------------------------------------------------------
int ofxVisualProgramming::getSubpatchIndex(string name){
    int ind = 0;
    for(map<string,vector<SubpatchConnection>>::iterator it = subpatchesMap.begin(); it != subpatchesMap.end(); it++ ){
        if(it->first == name){
            return ind;
        }
        ind++;
    }
    return 0;
}

//--------------------------------------------------------------
void ofxVisualProgramming::removeObject(int &id){
    resetTime = ofGetElapsedTimeMillis();

    bLoadingNewObject = true;

    if ( (id != -1) && (patchObjects[id] != nullptr) && (patchObjects[id]->getName() != "audio device") ){

        int targetID = id;
        bool found = false;

        if(targetID == lastAddedObjectID) lastAddedObjectID=0;

        ofxXmlSettings XML;
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        if (XML.loadFile(currentPatchFile)){
#else
        if (XML.load(currentPatchFile)){
#endif
            int totalObjects = XML.getNumTags("object");

            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == id){
                        targetID = i;
                        found = true;
                    }
                    XML.popTag();
                }
            }

            // remove links to the removed object
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) != id){
                        //ofLogNotice("id",ofToString(XML.getValue("id", -1)));
                        if(XML.pushTag("outlets")){
                            int totalLinks = XML.getNumTags("link");
                            for(int l=0;l<totalLinks;l++){
                                if(XML.pushTag("link",l)){
                                    int totalTo = XML.getNumTags("to");
                                    for(int t=0;t<totalTo;t++){
                                        if(XML.tagExists("to",t)){
                                            if(XML.pushTag("to",t)){
                                                bool delLink = false;
                                                if(XML.getValue("id", -1) == id){
                                                    //ofLogNotice("remove link id",ofToString(XML.getValue("id", -1)));
                                                    delLink = true;
                                                }
                                                XML.popTag();
                                                if(delLink){
                                                    XML.removeTag("to",t);
                                                }
                                            }
                                        }
                                    }
                                    XML.popTag();
                                }
                            }
                            XML.popTag();
                        }
                    }
                    XML.popTag();
                }
            }
            // remove object
            if(found){
                XML.removeTag("object", targetID);
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
                XML.saveFile();
#else
                XML.save();
#endif
            }
        }

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                vector<shared_ptr<PatchLink>> tempBuffer;
                for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                    if(it->second->outPut[j]->toObjectID != id){
                        tempBuffer.push_back(it->second->outPut[j]);
                    }else{
                        it->second->outPut[j]->isDisabled = true;
                        patchObjects[it->second->outPut[j]->toObjectID]->inletsConnected[it->second->outPut[j]->toInletID] = false;
                    }
                }
                it->second->outPut = tempBuffer;
            }
        }

        // check reference from subpatches map ( if the object was a wireless one ,sender or receiver )
        for(map<string,vector<SubpatchConnection>>::iterator it = subpatchesMap.begin(); it != subpatchesMap.end(); it++ ){
            for(int z=0;z<it->second.size();z++){
                if(it->second.at(z).objID == id){
                    it->second.at(z).objID = -1;
                    break;
                }
            }
        }

        nodeCanvas.removeNodeFromMap(id);

        //nodeCanvas.debugNodeMap();
    }

    bLoadingNewObject = false;
}

//--------------------------------------------------------------
void ofxVisualProgramming::duplicateObject(int &id){
    // disable duplicate for hardware&system related objects
    if(!patchObjects[id]->getIsHardwareObject()){
         ofVec3f tempPosition = (patchObjects[id]->getPos()/ofPoint(scaleFactor,scaleFactor)) + (ofPoint(patchObjects[id]->getObjectWidth()+40,40)/ofPoint(scaleFactor,scaleFactor));
         std::ifstream testPath(patchObjects[id]->getFilepath());
         if(testPath){ // object has a file in filepath
             addObject(patchObjects[id]->getName(),tempPosition,patchObjects[id]->getFilepath());
         }else{
             addObject(patchObjects[id]->getName(),tempPosition);
         }
    }else{
        ofLog(OF_LOG_NOTICE,"'%s' is one of the Mosaic objects that can't (for now) be duplicated due to hardware/system related issues.",patchObjects[id]->getName().c_str());
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::disconnectObject(int id){
    resetTime = ofGetElapsedTimeMillis();

    bLoadingNewObject = true;

    if ( (id != -1) && (patchObjects[id] != nullptr) && (patchObjects[id]->getName() != "audio device") ){

        bool found = false;

        ofxXmlSettings XML;
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        if (XML.loadFile(currentPatchFile)){
#else
        if (XML.load(currentPatchFile)){
#endif
            int totalObjects = XML.getNumTags("object");

            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) == id){
                        found = true;
                    }
                    XML.popTag();
                }
            }

            // disconnect all object links to other objects
            for(size_t i=0;i<patchObjects[id]->outPut.size();i++){
                patchObjects[id]->disconnectLink(patchObjects,patchObjects[id]->outPut.at(i)->id);
            }

            // remove links to the selected object
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    if(XML.getValue("id", -1) != id){
                        //ofLogNotice("id",ofToString(XML.getValue("id", -1)));
                        if(XML.pushTag("outlets")){
                            int totalLinks = XML.getNumTags("link");
                            for(int l=0;l<totalLinks;l++){
                                if(XML.pushTag("link",l)){
                                    int totalTo = XML.getNumTags("to");
                                    for(int t=0;t<totalTo;t++){
                                        if(XML.tagExists("to",t)){
                                            if(XML.pushTag("to",t)){
                                                bool delLink = false;
                                                if(XML.getValue("id", -1) == id){
                                                    //ofLogNotice("remove link id",ofToString(XML.getValue("id", -1)));
                                                    delLink = true;
                                                }
                                                XML.popTag();
                                                if(delLink){
                                                    XML.removeTag("to",t);
                                                }
                                            }
                                        }
                                    }
                                    XML.popTag();
                                }
                            }
                            XML.popTag();
                        }
                    }
                    XML.popTag();
                }
            }
            // save patch
            if(found){
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
                XML.saveFile();
#else
                XML.save();
#endif
            }
        }

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second != nullptr){
                vector<shared_ptr<PatchLink>> tempBuffer;
                for(int j=0;j<static_cast<int>(it->second->outPut.size());j++){
                    if(it->second->outPut[j]->toObjectID != id){
                        tempBuffer.push_back(it->second->outPut[j]);
                    }else{
                        it->second->outPut[j]->isDisabled = true;
                        patchObjects[it->second->outPut[j]->toObjectID]->inletsConnected[it->second->outPut[j]->toInletID] = false;
                    }
                }
                it->second->outPut = tempBuffer;
            }
        }

        // check reference from subpatches map ( if the object was a wireless one ,sender or receiver )
        for(map<string,vector<SubpatchConnection>>::iterator it = subpatchesMap.begin(); it != subpatchesMap.end(); it++ ){
            for(int z=0;z<it->second.size();z++){
                if(it->second.at(z).objID == id){
                    it->second.at(z).objID = -1;
                    break;
                }
            }
        }

    }

    bLoadingNewObject = false;
}

//--------------------------------------------------------------
bool ofxVisualProgramming::connect(int fromID, int fromOutlet, int toID,int toInlet, int linkType){
    bool connected = false;

    if((fromID != -1) && (patchObjects[fromID] != nullptr) && (toID != -1) && (patchObjects[toID] != nullptr) && (patchObjects[fromID]->getOutletType(fromOutlet) == patchObjects[toID]->getInletType(toInlet)) && !patchObjects[toID]->inletsConnected[toInlet]){

        //std::cout << "Mosaic :: "<< "Connect object " << patchObjects[fromID]->getName().c_str() << ":" << ofToString(fromID) << " to object " << patchObjects[toID]->getName().c_str() << ":" << ofToString(toID) << std::endl;

        shared_ptr<PatchLink> tempLink = shared_ptr<PatchLink>(new PatchLink());

        string tmpID = ofToString(fromID)+ofToString(fromOutlet)+ofToString(toID)+ofToString(toInlet);

        tempLink->id            = stoi(tmpID);
        tempLink->posFrom       = patchObjects[fromID]->getOutletPosition(fromOutlet);
        tempLink->posTo         = patchObjects[toID]->getInletPosition(toInlet);
        tempLink->type          = patchObjects[toID]->getInletType(toInlet);
        tempLink->fromOutletID  = fromOutlet;
        tempLink->toObjectID    = toID;
        tempLink->toInletID     = toInlet;
        tempLink->isDisabled    = false;
        tempLink->isDeactivated = false;

        patchObjects[fromID]->outPut.push_back(tempLink);

        patchObjects[toID]->inletsConnected[toInlet] = true;

        if(tempLink->type == VP_LINK_NUMERIC){
            patchObjects[toID]->_inletParams[toInlet] = new float();
        }else if(tempLink->type == VP_LINK_STRING){
            patchObjects[toID]->_inletParams[toInlet] = new string();
        }else if(tempLink->type == VP_LINK_ARRAY){
            patchObjects[toID]->_inletParams[toInlet] = new vector<float>();
        }else if(tempLink->type == VP_LINK_PIXELS){
            patchObjects[toID]->_inletParams[toInlet] = new ofPixels();
        }else if(tempLink->type == VP_LINK_TEXTURE){
            patchObjects[toID]->_inletParams[toInlet] = new ofTexture();
        }else if(tempLink->type == VP_LINK_FBO){
            patchObjects[toID]->_inletParams[toInlet] = new ofxPingPong();
        }else if(tempLink->type == VP_LINK_AUDIO){
            patchObjects[toID]->_inletParams[toInlet] = new ofSoundBuffer();
            if(patchObjects[fromID]->getIsPDSPPatchableObject() && patchObjects[toID]->getIsPDSPPatchableObject()){
                patchObjects[fromID]->pdspOut[fromOutlet] >> patchObjects[toID]->pdspIn[toInlet];
            }/*else if(patchObjects[fromID]->getName() == "audio device" && patchObjects[toID]->getIsPDSPPatchableObject()){
                patchObjects[fromID]->pdspOut[fromOutlet] >> patchObjects[toID]->pdspIn[toInlet];
            }*/
        }

        checkSpecialConnection(fromID,toID,linkType);

#ifdef OFXVP_DEBUG
        std::cout << "Connect from " << patchObjects[fromID]->getName() << " to " << patchObjects[toID]->getName() << std::endl;
#endif

        connected = true;
    }

    return connected;
}

//--------------------------------------------------------------
void ofxVisualProgramming::checkSpecialConnection(int fromID, int toID, int linkType){
    if(patchObjects[fromID]->getName() == "lua script"){
        if((patchObjects[toID]->getName() == "glsl shader" || patchObjects[toID]->getName() == "output window") && linkType == VP_LINK_TEXTURE){
            patchObjects[fromID]->resetResolution(toID,patchObjects[toID]->getOutputWidth(),patchObjects[toID]->getOutputHeight());
        }
    }
    if(patchObjects[fromID]->getName() == "glsl shader"){
        if(patchObjects[toID]->getName() == "output window" && linkType == VP_LINK_TEXTURE){
            patchObjects[fromID]->resetResolution(toID,patchObjects[toID]->getOutputWidth(),patchObjects[toID]->getOutputHeight());
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetSystemObjects(){
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            if(it->second->getIsSystemObject()){
                it->second->resetSystemObject();
                if(it->second->getName() != "audio device"){
                    resetObject(it->second->getId());
                }
                if(it->second->getIsAudioOUTObject()){
                    it->second->setupAudioOutObjectContent(*engine);
                }
            }
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetSpecificSystemObjects(string name){
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            if(it->second->getIsSystemObject() && it->second->getName() == name){
                it->second->resetSystemObject();
                resetObject(it->second->getId());
                if(it->second->getIsAudioOUTObject()){
                    it->second->setupAudioOutObjectContent(*engine);
                }
            }
        }
    }
}

//--------------------------------------------------------------
bool ofxVisualProgramming::weAlreadyHaveObject(string name){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            if(it->second->getName() == name){
                return true;
            }
        }

    }

    return false;
}

//--------------------------------------------------------------
shared_ptr<PatchObject> ofxVisualProgramming::selectObject(string objname){
    ofxVPObjects::factory::objectRegistry& reg = ofxVPObjects::factory::getObjectRegistry();
    ofxVPObjects::factory::objectRegistry::iterator it = reg.find(objname);

    if (it != reg.end()) {
        ofxVPObjects::factory::CreateObjectFunc func = it->second;
        return shared_ptr<PatchObject>( func() );
    }

    ofLogError("ofxVisualProgramming::selectObject") << "Object not found: " << objname << ". Maybe this PatchObject is not available on your platform or there might be a version error.";
    return shared_ptr<PatchObject>(nullptr);
}

//--------------------------------------------------------------
void ofxVisualProgramming::newPatch(string release){
    string newFileName = "patch_"+ofGetTimestampString("%y%m%d")+alphabet.at(newFileCounter)+".xml";
    ofFile fileToRead(ofToDataPath("empty_patch.xml",true));

    // set patch release
    ofxXmlSettings XML;
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(fileToRead.getAbsolutePath())){
#else
    if (XML.load(fileToRead.getAbsolutePath())){
#endif
        XML.setValue("release",release);
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
        XML.saveFile();
#else
        XML.save();
#endif
    }

    ofFile newPatchFile(ofToDataPath("temp/"+newFileName,true));

    ofFile::copyFromTo(fileToRead.getAbsolutePath(),newPatchFile.getAbsolutePath(),true,true);

    newFileCounter++;

    preloadPatch(newPatchFile.getAbsolutePath());

}

//--------------------------------------------------------------
void ofxVisualProgramming::newTempPatchFromFile(string patchFile){
    string newFileName = "patch_"+ofGetTimestampString("%y%m%d")+alphabet.at(newFileCounter)+".xml";
    ofFile fileToRead(patchFile);
    ofFile newPatchFile(ofToDataPath("temp/"+newFileName,true));
    ofFile::copyFromTo(fileToRead.getAbsolutePath(),newPatchFile.getAbsolutePath(),true,true);

    ofDirectory dataFolderOrigin;
    dataFolderOrigin.listDir(fileToRead.getEnclosingDirectory()+"data/");

    if(dataFolderOrigin.exists()){

        // remove previous data content
        ofDirectory oldData;
        oldData.listDir(ofToDataPath("temp/data/",true));
#ifdef OFXVP_DEBUG
        std::cout << "Removing content from directory: " << oldData.getAbsolutePath() << std::endl;
#endif
        for(size_t i=0;i<oldData.getFiles().size();i++){
            oldData.getFile(i).remove();
#ifdef OFXVP_DEBUG
            std::cout << "Removing file: " << oldData.getFile(i).getAbsolutePath() << std::endl;
#endif
        }

        string oldDataPath = oldData.getAbsolutePath();


        // copy new data content
#ifdef OFXVP_DEBUG
        std::cout << "Copying from  " << dataFolderOrigin.getAbsolutePath() << " to " << oldDataPath << std::endl;
#endif
        if(dataFolderOrigin.canRead() && oldData.canWrite()){
            for(size_t i=0;i<dataFolderOrigin.getFiles().size();i++){
                ofFile::copyFromTo(dataFolderOrigin.getFile(i).getAbsolutePath(),oldDataPath+"/"+dataFolderOrigin.getFile(i).getFileName(),true,true);
            }
        }

    }

    newFileCounter++;

    preloadPatch(newPatchFile.getAbsolutePath());

}

//--------------------------------------------------------------
void ofxVisualProgramming::preloadPatch(string patchFile){
    currentPatchFile = patchFile;
    tempPatchFile = currentPatchFile;

    actualObjectID          = 0;

    // clear previous patch
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            if(it->second->getName() != "audio device"){
                it->second->setWillErase(true);
            }else{
                for(int in=0;in<it->second->getNumInlets();in++){
                    it->second->inletsConnected[in] = false;
                    it->second->pdspIn[in].disconnectIn();
                }

                it->second->outPut.clear();
                it->second->setWillErase(true);
            }
        }
    }

    // clear canvas nodes map
    nodeCanvas.clearNodesMap();

    // clear subpatch navigation data
    subpatchesMap.clear();
    currentSubpatch         = "root";
    newSubpatchName         = "";
    vector<SubpatchConnection> rootBranch;
    subpatchesMap[currentSubpatch] = rootBranch;

    resetTime = ofGetElapsedTimeMillis();
    clearingObjectsMap = true;
}

//--------------------------------------------------------------
void ofxVisualProgramming::openPatch(string patchFile){

    bLoadingNewPatch = true;

    // reset subpatch level
    currentSubpatch = "root";

    currentPatchFile = patchFile;
    ofFile temp(currentPatchFile);
    currentPatchFolderPath  = temp.getEnclosingDirectory();

    ofFile patchDataFolder(currentPatchFolderPath+"data/");

    if(!patchDataFolder.exists()){
        patchDataFolder.create();
    }

    // load new patch
    loadPatch(currentPatchFile);

}

//--------------------------------------------------------------
void ofxVisualProgramming::loadPatch(string patchFile){

    loadPatchTime           = ofGetElapsedTimeMillis();

    ofxXmlSettings XML;
    string tstr;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(patchFile)){
#else
    if (XML.load(patchFile)){
#endif

        // Load main settings
        if (XML.pushTag("settings")){
            // Setup projector dimension
            output_width = XML.getValue("output_width",0);
            output_height = XML.getValue("output_height",0);
            // setup canvas
            ImVec2 tr = ImVec2(XML.getValue("canvasTranslationX",0),XML.getValue("canvasTranslationY",0));
            if(tr.x != 0 && tr.y != 0){
                nodeCanvas.SetCanvasTranslation(tr);
            }

            // setup audio
            dspON = XML.getValue("dsp",0);
            audioINDev = XML.getValue("audio_in_device",0);
            audioOUTDev = XML.getValue("audio_out_device",0);
            audioSampleRate = XML.getValue("sample_rate_out",0);
            audioBufferSize = XML.getValue("buffer_size",0);
            bpm = XML.getValue("bpm",0);
            // pre 0.4.0 patches auto fix
            if(bpm == 0){
                bpm = 120;
                XML.setValue("bpm",bpm);
            }

            delete engine;
            engine = nullptr;
            engine = new pdsp::Engine();

            soundStreamIN.close();
#if defined(TARGET_WIN32)
            audioDevices = soundStreamIN.getDeviceList(ofSoundDevice::Api::MS_DS);
#elif defined(TARGET_OSX)
            audioDevices = soundStreamIN.getDeviceList(ofSoundDevice::Api::OSX_CORE);
#else
            audioDevices = soundStreamIN.getDeviceList(ofSoundDevice::Api::PULSE);
#endif

            audioDevicesStringIN.clear();
            audioDevicesID_IN.clear();
            audioDevicesStringOUT.clear();
            audioDevicesID_OUT.clear();
            audioDevicesSR.clear();
            tstr = "------------------- AUDIO DEVICES";
            ofLog(OF_LOG_NOTICE,"%s",tstr.c_str());
            for(size_t i=0;i<audioDevices.size();i++){
                string tempSR = "";
                for(size_t sr=0;sr<audioDevices[i].sampleRates.size();sr++){
                    if(sr < audioDevices[i].sampleRates.size()-1){
                        tempSR += ofToString(audioDevices[i].sampleRates.at(sr))+", ";
                    }else{
                        tempSR += ofToString(audioDevices[i].sampleRates.at(sr));
                    }
                }

                bool haveMinSR = false;
                for(size_t sr=0;sr<audioDevices[i].sampleRates.size();sr++){
                    if(audioDevices[i].sampleRates.at(sr) >= 44100){
                        haveMinSR = true;
                        break;
                    }
                }
                if(audioDevices[i].inputChannels > 0 && haveMinSR){
                    audioDevicesStringIN.push_back("  "+audioDevices[i].name);
                    audioDevicesID_IN.push_back(i);
                    for(size_t sr=0;sr<audioDevices[i].sampleRates.size();sr++){
                        if(audioDevices[i].sampleRates.at(sr) >= 44100){
                            audioDevicesSR.push_back(ofToString(audioDevices[i].sampleRates.at(sr)));
                        }

                    }
                    ofLog(OF_LOG_NOTICE,"INPUT Device[%zu]: %s (IN:%i - OUT:%i), Sample Rates: %s",i,audioDevices[i].name.c_str(),audioDevices[i].inputChannels,audioDevices[i].outputChannels,tempSR.c_str());
                }
                if(audioDevices[i].outputChannels > 0 && haveMinSR){
                    audioDevicesStringOUT.push_back("  "+audioDevices[i].name);
                    audioDevicesID_OUT.push_back(i);
                    for(size_t sr=0;sr<audioDevices[i].sampleRates.size();sr++){
                        if(audioDevices[i].sampleRates.at(sr) >= 44100){
                            audioDevicesSR.push_back(ofToString(audioDevices[i].sampleRates.at(sr)));
                        }
                    }
                    ofLog(OF_LOG_NOTICE,"OUTPUT Device[%zu]: %s (IN:%i - OUT:%i), Sample Rates: %s",i,audioDevices[i].name.c_str(),audioDevices[i].inputChannels,audioDevices[i].outputChannels,tempSR.c_str());
                }

                // remove duplicates from sample rates vector
                std::sort( audioDevicesSR.begin(), audioDevicesSR.end() );
                audioDevicesSR.erase( std::unique( audioDevicesSR.begin(), audioDevicesSR.end() ), audioDevicesSR.end() );
                std::sort( audioDevicesSR.begin(), audioDevicesSR.end(), [] (const std::string& lhs, const std::string& rhs) {
                    return std::stoi(lhs) < std::stoi(rhs);
                } );

                //ofLog(OF_LOG_NOTICE,"Device[%zu]: %s (IN:%i - OUT:%i), Sample Rates: %s",i,audioDevices[i].name.c_str(),audioDevices[i].inputChannels,audioDevices[i].outputChannels,tempSR.c_str());
            }

            // check audio devices index
            audioGUIINIndex         = -1;
            audioGUIOUTIndex        = -1;

            audioGUIINChannels      = 0;
            audioGUIOUTChannels     = 0;

            // check input devices
            if(!audioDevicesID_IN.empty()){
                for(size_t i=0;i<audioDevicesID_IN.size();i++){
                    if(audioDevicesID_IN.at(i) == audioINDev){
                        audioGUIINIndex = i;
                        break;
                    }
                }
            }
            if(audioGUIINIndex == -1){ // no configured input device available
                // check if there is one available
                if(!audioDevicesID_IN.empty()){
                    isInputDeviceAvailable = true;
                    // select the first one available
                    audioGUIINIndex = 0;
                    audioINDev = audioDevicesID_IN.at(audioGUIINIndex);
                }else{
                    isInputDeviceAvailable = false;
                    audioGUIINIndex = 0;
                }
            }else{
                isInputDeviceAvailable = true;
                audioINDev = audioDevicesID_IN.at(audioGUIINIndex);
            }

            // check output devices
            if(!audioDevicesID_OUT.empty()){
                for(size_t i=0;i<audioDevicesID_OUT.size();i++){
                    if(audioDevicesID_OUT.at(i) == audioOUTDev){
                        audioGUIOUTIndex = i;
                        break;
                    }
                }
            }
            if(audioGUIOUTIndex == -1){ // no configured output device available
                // check if there is one available
                if(!audioDevicesID_OUT.empty()){
                    isOutputDeviceAvailable = true;
                    // select the first one available
                    audioGUIOUTIndex = 0;
                    audioOUTDev = audioDevicesID_OUT.at(audioGUIOUTIndex);
                }else{
                    isOutputDeviceAvailable = false;
                    audioGUIOUTIndex = 0;
                }
            }else{
                isOutputDeviceAvailable = true;
                audioOUTDev = audioDevicesID_OUT.at(audioGUIOUTIndex);
            }

            // select default devices
            if(isInputDeviceAvailable){
                audioGUIINChannels      = static_cast<int>(audioDevices[audioINDev].inputChannels);
                //audioSampleRate         = audioDevices[audioINDev].sampleRates[0];
            }else{
                audioGUIINChannels      = 0;
            }

            if(isOutputDeviceAvailable){
                audioGUIOUTChannels     = static_cast<int>(audioDevices[audioOUTDev].outputChannels);
                //audioSampleRate         = audioDevices[audioOUTDev].sampleRates[0];
            }else{
                audioGUIOUTChannels     = 0;
            }

            XML.setValue("buffer_size",audioBufferSize);
            XML.setValue("sample_rate_in",audioSampleRate);
            XML.setValue("sample_rate_out",audioSampleRate);
            XML.setValue("input_channels",audioGUIINChannels);
            XML.setValue("output_channels",audioGUIOUTChannels);
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            XML.saveFile();
#else
            XML.save();
#endif

            for(size_t bs=0;bs<audioDevicesBS.size();bs++){
                if(ofToInt(audioDevicesBS.at(bs)) == audioBufferSize){
                    audioGUIBSIndex = bs;
                    break;
                }
            }

            // at least we need one audio device available (input or output) to start the engine
            if(dspON && (isInputDeviceAvailable || isOutputDeviceAvailable)){
                engine->setChannels(audioGUIINChannels, audioGUIOUTChannels);
                this->setChannels(audioGUIINChannels,0);

                for(int in=0;in<audioGUIINChannels;in++){
                    engine->audio_in(in) >> this->in(in);
                }
                this->out_silent() >> engine->blackhole();

                if(isInputDeviceAvailable){
                    engine->setInputDeviceID(audioDevices[audioINDev].deviceID);
                }

                if(isOutputDeviceAvailable){
                    engine->setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
                }

                engine->setup(audioSampleRate, audioBufferSize, audioNumBuffers);
                engine->sequencer.setTempo(bpm);

                if(isInputDeviceAvailable){
                    tstr = "------------------- Soundstream INPUT Started on";
                    ofLog(OF_LOG_NOTICE,"%s",tstr.c_str());
                    ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());
                }else{
                    tstr = "------------------------------ Soundstream INPUT OFF, no input audio device available";
                    ofLog(OF_LOG_ERROR,"%s",tstr.c_str());
                }

                if(isOutputDeviceAvailable){
                    tstr = "------------------- Soundstream OUTPUT Started on";
                    ofLog(OF_LOG_NOTICE,"%s",tstr.c_str());
                    ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());

                }else{
                    tstr = "------------------------------ Soundstream OUTPUT OFF, no output audio device available";
                    ofLog(OF_LOG_ERROR,"%s",tstr.c_str());
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            XML.popTag();
        }

        bPopulatingObjectsMap   = true;

        int totalObjects = XML.getNumTags("object");

        if(totalObjects > 0){
            // load all the patch objects ( all the non GL sharing context )
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    string objname = XML.getValue("name","");
                    bool loaded = false;

                    if(isObjectInLibrary(objname)){
                        shared_ptr<PatchObject> tempObj = selectObject(objname);
                        if(tempObj != nullptr && !tempObj->getIsSharedContextObject()){
                            loaded = tempObj->loadConfig(mainWindow,*engine,i,patchFile);
                            if(loaded){
                                tempObj->setPatchfile(currentPatchFile);
                                tempObj->setIsRetina(isRetina,scaleFactor);
                                string objSubpatch = XML.getValue("subpatch","");
                                if(objSubpatch == "") objSubpatch = "root"; // retro compatibility for pre-subpatch patches
                                tempObj->setSubpatch(objSubpatch);
                                if (subpatchesMap.find(objSubpatch) == subpatchesMap.end()) {
                                    vector<SubpatchConnection> _sp;
                                    subpatchesMap[objSubpatch] = _sp;
                                }
                                ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
                                ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
                                ofAddListener(tempObj->reconnectOutletsEvent ,this,&ofxVisualProgramming::reconnectObjectOutlets);
                                ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);
                                // Insert the new object into the map
                                patchObjects[tempObj->getId()] = tempObj;
                                actualObjectID = tempObj->getId();
                                lastAddedObjectID = tempObj->getId();
                                nodeCanvas.addNodeToMap(tempObj->getId(),tempObj->getName());
                                // if wireless object, add reference to subpatch data map
                                if(objname == "sender"){
                                    SubpatchConnection _t;
                                    _t.objID = tempObj->getId();
                                    _t.inOut = 1;
                                    subpatchesMap[objSubpatch].push_back(_t);
                                }else if(objname == "receiver"){
                                    SubpatchConnection _t;
                                    _t.objID = tempObj->getId();
                                    _t.inOut = 0;
                                    subpatchesMap[objSubpatch].push_back(_t);
                                }

#ifdef OFXVP_DEBUG
                                std::cout << "Loading "<< tempObj->getName() << std::endl;
#endif

                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                            }
                        }
                    }
                    XML.popTag();
                }
            }

            // Load Links ( of all the non GL sharing context )
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    string objname = XML.getValue("name","");
                    if(isObjectInLibrary(objname)){
                        shared_ptr<PatchObject> tempObj = selectObject(objname);
                        if(tempObj != nullptr && !tempObj->getIsSharedContextObject()){
                            int fromID = XML.getValue("id", -1);
                            if (XML.pushTag("outlets")){
                                int totalOutlets = XML.getNumTags("link");
                                for(int j=0;j<totalOutlets;j++){
                                    if (XML.pushTag("link",j)){
                                        int linkType = XML.getValue("type", 0);
                                        if(linkType != VP_LINK_AUDIO){
                                            int totalLinks = XML.getNumTags("to");
                                            for(int z=0;z<totalLinks;z++){
                                                if(XML.pushTag("to",z)){
                                                    int toObjectID = XML.getValue("id", 0);
                                                    int toInletID = XML.getValue("inlet", 0);

                                                    // fix loading patches with non-existent objects (older OFXVP versions)
                                                    if(isObjectIDInPatchMap(toObjectID)){
                                                        if(connect(fromID,j,toObjectID,toInletID,linkType)){
                                                            //ofLog(OF_LOG_NOTICE,"Connected object %s, outlet %i TO object %s, inlet %i",patchObjects[fromID]->getName().c_str(),z,patchObjects[toObjectID]->getName().c_str(),toInletID);
                                                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                                        }
                                                    }

                                                    XML.popTag();
                                                }
                                            }
                                        }
                                        XML.popTag();
                                    }
                                }

                                XML.popTag();
                            }

                        }
                    }
                    XML.popTag();
                }
            }

        }

        bPopulatingObjectsMap   = false;

        activateDSP();

        if(totalObjects > 0){
            // activate all audio PDSP connection
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    string objname = XML.getValue("name","");
                    if(isObjectInLibrary(objname)){
                        shared_ptr<PatchObject> tempObj = selectObject(objname);
                        if(tempObj != nullptr && !tempObj->getIsSharedContextObject()){
                            int fromID = XML.getValue("id", -1);
                            if (XML.pushTag("outlets")){
                                int totalOutlets = XML.getNumTags("link");
                                for(int j=0;j<totalOutlets;j++){
                                    if (XML.pushTag("link",j)){
                                        int linkType = XML.getValue("type", 0);
                                        if(linkType == VP_LINK_AUDIO){
                                            int totalLinks = XML.getNumTags("to");
                                            for(int z=0;z<totalLinks;z++){
                                                if(XML.pushTag("to",z)){
                                                    int toObjectID = XML.getValue("id", 0);
                                                    int toInletID = XML.getValue("inlet", 0);

                                                    if(isObjectIDInPatchMap(toObjectID)){
                                                        if(connect(fromID,j,toObjectID,toInletID,linkType)){
                                                            //ofLog(OF_LOG_NOTICE,"Connected object %s, outlet %i TO object %s, inlet %i",patchObjects[fromID]->getName().c_str(),z,patchObjects[toObjectID]->getName().c_str(),toInletID);
                                                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                                        }
                                                    }

                                                    XML.popTag();
                                                }
                                            }
                                        }
                                        XML.popTag();
                                    }
                                }

                                XML.popTag();
                            }
                        }
                    }
                    XML.popTag();
                }
            }
        }

    }

    bLoadingNewPatch = false;

    deferredLoadTime = ofGetElapsedTimeMillis();
    deferredLoad = true;

}

//--------------------------------------------------------------
void ofxVisualProgramming::loadPatchSharedContextObjects(){

#ifdef OFXVP_DEBUG
    std::cout << "Loading GL sharing context objects" << std::endl;
#endif

    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(currentPatchFile)){
#else
    if (XML.load(currentPatchFile)){
#endif

        int totalObjects = XML.getNumTags("object");

        if(totalObjects > 0){

            // load the sharing context objects ( needs to be loaded at last )
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    string objname = XML.getValue("name","");
                    bool loaded = false;

                    if(isObjectInLibrary(objname)){
                        shared_ptr<PatchObject> tempObj = selectObject(objname);
                        if(tempObj != nullptr && tempObj->getIsSharedContextObject()){
                            loaded = tempObj->loadConfig(mainWindow,*engine,i,currentPatchFile);
                            if(loaded){
                                tempObj->setPatchfile(currentPatchFile);
                                tempObj->setIsRetina(isRetina,scaleFactor);
                                string objSubpatch = XML.getValue("subpatch","");
                                if(objSubpatch == "") objSubpatch = "root"; // retro compatibility for pre-subpatch patches
                                tempObj->setSubpatch(objSubpatch);
                                if (subpatchesMap.find(objSubpatch) == subpatchesMap.end()) {
                                    vector<SubpatchConnection> _sp;
                                    subpatchesMap[objSubpatch] = _sp;
                                }
                                ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
                                ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
                                ofAddListener(tempObj->reconnectOutletsEvent ,this,&ofxVisualProgramming::reconnectObjectOutlets);
                                ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);
                                // Insert the new patch into the map
                                patchObjects[tempObj->getId()] = tempObj;
                                actualObjectID = tempObj->getId();
                                lastAddedObjectID = tempObj->getId();
                                nodeCanvas.addNodeToMap(tempObj->getId(),tempObj->getName());
#ifdef OFXVP_DEBUG
                                std::cout << "Loading "<< tempObj->getName() << std::endl;
#endif

                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                            }
                        }
                    }
                    XML.popTag();
                }
            }

            // Load Links to shared context objects only
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    string objname = XML.getValue("name","");
                    if(isObjectInLibrary(objname)){
                        int fromID = XML.getValue("id", -1);
                        if (XML.pushTag("outlets")){
                            int totalOutlets = XML.getNumTags("link");
                            for(int j=0;j<totalOutlets;j++){
                                if (XML.pushTag("link",j)){
                                    int linkType = XML.getValue("type", 0);
                                    int totalLinks = XML.getNumTags("to");
                                    for(int z=0;z<totalLinks;z++){
                                        if(XML.pushTag("to",z)){
                                            int toObjectID = XML.getValue("id", 0);
                                            int toInletID = XML.getValue("inlet", 0);
                                            string toObjName = getObjectNameFromID(toObjectID);

                                            if(toObjName != ""){
                                                shared_ptr<PatchObject> _tempToObj = selectObject(toObjName);
                                                if(_tempToObj != nullptr && _tempToObj->getIsSharedContextObject()){
                                                    // fix loading patches with non-existent objects (older OFXVP versions)
                                                    if(isObjectIDInPatchMap(toObjectID)){
                                                        if(connect(fromID,j,toObjectID,toInletID,linkType)){
                                                            //ofLog(OF_LOG_NOTICE,"Connected object %s, outlet %i TO object %s, inlet %i",patchObjects[fromID]->getName().c_str(),z,patchObjects[toObjectID]->getName().c_str(),toInletID);
                                                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                                        }
                                                    }
                                                }
                                            }

                                            XML.popTag();
                                        }
                                    }
                                    XML.popTag();
                                }
                            }

                            XML.popTag();
                        }
                    }
                    XML.popTag();
                }
            }

        }

        //nodeCanvas.debugNodeMap();

    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::reloadPatch(){
    bLoadingNewPatch = true;

    // clear previous patch
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            it->second->removeObjectContent();
        }
    }

    patchObjects.clear();

    // clear subpatch navigation data
    subpatchesMap.clear();
    currentSubpatch         = "root";
    newSubpatchName         = "";
    vector<SubpatchConnection> rootBranch;
    subpatchesMap[currentSubpatch] = rootBranch;

    // load new patch
    loadPatch(currentPatchFile);
}

//--------------------------------------------------------------
void ofxVisualProgramming::savePatchAs(string patchFile){

    // Mosaic patch folder structure:
    //
    // > PATCH_NAME/
    //      > DATA/
    //        PATCH_NAME.xml


    // sanitize filename
    ofFile tempPF(patchFile);
    string preSanitizeFN = tempPF.getFileName();
    sanitizeFilename(preSanitizeFN);

    string sanitizedPatchFile = tempPF.getEnclosingDirectory()+preSanitizeFN;
    //ofLog(OF_LOG_NOTICE,"%s",patchFile.c_str());
    //ofLog(OF_LOG_NOTICE,"%s",sanitizedPatchFile.c_str());

    // copy patch file & patch data folder
    ofFile tempFile(sanitizedPatchFile);
    string tempFileName = tempFile.getFileName();
    string finalTempFileName = tempFile.getFileName().substr(0,tempFileName.find_last_of('.'));

    string newFileName = checkFileExtension(sanitizedPatchFile, ofToUpper(tempFile.getExtension()), "XML");
    ofFile fileToRead(currentPatchFile);
    ofDirectory dataFolderOrigin;
    dataFolderOrigin.listDir(currentPatchFolderPath+"data/");

    ofFile newPatchFile(newFileName);

    currentPatchFile = newPatchFile.getEnclosingDirectory()+finalTempFileName+"/"+newPatchFile.getFileName();
    ofFile temp(currentPatchFile);
    currentPatchFolderPath  = temp.getEnclosingDirectory();

    ofFile::copyFromTo(fileToRead.getAbsolutePath(),currentPatchFile,true,true);

    std::filesystem::path tp = currentPatchFolderPath+"/data/";

    dataFolderOrigin.copyTo(tp,true,true);

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second != nullptr){
            it->second->setPatchfile(currentPatchFile);
        }
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::setPatchVariable(string var, int value){
    ofxXmlSettings XML;

#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
    if (XML.loadFile(currentPatchFile)){
#else
    if (XML.load(currentPatchFile)){
#endif
        if (XML.pushTag("settings")){
            XML.setValue(var,value);
#if OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 12
            XML.saveFile();
#else
            XML.save();
#endif
            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioInDevice(int ind){

    int index = audioDevicesID_IN.at(ind);

    audioGUIINChannels = audioDevices[index].inputChannels;

    setPatchVariable("audio_in_device",index);
    setPatchVariable("input_channels",audioGUIINChannels);

}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioOutDevice(int ind){

    int index = audioDevicesID_OUT.at(ind);

    audioGUIOUTChannels = audioDevices[index].outputChannels;

    setPatchVariable("audio_out_device",index);
    setPatchVariable("output_channels",audioGUIOUTChannels);

}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioDevices(int ind, int outd){
    int indexIN = audioDevicesID_IN.at(ind);
    int indexOUT = audioDevicesID_OUT.at(outd);

    audioGUIINChannels = audioDevices[indexIN].inputChannels;
    audioGUIOUTChannels = audioDevices[indexOUT].outputChannels;

    setPatchVariable("audio_in_device",indexIN);
    setPatchVariable("sample_rate_in",audioSampleRate);
    setPatchVariable("input_channels",audioGUIINChannels);
    setPatchVariable("audio_out_device",indexOUT);
    setPatchVariable("sample_rate_out",audioSampleRate);
    setPatchVariable("output_channels",audioGUIOUTChannels);

    reloadPatch();
}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioSampleRate(int sr){
    audioGUISRIndex = sr;
    audioSampleRate = ofToInt(audioDevicesSR[audioGUISRIndex]);

    setPatchVariable("sample_rate_in",audioSampleRate);
    setPatchVariable("sample_rate_out",audioSampleRate);
}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioBufferSize(int bs){
    audioGUIBSIndex = bs;
    audioBufferSize = ofToInt(audioDevicesBS[audioGUIBSIndex]);

    setPatchVariable("buffer_size",audioBufferSize);
}

//--------------------------------------------------------------
void ofxVisualProgramming::activateDSP(){

    string tstr;

    engine->setChannels(0,0);

    //ofLog(OF_LOG_NOTICE,"%i IN CH - %i OUT CH",audioGUIINChannels, audioGUIOUTChannels);

    if(audioGUIINChannels > 0 || audioGUIOUTChannels > 0){
        engine->setChannels(audioGUIINChannels, audioGUIOUTChannels);
        this->setChannels(audioGUIINChannels,0);

        for(int in=0;in<audioGUIINChannels;in++){
            engine->audio_in(in) >> this->in(in);
        }
        this->out_silent() >> engine->blackhole();

        if(isInputDeviceAvailable){
            engine->setInputDeviceID(audioDevices[audioINDev].deviceID);
            tstr = "------------------- Soundstream INPUT Started on";
            ofLog(OF_LOG_NOTICE,"%s",tstr.c_str());
            ofLog(OF_LOG_NOTICE,"Audio device: %s, with %i INPUT channels",audioDevices[audioINDev].name.c_str(),audioGUIINChannels);
        }else{
            tstr = "------------------------------ Soundstream INPUT OFF, no input audio device available";
            ofLog(OF_LOG_ERROR,"%s",tstr.c_str());
        }

        if(isOutputDeviceAvailable){
            engine->setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
            tstr = "------------------- Soundstream OUTPUT Started on";
            ofLog(OF_LOG_NOTICE,"%s",tstr.c_str());
            ofLog(OF_LOG_NOTICE,"Audio device: %s, with %i OUTPUT channels",audioDevices[audioOUTDev].name.c_str(),audioGUIOUTChannels);
        }else{
            tstr = "------------------------------ Soundstream OUTPUT OFF, no output audio device available";
            ofLog(OF_LOG_ERROR,"%s",tstr.c_str());
        }

        engine->setup(audioSampleRate, audioBufferSize, audioNumBuffers);
        engine->sequencer.setTempo(bpm);

        bool found = weAlreadyHaveObject("audio device");

        if(!found){
            addObject("audio device",ofVec2f((ofGetScreenWidth()/2 - OBJECT_WIDTH/2*scaleFactor)/scaleFactor,(ofGetScreenHeight()/2 + OBJECT_HEIGHT*scaleFactor)/scaleFactor));
        }
        resetSystemObjects();

        setPatchVariable("dsp",1);
        dspON = true;
    }else{
        deactivateDSP();
        tstr = "The selected audio devices couldn't be compatible or couldn't be properly installed in your system!";
        ofLog(OF_LOG_ERROR,"%s",tstr.c_str());
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::deactivateDSP(){
    setPatchVariable("dsp",0);
    dspON = false;
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetCanvas(){
    nodeCanvas.resetCanvas();
}
