#pragma once

#include "ofMain.h"
#include "statsRecorder.h"
#include "ofxCv.h"

//trackerAnalizer -> performe Box a simple behauoviour detection ( Duck, Jump, Thin, Thick )
class trackerAnalizer : public ofxCv::RectFollower { //RectFollower works but not smoothing { //Want to use ObjectFinder but I need examples
//RectFollower -> setup we save original position, Then Update make comparisons. Then Kill remove them (dead = true).

protected:

	ofRectangle savedRect;
	ofRectangle actualRect;
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
	float smoothingRate = 0.5;
	int age = 0;

public:
	//age counter
	int getAge() { return age; }
	int getLabel() { return label; }

	//Smoothing
	cv::Rect smoothed;
	bool bSmoothing = true;//TODO Activate / deactivate?

	bool bLost = false;
	//extra actions
	float bJump;
	float bDuck;
	float bThin;
	float bThick;

	trackerAnalizer() /*:smoothingRate(.5)*/ {
		
	}
	//void setSmoothingRate(float _smoothingRate) {
	//	smoothingRate = _smoothingRate;
	//}
	//float getSmoothingRate() const {
	//	return smoothingRate;
	//}
	void setup(const cv::Rect& track) {
		savedRect = ofxCv::toOf(track); //Make a copy at start. TODO, make refinement during udpate
		stableRect = savedRect;
		smoothed = track;
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
		if (bSmoothing) {
			smoothed.x = ofLerp(smoothed.x, cur.x, smoothingRate);
			smoothed.y = ofLerp(smoothed.y, cur.y, smoothingRate);
			smoothed.width = ofLerp(smoothed.width, cur.width, smoothingRate);
			smoothed.height = ofLerp(smoothed.height, cur.height, smoothingRate);
		}
		else {
			smoothed = cur;
		}

		//My update process
		actualRect = ofxCv::toOf(smoothed/*cur*/);
		updateStableRect(actualRect, 0.1, 100);
		///////////////////////
		// Compare actualReck with savedRect
		float errorThreshold = 0.20;
		//Make Comparisons
		float diffWidthNorm = ofMap(actualRect.getWidth(), 0, savedRect.width, 0, 1);
		compareDimensionW(diffWidthNorm, 0.2);
		float diffHeightNorm = ofMap(actualRect.getHeight(), 0, savedRect.height, 0, 1);
		compareDimensionH(diffHeightNorm, 0.2);
		//cout << "label= " << label << " bLost = " << bLost << endl;
	}
	void kill() {
		bLost = true;
		dead = true;//Required true to erase it. Otherwise will never die. Check Virtual Kill func
		//cout << "Nothing special for now" << endl;
	}

	void draw() {
		ofPushStyle();
		ofEnableAlphaBlending();
		float alphaRects = 255;
		ofSetLineWidth(4);
		//Colors for Width Analisys
		if (bThick)ofSetColor(colorThick, alphaRects);
		else if (bThin)ofSetColor(colorThick, alphaRects);
		else {
			ofSetLineWidth(2);
			ofSetColor(colorNormal, alphaRects);
		}
	
		//TODO CHECK HIGHT LIGHT TEXT for LABEL and ID track and Thickness
		ofDrawLine(actualRect.getTopLeft().x, actualRect.getTopLeft().y, actualRect.getTopRight().x, actualRect.getTopRight().y);

		//Colors for height Analisys
		ofSetLineWidth(4);
		if (bLost)ofSetColor(ofColor::black, alphaRects);
		else if (bJump)ofSetColor(colorJump, alphaRects);
		else if (bDuck)ofSetColor(colorDuck, alphaRects);
		else {
			ofSetLineWidth(2);
			ofSetColor(colorNormal, alphaRects);
		}
		ofDrawLine(actualRect.getTopLeft().x, actualRect.getTopLeft().y, actualRect.getBottomLeft().x, actualRect.getBottomLeft().y);


		//Draw Rect
		//ofDrawRectangle(actualRect);
		ofDisableAlphaBlending();
		ofPopStyle();
	}

	//--------------------------------------------
	void compareDimensionW(float _newDiffValue, float _thresError) {
		if (_newDiffValue > 1 + _thresError) {
			//There is a bigger Width
			bThin = false;
			bThick = true;
		}
		else if (_newDiffValue < 1 - _thresError) {
			//There is a lower Width
			bThin = true;
			bThick = false;
		}
		else {
			//regular
			bThin = false;
			bThick = false;
		}
	}
	//--------------------------------------------
	void compareDimensionH(float _newDiffValue, float _thresError) {
		if (_newDiffValue > 1 + _thresError) {
			//There is a bigger Width
			bJump = true;
			bDuck = false;
		}
		else if (_newDiffValue < 1 - _thresError) {
			//There is a lower Width
			bJump = false;
			bDuck = true;
		}
		else {
			//regular
			bJump = false;
			bDuck = false;
		}
	}
};


