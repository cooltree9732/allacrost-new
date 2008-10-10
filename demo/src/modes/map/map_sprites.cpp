///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_sprites.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode sprites.
*** ***************************************************************************/

// Allacrost utilities
#include "utils.h"

// Allacrost engines
#include "audio.h"
#include "system.h"

// Allacrost globals
#include "global.h"

// Local map mode headers
#include "map.h"
#include "map_sprites.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "map_actions.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;

namespace hoa_map {

namespace private_map {

uint16 CalculateOppositeDirection(const uint16 direction) {
	switch (direction) {
		case NORTH:      return SOUTH;
		case SOUTH:      return NORTH;
		case WEST:       return EAST;
		case EAST:       return WEST;
		case NW_NORTH:   return SE_SOUTH;
		case NW_WEST:    return SE_EAST;
		case NE_NORTH:   return SW_SOUTH;
		case NE_EAST:    return SW_WEST;
		case SW_SOUTH:   return NE_NORTH;
		case SW_WEST:    return NE_EAST;
		case SE_SOUTH:   return NW_NORTH;
		case SE_EAST:    return NW_WEST;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "invalid direction argument: " << direction << endl;
			return SOUTH;
	}
}

// ****************************************************************************
// ********************* VirtualSprite Class Functions ************************
// ****************************************************************************

VirtualSprite::VirtualSprite() :
	direction(SOUTH),
	movement_speed(NORMAL_SPEED),
	moving(false),
	is_running(false),
	current_action(-1),
	forced_action(-1),
	_state_saved(false),
	_saved_direction(0),
	_saved_movement_speed(0.0f),
	_saved_moving(false),
	_saved_current_action(-1)
{
	MapObject::_object_type = VIRTUAL_TYPE;
}



VirtualSprite::~VirtualSprite() {
	for (uint32 i = 0; i < actions.size(); i++) {
		delete actions[i];
	}
	actions.clear();
}



void VirtualSprite::Update() {
	if (!updatable) {
		return;
	}

	// If the sprite was not forced to do a certain action
	if (forced_action < 0) {
		// Execute the sprite's action and if it is finished, update the action counter
		if (current_action >= 0) {
			actions[current_action]->Execute();
			if (actions[current_action]->IsFinishedReset()) {
				current_action++;
				if (static_cast<uint8>(current_action) >= actions.size())
					current_action = 0;
			}
		}
	}

	// The remainder of this function handles movement, so if the sprite is not moving there is nothing left to do
	if (moving == false)
		return;

	// Save the previous sprite's position temporarily
	float tmp_x = x_offset;
	float tmp_y = y_offset;

	float distance_moved = static_cast<float>(MapMode::_current_map->_time_elapsed) / movement_speed;
	// Double the distance to move if the sprite is running
	if (is_running == true)
		distance_moved *= 2.0f;
	// If the movement is diagonal, decrease the lateral movement distance by sin(45 degress)
	if (direction & MOVING_DIAGONALLY)
		distance_moved *= 0.707f;

	// TODO: the code below is very inefficient because first it moves in the y direction and does
	// full collision detection, then it moves in the x direction and does full collision detection.
	// I think we should only do collision detection once per move, not twice.

	// Move the sprite the appropriate distance in the appropriate Y direction
	if (direction & (NORTH | MOVING_NORTHWEST | MOVING_NORTHEAST))
		y_offset -= distance_moved;
	else if (direction & (SOUTH | MOVING_SOUTHWEST | MOVING_SOUTHEAST))
		y_offset += distance_moved;

	// Determine if the sprite may move to this new Y position
	if (MapMode::_current_map->_object_manager->DetectCollision(this)) {
		// Determine if we can slide on an object
		if( direction & (SOUTH | NORTH)) {
			//Start from a sprite's size away and get closer testing collision each time
			for( float i = 0; i < coll_half_width * 2; i += 0.1f ) {
				x_offset = tmp_x - ( coll_half_width * 2 ) + i;
				if (MapMode::_current_map->_object_manager->DetectCollision(this)) {
					//Try the other way, can't go that way
					x_offset = tmp_x + ( coll_half_width * 2 ) - i;
					if (MapMode::_current_map->_object_manager->DetectCollision(this)) {
						//Still can't slide, reset
						x_offset = tmp_x;
					}
					else {
						x_offset = tmp_x + distance_moved;
						break;
					}
				}
				else {
					x_offset = tmp_x - distance_moved;
					break;
				}
			}

			// Roll-over X position offsets if necessary
			while (x_offset < 0.0f) {
				x_position -= 1;
				x_offset += 1.0f;
			}
			while (x_offset > 1.0f) {
				x_position += 1;
				x_offset -= 1.0f;
			}
		}

		y_offset = tmp_y;

	}
	else {
		// Roll-over Y position offsets if necessary
		while (y_offset < 0.0f) {
			y_position -= 1;
			y_offset += 1.0f;
		}
		while (y_offset > 1.0f) {
			y_position += 1;
			y_offset -= 1.0f;
		}
	}

	// Move the sprite the appropriate distance in the appropriate X direction
	if (direction & (WEST | MOVING_NORTHWEST | MOVING_SOUTHWEST))
		x_offset -= distance_moved;
	else if (direction & (EAST | MOVING_NORTHEAST | MOVING_SOUTHEAST))
		x_offset += distance_moved;

	// Determine if the sprite may move to this new X position
	if (MapMode::_current_map->_object_manager->DetectCollision(this)) {
		// Determine if we can slide on an object
		if( direction & (WEST | EAST)) {
			//Start from a sprite's size away and get closer testing collision each time
			for( float i = 0; i < coll_height; i += 0.1f ) {
				y_offset = tmp_y - coll_height + i;
				if (MapMode::_current_map->_object_manager->DetectCollision(this)) {
					//Try the other way, can't go that way
					y_offset = tmp_y + coll_height - i;
					if (MapMode::_current_map->_object_manager->DetectCollision(this)) {
						//Still can't slide, reset
						y_offset = tmp_y;
					}
					else {
						y_offset = tmp_y + distance_moved;
						break;
					}
				}
				else {
					y_offset = tmp_y - distance_moved;
					break;
				}
			}

			// Roll-over Y position offsets if necessary
			while (y_offset < 0.0f) {
				y_position -= 1;
				y_offset += 1.0f;
			}
			while (y_offset > 1.0f) {
				y_position += 1;
				y_offset -= 1.0f;
			}
		}

		x_offset = tmp_x;
	}
	else {
		// Roll-over X position offsets if necessary
		while (x_offset < 0.0f) {
			x_position -= 1;
			x_offset += 1.0f;
		}
		while (x_offset > 1.0f) {
			x_position += 1;
			x_offset -= 1.0f;
		}
	}
} // void VirtualSprite::Update()



void VirtualSprite::SetDirection(uint16 dir) {
	// Nothing complicated needed for lateral directions
	if (dir & (NORTH | SOUTH | EAST | WEST)) {
		direction = dir;
	}
	// Otherwise if the direction is diagonal we must figure out which way the sprite should face.
	else if (dir & MOVING_NORTHWEST) {
		if (direction & (FACING_NORTH | FACING_EAST))
			direction = NW_NORTH;
		else
			direction = NW_WEST;
	}
	else if (dir & MOVING_SOUTHWEST) {
		if (direction & (FACING_SOUTH | FACING_EAST))
			direction = SW_SOUTH;
		else
			direction = SW_WEST;
	}
	else if (dir & MOVING_NORTHEAST) {
		if (direction & (FACING_NORTH | FACING_WEST))
			direction = NE_NORTH;
		else
			direction = NE_EAST;
	}
	else if (dir & MOVING_SOUTHEAST) {
		if (direction & (FACING_SOUTH | FACING_WEST))
			direction = SE_SOUTH;
		else
			direction = SE_EAST;
	}
	else {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to set an invalid direction: " << dir << endl;
	}
} // void VirtualSprite::SetDirection(uint16 dir)



void VirtualSprite::SetRandomDirection() {
	switch (RandomBoundedInteger(1, 8)) {
		case 1:
			SetDirection(NORTH);
			break;
		case 2:
			SetDirection(SOUTH);
			break;
		case 3:
			SetDirection(EAST);
			break;
		case 4:
			SetDirection(WEST);
			break;
		case 5:
			SetDirection(MOVING_NORTHEAST);
			break;
		case 6:
			SetDirection(MOVING_NORTHWEST);
			break;
		case 7:
			SetDirection(MOVING_SOUTHEAST);
			break;
		case 8:
			SetDirection(MOVING_SOUTHWEST);
			break;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "invalid randomized direction was chosen" << endl;
	}
}



void VirtualSprite::SaveState() {
	_state_saved = true;
	_saved_direction = direction;
	_saved_movement_speed = movement_speed;
	_saved_moving = moving;
	_saved_current_action = current_action;
}



void VirtualSprite::RestoreState() {
	if (_state_saved == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "restoring state when no saved state was detected" << endl;

	_state_saved = false;
	 direction = _saved_direction;
	 movement_speed = _saved_movement_speed;
	 moving = _saved_moving;
	 current_action = _saved_current_action;
}

// ****************************************************************************
// ********** MapSprite Class
// ****************************************************************************

// Constructor for critical class members. Other members are initialized via support functions
MapSprite::MapSprite() :
	_face_portrait(NULL),
	_has_running_animations(false),
	_current_animation(ANIM_STANDING_SOUTH),
	_next_dialogue(-1),
	_has_available_dialogue(false),
	_has_unseen_dialogue(false),
	_dialogue_icon_color(1.0f, 1.0f, 1.0f, 0.0f),
	_saved_was_moving(false),
	_saved_current_animation(0)
{
	MapObject::_object_type = SPRITE_TYPE;
}


// Free all allocated images and other data
MapSprite::~MapSprite() {
	if (_face_portrait != NULL) {
		delete _face_portrait;
		_face_portrait = NULL;
	}

	// Free _animations
	for (vector<AnimatedImage>::iterator i = _animations.begin(); i != _animations.end(); ++i)
		(*i).Clear();
}


// Load in the appropriate images and other data for the sprite
bool MapSprite::LoadStandardAnimations(std::string filename) {
	// The speed to display each frame in the walking animation
	uint32 frame_speed = static_cast<uint32>(movement_speed / 10.0f);

	// Prepare the four standing and four walking _animations
	for (uint8 i = 0; i < 8; i++)
		_animations.push_back(AnimatedImage());

	// Load the multi-image, containing 24 frames total
	vector<StillImage> frames(24);
	for (uint8 i = 0; i < 24; i++)
		frames[i].SetDimensions(img_half_width * 2, img_height);

	if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, filename, 4, 6) == false) {
		return false;
	}

