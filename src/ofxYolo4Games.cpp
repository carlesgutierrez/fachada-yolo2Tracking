#include "ofxYolo4Games.h"

using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
cv::Vec2f ofxYolo4Games::getVelocity(unsigned int i) const {
	return tracker.getVelocity(i);
}

unsigned int ofxYolo4Games::getLabel(unsigned int i) const {
	return tracker.getCurrentLabels()[i];
}

ofxCv::RectTracker& ofxYolo4Games::getTracker() {
	return tracker;
}

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
	ImGui::SliderFloat("Probability", &percentPersonDetected, 0.001, 1);
	ImGui::SliderFloat("Max Overlap", &maxOverlap, 0.01, 1);
	//gui.add(maxRectAreaDetection.set("MaxRectAreaDetection", 140, 10, 300));


	//TRACKER GUI options //TODO check how to detect changes values
	ImGui::Checkbox("Draw Tracking", &bDrawTracking);

	//TODO Follow Here!
	if (ImGui::SliderInt("trackerPersistence", &trackerPersistence, 1, 50))
		tracker.setPersistence(trackerPersistence);
	if(ImGui::SliderInt("trackerMaximimDistance", &trackerMaximimDistance, 1, 200))
		tracker.setMaximumDistance(trackerMaximimDistance);
	if(ImGui::SliderFloat("trackerSmoothingRate", &trackerSmoothingRate, 0.1, 1))
		tracker.setSmoothingRate(trackerSmoothingRate);

	ImGui::Checkbox("Draw Tracking", &bDrawTracking);

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
	tracker.setPersistence(trackerPersistence);
	tracker.setMaximumDistance(trackerMaximimDistance);
	tracker.setSmoothingRate(trackerSmoothingRate);

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

//------------------------------------------
void ofxYolo4Games::update()
{	

	////////////////////////////////////////////////////////
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

	//YOLO2 detected objects with confidence < threshold are omitted
	float thresh = percentPersonDetected;//ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 1);
	//float maxOverlap = 0.25; // setup at GUI

	//CROP
	if (bVideoPlayer)videoPlayer.getPixels().cropTo(cropedArea, videoPlayer.getWidth()*cropSizeX, videoPlayer.getHeight()*cropSizeY, videoPlayer.getWidth()*cropSizeW, videoPlayer.getHeight()*cropSizeH);
	else videoGrabber.getPixels().cropTo(cropedArea, videoGrabber.getWidth()*cropSizeX, videoGrabber.getHeight()*cropSizeY, videoGrabber.getWidth()*cropSizeW, videoGrabber.getHeight()*cropSizeH);

	
	if (bVideoPlayer) bNewFrameToProcess = videoPlayer.isFrameNew();
	else  bNewFrameToProcess = videoGrabber.isFrameNew();

	////////////////////////////////////////////////////
	//Detection and Tracker

	if (bNewFrameToProcess) {

		detections.clear();
		detections = darknet.yolo(cropedArea, thresh, maxOverlap);

		boundingRects.clear();

		//Pre clear
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
				if (d.label == "person")
					boundingRects.push_back(ofxCv::toCv(d.rect));
			}

		}

		//Update tracker
		tracker.track(boundingRects);

		//TEST
		//Optical Flow Test with the first Bounding detected
		if (bOpticalFlow && flow  != nullptr) {
			int oldest_BlobId = updateHardestBlobTracked();
			if(oldest_BlobId > -1)flow->calcOpticalFlow(cropedArea);
			else {
				flow->resetFlow();
				cout << "flow->resetFlow();" << endl;
			}
		}


		//If Data Tracked then Send OSC
		send_OSC_Data_AllInBlobs();
	}
}

