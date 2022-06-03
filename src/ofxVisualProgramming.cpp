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
    clearingObjectsMap      = false;

    livePatchingObiID       = -1;

    currentPatchFile        = "empty_patch.xml";
    currentPatchFolderPath  = ofToDataPath("temp/");

    currentSubpatch         = "root";

    vector<string> rootBranch;
    subpatchesTree[currentSubpatch] = rootBranch;

    resetTime               = ofGetElapsedTimeMillis();
    wait                    = 1000;

    alphabet                = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXZY";
    newFileCounter          = 0;

    audioSampleRate         = 44100;
    audioGUIINIndex         = -1;
    audioGUIOUTIndex        = -1;
    bpm                     = 120;
    dspON                   = false;
    audioINDev              = 0;
    audioOUTDev             = 0;

    profilerActive          = false;
    inspectorActive         = false;

    inited                  = false;

}

//--------------------------------------------------------------
ofxVisualProgramming::~ofxVisualProgramming(){
    delete font;
    font = nullptr;
}

//--------------------------------------------------------------
void ofxVisualProgramming::setRetina(bool retina){
    isRetina = retina;

    if(isRetina){
        scaleFactor = 2;
        fontSize    = 16;
        canvas.setScale(2);
    }else{
        scaleFactor = 1;
        fontSize    = 8;
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::setup(ofxImGui::Gui* _guiRef, string release){

    // Load resources
    font->load(MAIN_FONT,fontSize);

    // Initialise GUI
    if( _guiRef == nullptr ){
        ofxVPGui = new ofxImGui::Gui();
        ofxVPGui->setup();//nullptr, true, ImGuiConfigFlags_NavEnableSetMousePos);
        ofLogNotice("ofxVP","Automatically setting up a new ImGui instance. If your app has its own one, pass it's reference in setup();");
    }
    else {
        ofxVPGui = _guiRef;
        // Dummy call to IO which will crash if _guiRef is not initialised.
        ImGui::GetIO();
        // Ensure ImGui gets loaded correctly
        /*if(ImGui::GetCurrentContext()==nullptr || !ImGui::GetCurrentContext()->Initialized){
            ofxVPGui->setup();
        }
        // Manually force shared mode
        else {
            ofxVPGui->setSharedMode(true);
        }*/

        //ofLogError("ofxVP") << "Setting up ImGui from reference instance." << (ImGui::GetCurrentContext()->Initialized?'1':'0');
    }

    nodeCanvas.setContext(ImGui::GetCurrentContext());

    nodeCanvas.setRetina(isRetina);
    profiler.setIsRetina(isRetina);

    // Set pan-zoom canvas
    canvas.disableMouseInput();
    canvas.setbMouseInputEnabled(true);
    canvas.toggleOfCam();
    easyCam.enableOrtho();

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
    pluginsDir.listDir(ofToDataPath(PLUGINS_FOLDER));
    pluginsDir.sort();

    // load all plugins
    for(unsigned int i = 0; i < pluginsDir.size(); i++){
        ofLog(OF_LOG_NOTICE,"Loading plugin: %s",pluginsDir.getFile(i).getFileName().c_str());
        plugins_kernel.load_plugin(pluginsDir.getFile(i).getAbsolutePath().c_str());
    }

    // Create new empty file patch
    newPatch(release);

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
    clearObjectsMap();

    // update patch objects
    if(!bLoadingNewPatch && !patchObjects.empty()){

        std::lock_guard<std::mutex> lck(vp_mutex);

        // left to right computing order
        leftToRightIndexOrder.clear();
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            leftToRightIndexOrder.push_back(make_pair(static_cast<int>(floor(it->second->getPos().x)),it->second->getId()));
        }
        // sort the vector by it's pair first value (object X position)
        sort(leftToRightIndexOrder.begin(),leftToRightIndexOrder.end());

        ImGuiEx::ProfilerTask *pt = new ImGuiEx::ProfilerTask[leftToRightIndexOrder.size()];

        for(unsigned int i=0;i<leftToRightIndexOrder.size();i++){ 
            if(patchObjects[leftToRightIndexOrder[i].second]->subpatchName == currentSubpatch){

                string tmpon = patchObjects[leftToRightIndexOrder[i].second]->getName()+ofToString(patchObjects[leftToRightIndexOrder[i].second]->getId())+"_update";

                pt[i].color = profiler.cpuGraph.colors[static_cast<unsigned int>(i%16)];
                pt[i].startTime = ofGetElapsedTimef();
                pt[i].name = tmpon;

                patchObjects[leftToRightIndexOrder[i].second]->update(patchObjects,*engine);

                pt[i].endTime = ofGetElapsedTimef();

                // update scripts objects files map
                ofFile tempsofp(patchObjects[leftToRightIndexOrder[i].second]->getFilepath());
                string fileExt = ofToUpper(tempsofp.getExtension());
                if(fileExt == "LUA" || fileExt == "PY" || fileExt == "SH"){
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
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::updateCanvasViewport(){
    canvasViewport.set(0,20,ofGetWindowWidth(),ofGetWindowHeight()-20);
}

//--------------------------------------------------------------
void ofxVisualProgramming::draw(){

    #ifdef MOSAIC_ENABLE_PROFILING
    ZoneScopedN("ofxVisualProgramming::Draw()");
    #endif

    if(bLoadingNewPatch) return;

    // LIVE PATCHING SESSION
    drawLivePatchingSession();

    ofPushView();
    ofPushStyle();
    ofPushMatrix();

    // Init canvas
    nodeCanvas.SetTransform( ImVec2(canvas.getTranslation().x,canvas.getTranslation().y), canvas.getScale() );//canvas.getScrollPosition(), canvas.getScale(true) );

    // DEBUG
    if(OFXVP_DEBUG){
        ofSetColor(0,255,255,236);
        ofNoFill();
        ofDrawRectangle( canvasViewport.x, canvasViewport.y, canvasViewport.width, canvasViewport.height);
        ofFill();
        ofDrawCircle(ofGetMouseX(), ofGetMouseY(), 6);
    }


    canvas.begin(canvasViewport);

    ofEnableAlphaBlending();
    ofSetCurveResolution(50);
    ofSetColor(255);
    ofSetLineWidth(1);

    livePatchingObiID = -1;

    // Set GUI context

    // Already finished drawing to frame...
    // Please call ofxVP::draw() before rendering imgui.
    // Maybe you need to call it within your imgui draw() scope !
    if( (ImGui::GetDrawData()!=NULL) )
        ofLogError("ofxVisualProgramming::draw", "Warning, you're calling draw after rendering ImGui. Please call before.");

    ofxVPGui->begin();

    // DEBUG
    if(OFXVP_DEBUG){
        ImGui::ShowMetricsWindow();
    }

    // PROFILER
    if(profilerActive){
        profiler.Render(&profilerActive);
    }

    // Try to begin ImGui Canvas.
    // Should always return true, except if window is minimised or somehow not rendered.
    ImGui::SetNextWindowPos(ImVec2(canvasViewport.getTopLeft().x,canvasViewport.getTopLeft().y), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2(canvasViewport.width, canvasViewport.height), ImGuiCond_Always );
    bool isCanvasVisible = nodeCanvas.Begin("ofxVPNodeCanvas" );

    // Render objects.
    if(!bLoadingNewPatch && !patchObjects.empty()){
        ImGuiEx::ProfilerTask *pt = new ImGuiEx::ProfilerTask[leftToRightIndexOrder.size()];
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

        // INSPECTOR
        if(inspectorActive){
            if(isCanvasVisible){
                drawInspector();
            }
        }
    }

    // Close canvas
    nodeCanvas.End();

    // We're done drawing to IMGUI
    ofxVPGui->end();

    canvas.end();

    // Draw Bottom Bar
    ofSetColor(0,0,0,60);
    ofDrawRectangle(0,ofGetHeight() - (20*scaleFactor),ofGetWidth(),(20*scaleFactor));

    ofDisableAlphaBlending();

    ofPopMatrix();
    ofPopStyle();
    ofPopView();

    // Draw Subpatch Navigation
    drawSubpatchNavigation();

    // Graphical Context
    canvas.update();


}

//--------------------------------------------------------------
void ofxVisualProgramming::drawInspector(){

    ImGui::SetNextWindowSize(ImVec2(ofGetWindowWidth()/4,ofGetWindowHeight()/2), ImGuiCond_Appearing );
    //ImGui::SetNextWindowPos(ImVec2(ofGetWindowWidth()-200,26*scaleFactor), ImGuiCond_Appearing);

    ImGui::Begin("Inspector", &inspectorActive, ImGuiWindowFlags_NoCollapse);

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
    if(weAlreadyHaveObject("live patching") && livePatchingObiID != -1 && currentSubpatch == "root"){
        if(patchObjects[livePatchingObiID]->inletsConnected[0] && static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->isAllocated()){
            float lpDrawW, lpDrawH, lpPosX, lpPosY;
            if(static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth() >= static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight()){   // horizontal texture
                lpDrawW           = ofGetWidth();
                lpDrawH           = (ofGetWidth()/static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth())*static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight();
                lpPosX            = 0;
                lpPosY            = (ofGetHeight()-lpDrawH)/2.0f;
            }else{ // vertical texture
                lpDrawW           = (static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getWidth()*ofGetHeight())/static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->getHeight();
                lpDrawH           = ofGetHeight();
                lpPosX            = (ofGetWidth()-lpDrawW)/2.0f;
                lpPosY            = 0;
            }
            ofSetColor(255,*(float *)&patchObjects[livePatchingObiID]->_outletParams[0]);
            static_cast<ofTexture *>(patchObjects[livePatchingObiID]->_inletParams[0])->draw(lpPosX,lpPosY,lpDrawW,lpDrawH);
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::drawSubpatchNavigation(){
    if(currentSubpatch != "root"){
        // TODO
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetTempFolder(){
    ofDirectory dir;
    dir.removeDirectory(ofToDataPath("temp/",true),true,true);
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
                    if(fileExt == "PY" || fileExt == "SH" || fileExt == "FRAG"){
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
void ofxVisualProgramming::exit(){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->removeObjectContent();
    }

    if(dspON){
        deactivateDSP();
    }

    cleanPatchDataFolder();

    resetTempFolder();
}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseMoved(ofMouseEventArgs &e){

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseDragged(ofMouseEventArgs &e){

    if(ImGui::IsAnyItemActive() || nodeCanvas.isAnyNodeHovered() || ImGui::IsAnyItemHovered() )// || ImGui::IsAnyWindowHovered())
        return;

    canvas.mouseDragged(e);

}

//--------------------------------------------------------------
void ofxVisualProgramming::mousePressed(ofMouseEventArgs &e){

    if(ImGui::IsAnyItemActive() || nodeCanvas.isAnyNodeHovered() || ImGui::IsAnyItemHovered() )
        return;

    canvas.mousePressed(e);

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseReleased(ofMouseEventArgs &e){

    if(ImGui::IsAnyItemActive() || nodeCanvas.isAnyNodeHovered() || ImGui::IsAnyItemHovered() )
        return;

    canvas.mouseReleased(e);

}

//--------------------------------------------------------------
void ofxVisualProgramming::mouseScrolled(ofMouseEventArgs &e){

    if(ImGui::IsAnyItemActive() || nodeCanvas.isAnyNodeHovered() || ImGui::IsAnyItemHovered())// | ImGui::IsAnyWindowHovered() )
        return;

    canvas.mouseScrolled(e);
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

        if(audioDevices[audioINDev].inputChannels > 0){
            inputBuffer.copyFrom(input, bufferSize, nChannels, audioSampleRate);

            // compute audio input
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                it->second->audioIn(inputBuffer);
            }

            lastInputBuffer = inputBuffer;
        }
        if(audioDevices[audioOUTDev].outputChannels > 0){
            // compute audio output
            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
                it->second->audioOut(emptyBuffer);
            }
        }

    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::addObject(string name,ofVec2f pos){

    // check if object exists
    bool exists = false;
    for(ofxVPObjects::factory::objectRegistry::iterator it = ofxVPObjects::factory::getObjectRegistry().begin(); it != ofxVPObjects::factory::getObjectRegistry().end(); it++ ){
        if(it->first == name){
            exists = true;
            break;
        }
    }
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

    tempObj->newObject();
    tempObj->setPatchfile(currentPatchFile);
    tempObj->setup(mainWindow);
    tempObj->setupDSP(*engine);
    tempObj->move(static_cast<int>(pos.x-(OBJECT_STANDARD_WIDTH/2*scaleFactor)),static_cast<int>(pos.y-(OBJECT_STANDARD_HEIGHT/2*scaleFactor)));
    tempObj->setIsRetina(isRetina);
    tempObj->setSubpatch(currentSubpatch);
    ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
    ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
    ofAddListener(tempObj->reconnectOutletsEvent ,this,&ofxVisualProgramming::reconnectObjectOutlets);
    ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);

    actualObjectID++;

    bool saved = tempObj->saveConfig(false);

    if(saved){
        patchObjects[tempObj->getId()] = tempObj;
        lastAddedObjectID = tempObj->getId();
        nodeCanvas.setActiveNode(lastAddedObjectID);
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
        if (XML.loadFile(currentPatchFile)){

            for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

            XML.saveFile();

        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetObject(int id){
    if ((id != -1) && (patchObjects[id] != nullptr)){
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

//--------------------------------------------------------------
void ofxVisualProgramming::reconnectObjectOutlets(int &id){
    if ((id != -1) && (patchObjects[id] != nullptr)){

        ofxXmlSettings XML;
        if (XML.loadFile(currentPatchFile)){
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

                                            if(connect(fromID,j,toObjectID,toInletID,linkType)){
                                                //ofLog(OF_LOG_NOTICE,"Connected object %s, outlet %i TO object %s, inlet %i",patchObjects[fromID]->getName().c_str(),z,patchObjects[toObjectID]->getName().c_str(),toInletID);
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
void ofxVisualProgramming::deleteObject(int id){
    resetTime = ofGetElapsedTimeMillis();

    if ((id != -1) && (patchObjects[id] != nullptr)){

        int targetID = id;
        bool found = false;
        ofxXmlSettings XML;
        if (XML.loadFile(currentPatchFile)){
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
                XML.saveFile();
            }
        }

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

//--------------------------------------------------------------
void ofxVisualProgramming::clearObjectsMap(){
    if(ofGetElapsedTimeMillis()-resetTime > wait){
        resetTime = ofGetElapsedTimeMillis();
        eraseIndexes.clear();
        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
            if(it->second->getWillErase()){
                eraseIndexes.push_back(it->first);
            }
        }
        for(int x=0;x<static_cast<int>(eraseIndexes.size());x++){

            if(!clearingObjectsMap){
                for(int p=0;p<static_cast<int>(patchObjects.at(eraseIndexes.at(x))->outPut.size());p++){
                    patchObjects[patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toObjectID]->inletsConnected.at(patchObjects.at(eraseIndexes.at(x))->outPut.at(p)->toInletID) = false;
                }
            }

            // remove scripts objects filepath reference from scripts objects files map
            ofFile tempsofp(patchObjects.at(eraseIndexes.at(x))->getFilepath());
            string fileExt = ofToUpper(tempsofp.getExtension());
            if(fileExt == "LUA" || fileExt == "PY" || fileExt == "SH"){
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
string ofxVisualProgramming::getSubpatchParent(string subpatchName){
    for(map<string,vector<string>>::iterator it = subpatchesTree.begin(); it != subpatchesTree.end(); it++ ){
        for(unsigned int s=0;s<it->second.size();s++){
            if(it->second.at(s) == subpatchName){
                return it->first;
            }
        }
    }
    // return root if subpatch searched do not exists
    return "root";
}

//--------------------------------------------------------------
void ofxVisualProgramming::removeObject(int &id){
    resetTime = ofGetElapsedTimeMillis();

    if ( (id != -1) && (patchObjects[id] != nullptr) && (patchObjects[id]->getName() != "audio device") ){

        int targetID = id;
        bool found = false;
        ofxXmlSettings XML;
        if (XML.loadFile(currentPatchFile)){
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
                XML.saveFile();
            }
        }

        for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
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

//--------------------------------------------------------------
void ofxVisualProgramming::duplicateObject(int &id){
    // disable duplicate for hardware&system related objects
    if(patchObjects[id]->getName() != "audio device" && patchObjects[id]->getName() != "video grabber" && patchObjects[id]->getName() != "kinect grabber" && patchObjects[id]->getName() != "live patching" && patchObjects[id]->getName() != "projection mapping"){
        //ofVec2f newPos = ofVec2f(patchObjects[id]->getPos().x + patchObjects[id]->getObjectWidth(),patchObjects[id]->getPos().y);
        addObject(patchObjects[id]->getName(),patchObjects[id]->getPos());
    }else{
        ofLog(OF_LOG_NOTICE,"'%s' is one of the Mosaic objects that can't (for now) be duplicated due to hardware/system related issues.",patchObjects[id]->getName().c_str());
    }
}

//--------------------------------------------------------------
bool ofxVisualProgramming::connect(int fromID, int fromOutlet, int toID,int toInlet, int linkType){
    bool connected = false;

    if((fromID != -1) && (patchObjects[fromID] != nullptr) && (toID != -1) && (patchObjects[toID] != nullptr) && (patchObjects[fromID]->getOutletType(fromOutlet) == patchObjects[toID]->getInletType(toInlet)) && !patchObjects[toID]->inletsConnected[toInlet]){

        //cout << "Mosaic :: "<< "Connect object " << patchObjects[fromID]->getName().c_str() << ":" << ofToString(fromID) << " to object " << patchObjects[toID]->getName().c_str() << ":" << ofToString(toID) << endl;

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
            }else if(patchObjects[fromID]->getName() == "audio device" && patchObjects[toID]->getIsPDSPPatchableObject()){
                patchObjects[fromID]->pdspOut[fromOutlet] >> patchObjects[toID]->pdspIn[toInlet];
            }
        }

        checkSpecialConnection(fromID,toID,linkType);

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
        if(it->second->getIsSystemObject()){
            it->second->resetSystemObject();
            resetObject(it->second->getId());
            if(it->second->getIsAudioOUTObject()){
                it->second->setupAudioOutObjectContent(*engine);
            }
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::resetSpecificSystemObjects(string name){
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->getIsSystemObject() && it->second->getName() == name){
            it->second->resetSystemObject();
            resetObject(it->second->getId());
            if(it->second->getIsAudioOUTObject()){
                it->second->setupAudioOutObjectContent(*engine);
            }
        }
    }
}

//--------------------------------------------------------------
bool ofxVisualProgramming::weAlreadyHaveObject(string name){

    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        if(it->second->getName() == name){
            return true;
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
    if (XML.loadFile(fileToRead.getAbsolutePath())){
        XML.setValue("release",release);
        XML.saveFile();
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
    dataFolderOrigin.listDir(fileToRead.getEnclosingDirectory()+"/data/");
    std::filesystem::path tp = ofToDataPath("temp/data/",true);
    dataFolderOrigin.copyTo(tp,true,true);

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
        if(it->second->getName() != "audio device"){
            it->second->setWillErase(true);
        }else{
            for(int in=0;in<it->second->getNumInlets();in++){
                it->second->inletsConnected[in] = false;
                it->second->pdspIn[in].disconnectIn();
            }

            it->second->outPut.clear();

            //glm::vec3 temp = canvas.screenToWorld(glm::vec3(ofGetWindowWidth()/2,ofGetWindowHeight()/2 + 100,0));
            //it->second->move(temp.x,temp.y);
            it->second->setWillErase(true);

        }
    }
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

    ofxXmlSettings XML;

    if (XML.loadFile(patchFile)){

        // Load main settings
        if (XML.pushTag("settings")){
            // Setup projector dimension
            output_width = XML.getValue("output_width",0);
            output_height = XML.getValue("output_height",0);

            // setup audio
            dspON = XML.getValue("dsp",0);
            audioINDev = XML.getValue("audio_in_device",0);
            audioOUTDev = XML.getValue("audio_out_device",0);
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
            audioDevices = soundStreamIN.getDeviceList();
            audioDevicesStringIN.clear();
            audioDevicesID_IN.clear();
            audioDevicesStringOUT.clear();
            audioDevicesID_OUT.clear();
            ofLog(OF_LOG_NOTICE,"------------------- AUDIO DEVICES");
            for(size_t i=0;i<audioDevices.size();i++){
                if(audioDevices[i].inputChannels > 0){
                    audioDevicesStringIN.push_back("  "+audioDevices[i].name);
                    audioDevicesID_IN.push_back(i);
                    //ofLog(OF_LOG_NOTICE,"INPUT Device[%zu]: %s (IN:%i - OUT:%i)",i,audioDevices[i].name.c_str(),audioDevices[i].inputChannels,audioDevices[i].outputChannels);

                }
                if(audioDevices[i].outputChannels > 0){
                    audioDevicesStringOUT.push_back("  "+audioDevices[i].name);
                    audioDevicesID_OUT.push_back(i);
                    //ofLog(OF_LOG_NOTICE,"OUTPUT Device[%zu]: %s (IN:%i - OUT:%i)",i,audioDevices[i].name.c_str(),audioDevices[i].inputChannels,audioDevices[i].outputChannels);
                }
                string tempSR = "";
                for(size_t sr=0;sr<audioDevices[i].sampleRates.size();sr++){
                    if(sr < audioDevices[i].sampleRates.size()-1){
                        tempSR += ofToString(audioDevices[i].sampleRates.at(sr))+", ";
                    }else{
                        tempSR += ofToString(audioDevices[i].sampleRates.at(sr));
                    }
                }
                ofLog(OF_LOG_NOTICE,"Device[%zu]: %s (IN:%i - OUT:%i), Sample Rates: %s",i,audioDevices[i].name.c_str(),audioDevices[i].inputChannels,audioDevices[i].outputChannels,tempSR.c_str());
            }

            // check audio devices index
            audioGUIINIndex         = -1;
            audioGUIOUTIndex        = -1;

            for(size_t i=0;i<audioDevicesID_IN.size();i++){
                if(audioDevicesID_IN.at(i) == audioINDev){
                    audioGUIINIndex = i;
                    break;
                }
            }
            if(audioGUIINIndex == -1){
                audioGUIINIndex = 0;
                audioINDev = audioDevicesID_IN.at(audioGUIINIndex);
            }
            for(size_t i=0;i<audioDevicesID_OUT.size();i++){
                if(audioDevicesID_OUT.at(i) == audioOUTDev){
                    audioGUIOUTIndex = i;
                    break;
                }
            }
            if(audioGUIOUTIndex == -1){
                audioGUIOUTIndex = 0;
                audioOUTDev = audioDevicesID_OUT.at(audioGUIOUTIndex);
            }

            audioSampleRate = audioDevices[audioOUTDev].sampleRates[0];

            if(audioSampleRate < 44100){
                audioSampleRate = 44100;
            }

            XML.setValue("sample_rate_in",audioSampleRate);
            XML.setValue("sample_rate_out",audioSampleRate);
            XML.setValue("input_channels",static_cast<int>(audioDevices[audioINDev].inputChannels));
            XML.setValue("output_channels",static_cast<int>(audioDevices[audioOUTDev].outputChannels));
            XML.saveFile();

            if(dspON){
                engine->setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
                this->setChannels(audioDevices[audioINDev].inputChannels,0);

                for(unsigned int in=0;in<audioDevices[audioINDev].inputChannels;in++){
                    engine->audio_in(in) >> this->in(in);
                }
                this->out_silent() >> engine->blackhole();

                engine->setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
                engine->setInputDeviceID(audioDevices[audioINDev].deviceID);
                engine->setup(audioSampleRate, audioBufferSize, 3);
                engine->sequencer.setTempo(bpm);

                ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream INPUT Started on");
                ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());
                ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream OUTPUT Started on");
                ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            XML.popTag();
        }

        int totalObjects = XML.getNumTags("object");

        if(totalObjects > 0){
            // Load all the patch objects
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
                    string objname = XML.getValue("name","");
                    bool loaded = false;

                    shared_ptr<PatchObject> tempObj = selectObject(objname);
                    if(tempObj != nullptr){
                        loaded = tempObj->loadConfig(mainWindow,*engine,i,patchFile);
                        if(loaded){
                            tempObj->setPatchfile(currentPatchFile);
                            tempObj->setIsRetina(isRetina);
                            ofAddListener(tempObj->removeEvent ,this,&ofxVisualProgramming::removeObject);
                            ofAddListener(tempObj->resetEvent ,this,&ofxVisualProgramming::resetObject);
                            ofAddListener(tempObj->reconnectOutletsEvent ,this,&ofxVisualProgramming::reconnectObjectOutlets);
                            ofAddListener(tempObj->duplicateEvent ,this,&ofxVisualProgramming::duplicateObject);
                            // Insert the new patch into the map
                            patchObjects[tempObj->getId()] = tempObj;
                            actualObjectID = tempObj->getId();
                            lastAddedObjectID = tempObj->getId();

                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                    }
                    XML.popTag();
                }
            }

            // Load Links
            for(int i=0;i<totalObjects;i++){
                if(XML.pushTag("object", i)){
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

                                        if(connect(fromID,j,toObjectID,toInletID,linkType)){
                                            //ofLog(OF_LOG_NOTICE,"Connected object %s, outlet %i TO object %s, inlet %i",patchObjects[fromID]->getName().c_str(),z,patchObjects[toObjectID]->getName().c_str(),toInletID);
                                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
            }
        }

        #if !defined(TARGET_WIN32)
            activateDSP();
        #endif

    }

    bLoadingNewPatch = false;

}

//--------------------------------------------------------------
void ofxVisualProgramming::reloadPatch(){
    bLoadingNewPatch = true;

    // clear previous patch
    for(map<int,shared_ptr<PatchObject>>::iterator it = patchObjects.begin(); it != patchObjects.end(); it++ ){
        it->second->removeObjectContent();
    }

    patchObjects.clear();

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
        it->second->setPatchfile(currentPatchFile);
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::setPatchVariable(string var, int value){
    ofxXmlSettings XML;

    if (XML.loadFile(currentPatchFile)){
        if (XML.pushTag("settings")){
            XML.setValue(var,value);
            XML.saveFile();
            XML.popTag();
        }
    }
}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioInDevice(int ind){

    bool found = false;
    int index = audioDevicesID_IN.at(ind);
    for(size_t i=0;i<audioDevices[index].sampleRates.size();i++){
        if((int)audioDevices[index].sampleRates[i] == audioSampleRate){
            found = true;
        }
    }

    if(!found){
        ofLog(OF_LOG_WARNING,"------------------- INCOMPATIBLE INPUT DEVICE Sample Rate: %i", audioDevices[index].sampleRates[0]);
        ofLog(OF_LOG_WARNING,"------------------- PLEASE SELECT ANOTHER INPUT DEVICE");
        return;
    }else{
        setPatchVariable("audio_in_device",index);
        setPatchVariable("sample_rate_in",audioSampleRate);
        setPatchVariable("input_channels",static_cast<int>(audioDevices[audioINDev].inputChannels));

        reloadPatch();
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::setAudioOutDevice(int ind){

    int index = audioDevicesID_OUT.at(ind);

    audioSampleRate = audioDevices[audioOUTDev].sampleRates[0];
    if(audioSampleRate < 44100){
        audioSampleRate = 44100;
    }

    setPatchVariable("audio_out_device",index);
    setPatchVariable("sample_rate_out",audioSampleRate);
    setPatchVariable("output_channels",static_cast<int>(audioDevices[audioOUTDev].outputChannels));

    reloadPatch();

}

//--------------------------------------------------------------
void ofxVisualProgramming::activateDSP(){

    engine->setChannels(0,0);

    if(audioDevices[audioINDev].inputChannels > 0 && audioDevices[audioOUTDev].outputChannels > 0){
        engine->setChannels(audioDevices[audioINDev].inputChannels, audioDevices[audioOUTDev].outputChannels);
        this->setChannels(audioDevices[audioINDev].inputChannels,0);

        for(unsigned int in=0;in<audioDevices[audioINDev].inputChannels;in++){
            engine->audio_in(in) >> this->in(in);
        }
        this->out_silent() >> engine->blackhole();

        ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream INPUT Started on");
        ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioINDev].name.c_str());
        ofLog(OF_LOG_NOTICE,"[verbose]------------------- Soundstream OUTPUT Started on");
        ofLog(OF_LOG_NOTICE,"Audio device: %s",audioDevices[audioOUTDev].name.c_str());

        engine->setOutputDeviceID(audioDevices[audioOUTDev].deviceID);
        engine->setInputDeviceID(audioDevices[audioINDev].deviceID);
        engine->setup(audioSampleRate, audioBufferSize, 3);
        engine->sequencer.setTempo(bpm);

        bool found = weAlreadyHaveObject("audio device");

        if(!found){
            glm::vec3 temp = canvas.screenToWorld(glm::vec3(ofGetWindowWidth()/2,ofGetWindowHeight()/2 + 100,0));
            addObject("audio device",ofVec2f(temp.x,temp.y));
        }

        setPatchVariable("dsp",1);
        dspON = true;
    }else{
        ofLog(OF_LOG_ERROR,"The selected audio devices couldn't be compatible or couldn't be properly installed in your system!");
    }

}

//--------------------------------------------------------------
void ofxVisualProgramming::deactivateDSP(){
    setPatchVariable("dsp",0);
    dspON = false;
}
