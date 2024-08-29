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

#include "game.h"
#include "deck.h"
#include "player.h"
#include "settings.h"
#include "utils.h"

Holdem::Holdem(uint8_t n_players, uint16_t big_bling, uint16_t small_blind, uint8_t max_reraises)
    : m_n_players(n_players)
    , m_big_blind(big_bling)
    , m_small_blind(small_blind)
    , m_max_reraises(max_reraises)
    , m_winner({-1})
    , m_deck(Deck()) {
    m_actions = {Action(0, 'p', 0), Action(1, 'c', 0), Action(2, 'A', 1),
                 Action(3, 'B', 2), Action(4, 'C', 3), Action(5, 'D', 5)};
};

Holdem::Holdem(Holdem &h)
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
    , m_turn(h.m_turn)
    , m_river(h.m_river)
    , m_players(h.m_players)
    , m_actions(h.m_actions) {
    update_ranks();
}

void Holdem::start_game() {
    m_deck.shuffle();

    for (int8_t i = 0; i < m_n_players; i++) {
        PlayerState state = (i < m_n_players - 1) ? PlayerState::TO_CALL : PlayerState::NO_ACTION;
        int bet = m_big_blind;

        if (i < (m_n_players - 2)) {
            bet = 0;
        } else if (i < (m_n_players - 1)) {
            bet = m_small_blind;
        }
        m_players[i] = Player(state, bet);
        for (int j = 0; j < 2; j++) {
            m_players[i].draw_card(m_deck.draw());
        }
    }

    m_current_player = 0;
    m_round = Round::PREFLOP;
    // m_history = "-";
    m_winner = {-1};

    update_ranks();
    m_pot = m_big_blind + m_small_blind;

    for (int i = 0; i < 3; i++) {
        m_flop[i] = m_deck.draw();
    }
    m_turn = m_deck.draw();
    m_river = m_deck.draw();
}

bool Holdem::is_running() { return m_winner[0] < 0; }

int Holdem::get_reward(int8_t hero) {
    if (is_running()) {
        throw std::out_of_range("Asking for reward of running game");
    }
    if (m_winner.size() == 1) {
        if (m_winner[0] == hero) {
            return m_pot - m_players[hero].get_pot_contribution();
        } else {
            return -1 * m_players[hero].get_pot_contribution();
        }
    } else {
        /* Is hero among winners? */
        /* TODO only works for 2 player game */
        if (std::find(m_winner.begin(), m_winner.end(), hero) != m_winner.end()) {
            float pot_share = static_cast<float>(m_pot) / m_winner.size();
            return pot_share - m_players[hero].get_pot_contribution();
        } else {
            return -1 * m_players[hero].get_pot_contribution();
        }
    }
}

