#pragma once
#include "PPU466.hpp"
#include "Game.hpp"

#include <stdint.h>

#include <string>
#include <array>
#include <vector>
#include <unordered_map>

namespace GameView {
constexpr uint32_t NUM_TEXTURES = 6;
constexpr uint8_t TEXTURE_WIDTH = 8;
constexpr uint8_t TEXTURE_HEIGHT = 8;
constexpr uint8_t BACKGROUND_TILE_IND = 0;
constexpr uint8_t SELECTED_TILE_IND = 1;
constexpr uint8_t BOARD_OUTLINE_IND = 2;
constexpr uint8_t WINNER1_TILE_IND = 3;
constexpr uint8_t WINNER2_TILE_IND = 4;
constexpr uint8_t WINNER_TILE_SHIFT = 2;
constexpr uint8_t WINNER_SPRITE_IND = 0;

const std::array<std::string, NUM_TEXTURES> TEXTURE_PATHS {
    "assets/outlines/outline1.png",
    "assets/outlines/outline2.png",
    "assets/outlines/outline3.png",
    "assets/outlines/outline4.png",
    "assets/outlines/outline5.png",
    "assets/outlines/outline6.png"
};

struct rcpair {
    public:
        rcpair(uint8_t row, uint8_t col) {
            this->r = row; 
            this->c = col;
        }
        uint8_t r;
        uint8_t c;
};

// HARD CODED NOT PROGRAMMATICALLY MADE STUPID CODE I AM LAZY
// IF ACTUALLY MAKING BIGGER, EXTENSIBLE GAME THEN WILL DO BETTER 
const std::vector<rcpair> BOARD_OUTLINE_POSITIONS = {
    rcpair(19,11),
    rcpair(18,11),
    rcpair(17,11),
    rcpair(16,11),
    rcpair(15,11),
    rcpair(14,11),
    rcpair(13,11),
    rcpair(12,11),
    rcpair(11,11),
    rcpair(10,11),
    rcpair(19,20),
    rcpair(18,20),
    rcpair(17,20),
    rcpair(16,20),
    rcpair(15,20),
    rcpair(14,20),
    rcpair(13,20),
    rcpair(12,20),
    rcpair(11,20),
    rcpair(10,20),
    rcpair(19,19),
    rcpair(19,18),
    rcpair(19,17),
    rcpair(19,16),
    rcpair(19,15),
    rcpair(19,14),
    rcpair(19,13),
    rcpair(19,12),
    rcpair(10,19),
    rcpair(10,18),
    rcpair(10,17),
    rcpair(10,16),
    rcpair(10,15),
    rcpair(10,14),
    rcpair(10,13),
    rcpair(10,12)
};


struct GameView {

    GameView() = default;
    ~GameView() = default;
    void set_ppu(PPU466 *ppu_ptr);
    void load_textures (void);
    void display_winner (uint8_t winner);
    uint8_t create_sprite(uint8_t player, uint8_t number);
    void update_sprite(uint8_t sprite_ind, uint8_t player, uint8_t number);
    void release_sprite(uint8_t sprite_ind);
    void select_sprite(uint8_t sprite_ind);
    void place_sprite(uint8_t sprite_ind, uint8_t row, uint8_t col);
    bool xy_to_row_col(int32_t x, int32_t y, int w, int h, uint8_t *row, uint8_t *col);
    
    private:
        uint8_t board_bottom_left_x = (PPU466::ScreenWidth - (Game::BOARD_SIZE * TEXTURE_WIDTH)) / 2;
        uint8_t board_bottom_left_y = (PPU466::ScreenHeight - (Game::BOARD_SIZE * TEXTURE_HEIGHT)) / 2; 
        PPU466 *ppu;
        std::unordered_map<std::string, uint8_t> texture_to_tile_ind;
        bool in_board(int x, int y);
};

}