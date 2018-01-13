#include "ofApp.h"


//------------------------------------------------------------
void ofApp::setup() 
{
	
	myYoloSensor.setup();
}


//------------------------------------------
void ofApp::update()
{	
	myYoloSensor.update();
	
}

//----------------------------------------------------------
void ofApp::draw()
{
	ofSetColor(255);
	myYoloSensor.draw();
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	myYoloSensor.keyPressed(key);
}

//---------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	myYoloSensor.mouseDragged(x, y, button);
}

//----------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	myYoloSensor.mousePressed(x, y, button);
}

//----------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	myYoloSensor.mouseReleased(x, y, button);
}
