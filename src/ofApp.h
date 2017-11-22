#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxDarknet.h"
#include "ofxGui.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp
{
public:
	void setup();
	void resetVideoInterface();
	void updateGuiParamters();
	void update();
	void send_OSC_Data_AllInBlobs();
	void draw();

	ofxDarknet darknet;

	ofVideoGrabber videoGrabber;
	int totalNumDevices = 0;
	ofVideoPlayer videoPlayer;
	bool bNewFrameToProcess = false;
	ofPixels cropedArea;
	std::vector< detected_object > detections;

	/////////////////////////////////////
	//Tracker	
	void drawTracking();
	cv::Vec2f getVelocity(unsigned int i) const;
	unsigned int getLabel(unsigned int i) const;
	ofxCv::RectTracker & getTracker();
	void setupGui();
	ofxCv::RectTracker tracker;
	std::vector<cv::Rect> boundingRects;
	ofParameter<bool> bDrawTracking;


	//////////////////////
	//SocioGrama
	ofParameter<bool> bSociograma;
	ofParameter<ofColor> colorSociograma;
	void drawSociogramaConnections();
	void drawLineConnection(ofVec2f posBlobi, ofVec2f posBlobn, float gros);

	////////////
	//OpicalFlow
	void draw_OldestItem_OpticalFlowFeatures();
	ofParameter<bool> bOpticalFlow;
	cv::Mat grabberGray;
	ofxCv::FlowPyrLK flow;
	ofVec2f p1;
	ofRectangle rect;
	void resetOpticalFlowArea(ofRectangle _rect);

	int updateHardestBlobTracked();
	int last_oldestBlob = -1;
	ofParameter<int> numMinFramesOldest;
	int findOldestBlobId();
	ofParameter<int> numKeypointsInside;
	ofParameter<ofColor> colorFeatureLines;
	ofParameter<bool> bFlowManualMouseSel;

	///////////////
	//GUI
	ofxPanel gui;
	//Croping camera
	ofParameter<float> cropSizeX; float last_cropSizeX;
	ofParameter<float> cropSizeY; float last_cropSizeY;
	ofParameter<float> cropSizeW; float last_cropSizeW;
	ofParameter<float> cropSizeH; float last_cropSizeH;
	//yolo gui
	ofParameter<string> detectionLabel;
	ofParameter<float> maxRectAreaDetection;
	ofParameter<float> percentPersonDetected;
	ofParameter<float> maxOverlap;
	ofParameter<bool> bVideoPlayer;
	ofParameter<int> idVideoGrabber;

	ofParameter<bool> bSwapX;
	ofParameter<bool> bSwapY;
	ofParameter<bool> bDrawYoloInfo;
	//tracker gui
	ofParameter<float> trackerPersistence;
	float last_trackerPersistence;
	ofParameter<float> trackerMaximimDistance;
	float last_trackerMaximimDistance;
	ofParameter<float> trackerSmoothingRate;
	float last_trackerSmoothingRate;

	//mouse keyboard events
	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);



	///////////////////////////////
	//OSC filterd data
	ofxOscSender sender;

	//OSC CONFIG
	bool bResetHostIp = false;
	int PORT = 12345;
	string HOST = "127.0.0.1";//MLP: "192.168.2.254"; //192.168.1.158
};
