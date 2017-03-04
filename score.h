#include <iostream>
#include <vector>
#include <string.h>
#include "tiles.h"
using namespace std;

#ifndef SCORE_H
#define SCORE_H

const int yakuhai_val[10] = {0, 2, 0, 0, 0, 1, 1, 1};
const int maxed_points[100] = {0, 0, 0, 0, 0, 8000, 12000, 12000, 16000, 16000, 16000, 24000, 24000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000};

typedef struct {
  int score(grouped_hand_t& hand, bool is_ryanmen) {
    int han = 1; // We are counting riichi, but not tsumo
    int fu = 30 + 2 * (!is_ryanmen);

    int suit_freq[10];
    int freq[10][10][10];
    memset(suit_freq, 0, sizeof(suit_freq));
    memset(freq, 0, sizeof(freq));

    bool is_tanyao = true;
    bool is_chanta = true;
    int n_pons = 0;
    int n_peikou = 0;
    int n_yakuhai = 0;
    int n_dora = 0;
    int n_groups = hand.size();

    for(group_t group : hand) {
      freq[group.suit][group.val][group.type]++;
      suit_freq[group.suit]++;

      auto tiles = get_tiles(group);
      for(tile_t tile : tiles) {
        for(tile_t dora : dora_tiles) {
          n_dora += (tile == dora);
        }
      }

      bool is_yakuhai = (group.suit == HONORS && yakuhai_val[group.val] > 0);
      bool is_terminal = ( (group.suit == HONORS) || (group.val == 1 || group.val == 9 || (group.val == 7 && group.type == CHI)) );

      // Some bookkeeping
      if(is_terminal)
        is_tanyao = false;

      if(!is_terminal)
        is_chanta = false;

      if(group.type == PON) {
        n_pons++;
        if(is_terminal)
          fu += 8;
        else
          fu += 4;
      }

      if(group.type == PAIR && is_yakuhai)
        fu += 2;

      if(group.type == CHI && freq[group.suit][group.val][group.type] % 2 == 0)
        n_peikou++;
    }

    // Chitoitsu
    if(n_groups == 7) {
      han += 2;
      fu = 25;
    }

    // Tanyao
    if(is_tanyao)
      han += 1;

    // Pinfu
    if(fu == 30)
      han += 1;

    // Ippekou, ryanpeikou
    if(n_peikou == 1)
      han += 1;
    else if(n_peikou == 2)
      han += 3;

    // Yakuhai
    han += 2 * freq[HONORS][EAST][PON];
    han += freq[HONORS][HAKU][PON];
    han += freq[HONORS][HATSU][PON];
    han += freq[HONORS][CHUN][PON];

    // Sanshoku
    for(int val = 1; val <= 7; val++) {
      han += 2 * (freq[SOU][val][CHI] > 0) * (freq[PIN][val][CHI] > 0) * (freq[MAN][val][CHI] > 0);
    }

    // San ankou
    if(n_pons >= 3)
      han += 2;

    // Toitoi, suu ankou
    if(n_pons == 4) {
      han += 2;
      han += 13;
    }

    // Chanta, junchan 
    if(is_chanta) {
      han += 2;
      if(suit_freq[HONORS] == 0)
        han += 1;
    }

    // Itsu
    for(int suit = MAN; suit <= SOU; suit++) {
      han += 2 * (freq[suit][1][CHI] > 0) * (freq[suit][4][CHI] > 0) * (freq[suit][7][CHI] > 0);
    }

    // Honitsu, chinitsu
    int best_suit = max( max(suit_freq[MAN], suit_freq[PIN]), suit_freq[SOU]);
    if(best_suit == n_groups)
      han += 6;
    else if(best_suit == n_groups - suit_freq[HONORS])
      han += 3;

    // Dora
    han += n_dora;

    // cout << han << " han, " << fu << " fu (" << n_dora << " dora)\n";

    if(han >= 5) {
      return maxed_points[han];
    } else {
      int rounded_fu = 10 * ((fu + 9) / 10);
      int basic_points = (16 << han) * rounded_fu;
      basic_points = 100 * ((basic_points + 99) / 100);
      if(basic_points > 8000) basic_points = 8000;
      return basic_points;
    }
  }

  vector<tile_t> dora_tiles;
} game_t;

#endif
