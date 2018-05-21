#include "ofxYolo4Games.h"

using namespace cv;
using namespace ofxCv;

////External and personal uses of Tracker
////--------------------------------------------------------------
//cv::Vec2f ofxYolo4Games::getVelocity(unsigned int i) const {
//	return tracker.getVelocity(i);
//}
//
//unsigned int ofxYolo4Games::getLabel(unsigned int i) const {
//	return tracker.getCurrentLabels()[i];
//}
//
//ofxCv::RectTracker4Games* ofxYolo4Games::getTracker() {
//	return tracker;
//}

//-----------------------------------------------------------
void ofxYolo4Games::setupGui() {

	gui.setup();
	ImGui::SetNextWindowPos(ofVec2f(650, 20), ImGuiSetCond_FirstUseEver);
	auxListDevices = videoGrabber.listDevices();

	//gui.loadFromFile("settings.xml");
}

//-----------------------------------------------------------
void ofxYolo4Games::drawGui() {

	//required to call this at end
	gui.begin();
	ImGui::PushItemWidth(100);

#if defined(USE_SHARECAM_SPOUT)
	ImGui::Checkbox("SPOUT the Camera", &bSpoutCameraActive);
#endif

	ImGui::Checkbox("bVideoPlayer", &bVideoPlayer);
	if (!bVideoPlayer) {
		ImGui::SameLine();
		ImGui::SliderInt("Device Id ", &idVideoGrabber, 0, auxListDevices.size() - 1);
	}

	ImGui::SliderFloat("CropX", &cropSizeX, 0, 1);
	ImGui::SliderFloat("CropY", &cropSizeY, 0, 1);
	if (ImGui::SliderFloat("CropW", &cropSizeW, 0, 1)) {
		bOpticalFlow = false;
		if (flow != nullptr) {
			//Hard Reset
			delete flow;
			flow = nullptr;
			flow = new ofxCv::FlowPyrLK();
			bOpticalFlow = true;
		}
	}
	if (ImGui::SliderFloat("CropH", &cropSizeH, 0, 1)) {
		bOpticalFlow = false;
		if (flow != nullptr) {
			//Hard Reset
			delete flow;
			flow = nullptr;
			flow = new ofxCv::FlowPyrLK();
			bOpticalFlow = true;
		}
	}
		

	//Osc Gui
	ImGui::Checkbox("bSwapX", &bSwapX);
	ImGui::Checkbox("bSwapY", &bSwapY);

	//YOLO GUI DETECTIONS
	ImGui::Checkbox("bDrawYoloInfo", &bDrawYoloInfo);
	ImGui::Text("Filtered Label -> person");
	ImGui::SliderFloat("Probability", &minPercent4PersonDetected, 0.001, 1);
	ImGui::SliderFloat("Max Overlap", &maxOverlap, 0.01, 1);
	//gui.add(maxRectAreaDetection.set("MaxRectAreaDetection", 140, 10, 300));


	//TRACKER GUI options //TODO check how to detect changes values
	ImGui::Checkbox("Draw Tracking", &bDrawTracking);

	//TODO Follow Here!
	if (ImGui::SliderInt("trackerPersistence", &trackerPersistence, 1, 50))
		tracker.setPersistence(trackerPersistence);
	if(ImGui::SliderInt("trackerMaximimDistance", &trackerMaximimDistance, 1, 400))
		tracker.setMaximumDistance(trackerMaximimDistance);

	ImGui::SliderFloat("Dir X Thresh -> ", &variablesGui::getInstance()->directionXTrheshold, 0, 10);
	ImGui::SameLine();
	if (variablesGui::getInstance()->directionX > variablesGui::getInstance()->directionXTrheshold) {
		ImGui::TextColored(ImColor(0, 255, 0), ofToString(variablesGui::getInstance()->directionX, 0).c_str());
	}else 	ImGui::TextColored(ImColor(100, 100, 100), ofToString(variablesGui::getInstance()->directionX, 0).c_str());
	
	//not working
	//Values not saved finally...
	//if (ImGui::SliderFloat("trackerSmoothingRate", &trackerSmoothingRate, 0.01, 1)) {
	//	vector<trackerAnalizer> followers = tracker.getFollowers();
	//	for (int i = 0; i < followers.size(); i++) {
	//		followers[i].setSmoothingRate(trackerSmoothingRate);
	//	}
	//}
	
	ImGui::Checkbox("Send OSC GameBlobAllIn", &bSendAllBlobsIn);
	ImGui::Checkbox("Send OSC GameBlobYoloData", &bSendYoloDataTracking);

	ImGui::PopItemWidth();

	//Network Gui
	ImGui::Checkbox("Draw Network Tracking", &bSociograma);
	if (bSociograma) {
		ImGui::ColorEdit3("Color Network Lines", (float*)&colorSociograma);
		int auxAlphaLines;
		if (ImGui::SliderInt("Alpha Lines##Network", &auxAlphaLines, 0, 255)) {
			colorSociograma.a = auxAlphaLines;
		}
	}

	//OpicalFlow GUi
	if(ImGui::CollapsingHeader("OpticalFlow Options")) {
		ImGui::PushItemWidth(100);
		if (ImGui::Checkbox("bOpticalFlow", &bOpticalFlow)) {
			if (bOpticalFlow == true) {
				if (flow == NULL)flow = new ofxCv::FlowPyrLK;
				flow->resetFeaturesToTrack();
				flow->resetFlow();
				//resetOpticalFlowArea()
				cropedArea.resize(cropSizeW, cropSizeH);
				cout << "Reset manual Flow Vars, trying to fix error" << endl;
				//TODO Check if the size of the Image, is proportional 2 with original image, if not set reset to Dim 1
			}
		}
		ImGui::SliderInt("numKeypointsInside", &numKeypointsInside, 3, 30);
		ImGui::Checkbox("Manual Mouse Sel", &bFlowManualMouseSel);
		ImGui::PopItemWidth();
		ImGui::ColorEdit3("Color lines##Flow", (float*)&colorFeatureLines);
		int auxAlphaLines;
		if (ImGui::SliderInt("Alpha Lines##Flow", &auxAlphaLines, 0, 255)) {
			colorFeatureLines.a = auxAlphaLines;
		}
	}

	//required to call this at end
	
	gui.end();
}