	// Add standing frames to _animations
	_animations[ANIM_STANDING_SOUTH].AddFrame(frames[0], frame_speed);
	_animations[ANIM_STANDING_NORTH].AddFrame(frames[6], frame_speed);
	_animations[ANIM_STANDING_WEST].AddFrame(frames[12], frame_speed);
	_animations[ANIM_STANDING_EAST].AddFrame(frames[18], frame_speed);

	// Add walking frames to _animations
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[1], frame_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[2], frame_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[3], frame_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[1], frame_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[4], frame_speed);
	_animations[ANIM_WALKING_SOUTH].AddFrame(frames[5], frame_speed);

	_animations[ANIM_WALKING_NORTH].AddFrame(frames[7], frame_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[8], frame_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[9], frame_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[7], frame_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[10], frame_speed);
	_animations[ANIM_WALKING_NORTH].AddFrame(frames[11], frame_speed);

	_animations[ANIM_WALKING_WEST].AddFrame(frames[13], frame_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[14], frame_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[15], frame_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[13], frame_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[16], frame_speed);
	_animations[ANIM_WALKING_WEST].AddFrame(frames[17], frame_speed);

	_animations[ANIM_WALKING_EAST].AddFrame(frames[19], frame_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[20], frame_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[21], frame_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[19], frame_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[22], frame_speed);
	_animations[ANIM_WALKING_EAST].AddFrame(frames[23], frame_speed);

