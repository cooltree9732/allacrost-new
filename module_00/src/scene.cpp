/* 
 * scene.cpp
 *  Code for Hero of Allacrost scene mode
 *  (C) 2005 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */

#include <iostream>
#include "scene.h"

using namespace std;
using namespace hoa_global;
using namespace hoa_video;
using namespace hoa_pause;
using namespace hoa_scene::local_scene;

namespace hoa_scene {

SceneMode::SceneMode() {
	mtype = scene_m;
	input = &(SettingsManager->InputStatus);
	scene_timer = 0;
	
	// setup the scene Image Descriptor
	
	// VideoManager->LoadImage(scene);
}



// The destructor frees up our scene image
SceneMode::~SceneMode() { 
  // VideoManager->FreeImage(scene);
}



// Restores volume or unpauses audio, then pops itself from the game stack 
void SceneMode::Update(Uint32 time_elapsed) {
	scene_timer += time_elapsed;
	
	if (input->pause_press) {
	  PauseMode *PM = new PauseMode();
		ModeManager->Push(PM);
	}
	
	// User must wait 0.75 seconds before they can exit the scene
	else if ((input->confirm_press || input->cancel_press) && scene_timer < MIN_SCENE_UPDATES) {
		ModeManager->Pop();
	}
}



// Draws the scene
void SceneMode::Draw() { 
// 	Draw the scene, maybe with a filter that lets it fade in and out....?
}

} // namespace hoa_scene
