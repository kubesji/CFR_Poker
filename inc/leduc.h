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

#ifndef _HOLDEM_H
#define _HOLDEM_H

#include <string>
#include <array>

#include "settings.h"
#include "player.h"
#include "deck.h"
#include "action.h"

enum class Round{
    PREFLOP = 0,
    FLOP = 1,
    REVEAL = 2
};

class Leduc{
public:
    Leduc(uint16_t big_bling, uint16_t small_blind, uint8_t max_reraises);
    Leduc(Leduc& h);
    void start_game();
    bool is_running();
    int get_reward(int8_t hero);
    int8_t next_player();
    void take_action(const Action& a);
    bool can_call(uint8_t player_idx) const noexcept;
    bool can_raise(uint8_t player_idx, const Action& a) const noexcept;
    // inline std::string get_history() const noexcept {return m_history;};
    inline std::string get_player_cards_str(int player) const noexcept {return m_players[player].get_rank_str();};
    inline const std::array<Card*, 2>& get_player_cards(int player) const noexcept {return m_players[player].get_cards();};
    inline int8_t get_current_player() const noexcept {return m_current_player;};
    int8_t find_winner();
    void update_ranks();
    inline Round get_round() const noexcept {return m_round;};
    inline const Card* get_flop() const noexcept {return m_flop;};

    std::array<uint8_t, N_ACTIONS> get_valid_actions_mask(int player);
    std::vector<Action> get_valid_actions(int player);
    Action sample_action(std::array<float, N_ACTIONS> strategy, uint8_t player);
    inline std::array<Action, N_ACTIONS> get_actions() const noexcept {return m_actions;};

    std::string create_key(uint8_t player);
private:
    void shuffle_cards();
    inline Card* draw_card() noexcept { return &m_deck[m_pointer_to_deck++]; };  
    bool check_premature_end();
    uint8_t find_max_pot_contribution();
    bool is_round_end();
    void compress_history();
    void reset_player_states();
    void next_round();
    std::string round_to_str() const noexcept;
    int count_remaining_players() noexcept;

    int8_t m_current_player;
    Round m_round;
    int8_t m_winner;
    std::string m_history;
    uint8_t m_max_reraises;
    
    uint8_t m_n_players;
    std::array<Player, N_PLAYERS> m_players;
    std::array<int8_t, N_PLAYERS> m_ranks;

    std::array<Card, 6> m_deck;
    int m_pointer_to_deck;
    
    uint16_t m_big_blind;
    uint16_t m_small_blind;
    uint16_t m_pot;

    Card* m_flop;

    std::array<Action, N_ACTIONS> m_actions;
};

#endif