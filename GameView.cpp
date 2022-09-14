#include "GameView.hpp"

//for loading pngs
#include "load_save_png.hpp"

#include <map>

namespace GameView {
void GameView::set_ppu (PPU466 *ppu_ptr) {
    this->ppu = ppu_ptr; 
}
void GameView::load_textures() {
    const uint8_t w = TEXTURE_WIDTH;
    const uint8_t h = TEXTURE_HEIGHT;

    std::map<std::array<uint8_t, 3>, uint32_t> uniques;
    
    // HARD CODED PALETTES 
    // Black
    std::array<uint8_t, 3> black {0x00, 0x00, 0x00}; 
    std::array<uint8_t, 3> blue {0x25, 0xE3, 0xFF};
    std::array<uint8_t, 3> red {0xFF, 0x57, 0x57};
    std::array<uint8_t, 3> white {0xFF, 0xFF, 0xFF};

    this->ppu->palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x25, 0xE3, 0xFF, 0xff),
		glm::u8vec4(0xFF, 0x57, 0x57, 0xff),
		glm::u8vec4(0xFA, 0x70, 0x00, 0xff),
	}; 

    // Palette for board outline, only using the first color which is white
    this->ppu->palette_table[1] = {
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
		glm::u8vec4(0x25, 0xE3, 0xFF, 0xff), // unused
		glm::u8vec4(0xFF, 0x57, 0x57, 0xff), // unused
		glm::u8vec4(0xFA, 0x70, 0x00, 0xff), // unused
	}; 

    PPU466::Tile background_tile;
    for (int j = 0 ; j < 8; ++j) {
        background_tile.bit0[j] = 0b00000000;
        background_tile.bit1[j] = 0b00000000;
    }
    PPU466::Tile selected_tile;
    for (int j = 0 ; j < 8; ++j) {
        selected_tile.bit0[j] = 0b11111111;
        selected_tile.bit1[j] = 0b11111111;
    }
    // Board outline will use 
    PPU466::Tile board_outline_tile;
    for (int j = 0 ; j < 8; ++j) {
        board_outline_tile.bit0[j] = 0b00000000;
        board_outline_tile.bit1[j] = 0b00000000;
    }
    // Board outline will use 
    PPU466::Tile winner1_tile;
    for (int j = 0 ; j < 8; ++j) {
        winner1_tile.bit0[j] = 0b11111111;
        winner1_tile.bit1[j] = 0b00000000;
    }
    // Board outline will use 
    PPU466::Tile winner2_tile;
    for (int j = 0 ; j < 8; ++j) {
        winner2_tile.bit0[j] = 0b00000000;
        winner2_tile.bit1[j] = 0b11111111;
    }
    this->ppu->tile_table[BACKGROUND_TILE_IND] = background_tile;
    this->ppu->tile_table[SELECTED_TILE_IND] = selected_tile;
    this->ppu->tile_table[BOARD_OUTLINE_IND] = board_outline_tile;
    this->ppu->tile_table[WINNER1_TILE_IND] = winner1_tile;
    this->ppu->tile_table[WINNER2_TILE_IND] = winner2_tile;
    PPU466::Sprite& s= (this->ppu->sprites)[WINNER_SPRITE_IND]; 
    s.x = PPU466::ScreenWidth / 2 - 4;
    s.y = PPU466::ScreenHeight / 2 + 50;
    s.index = SELECTED_TILE_IND;
    s.attributes = 0;

    uint8_t next_tile_ind = 5;

    for (size_t i = 0; i < NUM_TEXTURES; ++i) {
        auto& path = TEXTURE_PATHS[i];
        glm::uvec2 wh = glm::uvec2(w,h);
        std::vector< glm::u8vec4 > data;
        load_png(path, &wh, &data, LowerLeftOrigin); 

        // Do red/blue for each outline
        for (int palette_ind = 1; palette_ind < 3; palette_ind++) {
            PPU466::Tile tile;
            
            for (int j = 0 ; j < 8; ++j) {
                tile.bit0[j] = 0b00000000;
                tile.bit1[j] = 0b00000000;
            }
            for (uint8_t r = 0; r < h ; ++r) {
                for (uint8_t c = 0; c < w ; ++c) {
                    uint32_t ind = (r * w) + c;
                    const auto& rgba = data[ind];
                    if (rgba.w > 0) {
                        if (palette_ind == 1) {
                            tile.bit0[r] = tile.bit0[r] | (0x1 << (8 - c - 1));
                        } 
                        else if (palette_ind == 2) {
                            tile.bit1[r] = tile.bit1[r] | (0x1 << (8 - c - 1));
                        }
                    }
                    else {
                        //printf("%d %d %d\n", rgba.x, rgba.y, rgba.z);
                    }
                } 
            }
            this->ppu->tile_table[next_tile_ind] = tile;
                       if (palette_ind == 1) {
                this->texture_to_tile_ind.emplace(path + "blue", next_tile_ind);
                //printf("%s huh\n", (path + "blue").c_str());
            }
            else if (palette_ind == 2) {
                this->texture_to_tile_ind.emplace(path + "red", next_tile_ind);
                //printf("%s huh\n", (path + "red").c_str());
            }
            next_tile_ind++;
        }
        
    }
}