//------------------------------------------------------------
void ofxYolo4Games::setup() 
{
	//////////
	//Darknet Yolo2 Data
	std::string cfgfile = ofToDataPath( "cfg/YOLOv2_608x608.cfg" );	//cfg/yolo-voc-YOLOv2.cfg	//yolo9000.cfg							//GPU (VOC) tiny-yolo-voc.cfg
	std::string weightfile = ofToDataPath( "YOLOv2_608x608_COCO_trainval.weights" );	// yolo-voc-YOLOv2.weights //yolo9000.weights//yolo.weights		//GPU (VOC) tiny-yolo-voc.weights
	std::string namesfile = ofToDataPath( "cfg/coco.names" );	//cfg/voc.names	//cfg/9k.names								//GPU (VOC) cfg/voc.names
    
	darknet.init( cfgfile, weightfile, namesfile);//"" OR namesfile


	///////////
	//GUI
	setupGui();

	///////////////////
	//tracker Configuratio // TODO add Gui
	cout << "setup trackerPersistence to " << trackerPersistence << endl;
	tracker.setPersistence(trackerPersistence);
	tracker.setMaximumDistance(trackerMaximimDistance);

	//////////////////
	//OSC
	// open an outgoing connection to HOST:PORT
	sender.setup(HOST, PORT);

	//////////////////
	//Flow
	if (bOpticalFlow)flow = new ofxCv::FlowPyrLK();

}

//----------------------------
void ofxYolo4Games::resetVideoInterface() {
	bool bLoadedVid = false;
	if (bVideoPlayer) {
		bLoadedVid = videoPlayer.load("videos/1.mov");
		if (bLoadedVid) {
			videoPlayer.play();
			cout << "Reset videoPlayer = " << videoPlayer.getMoviePath() << endl;
		}
		else bVideoPlayer = false;
	}

	if (!bVideoPlayer) {
		//videoGrabber.listDevices();
		videoGrabber.setDeviceID(idVideoGrabber);
		videoGrabber.setDesiredFrameRate( 30 );
		videoGrabber.initGrabber( 640, 480 );
		cout << "Reset videoGrabber ID= " << 0 << endl;
	}
}

