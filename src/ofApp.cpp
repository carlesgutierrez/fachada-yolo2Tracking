#include "ofApp.h"
//--------------------------------------------------------------
cv::Vec2f ofApp::getVelocity(unsigned int i) const {
	return tracker.getVelocity(i);
}

unsigned int ofApp::getLabel(unsigned int i) const {
	return tracker.getCurrentLabels()[i];
}

ofxCv::RectTracker& ofApp::getTracker() {
	return tracker;
}
//------------------------------------------------------------
void ofApp::setup() 
{
	//////////
	//Darknet Yolo2 Data
	std::string cfgfile = ofToDataPath( "cfg/YOLOv2_608x608.cfg" );	//cfg/yolo-voc-YOLOv2.cfg	//yolo9000.cfg							//GPU (VOC) tiny-yolo-voc.cfg
	std::string weightfile = ofToDataPath( "YOLOv2_608x608_COCO_trainval.weights" );	// yolo-voc-YOLOv2.weights //yolo9000.weights//yolo.weights		//GPU (VOC) tiny-yolo-voc.weights
	std::string namesfile = ofToDataPath( "cfg/coco.names" );	//cfg/voc.names	//cfg/9k.names								//GPU (VOC) cfg/voc.names
    
	darknet.init( cfgfile, weightfile, namesfile);//"" OR namesfile


	///////////
	//GUI
	gui.setup();
	// if a detected object overlaps >maxOverlap with another detected
	// object with a higher confidence, it gets omitted
	gui.add(bVideoPlayer.set("bVideoPlayer", true));
	vector<ofVideoDevice> auxListDevices = videoGrabber.listDevices();
	cout << "Num available VideoGrabber Devices =" << auxListDevices.size() << endl;
	gui.add(idVideoGrabber.set("Device Id ", 0, 0, auxListDevices.size()));
	gui.add(bSwapX.set("bSwapX", false));
	gui.add(bSwapY.set("bSwapY", false));

	gui.add(cropSizeX.set("CropX", 0.10, 0, 1));
	gui.add(cropSizeY.set("CropY", 0.20, 0, 1));
	gui.add(cropSizeW.set("CropW", 0.70, 0, 1));
	gui.add(cropSizeH.set("CropH", 0.70, 0, 1));
	
	//YOLO GUI DETECTIONS
	gui.add(bDrawYoloInfo.set("Draw Yolo Info", false));
	gui.add(detectionLabel.set("Filtered Label", "person"));
	gui.add(percentPersonDetected.set("Probability", 0.25, 0.001, 1));
	gui.add(maxOverlap.set("Max Overlap", 0.25, 0.01, 1));
	//gui.add(maxRectAreaDetection.set("MaxRectAreaDetection", 140, 10, 300));
	
	//TRACKER GUI options //TODO check how to detect changes values
	last_trackerPersistence = 200;
	gui.add(trackerPersistence.set("trackerPersistence", 200, 1, 50)); // milis time persistence blob tracked
	
	last_trackerMaximimDistance = 50;
	gui.add(trackerMaximimDistance.set("trackerMaxDistance", 50, 1, 200)); // depends on the camera resolution
	
	last_trackerSmoothingRate = 0.3;
	gui.add(trackerSmoothingRate.set("trackerSmoothingRate", 0.3, 0.1, 1)); //

	gui.loadFromFile("settings.xml");


	///////////////////
	//tracker Configuratio // TODO add Gui
	tracker.setPersistence(trackerPersistence);
	tracker.setMaximumDistance(trackerMaximimDistance);
	tracker.setSmoothingRate(trackerSmoothingRate);

	//////////////////
	//OSC
	// open an outgoing connection to HOST:PORT
	sender.setup(HOST, PORT);

}

//----------------------------
void ofApp::resetVideoInterface() {
	bool bLoadedVid = false;
	if (bVideoPlayer) {
		bLoadedVid = videoPlayer.load("videos/1.mov");
		if (bLoadedVid) {
			videoPlayer.play();
			cout << "Setup videoPlayer = " << videoPlayer.getMoviePath() << endl;
		}
		else bVideoPlayer = false;
	}

	if (!bVideoPlayer) {
		//videoGrabber.listDevices();
		videoGrabber.setDeviceID(idVideoGrabber);
		videoGrabber.setDesiredFrameRate( 30 );
		videoGrabber.initGrabber( 640, 480 );
		cout << "Setup videoGrabber ID= " << 0 << endl;
	}
}

//----------------------------------------
void ofApp::updateGuiParamters() {

	if (last_trackerPersistence != trackerPersistence) {
		tracker.setPersistence(trackerPersistence);
		last_trackerPersistence = trackerPersistence;
	}
	if (last_trackerMaximimDistance != trackerMaximimDistance) {
		tracker.setMaximumDistance(trackerMaximimDistance);
		last_trackerMaximimDistance = trackerMaximimDistance;
	}
	if (last_trackerSmoothingRate != trackerSmoothingRate) {
		tracker.setSmoothingRate(trackerSmoothingRate);
		last_trackerSmoothingRate = trackerSmoothingRate;
	}
}

//------------------------------------------
void ofApp::update()
{
	////////////////////////////////////////////////////
	//Update GUI
	updateGuiParamters();
	

	////////////////////////////////////////////////////////
	//ResetInterface if Video was not init & Update Internal VideoFrames
	if (!bVideoPlayer) {
		if (!videoGrabber.isInitialized())resetVideoInterface();
		videoGrabber.update();
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

		//Pre clear
		if (detections.size() > 0) {
			boundingRects.clear();

	
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

		//If Data Tracked then Send OSC
		send_OSC_Data_AllInBlobs();
	}
}

//-----------------------------------------
void ofApp::send_OSC_Data_AllInBlobs() {
	
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
		cv::Point2f centerBlobi = cv::Point2f(cur.object.x + cur.object.width*.5, cur.object.y + cur.object.height*.5);
		
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

		sender.sendMessage(m, false);
	}

}

//----------------------------------------------------------
void ofApp::draw()
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


	ofSetColor(200);
	ofDrawBitmapStringHighlight("Fps=" + ofToString(ofGetFrameRate(), 0), 10, 10);

	//GUI
	gui.draw();
}

//----------------------------------------------------------
void ofApp::drawTracking() {

	if (true) { //Show Labels

		for (int i = 0; i < boundingRects.size(); i++) {
			ofPoint center = ofPoint(boundingRects[i].x + boundingRects[i].width*.5, boundingRects[i].y + boundingRects[i].height*.5);
			ofPushMatrix();
			ofPushStyle();
			ofSetColor(ofColor::red);
			ofDrawRectangle(ofxCv::toOf(boundingRects[i]));
			ofTranslate(center.x, center.y);
			int label = tracker.getCurrentLabels()[i];
			string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
			ofDrawBitmapString(msg, 0, 0);
			ofVec2f velocity = ofxCv::toOf(tracker.getVelocity(i));
			ofDrawLine(0, 0, velocity.x, velocity.y);
			ofPopStyle();
			ofPopMatrix();

		}

	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 's') {
		gui.saveToFile("settings.xml");
	}
}