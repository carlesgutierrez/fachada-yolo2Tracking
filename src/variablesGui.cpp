#include "variablesGui.h"

// SINGLETON initalizations
bool variablesGui::instanceFlag = false;
variablesGui* variablesGui::single = NULL;

//----------------------------------------------

variablesGui* variablesGui::getInstance()
{
	if (!instanceFlag)
	{
		single = new variablesGui();
		instanceFlag = true;
		return single;
	}
	else {
		return single;
	}
}

//----------------------------------------------
variablesGui::variablesGui()
{

}
//----------------------------------------------
variablesGui::~variablesGui()
{}