//-----------------------------------------------------------------
bool ofxYolo4Games::updateAndCropVideoCapture() {

	bool bNewVideoFrame = false;
	/////////////////////////////////////////////////
	//ResetInterface if Video was not init & Update Internal VideoFrames
	if (!bVideoPlayer) {
		if (!videoGrabber.isInitialized())resetVideoInterface();
		videoGrabber.update();
		if (videoPlayer.isInitialized())videoPlayer.close();
	}
	else {
		if (!videoPlayer.isInitialized())resetVideoInterface();
		videoPlayer.update();
		if (videoGrabber.isInitialized())videoGrabber.close();

	}

	/////////////////////////////////////////////////
	//Get Desired Pixels Video data
	//CROP
	if (bVideoPlayer)videoPlayer.getPixels().cropTo(cropedArea, videoPlayer.getWidth()*cropSizeX, videoPlayer.getHeight()*cropSizeY, videoPlayer.getWidth()*cropSizeW, videoPlayer.getHeight()*cropSizeH);
	else videoGrabber.getPixels().cropTo(cropedArea, videoGrabber.getWidth()*cropSizeX, videoGrabber.getHeight()*cropSizeY, videoGrabber.getWidth()*cropSizeW, videoGrabber.getHeight()*cropSizeH);


	if (bVideoPlayer) bNewVideoFrame = videoPlayer.isFrameNew();
	else bNewVideoFrame = videoGrabber.isFrameNew();

	return bNewVideoFrame;
}


//------------------------------------------
void ofxYolo4Games::update()
{	

	bNewFrameToProcess = updateAndCropVideoCapture();

	////////////////////////////////////////////////////
	//Detection and Tracker

	if (bNewFrameToProcess) {

		detections.clear();
		//YOLO2 detected objects with confidence < threshold(%) are omitted
		detections = darknet.yolo(cropedArea, minPercent4PersonDetected, maxOverlap);

		boundingRects.clear();

		//Pre clear results with only desired labels
		if (detections.size() > 0) {
			
			for (detected_object d : detections)
			{
				// optionally, you can grab the 1024-length feature vector associated
				// with each detected object
				vector<float> & features = d.features;

				//////////////////////////////////
				//BlobYolo Tracking
				//extrack, udpate and track bounding boxes
				//filter only person labels
				if (d.label == "person") {
					//trackerAnalizer analizer(d.label, d.probability, 0.5);
					boundingRects.push_back(ofxCv::toCv(d.rect));
				}
					
			}

		}

		//Update tracker
		tracker.track(boundingRects); //TODO Find how to pass other kind of variable with more info about detections

		//TEST
		//Update Optical Flow with the oldest item. //TODO check how to use this calculations.. then do all if it's good.
		if (bOpticalFlow && flow  != nullptr) {
			int oldest_BlobId = getOldestBlobIdTracked();
			if(oldest_BlobId > -1)flow->calcOpticalFlow(cropedArea);
			else {
				flow->resetFlow();
				cout << "flow->resetFlow();" << endl;
			}
		}

		//If Data Tracked then Send OSC
		if(bSendAllBlobsIn)send_OSC_Data_AllInBlobs();

		if (bSendYoloDataTracking)send_OSC_YoloData();

		//Spout
#if defined(USE_SHARECAM_SPOUT)
		if (bSpoutCameraActive)senderSpout.sendTexture(ofImage(cropedArea).getTexture(), "Camera");
#endif
	}
}