	return true;
} // bool MapSprite::LoadStandardAnimations(std::string filename)



bool MapSprite::LoadRunningAnimations(std::string filename) {
	// The speed to display each frame in the running animation
	uint32 frame_speed = static_cast<uint32>(movement_speed / 10.0f);

	// Prepare to add the four running _animations
	for (uint8 i = 0; i < 4; i++)
		_animations.push_back(AnimatedImage());

	// Load the multi-image, containing 24 frames total
	vector<StillImage> frames(24);
	for (uint8 i = 0; i < 24; i++)
		frames[i].SetDimensions(img_half_width * 2, img_height);

	if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, filename, 4, 6) == false) {
		return false;
	}

	// Add walking frames to _animations
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[1], frame_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[2], frame_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[3], frame_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[1], frame_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[4], frame_speed);
	_animations[ANIM_RUNNING_SOUTH].AddFrame(frames[5], frame_speed);

	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[7], frame_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[8], frame_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[9], frame_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[7], frame_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[10], frame_speed);
	_animations[ANIM_RUNNING_NORTH].AddFrame(frames[11], frame_speed);

	_animations[ANIM_RUNNING_WEST].AddFrame(frames[13], frame_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[14], frame_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[15], frame_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[13], frame_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[16], frame_speed);
	_animations[ANIM_RUNNING_WEST].AddFrame(frames[17], frame_speed);

	_animations[ANIM_RUNNING_EAST].AddFrame(frames[19], frame_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[20], frame_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[21], frame_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[19], frame_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[22], frame_speed);
	_animations[ANIM_RUNNING_EAST].AddFrame(frames[23], frame_speed);

	_has_running_animations = true;
	return true;
} // bool MapSprite::LoadRunningAnimations(std::string filename)