void GameView::display_winner(uint8_t winner) {
    (this->ppu->sprites)[WINNER_SPRITE_IND].index = winner + WINNER_TILE_SHIFT; 
}

void GameView::select_sprite(uint8_t sprite_ind) {
    PPU466::Sprite& s= (this->ppu->sprites)[sprite_ind];
    s.index = SELECTED_TILE_IND;
}

// Returns sprite index
uint8_t GameView::create_sprite(uint8_t player, uint8_t number) {
    uint8_t free_sprite_ind = 64;
    // index 0 is reserved for winner indication sprite
    for (uint8_t i = 1; i < 64; ++i) {
        if ((this->ppu->sprites)[i].y >= 240) {
            free_sprite_ind = i;
            break;
        }
    }
    if (free_sprite_ind == 64) {
        return 64; 
    }
    PPU466::Sprite& s = (this->ppu->sprites)[free_sprite_ind];
    // Player 1, blue
    if (player == 1) {
        s.index = this->texture_to_tile_ind["assets/outlines/outline" + std::to_string(number) + ".pngblue"];
    } 
    // Player 2, 
    else if (player == 2) {
        s.index = this->texture_to_tile_ind["assets/outlines/outline" + std::to_string(number) + ".pngred"];
    }
    else {
        assert(false);
    }
    s.attributes = 0;
    // Set it bottom left first
    s.x = 0;
    s.y = 0;
    return free_sprite_ind;
}

void GameView::update_sprite(uint8_t sprite_ind, uint8_t player, uint8_t number) {
    PPU466::Sprite& s = (this->ppu->sprites)[sprite_ind];
    // Player 1, blue
    if (player == 1) {
        s.index = this->texture_to_tile_ind["assets/outlines/outline" + std::to_string(number) + ".pngblue"];
    } 
    // Player 2, 
    else if (player == 2) {
        s.index = this->texture_to_tile_ind["assets/outlines/outline" + std::to_string(number) + ".pngred"];
    }
    else {
        assert(false);
    }
    s.attributes = 0;
}

void GameView::release_sprite(uint8_t sprite_ind) {
    (this->ppu->sprites)[sprite_ind].y = 240; 
    (this->ppu->sprites)[sprite_ind].x = 0;
}

void GameView::place_sprite(uint8_t sprite_ind, uint8_t row, uint8_t col) {
    (this->ppu->sprites)[sprite_ind].y = this->board_bottom_left_y + row * TEXTURE_HEIGHT; 
    (this->ppu->sprites)[sprite_ind].x = this->board_bottom_left_x + col * TEXTURE_WIDTH;
}

// x and y are PPU coordinates, not screen coordinates
bool GameView::in_board(int x, int y) {
    return ((x >= this->board_bottom_left_x && x <= this->board_bottom_left_x + Game::BOARD_SIZE * TEXTURE_WIDTH) 
        &&  (y >= this->board_bottom_left_y && y <= this->board_bottom_left_y + Game::BOARD_SIZE * TEXTURE_HEIGHT));
}

// x and y are from top left of screen, but all logic is from bottom left so change
bool GameView::xy_to_row_col(int32_t x, int32_t y, int w, int h, uint8_t *row, uint8_t *col) {
    y = h - y;
    int ppu_x = int(std::round(double(x) / double(w) * double(PPU466::ScreenWidth)));
    int ppu_y = int(std::round(double(y) / double(h) * double(PPU466::ScreenHeight))); 
    if (!in_board(ppu_x, ppu_y)) {
        return false; 
    }
    *row = uint8_t((ppu_y - this->board_bottom_left_y) / TEXTURE_HEIGHT);
    *col = uint8_t((ppu_x - this->board_bottom_left_x) / TEXTURE_WIDTH); 
    return true;
}

}