//-----------------------------------------
void ofxYolo4Games::send_OSC_Data_AllInBlobs() {

	vector<trackerAnalizer> followers = tracker.getFollowers();
	int numTrackedObjects = followers.size();
	//Start OSC package
	ofxOscMessage m;
	m.clear();
	m.setAddress("/GameBlobAllIn");//TODO tracking Label
	m.addIntArg(numTrackedObjects); //Add the number of Blobs detected in order to read them properly and easy

									//TODO Test with std::vector<F>& getFollowers()

	for (int i = 0; i < followers.size(); i++) {

		cv::Point2f centerBlobi;
		cv::Rect smooth_cur;
		smooth_cur = followers[i].smoothed;

		centerBlobi = cv::Point2f(smooth_cur.x + smooth_cur.width*.5, smooth_cur.y + smooth_cur.height*.5);

		float resumedPosX = centerBlobi.x / cropedArea.getWidth(); //Forced to 0..1 inside the RectArea 
		float resumedPosY = centerBlobi.y / cropedArea.getHeight(); //Forced to 0..1 inside the RectArea 

																	//if swap values acive:
		if (bSwapX)resumedPosX = 1 - resumedPosX;
		if (bSwapY)resumedPosY = 1 - resumedPosY;

		m.addFloatArg(resumedPosX);
		m.addFloatArg(resumedPosY);

		//Size W H 
		m.addFloatArg(smooth_cur.width);
		m.addFloatArg(smooth_cur.height);

		//Adde info to the message
		//for Tracking add int ID & int TIME
		int idAux = followers[i].getLabel();
		m.addIntArg(idAux); //Sending ID Label
		int timeAux = followers[i].getAge();
		m.addIntArg(timeAux); //Sending Time Tracked
	}

	sender.sendMessage(m, false);

}

//-----------------------------------------
void ofxYolo4Games::send_OSC_YoloData() {

	vector<trackerAnalizer> followers = tracker.getFollowers();
	//int numTrackedObjects = tracker.getCurrentRaw().size(); //TODO get this other way. CurrentLabels + DeadLabels
	int numTrackedObjects = followers.size();
	//Start OSC package
	ofxOscMessage m;
	m.clear();
	m.setAddress("/BlobsTrackedYoloData");
	m.addIntArg(numTrackedObjects); //Add the number of Blobs detected in order to read them properly and easy

									//TODO Test with std::vector<F>& getFollowers()

	for (int i = 0; i < followers.size(); i++) {
		cv::Point2f centerBlobi;
		cv::Rect smooth_cur;
		cv::Rect currentRect;
	
		smooth_cur = followers[i].smoothed;
		currentRect = followers[i].current;
				
		centerBlobi = cv::Point2f(smooth_cur.x + smooth_cur.width*.5, smooth_cur.y + smooth_cur.height*.5);

		float resumedPosX = (float)smooth_cur.x / cropedArea.getWidth(); //Forced to 0..1 inside the RectArea  //centerBlobi.x
		float resumedPosY = (float)smooth_cur.y / cropedArea.getHeight(); //Forced to 0..1 inside the RectArea  //centerBlobi.y

		//if swap values acive:
		if (bSwapX)resumedPosX = 1 - resumedPosX;
		if (bSwapY)resumedPosY = 1 - resumedPosY;

		m.addFloatArg(resumedPosX);
		m.addFloatArg(resumedPosY);

		//Size W H 
		m.addFloatArg(smooth_cur.width);
		m.addFloatArg(smooth_cur.height);

		//Adde info to the message
		//for Tracking add int ID & int TIME
		int idAux = followers[i].getLabel();
		m.addIntArg(idAux); //Sending ID Label
		int timeAux = followers[i].getAge();
		m.addIntArg(timeAux); //Sending Time Tracked

		//Actions trackerAnalizer Width
		m.addIntArg(followers[i].statusActionW); //Sending ID Label
		m.addIntArg(followers[i].statusActionH); //Sending Time Tracked

		//TODO. Find correlated detections to tracked object. 
		//Not possible searching in detections. If detection is lost but tracker memorized, will not find it.
		//This variable need to be added at tracker. --> Do the own trackerFollower class with calcDistance personalized (equal as the other one)

	}

	sender.sendMessage(m, false);
}

//----------------------------------------------------------
void ofxYolo4Games::drawFollowerAnalisys() {

	vector<trackerAnalizer> followers = tracker.getFollowers();
	for (int i = 0; i < followers.size(); i++) {
		//if(!followers[i].bLost)
		followers[i].draw();
		int temp_label = followers[i].getLabel();
	}
}

