#include <iostream>
#include <string>
#include <fstream>
#include "solver.h"
#include "deck.h"
using namespace std;

#define N_TRIALS 200000

tile_t eval_state(solver_t& solver, state_t& initial_state, deck_t& initial_deck, int offset, int n_trials = N_TRIALS) {

  double score_sum = 0;
  double tile_value_sum[10][10][10];
  for(int i = 0; i < 10; i++) {
    for(int j = 0; j < 10; j++) {
      for(int k = 0; k < 10; k++) {
        tile_value_sum[i][j][k] = 0;
      }
    }
  }

  int tiles_left = min(12, 17 - offset);

  for(int n = 1; n <= n_trials; n++) {
    state_t state = initial_state;
    deck_t deck = initial_deck;
    deck.shuffle();
    for(int k = 0; k < tiles_left; k++) {
      tile_t tile = deck.get_next();
      state.insert(tile.suit, tile.val, tile.is_red_dora);
    }
    solver.full_solve(state, offset);

    double score = solver.best_score;
    score_sum += score;

    auto tile_value = solver.value_per_tile;
    for(int tile_ind = 0; tile_ind < 34; tile_ind++) {
      int suit = tile_ind / 9 + 1;
      int val = tile_ind % 9 + 1;
      for(int k = 0; k < 4; k++) {
        tile_value_sum[suit][val][k] += tile_value[suit][val][k];
      }
    }
  }
  printf("Score: %.02f\n", score_sum / n_trials);
  double best_discard_score = 0;
  tile_t best_discard = {0, 0};

  cout << "\nValue of hand after discard:\n";
  for(int tile_ind = 0; tile_ind < 34; tile_ind++) {
    int suit = tile_ind / 9 + 1;
    int val = tile_ind % 9 + 1;
    tile_t tile = {suit, val};
    for(int k = 0; k < initial_state.n_tiles[suit][val]; k++) {
      cout << tile << "\t" << k + 1 << "\t" << tile_value_sum[suit][val][k] / n_trials << "\n";
      if(tile_value_sum[suit][val][k] > best_discard_score) {
        best_discard_score = tile_value_sum[suit][val][k];
        best_discard = tile;
      }
    }
  }
  cout << "\nOptimal discard: " << best_discard << "\n\n";
  return best_discard;
}

void eval(string hand, int offset = 0, int n_trials = N_TRIALS) {
  solver_t solver;
  state_t initial_state = solver.init_from_string(hand, offset);
  deck_t initial_deck;
  initial_deck.init_from_state(initial_state);

  eval_state(solver, initial_state, initial_deck, offset, n_trials);
}

/*
void get_turn_data() {
  solver_t solver;
  state_t initial_state = solver.init_from_string("");
  deck_t initial_deck;
  initial_deck.init_from_state(initial_state);

  ofstream fout("out.txt");

  for(int n = 1; n <= 100000000; n++) {
    state_t state = initial_state;
    deck_t deck = initial_deck;
    deck.shuffle();
    for(int k = 0; k < 31; k++) {
      tile_t tile = deck.get_next();
      state.insert(tile.suit, tile.val);
    }
    int score = solver.full_solve(state);
    fout << score << "\n";
    if(n % 1000000 == 0) {
      printf("Finished trial %d\n", n);
    }
  }
}

void analyze_turn_data() {
  ifstream fin("out.txt");
  int freq[100];
  memset(freq, 0, sizeof(freq));
  for(int n = 1; n <= 100000000; n++) {
    int k;
    fin >> k;
    if(k < 100) {
      freq[k]++;
    }
  }
  for(int k = 1; k < 100; k++) {
    if(freq[k] > 0) {
      cout << k << " " << freq[k] << "\n";
    }
  }
}
*/

void test() {
  string hand = "138 6779 1889 113";
  // hand = "337 1234789 5677 -";
  eval(hand);
}

void eval_loop() {
  while(1) {
    cin.clear();
    string hand;
    cout << "Input hand: ";
    getline(cin, hand);
    cout << "How many discards so far: ";
    int n_discards;
    string s;
    getline(cin, s);
    n_discards = stoi(s);
    eval(hand, n_discards);
  }
}

void command_loop() {
  solver_t solver;

  state_t current_hand;
  state_t all_drawn_tiles;
  memset(&current_hand, 0, sizeof(current_hand));
  memset(&all_drawn_tiles, 0, sizeof(all_drawn_tiles));
  int n_discards = 0;

  while(1) {
    cin.clear();
    string input;
    cout << ">> ";
    getline(cin, input);
    char command = input[0];

    if(command == 'r' && input.size() == 1) {
      // reset state
      solver.reset_game_state();
      memset(&current_hand, 0, sizeof(current_hand));
      memset(&all_drawn_tiles, 0, sizeof(all_drawn_tiles));
      n_discards = 0;
      cout << "Game state completely reset.\n";
    } else if(command == 'd') {
      string data = input.substr(2);
      tile_t tile = parse_tile(data);
      cout << "Adding tile " << tile << " to list of dora\n";
      solver.game.dora_tiles.push_back(tile);
    } else if(command == 't') {
      string data = input.substr(2);
      int n = stoi(data);
      n_discards += n;
      all_drawn_tiles.n += n;
      current_hand.n += n;
      cout << "Thrown away " << n << " tiles.\n";
    } else if(input.size() > 5) {
      current_hand = solver.init_from_string(input);
      all_drawn_tiles = current_hand;
      cout << "Hand set to " << input << "\n";
    } else if(input.size() > 0) {
      tile_t drawn_tile = parse_tile(input);
      current_hand.insert(drawn_tile.suit, drawn_tile.val, drawn_tile.is_red_dora);
      all_drawn_tiles.insert(drawn_tile.suit, drawn_tile.val, drawn_tile.is_red_dora);

      deck_t initial_deck;
      initial_deck.init_from_state(all_drawn_tiles);

      tile_t best_discard = eval_state(solver, current_hand, initial_deck, n_discards);
      cout << best_discard << "\n";
      n_discards++;
      current_hand.remove(best_discard.suit, best_discard.val);
      cout << "Current hand: " << current_hand << "\n\n";
    }
  }
}

int main() {
  srand(time(0));
  command_loop();
  return 0;
}
