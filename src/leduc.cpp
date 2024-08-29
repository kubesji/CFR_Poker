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

#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <random>
#include <algorithm>
#include <chrono>

#include "leduc.h"
#include "player.h"
#include "settings.h"
#include "utils.h"

Leduc::Leduc(uint16_t big_bling, uint16_t small_blind, uint8_t max_reraises)
    : m_n_players(2)
    , m_big_blind(big_bling)
    , m_small_blind(small_blind)
    , m_max_reraises(max_reraises)
    , m_winner(-1)
    , m_pointer_to_deck(0) {
    m_actions = {Action(0, 'p', 0), Action(1, 'c', 0), Action(2, 'A', 1),
                 Action(3, 'B', 2), Action(4, 'C', 3), Action(5, 'D', 5)};
    m_deck = {Card(45), Card(44), Card(41), Card(40), Card(37), Card(36)};
};

Leduc::Leduc(Leduc &h)
    : m_deck(h.m_deck)
    , m_n_players(h.m_n_players)
    , m_big_blind(h.m_big_blind)
    , m_small_blind(h.m_small_blind)
    , m_max_reraises(h.m_max_reraises)
    , m_current_player(h.m_current_player)
    , m_round(h.m_round)
    , m_history(h.m_history)
    , m_winner(h.m_winner)
    , m_pot(h.m_pot)
    , m_flop(h.m_flop)
    , m_players(h.m_players)
    , m_actions(h.m_actions)
    , m_pointer_to_deck(h.m_pointer_to_deck) {
    update_ranks();
}

void Leduc::shuffle_cards() {    
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    m_pointer_to_deck = 0;
    std::shuffle(m_deck.begin(), m_deck.end(), rng);
}

void Leduc::start_game() {
    shuffle_cards();

    for (int8_t i = 0; i < m_n_players; i++) {
        PlayerState state = (i < m_n_players - 1) ? PlayerState::TO_CALL : PlayerState::IN;
        int bet = i == 0 ? m_small_blind : m_big_blind;

        m_players[i] = Player(state, bet);
        m_players[i].draw_card(draw_card());
    }

    m_current_player = 0;
    m_round = Round::PREFLOP;
    // m_history = "-";
    m_winner = -1;

    update_ranks();
    m_pot = m_big_blind + m_small_blind;

    m_flop = draw_card();
}

bool Leduc::is_running() { return m_winner < 0; }

int Leduc::get_reward(int8_t hero) {
    if (is_running()) {
        throw std::out_of_range("Asking for reward of running game");
    }
    if (m_winner == hero) {
        return m_pot - m_players[hero].get_pot_contribution();
    } else {
        return -1 * m_players[hero].get_pot_contribution();
    }
}

int8_t Leduc::next_player() {
    /* TODO bigblind player in preflop skipped now - fix it */
    if (m_history.size() == 0) {
        m_current_player = 0;
    } else {
        m_current_player = (m_current_player + 1) % m_n_players;
        while (m_players[m_current_player].get_state() == PlayerState::OUT ||
               m_players[m_current_player].get_state() == PlayerState::ALL_IN) {
            m_current_player = (m_current_player + 1) % m_n_players;
        }
    }
    return m_current_player;
}

void Leduc::take_action(const Action &a) {
    bool game_finished = false;
    char previous_player_action = m_players[(m_current_player + N_PLAYERS - 1) % N_PLAYERS].get_last_action();
    Player &player = m_players[m_current_player];

    /*  Fold or check */
    if (char(a) == 'p') {
        /* Fold, no further actions for check */
        player.new_action('p');
        if (player.get_state() == PlayerState::TO_CALL) {
            player.set_state(PlayerState::OUT);
            game_finished = check_premature_end();
        } else {
            player.set_state(PlayerState::IN);
        }
    }

    /* Call */
    else if (char(a) == 'c') {
        /* Find max pot contribution */
        int call_value = find_max_pot_contribution() - player.get_pot_contribution();
        m_pot += call_value;
        player.set_pot_contribution(player.get_pot_contribution() + call_value);
        player.set_state(PlayerState::IN);
        /* If my last action was raise -> opponent reraised it -> capital C; otherwise opponent was the first
           to raise -> lower c */
        player.new_action(std::tolower(player.get_last_action()) == 'r' ? 'C' : 'c');
    }
    /* Different values of bets */
    else {
        int raise_value = a.get_value() * m_big_blind;
        int call_value = find_max_pot_contribution() - player.get_pot_contribution();
        m_pot += call_value + raise_value;
        player.set_pot_contribution(player.get_pot_contribution() + call_value + raise_value);
        for (Player &p : m_players) {
            if (p.get_state() == PlayerState::IN || p.get_state() == PlayerState::NO_ACTION) {
                p.set_state(PlayerState::TO_CALL);
            }
        }
        player.set_state(PlayerState::IN);
        player.increase_raise_counter();
        /* If previous player raised or called bet of another player and I'm raising, it means it is reraise ->
           capital R; otherwise I'm the first to raise -> lower r */
        player.new_action((previous_player_action != 'p' && previous_player_action != '-') ? 'R' : 'r');
    }
    m_history.push_back(char(a));

    if (game_finished)
        return;

    if (is_round_end()) {
        next_round();
        if (m_round == Round::REVEAL) {
            m_winner = find_winner();
        } else {
            compress_history();
            update_ranks();
            reset_player_states();
        }
    }
}

bool Leduc::can_call(uint8_t player_idx) const noexcept {
    return m_players[player_idx].get_state() == PlayerState::TO_CALL;
}

