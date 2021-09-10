#include "SpriteReader.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"
#include <iostream>

// ---------- character sprite --------------- //

void SpriteReader::getSpriteGroupFromPNG(std::string const & path, int priority, SpriteGroup & spriteGroup){
    
    assert(palette_offset_index < 8); // 8 available palettes

    // read png into vector - pic
    glm::uvec2 size;
    std::vector< glm::u8vec4 > pic;
    load_png( data_path(path), &size, &pic, LowerLeftOrigin); //TODO add try-catch block

    // set up palette for this png 
    // !!! We assume the input png has no more than 4 colors !!!
    // So all tiles use the same palatte
    // --- set up palette ---
    PPU466::Palette & palatte = ppu->palette_table[palette_offset_index];
    {
        int size = 0;
        for(auto color : pic){
            // add color to palette until full
            addColorToPalette(palatte, size, color);
            if (size == 4)
                break;
        }
    }

    // calculate size of the png, in terms of tiles
    uint32_t width = size.x / 8;
    uint32_t height = size.y / 8;
    spriteGroup.width = width;
    spriteGroup.height = height;
    spriteGroup.ppu = ppu;

    // go over each tile (8x8 block) in the png, add them into ppu 
    for(uint32_t i =0; i < height; i++){
        for(uint32_t j=0; j< width; j++){ 
            // --- for each tile (8x8 block) ---

            PPU466::Tile & curTile = ppu->tile_table[tile_offset_index];
            // set all 8x8 bits of the current tile
            for (uint32_t y=0; y < 8; y++){
                for (uint32_t x=0; x < 8; x++){
                    // find the color of this bit, and convert into palatte index
                    int indexInPic = (i*8 + y) * size.x + (j*8 + x);
                    int colorIndex = findColorInPalette(palatte, pic[indexInPic]);
                    // set this bit 
                    setTileBits(curTile, y, x, colorIndex);
                }
            }

            // setup a sprite from the tile/palette
            ppu->sprites[sprite_offset_index].index = tile_offset_index;
            uint8_t attributes = 0;
            attributes |= priority << 7;    // setup priority bit
            attributes |= palette_offset_index; // set palatte index
            ppu->sprites[sprite_offset_index].attributes = attributes;
            // add the sprite into spriteGroup
            spriteGroup.sprite_indices.push_back(sprite_offset_index);

            // done with current tile. increment tile offset & sprite offset
            tile_offset_index += 1;
            sprite_offset_index += 1;
        }
    }

    // done with this png, incre palette_offset_index
    palette_offset_index += 1;
    std::cout<< "Palatte used: " << palette_offset_index <<std::endl;
}

// duplicate an existing spriteGroup
void SpriteReader::duplicateSpriteGroup(SpriteGroup const & from, SpriteGroup & to){
    // basic copy
    to.width = from.width;
    to.height = from.height;
    to.ppu = from.ppu;
    // add new sprites to ppu 
    for(uint32_t i =0; i < to.height; i++){
        for(uint32_t j=0; j< to.width; j++){ 
            int fromSpriteGroupIndex = from.sprite_indices[i * from.width + j];
            // copy data from the "from" spriteGroup
            ppu->sprites[sprite_offset_index].attributes = ppu->sprites[fromSpriteGroupIndex].attributes;
            ppu->sprites[sprite_offset_index].index = ppu->sprites[fromSpriteGroupIndex].index;
            // add the sprite into the "to" spriteGroup
            to.sprite_indices.push_back(sprite_offset_index);
            // increment sprite offset
            sprite_offset_index += 1;
        }
    }
}

// helper function that check if the input color is in the Palette
// it will create the color if did not exist 
void addColorToPalette(PPU466::Palette & palatte, int & curPaletteSize, glm::u8vec4 const & color){
    // check within the size
    for(int i =0; i< curPaletteSize; i++){
        if(palatte[i] == color)
            return ;
    }
    // not exist, create
    assert(curPaletteSize <= 3); // at most 4 colors in one png
    palatte[curPaletteSize] = color;
    curPaletteSize += 1;
}

// helper function that return the index of the input color in the Palette
int findColorInPalette(PPU466::Palette const & palatte,  glm::u8vec4 const & color){
    // check within the size
    for(int i =0; i< 4; i++){
        if(palatte[i] == color)
            return i;
    }
    // !!! return the first color if it does not exist
    return 0;
}

// helper function that set the specific bit of the tile (8x8 block)
void setTileBits(PPU466::Tile & tile, int row, int col, int colorIndex){
    int bit0 = colorIndex % 2;
    int bit1 = colorIndex >> 1;

    // first reset bit to 0
    tile.bit0[row] &= ~(1UL << col);
    // set to actual bit
    tile.bit0[row] |= bit0 << col;

    // first reset bit to 0
    tile.bit1[row] &= ~(1UL << col);
    // set to actual bit
    tile.bit1[row] |= bit1 << col;
}

// ---------- background sprite --------------- //

