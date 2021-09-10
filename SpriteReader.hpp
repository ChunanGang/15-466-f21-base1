#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "PPU466.hpp"

// A group of sprites, which represent an png picture
struct SpriteGroup{
    PPU466 * ppu; // pointer to the ppu used in the program
    std::vector<uint32_t> sprite_indices; // indices of sprites on the ppu
    uint32_t width, height;   // width/height of the sprite group, with one sprite/tile as a unit
    void draw(int32_t pos_x,int32_t pos_y ); // draw the SpriteGroup in pos (left bottom point)
};

class SpriteReader{

    PPU466 * ppu; 
    uint32_t tile_offset_index = 0; // tiles of the ppu are used before this offset index
    uint32_t sprite_offset_index = 0; // sprites of the ppu are used before this offset index
    uint32_t palette_offset_index = 0; // palettes of the ppu are used before this offset index

public:
    void setPPUPtr(PPU466 * ppu_ptr){ppu = ppu_ptr;}
    // read a png file with "size", and load it into "spriteGroup" for future use
    void getSpriteGroupFromPNG(std::string const & path, glm::uvec2 size, int priority, SpriteGroup & spriteGroup);
    // read a 8x8 png and set it as basic background
    void setBackgroundGeneralSprite(std::string const & path);
    // read a png imag and set it into background in a specified location
    void setPNGIntoBackground(std::string const & path, glm::uvec2 size, glm::uvec2 location );
    // duplicate an existing spriteGroup
    void duplicateSpriteGroup(SpriteGroup const & from, SpriteGroup & to);
};

// helper function that check if the input color is in the Palette
// it will create the color if did not exist 
void addColorToPalette(PPU466::Palette & palatte, int & curPaletteSize, glm::u8vec4 const & color);

// helper function that return the index of the input color in the Palette
int findColorInPalette(PPU466::Palette const & palatte,  glm::u8vec4 const & color);

// helper function that set the specific bit of the tile (8x8 block)
void setTileBits(PPU466::Tile & tile, int row, int col, int colorIndex);