void MapSprite::LoadFacePortrait(std::string pn) {
	if (_face_portrait != NULL) {
		delete _face_portrait;
	}

	_face_portrait = new StillImage();
	if (_face_portrait->Load(pn) == false) {
		delete _face_portrait;
		_face_portrait = NULL;
		PRINT_ERROR << "failed to load face portrait" << endl;
	}
}


// Updates the state of the sprite
void MapSprite::Update() {
	// Update the alpha of the dialogue icon according to it's distance from the player sprite
	const float DIALOGUE_ICON_VISIBLE_RANGE = 30.0f;
	float icon_alpha = 1.0f - (fabs(ComputeXLocation() - MapMode::_current_map->_camera->ComputeXLocation()) + fabs(ComputeYLocation() -
		MapMode::_current_map->_camera->ComputeYLocation())) / DIALOGUE_ICON_VISIBLE_RANGE;

	if (icon_alpha < 0)
		icon_alpha = 0;
	_dialogue_icon_color.SetAlpha(icon_alpha);
	MapMode::_current_map->_new_dialogue_icon.Update();

	// Set the sprite's animation to the standing still position if movement has just stopped
	if (!moving) {
		if (_was_moving) {
			// Set the current movement animation to zero progress
			_animations[_current_animation].SetTimeProgress(0);
			_was_moving = false;
		}

		// Determine the correct standing frame to display
		if (current_action == -1) {
			if (direction & FACING_NORTH) {
				_current_animation = ANIM_STANDING_NORTH;
			}
			else if (direction & FACING_SOUTH) {
				_current_animation = ANIM_STANDING_SOUTH;
			}
			else if (direction & FACING_WEST) {
				_current_animation = ANIM_STANDING_WEST;
			}
			else if (direction & FACING_EAST) {
				_current_animation = ANIM_STANDING_EAST;
			}
			else {
				cerr << "MAP ERROR: could not find proper standing animation to draw" << endl;
			}
		}
	} // if (!moving)

	// This call will update the sprite's position and perform collision detection
	VirtualSprite::Update();

	if (moving) {
		// Save the previous animation
		uint8 last_animation = _current_animation;

		// Determine the correct animation to display
		if (direction & FACING_NORTH) {
			_current_animation = ANIM_WALKING_NORTH;
		}
		else if (direction & FACING_SOUTH) {
			_current_animation = ANIM_WALKING_SOUTH;
		}
		else if (direction & FACING_WEST) {
			_current_animation = ANIM_WALKING_WEST;
		}
		else if (direction & FACING_EAST) {
			_current_animation = ANIM_WALKING_EAST;
		}
		else {
			cerr << "MAP ERROR: could not find proper movement animation to draw" << endl;
		}

		// Increasing the animation index by four from the walking _animations leads to the running _animations
		if (is_running && _has_running_animations)
			_current_animation += 4;

		// If the direction of movement changed in mid-flight, update the animation timer on the
		// new animated image to reflect the old, so the walking _animations do not appear to
		// "start and stop" whenever the direction is changed.
		if (_current_animation != last_animation) {
			_animations[_current_animation].SetTimeProgress(_animations[last_animation].GetTimeProgress());
			_animations[last_animation].SetTimeProgress(0);
		}
		_animations[_current_animation].Update();

		_was_moving = true;
	} // if (moving)
} // void MapSprite::Update()


