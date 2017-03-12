#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>
#include <ctime>
#include "score.h"
using namespace std;

#ifndef SOLVER_H
#define SOLVER_H

// Todo: add discarded tiles
typedef struct {
  // suit, val
  int n_tiles[8][10];
  int max_order_ind[8][10];
  int order[8][10][8];
  int red_dora[8];
  int n;

  inline bool can_insert(int suit, int val) {
    return n_tiles[suit][val] < 4;
  }

  inline void insert(int suit, int val, bool is_red_dora) {
    int k = ++n_tiles[suit][val];
    order[suit][val][k] = ++n;
    max_order_ind[suit][val] = k;
    if(is_red_dora)
      red_dora[suit]++;
  }

  inline void remove(int suit, int val) {
    int k = --n_tiles[suit][val];
    order[suit][val][k+1] = 0;
    max_order_ind[suit][val] = k;
    // Can't have more red dora than 5 tiles
    if(val == 5) {
      red_dora[suit] = min(red_dora[suit], n_tiles[suit][val]);
    }
  }
} state_t;

ostream& operator<<(ostream& stream, const state_t& s) {
  for(int suit = 1; suit <= 4; suit++) {
    if(suit > 1)
      stream << " ";
    bool suit_exists = false;
    for(int val = 1; val <= 9; val++) {
      for(int k = 0; k < s.n_tiles[suit][val]; k++) {
        stream << val;
        suit_exists = true;
      }
    }
    if(!suit_exists)
      stream << "-";
  }
  return stream;
}

