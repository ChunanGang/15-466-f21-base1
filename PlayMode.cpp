#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

PlayMode::PlayMode() {
	
	// link the using ppu to the sprite reader
	spriteReader.setPPUPtr(&ppu);

	// ---- character sprites ---- //
	// read a player.png into playerSprite. Which can be draw directly later. 
	spriteReader.getSpriteGroupFromPNG("../player.png", 0, playerSprite);

	// load the second picture for monster
	spriteReader.getSpriteGroupFromPNG("../player2.png", 0, monsterSprite);

	//spriteReader.getSpriteGroupFromPNG("../head.png", 0, targetSprite);

	// read a bullet.png into the bullet spriteGroup
	spriteReader.getSpriteGroupFromPNG("../bullet.png", 0, bulletSprite[0]);
	
	// if the spriteGroups are using the same image, we can call duplicateSpriteGroup
	for(int i = 1; i < MAX_BULLETS; i++) {
		spriteReader.duplicateSpriteGroup(bulletSprite[0], bulletSprite[i]);
	} 

	// ---- backgrounds ---- //
	// read a 8x8 png and set it the basic background (covers the whole background)
	spriteReader.setBackgroundGeneralSprite("../back.png");

	// read a png imag and set it into background in a specified location
	// In this case, we are reading a head.png and put in into
	//  the background in location [20,20] from left bot, with tile as the unit. 
	//  (note the whole background is 64x06)
	//spriteReader.setPNGIntoBackground("../head.png", glm::uvec2(20,20));
	// another png into background
	//spriteReader.setPNGIntoBackground("../house.png", glm::uvec2(5,10));
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	switch (evt.type) {

		//keyboard up
		case SDL_KEYUP:
			//cout << "KeyUp" << endl;
			switch (evt.key.keysym.sym) {
				case SDLK_a:
					A_pressed = false;
					break;
				case SDLK_d:
					D_pressed = false;
					break;
				case SDLK_w:
					W_pressed = false;
					break;
				case SDLK_s:
					S_pressed = false;
					break;
				default:
					break;
			}
			break;

		//keyboard down
		case SDL_KEYDOWN:
			//cout << "KeyDown" << endl;
			switch (evt.key.keysym.sym) {
				case SDLK_a:
					A_pressed = true;
					break;
				case SDLK_d:
					D_pressed = true;
					break;
				case SDLK_w:
					W_pressed = true;
					break;
				case SDLK_s:
					S_pressed = true;
					break;
				default:
					break;
			}
			break;

		//mouse motion
		case SDL_MOUSEMOTION:
			pos_target = vec2(
				(float)(evt.motion.x) / window_size.x,
				(float)(evt.motion.y) / window_size.y * -1.0f + 1.0f
			) * court_radius;
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (evt.button.button == SDL_BUTTON_LEFT) {
				Mouse_Left = true;
			}			
			break;

		case SDL_MOUSEBUTTONUP:
			if (evt.button.button == SDL_BUTTON_LEFT) {
				Mouse_Left = false;
			}
			break;

		default:
			break;
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//----- player movement -----

	if (W_pressed) {
		pos_player.y += player_speed * elapsed;
		pos_player.y = std::min(pos_player.y, court_radius.y - player_radius.y);
	}

	if (A_pressed) {
		pos_player.x -= player_speed * elapsed;
		pos_player.x = std::max(pos_player.x, player_radius.x);
	}

	if (S_pressed) {
		pos_player.y -= player_speed * elapsed;
		pos_player.y = std::max(pos_player.y, player_radius.y);
	}

	if (D_pressed) {
		pos_player.x += player_speed * elapsed;
		pos_player.x = std::min(pos_player.x, court_radius.x - player_radius.x);
	}
	

	//----- shoot handling -----

	if (shoot_timer > 0.0f)
		shoot_timer -= elapsed;

	if (Mouse_Left && shoot_timer <= 0.0f) {
		bullets.push_back(new Bullet(pos_player, normalize(pos_target - pos_player)));
		if (bullets.size() > MAX_BULLETS) {
			bullets.pop_front();
		}
		shoot_timer = shoot_interval;
	}

	//----- update bullets -----
	for (int i = 0; i < bullets.size(); i++) {
		if (bullets[i]->isActive) {

			//position update
			bullets[i]->Position += bullets[i]->Direction * bullet_speed * elapsed;

			//wall collision
			if (
				bullets[i]->Position.y > court_radius.y - bullet_radius.y ||
				bullets[i]->Position.y < bullet_radius.y ||
				bullets[i]->Position.x > court_radius.x - bullet_radius.x ||
				bullets[i]->Position.x < bullet_radius.x
			) {
				bullets[i]->isActive = false;
			}

			//monster bullet collision
			if (
				bullets[i]->Position.y + bullet_radius.y > pos_monster.y - monster_radius.y &&
				bullets[i]->Position.y - bullet_radius.y < pos_monster.y + monster_radius.y &&
				bullets[i]->Position.x + bullet_radius.x > pos_monster.x - monster_radius.x &&
				bullets[i]->Position.x - bullet_radius.x < pos_monster.x + monster_radius.x 
			) {
				monster_reset_timer = monster_reset_time;
				vel_monster -= bullet_accel;
				bullets[i]->isActive = false;
				monster_health--;
			}	
		}
	}

	//----- monster player collision -----
	if (
		pos_player.y + player_radius.y > pos_monster.y - monster_radius.y &&
		pos_player.y - player_radius.y < pos_monster.y + monster_radius.y &&
		pos_player.x + player_radius.x > pos_monster.x - monster_radius.x &&
		pos_player.x - player_radius.x < pos_monster.x + monster_radius.x &&
		player_reset_timer == 0.0f
	){
		vel_monster = 0.0f;
		player_reset_timer = player_reset_time;
		player_health--;
	}

	//----- monster movement -----
	vel_monster += monster_accel * elapsed;
	vel_monster = std::clamp(vel_monster, 0.0f, monster_speed);
	pos_monster += vel_monster * elapsed * normalize(pos_player - pos_monster);

	// ----- update timer -----
	player_reset_timer -= elapsed;
	player_reset_timer = std::max(player_reset_timer, 0.0f);

	monster_reset_timer -= elapsed;
	monster_reset_timer = std::max(monster_reset_timer, 0.0f);

	monster_flick_timer -= elapsed;
	monster_flick_timer = std::max(monster_flick_timer, 0.0f);

	player_flick_timer -= elapsed;
	player_flick_timer = std::max(player_flick_timer, 0.0f);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	// set visibility
	if (player_reset_timer > 0.0f) {
		if (player_flick_timer == 0.0f) {
			player_flick_timer = player_flick_time;
			if (player_visible) {
				playerSprite.setInVisible();
				player_visible = false;
			} else {
				playerSprite.setVisible();
				player_visible = true;
			}
		}
	} else {
		playerSprite.setVisible();
	}

	if (monster_reset_timer > 0.0f) {
		if (monster_flick_timer == 0.0f) {
			monster_flick_timer = monster_flick_time;
			if (monster_visible) {
				monsterSprite.setInVisible();
				monster_visible = false;
			} else {
				monsterSprite.setVisible();
				monster_visible = true;
			}
		}
	} else {
		monsterSprite.setVisible();
	}

	// draw the spriteGround
	playerSprite.draw(int32_t(pos_player.x - player_radius.x), int32_t(pos_player.y - player_radius.y));

	monsterSprite.draw(int32_t(pos_monster.x - monster_radius.x), int32_t(pos_monster.y - monster_radius.y));

	//bullets
	for (int i = 0; i < bullets.size(); i++) {
		if (bullets[i]->isActive) {
			bulletSprite[i].setVisible();
			bulletSprite[i].draw(int32_t(bullets[i]->Position.x - bullet_radius.x), int32_t(bullets[i]->Position.y - bullet_radius.y));
		} else {
			bulletSprite[i].setInVisible();
		}
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
