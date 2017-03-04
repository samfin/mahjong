#include <iostream>
#include <string>
#include "solver.h"
#include "deck.h"
using namespace std;

#define N_TRIALS 10000

int eval(string hand, int n_trials = N_TRIALS) {
  solver_t solver;
  state_t initial_state = solver.init_from_string(hand);
  deck_t initial_deck;
  initial_deck.init_from_state(initial_state);

  int win_sum = 0;
  int turn_sum = 0;

  for(int n = 1; n <= n_trials; n++) {
    state_t state = initial_state;
    deck_t deck = initial_deck;
    deck.shuffle();
    for(int k = 0; k < 16; k++) {
      tile_t tile = deck.get_next();
      state.insert(tile.suit, tile.val);
    }
    int score = solver.full_solve(state);

    if(score < INFINITE) {
      win_sum += 1;
      turn_sum += score;
    }
  }
  float winrate = (win_sum * 1.0 / n_trials);
  float average_turn = (turn_sum * 1.0 / win_sum);
  printf("[%s] has win rate %.02f and wins by turn %.02f on average.", hand.c_str(), winrate, average_turn);
}

void test() {
  string hand = "23357 11238 688 3";
  eval(hand);
}

int main() {
  srand(time(0));
  test();
  return 0;
}
