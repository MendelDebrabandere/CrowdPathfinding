#pragma once
// Here we determine which application is currently active
// Create the define here using the "ActiveApp_..." convention and set up the correct include and typedef in the #ifdef below.

#define ActiveApp_ResearchTopic

//---------- Registered Applications -----------

#ifdef ActiveApp_ResearchTopic
#include "projects/ResearchTopic/App_ResearchTopic.h"
typedef Elite::App_ResearchTopic CurrentApp;
#endif


class App_Selector {
public:  
	static IApp* CreateApp() {
		IApp* myApp = new CurrentApp();
		return myApp;
	}
};