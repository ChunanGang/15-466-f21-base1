#include "SpriteReader.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"
#include <iostream>


void SpriteReader::getSpriteGroupFromPNG(std::string const & path, glm::uvec2 size, SpriteGroup & spriteGroup){
    
    // read png into vector pic
    std::vector< glm::u8vec4 > pic;
    load_png( data_path(path), &size, &pic, LowerLeftOrigin); //TODO add try-catch block

    // set up palette for this picture 
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

    // go over each tile (8x8 block) in the png, add them into ppu 
    for(uint32_t i =0; i < size.x; i+=8){
        for(uint32_t j=0; j< size.y; j+=8){
            //std::cout << std::hex << pic[i * size.x + j].r << ", " <<  pic[i * size.x + j].g << ", " <<  pic[i * size.x + j].b << "  | ";
            // for each tile (8x8 block)
            //ppu.tile_table[tile_offset_index];
        }
        //std::cout << " \n";
    }
    ppu->palette_table[7][1] = pic[0];
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
    //static_assert(curPaletteSize <= 3, "ONLY 4 colors allowed");
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