// Draw the appropriate sprite frame at the correct position on the screen
void MapSprite::Draw() {
	if (MapObject::ShouldDraw() == true) {
		_animations[_current_animation].Draw();

		if (_has_available_dialogue == true && _has_unseen_dialogue == true && MapMode::_IsShowingDialogueIcons()) {
			VideoManager->MoveRelative(0, -GetImgHeight());
			MapMode::_current_map->_new_dialogue_icon.Draw(_dialogue_icon_color);
		}
	}
}




void MapSprite::AddDialogueReference(uint32 dialogue_id) {
	_dialogue_references.push_back(dialogue_id);
	MapMode::_loading_map->_dialogue_supervisor->AddSpriteReference(dialogue_id, GetObjectID());
}



void MapSprite::UpdateDialogueStatus() {
	_has_available_dialogue = false;
	_has_unseen_dialogue = false;
	
	for (uint32 i = 0; i < _dialogue_references.size(); i++) {
		MapDialogue* dialogue = MapMode::_current_map->_dialogue_supervisor->GetDialogue(_dialogue_references[i]);
		if (dialogue == NULL) {
			IF_PRINT_WARNING(MAP_DEBUG) << "sprite: " << object_id << " is referencing unknown dialogue: " << _dialogue_references[i] << endl;
			continue;
		}
		
		if (dialogue->IsAvailable()) {
			_has_available_dialogue = true;
			if (_next_dialogue < 0)
				_next_dialogue = i;
		}
		if (dialogue->HasAlreadySeen() == false) {
			_has_unseen_dialogue = true;
		}
	}

	// TODO: if the sprite has available, unseen dialogue and the _next_dialogue pointer is pointing to a dialogue that is already seen, change it
	// to point to the unseen available dialogue
}