//----------------------------------------------------------
void ofxYolo4Games::draw()
{
	ofSetColor(255);
	//Draw Croped Area
	ofImage(cropedArea).draw(0, 0);

	ofNoFill();
	for (detected_object d : detections)
	{
		if (bDrawYoloInfo) {
			ofSetColor(d.color);
			glLineWidth(ofMap(d.probability, 0, 1, 0, 8));
			ofNoFill();
			ofDrawRectangle(d.rect);
			//cout << "detected_object = d.rect " << d.rect.position << endl;
			ofDrawBitmapStringHighlight(d.label + ": " + ofToString(d.probability), d.rect.x, d.rect.y + 20);
		}
	}
	//Draw Tracker results
	drawTracking();
	drawFollowerAnalisys();


	ofEnableAlphaBlending();
	if (bSociograma) {
		ofSetColor(colorSociograma);
		drawSociogramaConnections();
	}
	if (bOpticalFlow) {
		ofSetColor(colorFeatureLines);
		draw_OldestItem_OpticalFlowFeatures();
	}
	ofDisableAlphaBlending();

	ofSetColor(200);
	ofDrawBitmapStringHighlight("Fps=" + ofToString(ofGetFrameRate(), 0), ofGetWidth() - 130, 10);

	//OpticalFlow
	if (bOpticalFlow && flow  != nullptr) {
		flow->draw();
		if (ofGetMousePressed()) {
			ofNoFill();
			ofDrawRectangle(rect);
		}
	}

	//GUI
	ofFill();
	drawGui();
}
//----------------------------------------------------------
void ofxYolo4Games::drawSociogramaConnections() {

	vector<trackerAnalizer> followers = tracker.getFollowers();
	for (int i = 0; i < followers.size(); i++) {
		cv::Rect smooth_curI = followers[i].smoothed;
		for (int j = 0; j < followers.size(); j++) {
			cv::Rect smooth_curJ = followers[j].smoothed;
			drawLineConnection(ofVec2f(smooth_curI.x, smooth_curI.y), ofVec2f(smooth_curJ.x, smooth_curJ.y), 1);
		}
	}
}

//----------------------------------------------------------
void ofxYolo4Games::draw_OldestItem_OpticalFlowFeatures() {
	if (bOpticalFlow && flow  != nullptr) {
		for (int i = 0; i < flow->getCurrent().size(); i++) {
			ofPoint curI = flow->getCurrent()[i];
			for (int j = 0; j < flow->getCurrent().size(); j++) {
				ofPoint curJ = flow->getCurrent()[j];
				drawLineConnection(ofVec2f(curI.x, curI.y), ofVec2f(curJ.x, curJ.y), 1);
				// << "drawinlines curI.object.x = " << curJ.object.x << "curI.object.y = " << curJ.object.y << endl;
			}
		}
	}
}
//----------------------------------------------------------
void ofxYolo4Games::drawLineConnection(ofVec2f posBlobi, ofVec2f posBlobn, float gros)
{

	ofPushStyle();
	ofSetLineWidth(gros);

	//Draw a line between items
	ofLine(posBlobi, posBlobn);

	ofPopStyle();
}

//----------------------------------------------------------
void ofxYolo4Games::drawTracking() {

	if (bDrawTracking) { //Show Labels

		vector<trackerAnalizer> followers = tracker.getFollowers();
		for (int i = 0; i < followers.size(); i++) {
			cv::Point2f centerBlobi;
			cv::Rect smooth_cur = followers[i].smoothed;

			centerBlobi = cv::Point2f(smooth_cur.x + smooth_cur.width*.5, smooth_cur.y + smooth_cur.height*.5);

			float resumedPosX = centerBlobi.x / cropedArea.getWidth(); //Forced to 0..1 inside the RectArea 
			float resumedPosY = centerBlobi.y / cropedArea.getHeight(); //Forced to 0..1 inside the RectArea 

																		//if swap values acive:
			if (bSwapX)resumedPosX = 1 - resumedPosX;
			if (bSwapY)resumedPosY = 1 - resumedPosY;

			ofPushMatrix();
			ofPushStyle();

			//Draw Smoothed rectangle detected
			ofSetColor(ofColor::hotPink);
			ofRectangle auxPlayerArea = ofxCv::toOf(smooth_cur);
			ofDrawRectangle(auxPlayerArea);

			ofSetColor(ofColor::white);
			int label = followers[i].getLabel();
			string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
			ofDrawBitmapString(msg, auxPlayerArea.getBottomLeft().x, auxPlayerArea.getBottomLeft().y);

			ofPopStyle();
			ofPopMatrix();
		}
	}
}

