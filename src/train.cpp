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

#include <algorithm>
#include <array>
#include <iostream>
#include <unordered_map>
#include <chrono>
#include <random>
#include <mutex>
#include <thread>
#include <cmath>

#include "action.h"
#include "game.h"
// #include "leduc.h"
#include "node.h"
#include "settings.h"
#include "utils.h"

using namespace std;

/* Game tree */
unordered_map<string, Node> g_tree;
mutex g_mutex;
mutex g_mutex_iter;
unsigned int g_iterations = 0;
bool g_run = true;
// std::vector<std::string> g_keys;

float cfr(Holdem &game, int hero) {
    /* check for terminal condition */
    if (!game.is_running()) {
        return static_cast<float>(game.get_reward(hero));
    }

    /* Get next player and create game state string for that player */
    int player = game.next_player();
    string key = game.create_key(player);

    /* Get existing node from game tree if it exists or create a new one */
    g_mutex.lock();
    size_t count = g_tree.size();
    Node node = g_tree[key];
    if (g_tree.size() != count){
        /* New element added -> have to set mask */
        node.set_mask(game.get_valid_actions_mask(player));
    }
    g_mutex.unlock();

    float node_util = 0.0;
    array<float, N_ACTIONS> strategy = node.get_strategy();

    if (player == hero) {
        /* Full exploration for hero player */
        array<float, N_ACTIONS> utilities{};
        vector<Action> valid_actions = game.get_valid_actions(player);
        std::array<float, N_ACTIONS> regrets = node.get_regrets();

        /* Explore all valid actions */
        for (Action &a : valid_actions) {
            int a_int = int(a);

            /* If regret is too low, do not explore this particual action */
            if (regrets[a_int] < REGRET_TRESHOLD) continue;
            
            /* Copy game to prevent overrides down the line */
            Holdem game_copy = Holdem(game);
            
            /* Take action, perform chance event if applicable */
            game_copy.take_action(a);

            /* Explore */
            utilities[a_int] = cfr(game_copy, hero);
            node_util += utilities[a_int] * strategy[a_int];
        }
        
        /* Update regret sums */
        float regret_element;
        for (int i = 0; i < N_ACTIONS; i++) {
            regret_element = utilities[i] - node_util;
            node.update_regret_sum(i, regret_element);
        }
        /* Increase # of visits for inspection */
        node.inc_visits();

    } else {
        /* Sample valid action for other players */
        Action a = game.sample_action(strategy, node.get_valid_actions(), player);

        /* Copy game state and take sampled action*/
        Holdem game_copy = Holdem(game);        
        game_copy.take_action(a);

        /* Explore further */
        node_util = cfr(game_copy, hero);

        /* Update average strategy */
        node.update_avg_strategy(strategy);

        /* Increase # of visits for inspection */
        node.inc_visits2();
    }

    /* Write node back to the tree */
    g_mutex.lock();
    g_tree[key] = node;
    g_mutex.unlock();

    return node_util;
};

void train() {
    long int util = 0;
    array<float, N_PLAYERS> probs;
    int i = 0; 
    int hero = 0;   

    while (g_run) {
        g_mutex_iter.lock();
        g_iterations++;
        g_mutex_iter.unlock();

        Holdem game = Holdem(N_PLAYERS, BIG_BLIND, SMALL_BLIND, MAX_RERAISES);
        // Leduc game = Leduc(BIG_BLIND, SMALL_BLIND, MAX_RERAISES);
        game.start_game();

        /* Explore */
        util += cfr(game, hero);
        
        /* Change traversal player */
        hero = (hero + 1) % N_PLAYERS;
    }
};

void monitor(){
    auto t1 = chrono::high_resolution_clock::now();
    bool saved = true; /* True to skip the first minute save */

    while(true){
        if (g_iterations >= N_ITERATIONS) {
            g_run = false;
            break;
        }

        std::this_thread::sleep_for(30s);

        auto t2 = std::chrono::high_resolution_clock::now();
        chrono::duration<float> elapsed = t2 - t1;
        int hours = static_cast<int>(elapsed.count()) / 3600;
        int minutes = (static_cast<int>(elapsed.count()) % 3600) / 60;
        int seconds = (static_cast<int>(elapsed.count()) % 3600) % 60;

        cout << "Iteration: " << (g_iterations+1) << ", memory used: " << get_ram_usage() << " kb, " << "# of nodes: " 
             << g_tree.size() << ", elapsed time: " << hours << "h " << minutes << "m " << seconds << "s\n";
        if ((minutes % SAVE_EVERY) == 0 && !saved) {
            saveModel(g_tree);
            // save_card_combination_keys(g_keys);
            saved = true;
        } else if ((minutes % SAVE_EVERY) != 0) {
            saved = false;
        }
    }
}