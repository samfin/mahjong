#include <iostream>
#include <vector>
#include <string.h>
#include <algorithm>
#include "solver.h"
using namespace std;

int randn(int n) {
  return rand() % n;
}

typedef struct {
  tile_list_t tiles;

  void init() {
    tiles.clear();
    for(int tile_ind = 0; tile_ind < 34; tile_ind++) {
      int suit = tile_ind / 9 + 1;
      int val = tile_ind % 9 + 1;
      for(int k = 0; k < 4; k++) {
        bool is_red_dora = false;
        if(k == 3 && suit != HONORS && val == 5)
          is_red_dora = true;
        tiles.push_back({ suit, val, is_red_dora });
      }
    }
  }

  void init_from_state(state_t& state) {
    tiles.clear();
    for(int tile_ind = 0; tile_ind < 34; tile_ind++) {
      int suit = tile_ind / 9 + 1;
      int val = tile_ind % 9 + 1;
      for(int k = state.n_tiles[suit][val]; k < 4; k++) {
        bool is_red_dora = false;
        int red_dora_left = 1 - state.red_dora[suit];
        if(k >= (4 - red_dora_left) && suit != HONORS && val == 5)
          is_red_dora = true;
        tiles.push_back({ suit, val, is_red_dora });
      }
    }
  }

  void shuffle() {
    random_shuffle(tiles.begin(), tiles.end(), randn);
  }

  tile_t get_next() {
    tile_t tile = tiles.back();
    tiles.pop_back();
    return tile;
  }
} deck_t;
