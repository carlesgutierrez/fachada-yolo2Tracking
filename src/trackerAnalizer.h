#pragma once

#include "ofMain.h"
#include "statsRecorder.h"
#include "ofxCv.h"
#include "variablesGui.h"

//trackerAnalizer -> performe Box a simple behauoviour detection ( Duck, Jump, Thin, Thick )
class trackerAnalizer : public ofxCv::RectFollower { //RectFollower works but not smoothing { //Want to use ObjectFinder but I need examples
//RectFollower -> setup we save original position, Then Update make comparisons. Then Kill remove them (dead = true).

protected:

	ofRectangle savedRect;
	ofRectangle actualRect;

	//last position values
	ofPoint lastPositionRect;
	float lastDirectionX;

	//Calc stable Rect
	ofRectangle stableRect;
	int counterSimilarFrames = 0;
	//visual feedback
	ofColor colorInteractionHeight = ofColor::gray;
	ofColor colorInteractionWidth = ofColor::gray;
	//TODO add Color Gradient Transitions... add ofxColorGradien to project
	ofColor colorJump = ofColor::skyBlue;
	ofColor colorDuck = ofColor::indianRed;
	ofColor colorThick = ofColor::lightBlue;
	ofColor colorWide = ofColor::lightSalmon;
	ofColor colorNormal = ofColor::pink;

	////Smoothing copied from RectTracker
	float smoothingRateRect = 0.5;
	int age = 0;

	float probability;
	string labelDetected;

public:

	//extra actions
	int statusActionW = 0;
	int statusActionH = 0;
	int statusActionX = 0;
	int statusActionY = 0;

	//age counter
	int getAge() { return age; }
	int getLabel() { return label; }
	bool bLost = false;

	//Saved Rect
	cv::Rect current;
	cv::Rect smoothed;	//Smoothing

	trackerAnalizer() /*:smoothingRateRect(.5)*/  {
	}

	trackerAnalizer(string _label, float _probability,float _smoothingRate)  {
		labelDetected = _label;
		probability = _probability;
		smoothingRateRect = _smoothingRate;
	}

	//void setSmoothingRate(float _smoothingRate) {
	//	smoothingRateRect = _smoothingRate;
	//}
	//float getSmoothingRate() const {
	//	return smoothingRateRect;
	//}
	void setup(const cv::Rect& track) {
		savedRect = ofxCv::toOf(track); //Make a copy at start. TODO, make refinement during udpate
		stableRect = savedRect;
		smoothed = track;
		current = track;
	}
	void updateStableRect(ofRectangle newRectValue, float marginSimilarValues, int numSimilarFrames) {
		//Do some stats and save most stable dimensions ( stand up ) checking AspectRatio
		float aspectRatioStable = stableRect.getAspectRatio();
		float aspectRatioUpdate = newRectValue.getAspectRatio();
		if (aspectRatioUpdate > aspectRatioStable - marginSimilarValues) {
			if (aspectRatioUpdate < aspectRatioStable + marginSimilarValues) {
				counterSimilarFrames++;
				if (counterSimilarFrames > numSimilarFrames) {
					stableRect = newRectValue; //Saving the new Stable Dimensions for future comparisons
					//cout << "Saving the new Stable Dimensions label=" << label << endl;
				}
			}
			else counterSimilarFrames = 0;
		}
		else counterSimilarFrames = 0;


	}
	void update(const cv::Rect& track) {

		//Counter age
		age++;

		////Kind of Smoothing
		const cv::Rect& cur = track;

		smoothed.x = ofLerp(smoothed.x, cur.x, smoothingRateRect);
		smoothed.y = ofLerp(smoothed.y, cur.y, smoothingRateRect);
		smoothed.width = ofLerp(smoothed.width, cur.width, smoothingRateRect);
		smoothed.height = ofLerp(smoothed.height, cur.height, smoothingRateRect);
		current = cur;


		//My update process
		actualRect = ofxCv::toOf(smoothed/*cur*/);
		updateStableRect(actualRect, 0.1, 100);
		///////////////////////
		// Compare actualReck with savedRect
		float errorThreshold = 0.20;
		//Make Comparisons
		float diffWidthNorm = ofMap(actualRect.getWidth(), 0, stableRect.width, 0, 1);
		compareDimensionW(diffWidthNorm, 0.2);
		float diffHeightNorm = ofMap(actualRect.getHeight(), 0, stableRect.height, 0, 1);
		compareDimensionH(diffHeightNorm, 0.2);
		//cout << "label= " << label << " bLost = " << bLost << endl;

		////Compare actual direction X
		if(lastPositionRect.x == 0)lastPositionRect = actualRect.getCenter(); // first iter
		variablesGui::getInstance()->directionX = lastPositionRect.x - actualRect.getCenter().x;
				
		if (abs(variablesGui::getInstance()->directionX) > variablesGui::getInstance()->directionXTrheshold)
			lastDirectionX = variablesGui::getInstance()->directionX;
		else lastDirectionX = 0;
		

		lastPositionRect = actualRect.getCenter(); // 0..1
		//compareActionX
	}
	void kill() {
		bLost = true;
		dead = true;//Required true to erase it. Otherwise will never die. Check Virtual Kill func
		//cout << "Nothing special for now" << endl;
		//if (bPrincipalNode) {
		//}
	}

