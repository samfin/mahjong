#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>
#include <string>
#include <sstream>
using namespace std;

#ifndef TILE_H
#define TILE_H

#define MAN 1
#define PIN 2
#define SOU 3
#define HONORS 4

#define EAST 1
#define SOUTH 2
#define WEST 3
#define NORTH 4
#define HAKU 5
#define HATSU 6
#define CHUN 7

#define PON 1
#define CHI 2
#define PAIR 3

#define RYANMEN 1
#define PENCHAN 2
#define KANCHAN 3
#define SHANPON 4
#define TANKI 5

typedef struct _tile_t {
  int suit:8;
  int val:8;

  bool operator==(struct _tile_t& tile) {
    return suit == tile.suit && val == tile.val;
  }
} tile_t;

typedef struct {
  int suit;
  int val;
  int type;

  bool contains(tile_t tile) {
    if(type == CHI) {
      return (suit == tile.suit) && (tile.val >= val) && (tile.val <= val + 2);
    } else {
      return (suit == tile.suit) && (val == tile.val);
    }
  }
} group_t;

typedef vector<tile_t> tile_list_t;
typedef vector<group_t> grouped_hand_t;

tile_list_t get_tiles(const group_t& group) {
  vector<tile_t> out;
  int n = 3 - (group.type == PAIR);
  for(int i = 0; i < n; i++) {
    int val = (group.type == CHI) * i + group.val;
    out.push_back({ group.suit, val });
  }
  return out;
}

const string honor_names[10] = {"kappa", "East", "South", "West", "North", "Haku", "Hatsu", "Chun"};
const string abbreviated_suit_names[4] = {"kappaross", "m", "p", "s"};
ostream& operator<<(ostream& stream, const tile_t& t) {
  if(t.suit == HONORS)
    stream << honor_names[t.val];
  else
    stream << t.val << abbreviated_suit_names[t.suit];
  return stream;
}

ostream& operator<<(ostream& stream, const group_t& g) {
  tile_list_t tiles = get_tiles(g);
  stream << "[";
  for(int i = 0; i < tiles.size(); i++) {
    if(i > 0) stream << " ";
    stream << tiles[i];
  }
  stream << "]";
  return stream;
}

ostream& operator<<(ostream& stream, const grouped_hand_t& hand) {
  for(int i = 0; i < hand.size(); i++) {
    if(i > 0) stream << " ";
    stream << hand[i];
  }
  return stream;
}

int get_wait_type(group_t& group, tile_t& last_tile) {
  if(group.type == PON)
    return SHANPON;

  if(group.type == PAIR)
    return TANKI;

  if(last_tile.val == group.val + 1)
    return KANCHAN;

  if(group.val == 1 || group.val == 7)
    return PENCHAN;

  return RYANMEN;
}

#endif
