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

#ifndef _PLYER_H
#define _PLAYER_H

#include <string>
#include <array>

#include "rank.h"
#include "card.h"

enum class PlayerState {
    NO_ACTION = 0,
    IN = 1,
    OUT = 2,
    TO_CALL = 3,
    ALL_IN = 4
};


class Player{
public:
    Player(): m_state(PlayerState::NO_ACTION), 
              m_pot_contribution(0),
              m_rank_str(""),
              m_cards(),
              m_n_raises(0),
              m_card_idx(0),
              m_rank(),
              m_history("-") {};

    Player(PlayerState state, int blind): m_state(state), 
                                          m_pot_contribution(blind),
                                          m_rank_str(""),
                                          m_cards(),
                                          m_n_raises(0),
                                          m_card_idx(0),
                                          m_rank(),
                                          m_history("-") {};

    // Player(Player& p)
    // {
    //     m_state = p.m_state; 
    //     m_pot_contribution = p.m_pot_contribution;
    //     m_card_str = "";
    //     m_cards = p.m_cards;
    //     m_n_raises = p.m_n_raises;
    //     m_card_idx = p.m_card_idx;
    // }

    inline void reset_counters() noexcept {m_n_raises = 0;}
    inline void increase_raise_counter() noexcept{m_n_raises++;}
    inline int get_raise_counter() const noexcept{return m_n_raises;}

    inline void draw_card(Card* card) noexcept {m_cards[m_card_idx++] = card;}
    inline std::string get_card_str(int i) const noexcept {return std::string(*m_cards[i]);}
    inline const std::array<Card*, 2>& get_cards() const noexcept {return m_cards;}

    inline int get_pot_contribution() const noexcept{return m_pot_contribution;}
    inline void set_pot_contribution(int pot_cntr) noexcept{m_pot_contribution = pot_cntr;}

    inline PlayerState get_state() const noexcept{return m_state;}
    inline void set_state(const PlayerState& state) noexcept{m_state = state;}

    inline std::string get_rank_str() const noexcept {return m_rank_str;}
    inline void set_rank_str(const std::string &rank_str) noexcept {m_rank_str = rank_str;}

    inline Rank get_rank() const noexcept {return m_rank;}
    inline void set_rank(Rank rank) noexcept {m_rank = rank;}

    inline int count_cards(Card* card) const noexcept{
        int count = 0;
        for (Card* c : m_cards){
            if (*c == *card) count++;
        }
        return count;
    }

    inline int count_cards(char card) const noexcept{
        int count = 0;
        for (Card* c : m_cards){
            if (c->get_value_str() == card) count++;
        }
        return count;
    }

    inline int count_cards(char card, char suit) const noexcept{
        int count = 0;
        for (Card* c : m_cards){
            if (c->get_value_str() == card && c->get_suit() == suit) count++;
        }
        return count;
    }

    inline void new_action(char a) noexcept {
        if (m_history.back() == '-') m_history.push_back(a);
        else m_history[m_history.size() - 1] = a;
    };

    inline char get_last_action() const noexcept {return m_history.back();};
    inline void reset_action() noexcept {m_history.push_back('-');};
    inline std::string get_history() const noexcept {return m_history;};
    inline std::string get_history_without_current_round() const noexcept { 
        return m_history.substr(0, m_history.find_last_of('-')+1);
    };

private:
    PlayerState m_state;
    int m_pot_contribution;
    std::string m_rank_str;
    std::array<Card*, 2> m_cards;
    int m_n_raises;
    int m_card_idx;
    Rank m_rank;
    std::string m_history;
};
#endif