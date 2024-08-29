/*
 *  Copyright 2024 Jiri Kubes
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 
 #ifndef _RANK_H
#define _RANK_H

#include <stdbool.h>
#include <array>
#include <string>
#include <vector>
#include <cstdint>

#include "card.h"
#include "tables/tables.h"

enum class RankCategory {
  // FIVE_OF_A_KIND = 0, // Reserved
  StraightFlush = 1,
  FourOfAKind,
  FullHouse,
  Flush,
  Straight,
  ThreeOfAKind,
  TwoPairs,
  OnePair,
  HighCard,
};


class Rank {
public:
    Rank(){m_value = 0xFFFF;};
    Rank(std::array<Card*, 2> player, const std::array<Card*, 3>& flop);
    Rank(std::array<Card*, 2> player, const std::array<Card*, 3>& flop, Card* turn);
    Rank(std::array<Card*, 2> player, const std::array<Card*, 3>& flop, Card* turn, Card* river);

    bool operator<(const Rank& other) const { return m_value > other.m_value; }
    bool operator<=(const Rank& other) const { return m_value >= other.m_value; }
    bool operator>(const Rank& other) const { return m_value < other.m_value; }
    bool operator>=(const Rank& other) const { return m_value <= other.m_value; }
    bool operator==(const Rank& other) const { return m_value == other.m_value; }
    bool operator!=(const Rank& other) const { return m_value != other.m_value; }

    std::string get_string_representation() const noexcept;

private:
    int hash_nonflush(const uint8_t q[], int k);

    RankCategory get_rank_category() const noexcept;
    char bin_card(const Card &c) const;
    char bin_card(char card) const;
    inline char bool_to_str(bool b) const noexcept { return b ? 't' : 'f'; }

    inline const std::string& cards_in_combo() const noexcept {return rank_description[m_value][0];}
    inline const std::string& describe_rank() const noexcept {return rank_description[m_value][1];}

    inline int count_player_cards(char card) const noexcept;
    inline int count_player_cards(char card, int8_t suit) const noexcept;
    char find_non_poker_pair(char poker_card) const noexcept;

    bool compare_cards(char a, char b) const noexcept;

    uint16_t m_value = 0;
    char m_possible_flush = 'x';
    char m_board_flush = 'x';
    char m_possible_straight = 'x';
    char m_board_straight = 'x';
    int8_t m_suit = -1;

    char m_is_flush = 'o';

    std::array<Card*, 2> m_player_cards;
    std::vector<Card*> m_board_cards;

    bool m_three_of_a_kind_on_board = false;
    bool m_four_of_a_kind_on_board = false;
    uint8_t m_pairs_on_board = 0;
};

#endif