//-----------------------------------------
void ofxYolo4Games::send_OSC_Data_AllInBlobs() {
	
	int numTrackedObjects = tracker.getCurrentRaw().size();
	//cout << "numTrackedObjects = " << numTrackedObjects << endl;
	
	//Start OSC package
	ofxOscMessage m;
	m.clear();
	m.setAddress("/GameBlobAllIn");//TODO tracking Label
	m.addIntArg(numTrackedObjects); //Add the number of Blobs detected in order to read them properly and easy

	//Later tracker.getDeadLabels().size();
	for (int i = 0; i < numTrackedObjects; i++) {
		
		ofxCv::TrackedObject<cv::Rect> cur = tracker.getCurrentRaw()[i];
		unsigned int auxSmoothLabel = cur.getLabel();
		//Getting Smoothed Values Directly
		cv::Point2f centerBlobi;
		cv::Rect smooth_cur;
		smooth_cur = tracker.getSmoothed(auxSmoothLabel);
		//centerBlobi = cv::Point2f(cur.object.x + cur.object.width*.5, cur.object.y + cur.object.height*.5);
		centerBlobi = cv::Point2f(smooth_cur.x + smooth_cur.width*.5, smooth_cur.y + smooth_cur.height*.5);

		float resumedPosX = centerBlobi.x / cropedArea.getWidth(); //Forced to 0..1 inside the RectArea 
		float resumedPosY = centerBlobi.y / cropedArea.getHeight(); //Forced to 0..1 inside the RectArea 

		//if swap values acive:
		if (bSwapX)resumedPosX = 1 - resumedPosX;
		if (bSwapY)resumedPosY = 1 - resumedPosY;

		m.addFloatArg(resumedPosX);
		m.addFloatArg(resumedPosY);

		//Size W H 
		m.addFloatArg(cur.object.width);
		m.addFloatArg(cur.object.height);

		//Adde info to the message
		//for Tracking add int ID & int TIME
		int idAux = cur.getLabel();
		m.addIntArg(idAux); //Sending ID Label
		int timeAux = cur.getAge();
		m.addIntArg(timeAux); //Sending Time Tracked

		//This must be done using FollowerTracker class
		//if (detections.size() > i) {
		//	float auxProb = detections[i].probability;
		//	m.addFloatArg(auxProb);
		//}
	}

	sender.sendMessage(m, false);

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
	for (int i = 0; i < tracker.getCurrentRaw().size(); i++) {
		ofxCv::TrackedObject<cv::Rect> curI = tracker.getCurrentRaw()[i];
		for (int j = 0; j < tracker.getCurrentRaw().size(); j++) {
			ofxCv::TrackedObject<cv::Rect> curJ = tracker.getCurrentRaw()[j];
			drawLineConnection(ofVec2f(curI.object.x, curI.object.y), ofVec2f(curJ.object.x, curJ.object.y), 1);
			// << "drawinlines curI.object.x = " << curJ.object.x << "curI.object.y = " << curJ.object.y << endl;
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

		//for (int i = 0; i < boundingRects.size(); i++) {
		//	ofPoint center = ofPoint(boundingRects[i].x + boundingRects[i].width*.5, boundingRects[i].y + boundingRects[i].height*.5);
		//	ofPushMatrix();
		//	ofPushStyle();
		//	ofSetColor(ofColor::red);
		//	ofDrawRectangle(ofxCv::toOf(boundingRects[i]));
		//	ofTranslate(center.x, center.y);
		//	int label = tracker.getCurrentLabels()[i];
		//	string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
		//	ofDrawBitmapString(msg, 0, 0);
		//	ofVec2f velocity = ofxCv::toOf(tracker.getVelocity(i));
		//	ofDrawLine(0, 0, velocity.x, velocity.y);
		//	ofPopStyle();
		//	ofPopMatrix();

		//}

		//Later tracker.getDeadLabels().size();
		for (int i = 0; i < tracker.getCurrentRaw().size(); i++) {

			ofxCv::TrackedObject<cv::Rect> cur = tracker.getCurrentRaw()[i];
			unsigned int auxSmoothLabel = cur.getLabel();
			//Getting Smoothed Values Directly
			cv::Point2f centerBlobi;
			cv::Rect smooth_cur;
			smooth_cur = tracker.getSmoothed(auxSmoothLabel);
			//centerBlobi = cv::Point2f(cur.object.x + cur.object.width*.5, cur.object.y + cur.object.height*.5);
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
			ofDrawRectangle(ofxCv::toOf(smooth_cur));

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
				ofxCv::TrackedObject<cv::Rect> oldItem = tracker.getCurrentRaw()[oldestId];
				ofRectangle auxRect(oldItem.object.x, oldItem.object.y, oldItem.object.width, oldItem.object.height);
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
int ofxYolo4Games::updateHardestBlobTracked() {
	int oldestBlob = findOldestBlobId();
	if (oldestBlob > -1) {
		if (last_oldestBlob != oldestBlob) {
			last_oldestBlob = oldestBlob;
			//if (tracker.getCurrentRaw()[oldestBlob].getAge() > numMinFramesOldest) {
				//Diferent, new and already oldest: perfect, set new Flow points area.
				ofxCv::TrackedObject<cv::Rect> oldItem = tracker.getCurrentRaw()[oldestBlob];
				ofRectangle auxRect(oldItem.object.x, oldItem.object.y, oldItem.object.width, oldItem.object.height);
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
	
	for (int i = 0; i < tracker.getCurrentRaw().size(); i++) {
		ofxCv::TrackedObject<cv::Rect> cur = tracker.getCurrentRaw()[i];
		if (cur.getAge() > maxAge) {
			maxAge = cur.getAge();
			idOldest = i;
		}

	}

	return idOldest;
}