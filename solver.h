#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>
#include <ctime>
#include "score.h"
using namespace std;

#ifndef SOLVER_H
#define SOLVER_H

#define INFINITE 9999

// Todo: add discarded tiles
typedef struct {
  // suit, val
  int n_tiles[8][10];
  int max_order_ind[8][10];
  int order[8][10][8];
  int n;

  inline bool can_insert(int suit, int val) {
    return n_tiles[suit][val] < 4;
  }

  inline void insert(int suit, int val) {
    int k = ++n_tiles[suit][val];
    order[suit][val][k] = ++n;
    max_order_ind[suit][val] = k;
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
  void preprocess_patterns(state_t& state) {
    all_patterns.clear();
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
      }
    }
  }

  int get_score(state_t& state) {

    // Find out how long this hand takes to make
    tile_t last_tile;
    int last_draw = -1; // 1 indexed
    for(int s = 1; s <= 4; s++) {
      for(int v = 1; v <= 9; v++) {
        int k = state.max_order_ind[s][v] - state.n_tiles[s][v];
        int n_turns = state.order[s][v][k] - 13;
        if(n_turns > last_draw) {
          last_draw = n_turns;
          last_tile = {s, v};
        }
      }
    }

    // Figure out possible waits
    bool has_ryanmen = false;
    bool has_other = false;

    for(group_t group : accumulator) {
      if(group.contains(last_tile)) {
        int wait_type = get_wait_type(group, last_tile);
        if(wait_type == RYANMEN)
          has_ryanmen = true;
        else
          has_other = true;
      }
    }

    // // cout << accumulator << " " << last_tile << " " << has_ryanmen << " " << has_other << "\n";

    int score = 0;
    if(has_ryanmen) {
      score = max(game.score(accumulator, true), score);
    }
    if(has_other) {
      score = max(game.score(accumulator, false), score);
    }

    // printf("Turn %d, score %d\n", last_draw, score);
    return last_draw;
  }

  int chitoitsu_solve(state_t& state, int start = 0) {
    if(accumulator.size() == 7) {
      return get_score(state);
    }

    int best_score = INFINITE;
    for(int tile_ind = start; tile_ind < 34; tile_ind++) {
      int suit = tile_ind / 9 + 1;
      int val = tile_ind % 9 + 1;

      if(state.n_tiles[suit][val] >= 2) {
        state.n_tiles[suit][val] -= 2;
        accumulator.push_back({ suit, val, PAIR });

        int score = chitoitsu_solve(state, tile_ind + 1);
        best_score = min(best_score, score);

        accumulator.pop_back();
        state.n_tiles[suit][val] += 2;
      }
    }
    return best_score;
  }

  // For hands which are missing only the pair
  int pair_solve(state_t& state) {
    int best_score = INFINITE;
    // Find the pairs
    for(int suit = MAN; suit <= HONORS; suit++) {
      for(int val = 1; val <= 9; val++) {
        if(state.n_tiles[suit][val] >= 2) {
          state.n_tiles[suit][val] -= 2;
          accumulator.push_back({ suit, val, PAIR });

          int score = get_score(state);
          best_score = min(best_score, score);

          accumulator.pop_back();
          state.n_tiles[suit][val] += 2;
        }
      }
    }
    return best_score;
  }

  // For hands which do not yet have all melds
  int meld_solve(state_t& state, int start = 0) {
    if(accumulator.size() == 4) {
      return pair_solve(state);
    }

    int best_score = INFINITE;
    for(int pattern_ind = start; pattern_ind < all_patterns.size(); pattern_ind++) {
      group_t group = all_patterns[pattern_ind];
      if(group.type == CHI && state.n_tiles[group.suit][group.val] && state.n_tiles[group.suit][group.val + 1] && state.n_tiles[group.suit][group.val + 2]) {
        state.n_tiles[group.suit][group.val]--;
        state.n_tiles[group.suit][group.val + 1]--;
        state.n_tiles[group.suit][group.val + 2]--;
        accumulator.push_back(group);

        int score = meld_solve(state, pattern_ind);
        best_score = min(best_score, score);

        accumulator.pop_back();
        state.n_tiles[group.suit][group.val]++;
        state.n_tiles[group.suit][group.val + 1]++;
        state.n_tiles[group.suit][group.val + 2]++;
      } else if(group.type == PON && state.n_tiles[group.suit][group.val] >= 3) {
        state.n_tiles[group.suit][group.val] -= 3;
        accumulator.push_back(group);

        int score = meld_solve(state, pattern_ind);
        best_score = min(best_score, score);

        accumulator.pop_back();
        state.n_tiles[group.suit][group.val] += 3;
      }
    }
    return best_score;
  }

  int full_solve(state_t& state) {
    accumulator.clear();
    preprocess_patterns(state);
    int normal_score = meld_solve(state);
    int chitoitsu_score = chitoitsu_solve(state);
    return min(normal_score, chitoitsu_score);
  }

  void test_random_solve() {
    srand(time(0));
    state_t initial_state;
    memset(&initial_state, 0, sizeof(initial_state));

    int dora = rand() % 34;
    game.dora_tiles.push_back({ dora / 9 + 1, dora % 9 + 1 });
    cout << "Dora: " << game.dora_tiles[0] << "\n";

    for(int n = 0; n < 14; n++) {
      int suit, val;
      do {
        int s = rand() % 34;
        suit = s / 9 + 1;
        val = s % 9 + 1;
      } while(!initial_state.can_insert(suit, val));
      initial_state.insert(suit, val);
    }
    cout << "Starting hand: " << initial_state << "\n";

    int win_sum = 0;
    int turn_sum = 0;
    for(int s = 0; s < 10000; s++) {
      state_t state = initial_state;
      for(int n = 0; n < 16; n++) {
        int suit, val;
        do {
          int s = rand() % 34;
          suit = s / 9 + 1;
          val = s % 9 + 1;
        } while(!state.can_insert(suit, val));
        state.insert(suit, val);
      }
      // cout << "\nAll tiles: " << state << "\n";
      int turn = full_solve(state);
      if(turn < INFINITE) {
        win_sum += 1;
        turn_sum += turn;
      }
      // cout << "Turn " << turn << "\n";
    }
    float winrate = (win_sum * 1.0 / 10000);
    float average_turn = (turn_sum * 1.0 / win_sum);
    printf("This hand has win rate %.02f and wins by turn %.02f on average.", winrate, average_turn);
  }

  state_t init_from_string(string hand) {
    game.dora_tiles.clear();
    state_t state;
    memset(&state, 0, sizeof(state));

    int suit = 1;
    for(char c : hand) {
      if(c == '-') continue;
      if(c == ' ') {
        suit++;
        continue;
      }
      int val = c - '0';
      state.insert(suit, val);
    }

    return state;
  }

  vector<group_t> all_patterns;
  game_t game;
  grouped_hand_t accumulator;
} solver_t;

void test_scorer() {
  game_t game;
  game.dora_tiles.push_back({SOU, 1});
  game.dora_tiles.push_back({SOU, 1});
  grouped_hand_t hand = {{SOU, 1, CHI}, {SOU, 1, CHI}, {SOU, 1, CHI}, {SOU, 1, CHI}, {SOU, 5, PAIR}};
  cout << game.score(hand, RYANMEN) << "\n";
}

#endif