int8_t Holdem::next_player() {
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

void Holdem::take_action(const Action &a) {
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

bool Holdem::can_call(uint8_t player_idx) const noexcept {
    return m_players[player_idx].get_state() == PlayerState::TO_CALL;
}

bool Holdem::can_raise(uint8_t player_idx, const Action &a) const noexcept {
    /* Works only for two player game at the moment */
    bool reraise_allowed = m_players[player_idx].get_raise_counter() < m_max_reraises;
    char b = m_history.size() ? m_history.back() : 'x';
    bool bet_too_low = std::isupper(char(a)) && std::isupper(b) && char(a) < b;
    return reraise_allowed && !bet_too_low;
}

void Holdem::update_ranks() {
    Rank rank;
    for (Player &p : m_players) {
        if (m_round == Round::PREFLOP) {
            std::string rank_str = "";
            const std::array<Card *, 2> &cards = p.get_cards();
            if (*cards[0] > *cards[1]) {
                rank_str += cards[0]->get_value_str();
                rank_str += cards[1]->get_value_str();
            } else {
                rank_str += cards[1]->get_value_str();
                rank_str += cards[0]->get_value_str();
            }
            rank_str.append(cards[0]->get_suit() == cards[1]->get_suit() ? "s" : "o");
            p.set_rank_str(rank_str);
        } else {
            if (m_round == Round::FLOP) {
                rank = Rank(p.get_cards(), m_flop);
            } else if (m_round == Round::TURN) {
                rank = Rank(p.get_cards(), m_flop, m_turn);
            } else if (m_round == Round::RIVER) {
                rank = Rank(p.get_cards(), m_flop, m_turn, m_river);
            }
            p.set_rank(rank);
            // p.set_rank_str(round_to_str() + rank.get_string_representation());
            p.set_rank_str(rank.get_string_representation());
        }
    }
}

void Holdem::reset_player_states() {
    for (Player &p : m_players) {
        if (p.get_state() == PlayerState::IN) {
            p.set_state(PlayerState::NO_ACTION);
        }
        p.reset_counters();
        p.reset_action();
    }
}

void Holdem::next_round() {
    if (m_round == Round::REVEAL)
        return;
    int i = static_cast<int>(m_round);
    m_round = static_cast<Round>((i + 1) % (static_cast<int>(Round::REVEAL) + 1));
}

std::string Holdem::round_to_str() const noexcept {
    switch (m_round) {
    case Round::PREFLOP:
        return std::string("P.");
    case Round::FLOP:
        return std::string("F.");
    case Round::TURN:
        return std::string("T.");
    case Round::RIVER:
        return std::string("R.");
    case Round::REVEAL:
        return std::string("E.");
    default:
        return std::string("?.");
    }
}

bool Holdem::check_premature_end() {
    uint8_t n_in = 0;
    for (Player &p : m_players) {
        if (p.get_state() != PlayerState::OUT)
            n_in++;
    }
    if (n_in == 1) {
        for (int8_t i = 0; i < m_n_players; i++) {
            if (m_players[i].get_state() == PlayerState::IN) {
                m_winner = {i};
                return true;
            }
        }
    }
    return false;
}

uint8_t Holdem::find_max_pot_contribution() {
    int max = 0;
    for (Player &p : m_players) {
        if (p.get_pot_contribution() > max) {
            max = p.get_pot_contribution();
        }
    }
    return max;
}

bool Holdem::is_round_end() {
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

std::vector<int8_t> Holdem::find_winner() {
    std::vector<int8_t> best_players;
    /* Worst combo + 1*/
    Rank best_combo = Rank();
    for (int8_t i = 0; i < m_n_players; i++) {
        if (m_players[i].get_state() == PlayerState::IN) {
            Rank rank = m_players[i].get_rank();
            if (rank > best_combo) {
                best_combo = rank;
                best_players.clear();
                best_players.push_back(i);
            } else if (rank == best_combo) {
                best_players.push_back(i);
            }
        }
    }
    return best_players;
}

void Holdem::compress_history() { m_history = std::to_string(count_remaining_players());}

std::array<uint8_t, N_ACTIONS> Holdem::get_valid_actions_mask(int player) { 
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


std::vector<Action> Holdem::get_valid_actions(int player){
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
Action Holdem::sample_action(const std::array<float, N_ACTIONS>& strategy, const std::array<uint8_t, N_ACTIONS>& valid, uint8_t player){
    std::uniform_real_distribution<double> unif(0, 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::array<float, N_ACTIONS> s;

    if (unif(gen) > EPSILON){
        s = strategy;
    } else {
        for (int i = 0; i < N_ACTIONS; i++){
            s[i] = static_cast<float>(valid[i]);
        }
    }
    std::discrete_distribution<> d(s.begin(), s.end());
    int a = d(gen);
    return m_actions[a];
}

Action Holdem::sample_action(const std::array<float, N_ACTIONS>& strategy, uint8_t player){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d(strategy.begin(), strategy.end());
    int a = d(gen);
    return m_actions[a];
}
std::string Holdem::create_key(uint8_t player)
{
    std::string key;
    key.append(get_player_cards_str(player));
    key.append(m_players[player].get_history_without_current_round());
    key.append(m_history);
    return pad_string(key, KEY_LENGTH);
}

int Holdem::count_remaining_players() noexcept{
    int count = 0;
    for (Player &p : m_players) {
        if (p.get_state() == PlayerState::IN || p.get_state() == PlayerState::NO_ACTION){
            count++;
        }
    }
    return count;
}