bool Leduc::can_raise(uint8_t player_idx, const Action &a) const noexcept {
    /* Works only for two player game at the moment */
    bool reraise_allowed = m_players[player_idx].get_raise_counter() < m_max_reraises;
    char b = m_history.size() ? m_history.back() : 'x';
    bool bet_too_low = std::isupper(char(a)) && std::isupper(b) && char(a) < b;
    return reraise_allowed && !bet_too_low;
}

void Leduc::update_ranks() {
    for (int i = 0; i < m_n_players; i++) {
        Player &p = m_players[i];
        if (m_round == Round::PREFLOP) {
            std::string rank_str = "";
            Card* card = p.get_cards()[0];
            p.set_rank_str(std::string(*card)); 
            m_ranks[i] = card->get_value();
        } else {
            Card* p_card = p.get_cards()[0];
            if (p_card->get_value() == m_flop->get_value()){
                /* Pair */
                p.set_rank_str("2" + std::string(1, m_flop->get_value_str()));   
                m_ranks[i] = m_flop->get_value() + 100;
            } else {
                if (p_card->get_suit() == m_flop->get_suit()){
                    /* Suited but no pair */
                    std::string s = std::string(1, p_card->get_value_str());
                    s.append(std::string(1, m_flop->get_value_str()));
                    p.set_rank_str(s + "s");  
                    m_ranks[i] = p_card->get_value() + m_flop->get_value() + 50;
                } else {
                    /* No pair, not suited*/
                    /* Suited but no pair */
                    std::string s = std::string(1, p_card->get_value_str());
                    s.append(std::string(1, m_flop->get_value_str()));
                    p.set_rank_str(s + "o");  
                    m_ranks[i] = p_card->get_value() + m_flop->get_value();
                }
            }
        }
    }
}

void Leduc::reset_player_states() {
    for (Player &p : m_players) {
        if (p.get_state() == PlayerState::IN) {
            p.set_state(PlayerState::NO_ACTION);
        }
        p.reset_counters();
        p.reset_action();
    }
}

void Leduc::next_round() {
    if (m_round == Round::REVEAL)
        return;
    int i = static_cast<int>(m_round);
    m_round = static_cast<Round>((i + 1) % (static_cast<int>(Round::REVEAL) + 1));
}

std::string Leduc::round_to_str() const noexcept {
    switch (m_round) {
    case Round::PREFLOP:
        return std::string("P.");
    case Round::FLOP:
        return std::string("F.");
    case Round::REVEAL:
        return std::string("E.");
    default:
        return std::string("?.");
    }
}

bool Leduc::check_premature_end() {
    uint8_t n_in = 0;
    for (Player &p : m_players) {
        if (p.get_state() != PlayerState::OUT)
            n_in++;
    }
    if (n_in == 1) {
        for (int8_t i = 0; i < m_n_players; i++) {
            if (m_players[i].get_state() == PlayerState::IN) {
                m_winner = i;
                return true;
            }
        }
    }
    return false;
}

uint8_t Leduc::find_max_pot_contribution() {
    int max = 0;
    for (Player &p : m_players) {
        if (p.get_pot_contribution() > max) {
            max = p.get_pot_contribution();
        }
    }
    return max;
}

bool Leduc::is_round_end() {
    bool noone_to_call = true, all_played = true;
    for (Player &p : m_players) {
        PlayerState state = p.get_state();
        if (state == PlayerState::TO_CALL)
            noone_to_call = false;
        if (state == PlayerState::NO_ACTION)
            all_played = false;
    }
    return noone_to_call && all_played;
}

int8_t Leduc::find_winner() {
    int8_t best_player = 0;
    /* Worst combo + 1*/
    int8_t best_combo = 0;
    for (int8_t i = 0; i < m_n_players; i++) {
        if (m_players[i].get_state() == PlayerState::IN) {
            if (m_ranks[i] > best_combo) {
                best_combo = m_ranks[i];
                best_player = i;
            }
        }
    }
    return best_player;
}

void Leduc::compress_history() { m_history = std::to_string(count_remaining_players());}

std::array<uint8_t, N_ACTIONS> Leduc::get_valid_actions_mask(int player) { 
    std::array<uint8_t, N_ACTIONS> mask;
    mask.fill(1); 

    for (int i = 0; i < N_ACTIONS; i++) {
        if (char(m_actions[i]) == 'c' && !can_call(player)) {
            mask[i] = 0;
        }
        if (std::isupper(char(m_actions[i])) && !can_raise(player, m_actions[i])) {
            mask[i] = 0;
        }
    }
    return mask;
}


std::vector<Action> Leduc::get_valid_actions(int player){
    std::vector<Action> valid{}; 
    for (Action a : m_actions) {
        if (char(a) == 'c' && !can_call(player)) {
            continue;
        }
        if (std::isupper(char(a)) && !can_raise(player, a)) {
            continue;
        }
        valid.push_back(a);
    }
    return valid;
}

/* TODO strategy as reference or not? */
Action Leduc::sample_action(std::array<float, N_ACTIONS> strategy, uint8_t player){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d(strategy.begin(), strategy.end());
    int a = d(gen);
    return m_actions[a];
}

std::string Leduc::create_key(uint8_t player)
{
    std::string key;
    key.append(get_player_cards_str(player));
    key.append(m_players[player].get_history_without_current_round());
    key.append(m_history);
    return pad_string(key, KEY_LENGTH);
}

int Leduc::count_remaining_players() noexcept{
    int count = 0;
    for (Player &p : m_players) {
        if (p.get_state() == PlayerState::IN || p.get_state() == PlayerState::NO_ACTION){
            count++;
        }
    }
    return count;
}