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
	// load the second picture for player (playerSprite2), but set invisible for now
	spriteReader.getSpriteGroupFromPNG("../player2.png", 0, playerSprite2);
	playerSprite2.setInVisible();

	// read a bullet.png into the bullet spriteGroup
	spriteReader.getSpriteGroupFromPNG("../bullet.png", 0, bullet);
	// if the spriteGroups are using the same image, we can call duplicateSpriteGroup 
	spriteReader.duplicateSpriteGroup(bullet, bullet2);
	spriteReader.duplicateSpriteGroup(bullet, bullet3);

	// ---- backgrounds ---- //
	// read a 8x8 png and set it the basic background (covers the whole background)
	spriteReader.setBackgroundGeneralSprite("../back.png");

	// read a png imag and set it into background in a specified location
	// In this case, we are reading a head.png and put in into
	//  the background in location [20,20] from left bot, with tile as the unit. 
	//  (note the whole background is 64x06)
	spriteReader.setPNGIntoBackground("../head.png", glm::uvec2(20,20));
	// another png into background
	spriteReader.setPNGIntoBackground("../house.png", glm::uvec2(5,10));
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			playerSprite.setVisible();
			playerSprite2.setInVisible();
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			playerSprite2.setVisible();
			playerSprite.setInVisible();
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}

	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	constexpr float PlayerSpeed = 30.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 2.0f / 3.0f) ) ) ))),
		0xff
	);

	// draw the spriteGround
	playerSprite.draw(int32_t(player_at.x),int32_t(player_at.y));	// one of these two playerSprites is set invisible
	playerSprite2.draw(int32_t(player_at.x),int32_t(player_at.y));
	bullet.draw(int32_t(player_at.x + 50),int32_t(player_at.y + 50));
	bullet2.draw(int32_t(player_at.x + 60),int32_t(player_at.y + 60));
	bullet3.draw(int32_t(player_at.x + 70),int32_t(player_at.y + 70));

	//--- actually draw ---
	ppu.draw(drawable_size);
}
