#include "SpriteReader.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"
#include <iostream>


void SpriteReader::getSpriteGroupFromPNG(std::string const & path, glm::uvec2 size, int priority, SpriteGroup & spriteGroup){
    
    assert(palette_offset_index < 8); // 8 available palettes

    // read png into vector - pic
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
            //std::cout << std::hex << pic[i * size.x + j].r << ", " <<  pic[i * size.x + j].g << ", " <<  pic[i * size.x + j].b << "  | ";
            
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
        //std::cout << " \n";
    }

    // done with this png, incre palette_offset_index
    palette_offset_index += 1;
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

// draw function of SpriteGroup
void SpriteGroup::draw(int32_t pos_x,int32_t pos_y ){
    // go over all sprites, and change pos accordingly
    for(uint32_t i =0; i < height; i++){
        for(uint32_t j=0; j< width; j++){
            uint32_t sprite_index = sprite_indices[i*width + j];
            ppu->sprites[sprite_index].x = pos_x + j*8;
	        ppu->sprites[sprite_index].y = pos_y + i*8;
        }
    }
}



