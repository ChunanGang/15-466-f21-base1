#include "PPU466.hpp"
#include "Mode.hpp"
#include "SpriteReader.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

using namespace glm;
using namespace std;

#define MAX_BULLETS ( 10 )

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//----- input state -----
	bool Mouse_Left = false;
	bool W_pressed = false;
	bool A_pressed = false;
	bool S_pressed = false;
	bool D_pressed = false;

	//----- game setting -----
	float background_fade = 0.5f;
	vec2 court_radius = vec2(256.0f, 240.0f);
	vec2 bullet_radius = vec2(4.0f, 4.0f);
	vec2 player_radius = vec2(8.0f, 8.0f);
	vec2 monster_radius = vec2(8.0f, 8.0f);

	float player_speed = 64.0f;
	float monster_speed = 128.0f;
	float monster_accel = 64.0f;
	float bullet_speed = 128.0f;

	float shoot_interval = 0.5f;

	//----- game state -----
	vec2 pos_player = court_radius / 2.0f;

	vec2 pos_monster = (vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX)) * court_radius;
	float vel_monster = 0.0f;

	vec2 pos_target;

	float shoot_timer = 0.0f;
	float bullet_accel = 64.0f;

	bool monster_visible = true;
	bool player_visible = true;
	
	//----- bullet instance -----
	struct Bullet {
		Bullet(
			vec2 const& Position_, vec2 const& Direction_) :
			Position(Position_), Direction(Direction_), isActive(true){ }
		vec2 Position;
		vec2 Direction;
		bool isActive;
	};

	deque<Bullet*> bullets;

	//----- score and time -----
	uint player_health = 3;
	uint monster_health = 10;

	float player_reset_time = 1.0f;
	float monster_reset_time = 0.3f;
	float player_reset_timer = 0.0f;
	float monster_reset_timer = 0.0f;

	float player_flick_time = 0.05f;
	float player_flick_timer = 0.0f;

	float monster_flick_time = 0.05f;
	float monster_flick_timer = 0.0f;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;

	// sprite reader to read from png
	SpriteReader spriteReader;

	// character
	SpriteGroup playerSprite;

	// monster
	SpriteGroup monsterSprite;

	// target
	SpriteGroup targetSprite;

	//SpriteGroup bullet;
	SpriteGroup bulletSprite[MAX_BULLETS];
	// !!!!! REMEMBER !!!! only 64 sprites available 
};