// read a 8x8 png and set it as basic background
void SpriteReader::setBackgroundGeneralSprite(std::string const & path){

    // read png into vector - pic
    std::vector< glm::u8vec4 > pic;
    glm::uvec2 size(8,8);
    load_png( data_path(path), &size, &pic, LowerLeftOrigin); //TODO add try-catch block

    // set up palette for this png 
    // !!! We assume the input png has no more than 4 colors !!!
    // --- set up palette ---
    PPU466::Palette & palatte = ppu->palette_table[palette_offset_index];
    {
        int size = 0;
        for(auto color : pic){
            // add color to palette until full
            addColorToPalette(palatte, size, color);
            if (size == 4)
                break;
        }
    }

    // set the tile into ppu
    PPU466::Tile & curTile = ppu->tile_table[tile_offset_index];
    for (uint32_t y=0; y < 8; y++){
        for (uint32_t x=0; x < 8; x++){
            // find the color of this bit, and convert into palatte index
            int indexInPic =  y * 8 + x;
            int colorIndex = findColorInPalette(palatte, pic[indexInPic]);
            // set this bit 
            setTileBits(curTile, y, x, colorIndex);
        }
    }

    // link the tile with background
    for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu->background[x+PPU466::BackgroundWidth*y] = tile_offset_index | (palette_offset_index << 8);
		}
	}

    // incre offsets
    tile_offset_index += 1;
    palette_offset_index += 1;
    std::cout<< "Palatte used: " << palette_offset_index <<std::endl;
}

// read a png imag and set it into background in a specified location
void SpriteReader::setPNGIntoBackground(std::string const & path, glm::uvec2 location ){

    assert(palette_offset_index < 8); // 8 available palettes

    // read png into vector - pic
    glm::uvec2 size;
    std::vector< glm::u8vec4 > pic;
    load_png( data_path(path), &size, &pic, LowerLeftOrigin); //TODO add try-catch block

    // set up palette for this png 
    // !!! We assume the input png has no more than 4 colors !!!
    // So all tiles use the same palatte
    // --- set up palette ---
    PPU466::Palette & palatte = ppu->palette_table[palette_offset_index];
    {
        int size = 0;
        for(auto color : pic){
            // add color to palette until full
            addColorToPalette(palatte, size, color);
            if (size == 4)
                break;
        }
    }

    // calculate size of the png, in terms of tiles
    uint32_t width = size.x / 8;
    uint32_t height = size.y / 8;
    
    // go over each tile (8x8 block) in the png, add them into ppu 
    for(uint32_t i =0; i < height; i++){
        for(uint32_t j=0; j< width; j++){ 
            // --- for each tile (8x8 block) ---

            PPU466::Tile & curTile = ppu->tile_table[tile_offset_index];
            // set all 8x8 bits of the current tile
            for (uint32_t y=0; y < 8; y++){
                for (uint32_t x=0; x < 8; x++){
                    // find the color of this bit, and convert into palatte index
                    int indexInPic = (i*8 + y) * size.x + (j*8 + x);
                    int colorIndex = findColorInPalette(palatte, pic[indexInPic]);
                    // set this bit 
                    setTileBits(curTile, y, x, colorIndex);
                }
            }

            // link this tile into background
            uint32_t x_on_bg = std::min(location.x + j, PPU466::BackgroundWidth-1);
            uint32_t y_on_bg = std::min(location.y + i, PPU466::BackgroundHeight-1);
            ppu->background[x_on_bg+PPU466::BackgroundWidth*y_on_bg] = 
                tile_offset_index | (palette_offset_index << 8);

            // done with current tile. increment tile offset
            tile_offset_index += 1;
        }
    }

    // done with this png, incre palette_offset_index
    palette_offset_index += 1;
    std::cout<< "Palatte used: " << palette_offset_index <<std::endl;

}


// --------------- SpriteGroup Functions ------------------- //

// draw function of SpriteGroup
void SpriteGroup::draw(int32_t pos_x,int32_t pos_y ){
    // only draw if visible
    if (!visible)
        return;
    // go over all sprites, and change pos accordingly
    for(uint32_t i =0; i < height; i++){
        for(uint32_t j=0; j< width; j++){
            uint32_t sprite_index = sprite_indices[i*width + j];
            ppu->sprites[sprite_index].x = pos_x + j*8;
	        ppu->sprites[sprite_index].y = pos_y + i*8;
        }
    }
}

// set the spriteGroup obj to be invisible 
void SpriteGroup::setInVisible(){
    // go over all sprites
    for(uint32_t i =0; i < height; i++){
        for(uint32_t j=0; j< width; j++){
            // move the sprite off the screen (see PPU466.hpp, line 136 for why)
            uint32_t sprite_index = sprite_indices[i*width + j];
            ppu->sprites[sprite_index].x = 0;
	        ppu->sprites[sprite_index].y = 250;
        }
    }
    visible = false;
}

// set the spriteGroup obj to be visible
void SpriteGroup::setVisible(){
    visible = true; // this allow draw() to set the position back on the screen
}