//--------------------------------------------------------------
void ofxYolo4Games::keyPressed(int key) {
	if (key == 's') {
		//gui.saveToFile("settings.xml");
	}
	if (key == 'c') {
		videoGrabber.videoSettings();
	}

	if (key == ' ') {
		if (bOpticalFlow) {
			//Manual method
			int oldestId = findOldestBlobId();
			if (oldestId > -1) {
				trackerAnalizer oldItem = tracker.getFollowers()[oldestId];
				ofRectangle auxRect(oldItem.smoothed.x, oldItem.smoothed.y, oldItem.smoothed.width, oldItem.smoothed.height);
				resetOpticalFlowArea(auxRect);
			}
		}
	}
}

//---------------------------------
void ofxYolo4Games::mouseDragged(int x, int y, int button) {
	if (bFlowManualMouseSel) {
		ofVec2f p2(x, y);
		rect.set(p1, p2.x - p1.x, p2.y - p1.y);
	}
}

//----------------------------------
void ofxYolo4Games::mousePressed(int x, int y, int button) {
	if(bFlowManualMouseSel)p1.set(x, y);
}

//----------------------------------
void ofxYolo4Games::mouseReleased(int x, int y, int button) {
	if (bFlowManualMouseSel) {
		ofVec2f p2(x, y);
		rect.set(p1, p2.x - p1.x, p2.y - p1.y);
		resetOpticalFlowArea(rect);
	}
}

//-----------------------------------
void ofxYolo4Games::resetOpticalFlowArea(ofRectangle _rect) {
	
	if (flow  != nullptr) {
		vector<KeyPoint> keypoints;
		vector<KeyPoint> keypointsInside;
		vector<cv::Point2f> featuresToTrack;
		copyGray(cropedArea, grabberGray);
		FAST(grabberGray, keypoints, 2);
		for (int i = 0; i < keypoints.size(); i++) {
			if (_rect.inside(toOf(keypoints[i].pt))) {
				keypointsInside.push_back(keypoints[i]);
			}
		}
		KeyPointsFilter::retainBest(keypointsInside, numKeypointsInside);
		KeyPoint::convert(keypointsInside, featuresToTrack);
		flow->setFeaturesToTrack(featuresToTrack);
	}

}

//-------------------
int ofxYolo4Games::getOldestBlobIdTracked() {
	int oldestBlob = findOldestBlobId();
	if (oldestBlob > -1) {
		if (last_oldestBlob != oldestBlob) {
			last_oldestBlob = oldestBlob;
			//if (tracker.getCurrentRaw()[oldestBlob].getAge() > numMinFramesOldest) {
				//Diferent, new and already oldest: perfect, set new Flow points area.

				//ofxCv::TrackedObject<cv::Rect> oldItem = tracker.getCurrentRaw()[oldestBlob];
				trackerAnalizer oldItem = tracker.getFollowers()[oldestBlob];
				ofRectangle auxRect(oldItem.smoothed.x, oldItem.smoothed.y, oldItem.smoothed.width, oldItem.smoothed.height);
				resetOpticalFlowArea(auxRect);
			//}
		}
		//equals
		else {
			//if (tracker.getCurrentRaw()[oldestBlob].getAge() > numMinFramesOldest) {
			//	cout << "Check when to update flow points again, may work look at that points if are our or still inside the image or dinamic detection area" << endl;
			//}
		}
	}

	return oldestBlob;
}

//-------------------
//look for the older blob and track it with optical flow
int ofxYolo4Games::findOldestBlobId() {
	int idOldest = -1;
	int maxAge = 0;
	
	for (int i = 0; i < tracker.getFollowers().size(); i++) {
		//ofxCv::TrackedObject<cv::Rect> cur = tracker.getCurrentRaw()[i];
		trackerAnalizer cur = tracker.getFollowers()[i];
		if (cur.getAge() > maxAge) {
			maxAge = cur.getAge();
			idOldest = i;
		}

	}

	return idOldest;
}