#pragma once

#include "Scene.hpp"

#include <stdint.h>
#include <vector>
#include <array>
#include <memory>
#include <cassert>

namespace Game {

// Make this at least like 10 or something
constexpr uint32_t SPACES_BOARD_SIZE = 7; 
constexpr uint32_t WALLS_BOARD_SIZE = SPACES_BOARD_SIZE*2+1;
constexpr double TOP_LEFT_START_X = 0.0;
constexpr double TOP_LEFT_START_Y = 0.0;
constexpr double WALL_SIZE = 3.45;
constexpr double PLAYER_START_X = 3.0;
constexpr double PLAYER_START_Y = 3.0;
constexpr double PLAYER_BOX_SIZE = 1.0;
constexpr double END_X = WALL_SIZE*(WALLS_BOARD_SIZE-1) - 3.0;
constexpr double END_Y = WALL_SIZE*(WALLS_BOARD_SIZE-1) - 3.0;
constexpr double END_BOX_SIZE = 1.0;
constexpr double OFFSCREEN_X = 100000.0;
constexpr double OFFSCREEN_Y = 100000.0;
constexpr double CAMERA_HEIGHT = 30.0;
constexpr uint32_t SPANNING_TREE_SPACES = SPACES_BOARD_SIZE*SPACES_BOARD_SIZE-1;
constexpr uint32_t TOTAL_MODELS_NEEDED = 2 * (WALLS_BOARD_SIZE - 2) * (WALLS_BOARD_SIZE / 2) - SPANNING_TREE_SPACES;

enum Orientation {
    VERTICAL,
    HORIZONTAL 
};

struct Edge {
    uint32_t u_ind;
    uint32_t v_ind;
};

struct BoundingBox {
    BoundingBox() {
        x = 0.0;
        y = 0.0;
        left = 0.0; 
        right = 0.0;
        top = 0.0;
        bottom = 0.0;
        size = 0.0;
    }
    BoundingBox(double x, double y, double s) {
        this->x = x;
        this->y = y;
        size = s;
        left = x - (size / 2.0);     
        right = x + (size / 2.0);
        top = y + (size / 2.0);
        bottom = y - (size / 2.0);
    }
    double x;
    double y;
    double left;
    double right;
    double top;
    double bottom;
    double size;

    void move(double dx, double dy);
    bool intersect(const BoundingBox& b);
};

struct Wall {
    Wall () {
        active = false; 
        model = nullptr;
    }
    bool active; 
    BoundingBox box;
    Orientation o;   
    Scene::Transform *model;
};

struct Game {
    Game() : player(PLAYER_START_X, PLAYER_START_Y, PLAYER_BOX_SIZE), end(END_X, END_Y, END_BOX_SIZE) {
        for (uint32_t r = 0; r < SPACES_BOARD_SIZE; ++r) {
            for (uint32_t c = 0; c < SPACES_BOARD_SIZE; ++c) {
                uint32_t ind = r * SPACES_BOARD_SIZE + c;
                assert(spaces[ind].size() == 0);
                uint32_t other_ind;
                
                if (r+1 < SPACES_BOARD_SIZE) {
                    other_ind = (r+1) * SPACES_BOARD_SIZE + c; 
                    spaces[ind].emplace_back(other_ind);
                }
                if (r-1 < SPACES_BOARD_SIZE){
                    other_ind = (r-1) * SPACES_BOARD_SIZE + c; 
                    spaces[ind].emplace_back(other_ind);
                }
                if (c+1 < SPACES_BOARD_SIZE) {
                    other_ind = r * SPACES_BOARD_SIZE + c+1; 
                    spaces[ind].emplace_back(other_ind);
                }
                if (c-1 < SPACES_BOARD_SIZE) {
                    other_ind = r * SPACES_BOARD_SIZE + c-1; 
                    spaces[ind].emplace_back(other_ind);
                }
            }
        }
    }
    
    ~Game() = default;
    void generate_maze(void);
    void print_walls(void);
    void set_models(const Scene& scene);
    void move_player(double dx, double dy);
    void set_camera(Scene::Camera *c);
    bool is_game_over(void) {return game_over;}

    

    // Code logic adapted from approach outlined in the 15122 lecture note slides
    // that I still have from when I took the class
    // Prims algorithm in generating the maze also based off of description of
    // prims algorithm from 15122 lecture notes that I still have.
    private:
    void generate_st();
    void generate_walls();
    uint32_t edge_to_wall (const Edge& e);  
        


    

    // Adj matrix
    std::array<std::vector<uint32_t>, SPACES_BOARD_SIZE*SPACES_BOARD_SIZE> spaces;
    // Spanning tree edges
    std::array<Edge, SPACES_BOARD_SIZE*SPACES_BOARD_SIZE-1> st_edges;
    // Walls
    std::array<Wall, WALLS_BOARD_SIZE*WALLS_BOARD_SIZE> walls;
    std::vector<uint32_t> active_walls;
    Scene::Camera *camera;
    BoundingBox player;
    BoundingBox end;
    Scene::Transform *player_model = nullptr;
    Scene::Transform *end_model = nullptr;
    bool game_over = false;
};

}
