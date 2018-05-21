#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxDarknet.h"
#include "ofxImGui.h"
#include "ofxOsc.h"
#include "trackerAnalizer.h"


//Uncomment this to Use it
#define USE_SHARECAM_SPOUT

#if defined(USE_SHARECAM_SPOUT)//and (TARGET_WIN32)
#include "ofxSpout2Sender.h"
#endif

class ofxYolo4Games
{
public:
	void setup();
	void update();
	void draw();

	void resetVideoInterface();
	bool updateAndCropVideoCapture();

	void send_OSC_Data_AllInBlobs();
	void send_OSC_YoloData();

	void drawFollowerAnalisys();

	ofxDarknet darknet;

	ofVideoGrabber videoGrabber;
	int totalNumDevices = 0;
	ofVideoPlayer videoPlayer;
	bool bNewFrameToProcess = false;
	ofPixels cropedArea;
	std::vector< detected_object > detections;
	std::vector< detected_object > trackedDetections;

	/////////////////////////////////////
	//Tracker	
	void drawTracking();
	//cv::Vec2f getVelocity(unsigned int i) const;
	//unsigned int getLabel(unsigned int i) const;
	
	void setupGui();
	void drawGui();
	//ofxCv::RectTracker4Games * tracker;
	//float smoothingRateHacked = 0.5;
	ofxCv::RectTrackerFollower<trackerAnalizer> tracker;
	std::vector<cv::Rect> boundingRects;
	//TODO //std::vector<cv::TrackedYoloObjects> trackedObjects;
	bool bDrawTracking = true;


	//////////////////////
	//SocioGrama
	bool bSociograma = false;
	ofColor colorSociograma = ofColor::blueSteel;
	void drawSociogramaConnections();
	void drawLineConnection(ofVec2f posBlobi, ofVec2f posBlobn, float gros);

	////////////
	//OpicalFlow
	void draw_OldestItem_OpticalFlowFeatures();
	bool bOpticalFlow = false;
	cv::Mat grabberGray;
	ofxCv::FlowPyrLK * flow;
	ofVec2f p1;
	ofRectangle rect;
	void resetOpticalFlowArea(ofRectangle _rect);

	int getOldestBlobIdTracked();
	int last_oldestBlob = -1;
	int numMinFramesOldest;
	int findOldestBlobId();
	int numKeypointsInside = 10;
	ofColor colorFeatureLines = ofColor::indianRed;
	bool bFlowManualMouseSel = false;

	///////////////
	//GUI
	ofxImGui::Gui gui;
	vector<ofVideoDevice> auxListDevices;
	//Croping camera
	float cropSizeX = 0; float last_cropSizeX;
	float cropSizeY = 0; float last_cropSizeY;
	float cropSizeW = 1; float last_cropSizeW;
	float cropSizeH = 1; float last_cropSizeH;
	//yolo gui
	string detectionLabel;
	float maxRectAreaDetection;
	float minPercent4PersonDetected = 0.25;
	float maxOverlap = 0.25;
	bool bVideoPlayer = false;
	int idVideoGrabber = 0;

	bool bSwapX = false;
	bool bSwapY = false;
	bool bDrawYoloInfo;
	//tracker gui
	int trackerPersistence = 10; float last_trackerPersistence;
	int trackerMaximimDistance = 200; float last_trackerMaximimDistance;
	float trackerSmoothingRate = 0.01; float last_trackerSmoothingRate;

	//mouse keyboard events
	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

	///////////////////////////////
	//OSC filterd data
	ofxOscSender sender;
	bool bSendAllBlobsIn = false;
	bool bSendYoloDataTracking = true;

	//OSC CONFIG
	bool bResetHostIp = false;
	int PORT = 12345;
	string HOST = "localhost";//MLP: "192.168.2.254"; //192.168.1.158

#if defined(USE_SHARECAM_SPOUT)//AND (TARGET_WIN32)
	bool bSpoutCameraActive = false;
	ofxSpout2::Sender senderSpout;
#endif

};