typedef struct {

  vector<group_t> all_patterns;
  vector<group_t> all_pairs;
  game_t game;
  grouped_hand_t accumulator;
  double best_score;
  double value_per_tile[8][10][8];

  void preprocess_patterns(state_t& state) {
    all_patterns.clear();
    all_pairs.clear();
    for(int i = 1; i <= 7; i++) {
      for(int suit = MAN; suit <= SOU; suit++) {
        if(state.n_tiles[suit][i] > 0 && state.n_tiles[suit][i + 1] > 0 && state.n_tiles[suit][i + 2] > 0) {
          all_patterns.push_back({ suit, i, CHI });
        }
      }
    }
    for(int i = 1; i <= 9; i++) {
      for(int suit = MAN; suit <= HONORS; suit++) {
        if(state.n_tiles[suit][i] >= 3) {
          all_patterns.push_back({ suit, i, PON });
        }
        if(state.n_tiles[suit][i] >= 2) {
          all_pairs.push_back({ suit, i, PAIR });
        }
      }
    }
  }

  void reset_results() {
    best_score = 0;
    for(int i = 0; i < 8; i++) {
      for(int j = 0; j < 10; j++) {
        for(int k = 0; k <= 4; k++) {
          value_per_tile[i][j][k] = 0;
        }
      }
    }
  }

  void process_result(state_t& state, double score) {
    best_score = max(best_score, score);
    for(int suit = 1; suit <= 4; suit++) {
      for(int val = 1; val <= 9; val++) {
        int used_tiles = state.max_order_ind[suit][val] - state.n_tiles[suit][val];
        for(int k = used_tiles; k < state.max_order_ind[suit][val]; k++) {
          value_per_tile[suit][val][k] = max(value_per_tile[suit][val][k], score);
        }
      }
    }
  }

  void finished_hand(state_t& state) {

    // Find out how long this hand takes to make
    // Also count red dora
    int n_red_dora = 0;
    tile_t last_tile = {0, 0};
    int last_draw = -1; // 1 indexed
    for(int s = 1; s <= 4; s++) {
      for(int v = 1; v <= 9; v++) {
        int k = state.max_order_ind[s][v] - state.n_tiles[s][v];
        int n_turns = state.order[s][v][k] - 13;
        if(n_turns > last_draw) {
          last_draw = n_turns;
          last_tile = {s, v};
        }
        if(v == 5) {
          n_red_dora += min(state.red_dora[s], k);
        }
      }
    }

    // Figure out possible waits
    bool has_ryanmen = true;
    bool has_other = false;

    for(unsigned int i = 0; i < accumulator.size(); i++) {
      group_t& group = accumulator[i];
      if(group.count(last_tile)) {
        int wait_type = get_wait_type(group, last_tile);
        if(wait_type == RYANMEN)
          has_ryanmen = true;
        else
          has_other = true;
      }
    }

    // cout << last_draw << "\n";

    // // cout << accumulator << " " << last_tile << " " << has_ryanmen << " " << has_other << "\n";

    int score = 0;
    if(has_ryanmen) {
      score = max(game.score(accumulator, true, n_red_dora), score);
    }
    if(has_other) {
      score = max(game.score(accumulator, false, n_red_dora), score);
    }

    double true_score = get_adjusted_score(score, last_draw);
    process_result(state, true_score);
    // cout << score << " " << last_draw << " " << true_score << "\n";

    // printf("Turn %d, score %d\n", last_draw, score);
  }

  void chitoitsu_solve(state_t& state, int start = 0) {
    if(accumulator.size() == 7) {
      finished_hand(state);
    }
    if(all_pairs.size() - start < 7 - accumulator.size())
      return;

    for(unsigned int pair_ind = start; pair_ind < all_pairs.size(); pair_ind++) {
      group_t group = all_pairs[pair_ind];

      state.n_tiles[group.suit][group.val] -= 2;
      accumulator.push_back(group);

      chitoitsu_solve(state, pair_ind + 1);

      accumulator.pop_back();
      state.n_tiles[group.suit][group.val] += 2;
    }
  }

  // For hands which are missing only the pair
  void pair_solve(state_t& state) {
    // Find the pairs
    for(int suit = MAN; suit <= HONORS; suit++) {
      for(int val = 1; val <= 9; val++) {
        if(state.n_tiles[suit][val] >= 2) {
          state.n_tiles[suit][val] -= 2;
          accumulator.push_back({ suit, val, PAIR });

          finished_hand(state);

          accumulator.pop_back();
          state.n_tiles[suit][val] += 2;
        }
      }
    }
  }

  // For hands which do not yet have all melds
  void meld_solve(state_t& state, int start = 0) {
    if(accumulator.size() == 4) {
      return pair_solve(state);
    }

    for(unsigned int pattern_ind = start; pattern_ind < all_patterns.size(); pattern_ind++) {
      group_t group = all_patterns[pattern_ind];
      if(group.type == CHI && state.n_tiles[group.suit][group.val] && state.n_tiles[group.suit][group.val + 1] && state.n_tiles[group.suit][group.val + 2]) {
        state.n_tiles[group.suit][group.val]--;
        state.n_tiles[group.suit][group.val + 1]--;
        state.n_tiles[group.suit][group.val + 2]--;
        accumulator.push_back(group);

        meld_solve(state, pattern_ind);

        accumulator.pop_back();
        state.n_tiles[group.suit][group.val]++;
        state.n_tiles[group.suit][group.val + 1]++;
        state.n_tiles[group.suit][group.val + 2]++;
      } else if(group.type == PON && state.n_tiles[group.suit][group.val] >= 3) {
        state.n_tiles[group.suit][group.val] -= 3;
        accumulator.push_back(group);

        meld_solve(state, pattern_ind);

        accumulator.pop_back();
        state.n_tiles[group.suit][group.val] += 3;
      }
    }
  }

  void full_solve(state_t& state, int offset) {
    init_win_probability(offset);
    accumulator.clear();
    reset_results();
    preprocess_patterns(state);
    meld_solve(state);
    // chitoitsu_solve(state);
  }

  void reset_game_state() {
    game.dora_tiles.clear();
  }

  state_t init_from_string(string hand, int offset = 0) {
    state_t state;
    memset(&state, 0, sizeof(state));
    state.n = offset;

    int suit = 1;
    for(char c : hand) {
      if(c == '-') continue;
      if(c == ' ') {
        suit++;
        continue;
      }
      bool is_red_dora = false;
      int val = c - '0';
      if(c == 'r') {
        is_red_dora = true;
        val = 5;
      }
      state.insert(suit, val, is_red_dora);
    }

    return state;
  }
} solver_t;

void test_scorer() {
  game_t game;
  game.dora_tiles.push_back({SOU, 1});
  game.dora_tiles.push_back({SOU, 1});
  grouped_hand_t hand = {{SOU, 1, CHI}, {SOU, 1, CHI}, {SOU, 1, CHI}, {SOU, 1, CHI}, {SOU, 5, PAIR}};
  cout << game.score(hand, RYANMEN) << "\n";
}

#endif