void MapSprite::IncrementNextDialogue() {
	// Handle the case where no dialogue is referenced by the sprite
	if (_next_dialogue < 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function invoked when no dialogues were referenced by the sprite" << endl;
		return;
	}

	int16 last_dialogue = _next_dialogue;

	while (true) {
		_next_dialogue++;
		if (static_cast<uint16>(_next_dialogue) >= _dialogue_references.size())
			_next_dialogue = 0;

		MapDialogue* dialogue = MapMode::_current_map->_dialogue_supervisor->GetDialogue(_dialogue_references[_next_dialogue]);
		if (dialogue != NULL && dialogue->IsAvailable() == true) {
			return;
		}
		// If this case occurs, all dialogues are now unavailable
		else if (_next_dialogue == last_dialogue) {
			IF_PRINT_WARNING(MAP_DEBUG) << "all referenced dialogues are now unavailable for this sprite" << endl;
			_has_available_dialogue = false;
			_has_unseen_dialogue = false;
			return;
		}
	}
}




void MapSprite::SetNextDialogue(uint16 next) {
	// If a negative value is passed in, this means the user wants to disable
	if (next >= _dialogue_references.size()) {
		IF_PRINT_WARNING(MAP_DEBUG) << "tried to set _next_dialogue to an value that was invalid (exceeds maximum bounds): " << next << endl;
	}
	else {
		_next_dialogue = static_cast<int16>(next);
	}
}



void MapSprite::SaveState() {
	VirtualSprite::SaveState();

	_saved_was_moving = _was_moving;
	_saved_current_animation = _current_animation;
}



void MapSprite::RestoreState() {
	VirtualSprite::RestoreState();

	_was_moving = _saved_was_moving;
	_current_animation = _saved_current_animation;
}

// *****************************************************************************
// ********** EnemySprite Class
// *****************************************************************************

EnemySprite::EnemySprite() :
	_zone(NULL),
	_color(1.0f, 1.0f, 1.0f, 0.0f),
	_aggro_range(8.0f),
	_time_dir_change(2500),
	_time_to_spawn(3500),
	_music_theme("")
{
	filename = "";
	MapObject::_object_type = ENEMY_TYPE;
	moving = true;
	Reset();
}



EnemySprite::EnemySprite(std::string file) :
	_zone(NULL),
	_color(1.0f, 1.0f, 1.0f, 0.0f),
	_aggro_range(8.0f),
	_time_dir_change(2500),
	_time_to_spawn(3500)
{
	filename = file;
	MapObject::_object_type = ENEMY_TYPE;
	moving = true;
	Reset();
}


// Load in the appropriate images and other data for the sprite from a Lua file
bool EnemySprite::Load() {
	ReadScriptDescriptor sprite_script;
	if (sprite_script.OpenFile(filename) == false) {
		return false;
	}

	ScriptCallFunction<void>(sprite_script.GetLuaState(), "Load", this);
	string sprite_sheet = sprite_script.ReadString("sprite_sheet");
	return MapSprite::LoadStandardAnimations(sprite_sheet);
}



void EnemySprite::Reset() {
	updatable = false;
	no_collision = true;
	_state = DEAD;
	_time_elapsed = 0;
	_color.SetAlpha(0.0f);
	_out_of_zone = false;
}



void EnemySprite::AddEnemy(uint32 enemy_id) {
	if (_enemy_parties.empty()) {
		IF_PRINT_WARNING(MAP_DEBUG) << "can not add new enemy when no parties have been declared" << endl;
		return;
	}

	_enemy_parties.back().push_back(enemy_id);

	// Make sure that the GlobalEnemy has already been created for this enemy_id
	if (MAP_DEBUG) {
		bool found = false;
		for (uint32 i = 0; i < MapMode::_loading_map->_enemies.size(); i++) {
			if (MapMode::_loading_map->_enemies[i]->GetID() == enemy_id) {
				found = true;
				break;
			}
		}

		if (found == false) {
			IF_PRINT_WARNING(MAP_DEBUG) << "enemy to add has id " << enemy_id << ", which does not exist in MapMode::_enemies" << endl;
		}
	}
}



