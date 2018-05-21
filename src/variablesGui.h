#pragma once

#include "ofMain.h"

class variablesGui {

	// variables & methods for singleton
private:
	static bool	instanceFlag;
	static variablesGui *single;
public:

public:
	static variablesGui* getInstance();
	// end singleton


public:

	variablesGui();
	~variablesGui();

	float directionXTrheshold = 2;
	

	//Show Vars
	float directionX;
};