	void draw() {
		ofPushStyle();
		ofEnableAlphaBlending();
		float alphaRects = 255;
		ofSetLineWidth(4);
		//Colors for Width Analisys
		if (statusActionW > 0)ofSetColor(colorThick, alphaRects);
		else if (statusActionW < 0)ofSetColor(colorThick, alphaRects);
		else {
			ofSetLineWidth(2);
			ofSetColor(colorNormal, alphaRects);
		}
	
		//TODO CHECK HIGHT LIGHT TEXT for LABEL and ID track and Thickness
		ofDrawLine(actualRect.getTopLeft().x, actualRect.getTopLeft().y, actualRect.getTopRight().x, actualRect.getTopRight().y);

		//Colors for height Analisys
		ofSetLineWidth(4);
		if (bLost)ofSetColor(ofColor::black, alphaRects);
		else if (statusActionH > 0)ofSetColor(colorJump, alphaRects);
		else if (statusActionH < 0)ofSetColor(colorDuck, alphaRects);
		else {
			ofSetLineWidth(2);
			ofSetColor(colorNormal, alphaRects);
		}
		ofDrawLine(actualRect.getTopLeft().x, actualRect.getTopLeft().y, actualRect.getBottomLeft().x, actualRect.getBottomLeft().y);

		//Draw Arrow Direction
		ofPoint topArrow = ofPoint(actualRect.getCenter().x, actualRect.getCenter().y - actualRect.getHeight()*0.5);
		ofPoint bottomArrow = ofPoint(actualRect.getCenter().x, actualRect.getCenter().y + actualRect.getHeight()*0.5);
		ofPoint leftArrow = ofPoint(actualRect.getCenter().x - actualRect.getWidth()*0.5, actualRect.getCenter().y);
		ofPoint rightArrow = ofPoint(actualRect.getCenter().x + actualRect.getWidth()*0.5, actualRect.getCenter().y);

		//Draw DirectionX
		if (lastDirectionX > 0) {//left dir , left arrow triangle
			ofDrawTriangle(topArrow, leftArrow, bottomArrow);
		}
		if (lastDirectionX < 0) {//right dir, right arrow triangle
			ofDrawTriangle(topArrow, rightArrow, bottomArrow);
		}
		

		//Draw Rect
		//ofDrawRectangle(actualRect);
		ofDisableAlphaBlending();
		ofPopStyle();
	}

	////--------------------------------------------
	//void compareActionX(float _newDiffValue, float _thresError) {
	//	if (_newDiffValue > 1 + _thresError)statusActionW = 1;//There is a bigger Width
	//	else if (_newDiffValue < 1 - _thresError)statusActionW = -1;//There is a lower Width
	//	else statusActionW = 0;//regular
	//}
	////--------------------------------------------
	//void compareActionY(float _newDiffValue, float _thresError) {
	//	if (_newDiffValue > 1 + _thresError)statusActionW = 1;//There is a bigger Width
	//	else if (_newDiffValue < 1 - _thresError)statusActionW = -1;//There is a lower Width
	//	else statusActionW = 0;//regular
	//}

	//--------------------------------------------
	void compareDimensionW(float _newDiffValue, float _thresError) {
		if (_newDiffValue > 1 + _thresError)statusActionW = 1;//There is a bigger Width
		else if (_newDiffValue < 1 - _thresError)statusActionW = -1;//There is a lower Width
		else statusActionW = 0;//regular
	}
	//--------------------------------------------
	void compareDimensionH(float _newDiffValue, float _thresError) {
		if (_newDiffValue > 1 + _thresError)statusActionH = 1;//There is a bigger Height
		else if (_newDiffValue < 1 - _thresError)statusActionH = -1;//There is a lower Height
		else statusActionH = 0;//normal
	}
};