const std::vector<uint32>& EnemySprite::RetrieveRandomParty() {
	if (_enemy_parties.empty()) {
		PRINT_ERROR << "call invoked when no enemy parties existed" << endl;
		exit(1);
	}

	return _enemy_parties[rand() % _enemy_parties.size()];
}



void EnemySprite::Update() {
	if (current_action != -1) {
		MapSprite::Update();
		return;
	}

	switch (_state) {
		// Gradually increase the alpha while the sprite is fading in during spawning
		case SPAWNING:
			_time_elapsed += SystemManager->GetUpdateTime();
			if (_color.GetAlpha() < 1.0f) {
				_color.SetAlpha((_time_elapsed / static_cast<float>(_time_to_spawn)) * 1.0f);
			}
			else {
				ChangeStateHostile();
			}
			break;

		// Set the sprite's direction so that it seeks to collide with the map camera's position
		case HOSTILE:
			// Holds the x and y deltas between the sprite and map camera coordinate pairs
			float xdelta, ydelta;
			_time_elapsed += SystemManager->GetUpdateTime();

			xdelta = ComputeXLocation() - MapMode::_current_map->_camera->ComputeXLocation();
			ydelta = ComputeYLocation() - MapMode::_current_map->_camera->ComputeYLocation();

			// If the sprite has moved outside of its zone and it should not, reverse the sprite's direction
			if ( _zone != NULL && _zone->IsInsideZone(x_position, y_position) == false && _zone->IsRestrained() ) {
				// Make sure it wasn't already out (stuck on boundaries fix)
				if( !_out_of_zone )
				{
					SetDirection(CalculateOppositeDirection(GetDirection()));
					// The sprite is now finding its way back into the zone
					_out_of_zone = true;
				}
			}
			// Otherwise, determine the direction that the sprite should move if the camera is within the sprite's aggression range
			else {
				_out_of_zone = false;

				// Enemies will only aggro if the camera is inside the zone, or the zone is non-restrictive
				// The order of comparaisons here is important, the NULL check MUST come before the rest or a null pointer exception could happen if no zone is registered
				if ( _zone == NULL || ( fabs(xdelta) <= _aggro_range && fabs(ydelta) <= _aggro_range
					 && (!_zone->IsRestrained() || _zone->IsInsideZone(MapMode::_current_map->_camera->x_position, MapMode::_current_map->_camera->y_position)) ) )
				{
					if (xdelta > -0.5 && xdelta < 0.5 && ydelta < 0)
						SetDirection(SOUTH);
					else if (xdelta > -0.5 && xdelta < 0.5 && ydelta > 0)
						SetDirection(NORTH);
					else if (ydelta > -0.5 && ydelta < 0.5 && xdelta > 0)
						SetDirection(WEST);
					else if (ydelta > -0.5 && ydelta < 0.5 && xdelta < 0)
						SetDirection(EAST);
					else if (xdelta < 0 && ydelta < 0)
						SetDirection(MOVING_SOUTHEAST);
					else if (xdelta < 0 && ydelta > 0)
						SetDirection(MOVING_NORTHEAST);
					else if (xdelta > 0 && ydelta < 0)
						SetDirection(MOVING_SOUTHWEST);
					else
						SetDirection(MOVING_NORTHWEST);
				}
				// If the sprite is not within the aggression range, pick a random direction to move
				else {
					if (_time_elapsed >= GetTimeToChange()) {
						// TODO: needs comment
						SetDirection(1 << hoa_utils::RandomBoundedInteger(0,11));
						_time_elapsed = 0;
					}
				}
			}

			MapSprite::Update();
			break;

		// Do nothing if the sprite is in the DEAD state, or any other state
		case DEAD:
		default:
			break;
	}
} // void EnemySprite::Update()



void EnemySprite::Draw() {
	// Otherwise, only draw it if it is not in the DEAD state
	if (MapObject::ShouldDraw() == true && _state != DEAD) {
		_animations[_current_animation].Draw(_color);
		return;
	}
}

} // namespace private_map

} // namespace hoa_map
