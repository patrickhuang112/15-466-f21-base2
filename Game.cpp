#include "Game.hpp"
#include <cmath>
#include <unordered_set>
#include <algorithm>
#include <stdlib.h>
#include <random>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <string>

namespace Game {

void BoundingBox::move(double dx, double dy) {
    x += dx;
    y += dy;
    left += dx;
    right += dx;
    top += dy;
    bottom += dy;
}

bool BoundingBox::intersect(const BoundingBox& b) {
    if (left > b.right || right < b.left || top < b.bottom || bottom > b.top) {
        return false; 
    }
    return true;
}


void Game::set_camera(Scene::Camera *c) { 
    camera = c;
    camera->transform->position = glm::vec3(PLAYER_START_X, PLAYER_START_Y, CAMERA_HEIGHT);
}


void Game::set_models(const Scene& scene) {
    for (auto &transform : scene.transforms) {
        if (transform.name.substr(0, 6) == "Cube1.") {
            uint32_t ind = stoi(transform.name.substr(6, 3));
            ind -= 1;
            if (ind < active_walls.size()) {
                Wall &w = walls[active_walls[ind]];
                w.model = const_cast<Scene::Transform *>(&transform);
                w.model->position = glm::vec3(w.box.x, w.box.y, 0.0f); 
            }
            else {
                Scene::Transform * c = const_cast<Scene::Transform *>(&transform);
                c->position = glm::vec3(OFFSCREEN_X, OFFSCREEN_Y, 0.0f); 
            }
            
        }
        else if (transform.name == "Player") {
            player_model = const_cast<Scene::Transform *>(&transform);
            player_model->position = glm::vec3(PLAYER_START_X, PLAYER_START_Y, 0.0);
        } 
        else if (transform.name == "End") {
            end_model = const_cast<Scene::Transform *>(&transform);
            end_model->position = glm::vec3(END_X, END_Y, 0.0); 
        }
    }
}

void Game::generate_st() {
    // visited
    std::unordered_set<uint32_t> visited;


    std::default_random_engine e;
    std::uniform_real_distribution<> dis(0, 1); // rage 0 - 1
    // Fake randomness, don't know what will be begin but it'll be something
    // by using an unordered set IM LAZY
    std::unordered_set<uint32_t> random_set;
    random_set.emplace(0);
    visited.emplace(0);
    size_t cur_st_ind = 0;
    while (!random_set.empty()) {
        // Use time to make it more random each time
        double random_val = (dis(e) * std::time(0) * dis(e)) ;
        double decimals = random_val - static_cast<uint32_t>(random_val);
        const auto& random_it = std::next(random_set.begin(), static_cast<int>(decimals * random_set.size()));
        const uint32_t v = *random_it;
        for (uint32_t n : spaces[v]) {
            if (visited.find(n) == visited.end()) {
                visited.emplace(n); 
                random_set.emplace(n);
                st_edges[cur_st_ind].u_ind = v;
                st_edges[cur_st_ind].v_ind = n;
                cur_st_ind++;
            } 
        }           
        random_set.erase(random_it);
    }
    assert(cur_st_ind == st_edges.size());
}

uint32_t Game::edge_to_wall (const Edge& e) {
    uint32_t max_ind = std::max(e.u_ind, e.v_ind);
    uint32_t min_ind = std::min(e.u_ind, e.v_ind);
    uint32_t min_r = min_ind / SPACES_BOARD_SIZE;
    uint32_t min_c = min_ind % SPACES_BOARD_SIZE;
    uint32_t max_r = max_ind / SPACES_BOARD_SIZE;
    uint32_t max_c = max_ind % SPACES_BOARD_SIZE;
    assert((max_r - min_r == 1 && max_c == min_c) 
        || (max_c - min_c == 1 && max_r == min_r));

    uint32_t w_r;
    uint32_t w_c;
    // Same columns, so its vertical
    if (max_c == min_c) {
        // space to wall offset + wall_row + in between 
        w_r = 1        + min_r * 2 + 1;
        w_c = 1        + min_c * 2;
    }
    else {
        w_r = 1        + min_r * 2;
        w_c = 1        + min_c * 2 + 1;
    }
    return w_r * WALLS_BOARD_SIZE + w_c;
}

// Precondition, SPANNING TREE ALREADY MADE
void Game::generate_walls() {
    // Generate horizontal walls
    for (size_t c = 1; c < WALLS_BOARD_SIZE - 1; ++c) {
        for (size_t r = 0; r < WALLS_BOARD_SIZE; r += 2) {
            size_t ind = r * WALLS_BOARD_SIZE + c; 
            walls[ind].active = true;
            walls[ind].o = HORIZONTAL;
            double wall_mid_x = c * WALL_SIZE + TOP_LEFT_START_X;
            double wall_mid_y = r * WALL_SIZE + TOP_LEFT_START_Y;
            walls[ind].box = BoundingBox(
                wall_mid_x,
                wall_mid_y, 
                WALL_SIZE
            );
        }
    }

    // Generate left and right walls 
    for (size_t r = 1; r < WALLS_BOARD_SIZE - 1; ++r) {
        for (size_t c = 0; c < WALLS_BOARD_SIZE; c += 2) {
            size_t ind = r * WALLS_BOARD_SIZE + c; 
            walls[ind].active = true;
            walls[ind].o = VERTICAL;
            double wall_mid_x = c * WALL_SIZE + TOP_LEFT_START_X;
            double wall_mid_y = r * WALL_SIZE + TOP_LEFT_START_Y;
            walls[ind].box = BoundingBox(
                wall_mid_x,
                wall_mid_y, 
                WALL_SIZE
            );
        }
    }

    // Remove inner walls that correspond to ST edges
    for (const auto& e : st_edges) {
        walls[edge_to_wall(e)].active = false;
    }
    for (uint32_t i = 0; i < WALLS_BOARD_SIZE*WALLS_BOARD_SIZE; ++i) {
        if (walls[i].active)  {
            active_walls.push_back(i); 
        }
    }
    printf("size: %d", (int)active_walls.size());
}

void Game::generate_maze() {
    generate_st(); 
    generate_walls();
}

void Game::print_walls() {
    for (size_t r = 0; r < WALLS_BOARD_SIZE; ++r) {
        for (size_t c = 0; c < WALLS_BOARD_SIZE; ++c)  {
            size_t ind = r * WALLS_BOARD_SIZE + c;  
            if (walls[ind].active) {
                if (walls[ind].o == HORIZONTAL) {
                    printf("--");
                } 
                else {
                    printf("||");
                }
            }
            else {
                printf("  ");
            }
        }
        printf("\n");
    } 
}

void Game::move_player(double dx, double dy) {
    player.move(dx, dy);
    for (size_t i = 0; i < WALLS_BOARD_SIZE * WALLS_BOARD_SIZE; ++i) {
        if (walls[i].active && walls[i].box.intersect(player)) {
            // Undo move
            player.move(-dx, -dy);
            return; 
        }
    }
    camera->transform->position = glm::vec3(player.x, player.y, CAMERA_HEIGHT); 
    if (player_model != nullptr) {
        player_model->position = glm::vec3(player.x, player.y, 0.0); 
    }
    if (end.intersect(player)) {
        printf("GAME OVER!\n");
        game_over = true;
    }
}

}