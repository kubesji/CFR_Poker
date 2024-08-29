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

#include "action.h"
#include "deck.h"
#include "game.h"
#include "node.h"
#include "settings.h"
#include "utils.h"

using namespace std;

int main() {
    Deck deck = Deck();
    int i = 0; 
    int hero = 0, player = 0, opponent = 1;  
    std::array<int, N_PLAYERS> chips = {0, 0};  

    unordered_map<string, Node> tree = loadModel();

    while (1) {
        Holdem game = Holdem(N_PLAYERS, BIG_BLIND, SMALL_BLIND, MAX_RERAISES);
        std::array<Action, N_ACTIONS> actions = game.get_actions();
        game.start_game();
        const array<Card*, 2>& my_cards = game.get_player_cards(hero);
        cout << "You've been dealt " << string(*my_cards[0]) << " & " << string(*my_cards[1]) << ".\n";
        Action a;
        Round round = game.get_round();

        while (game.is_running()){
            player = game.next_player();
            if (player == hero){
                string s;
                cout << "Enter your action 0=p";
                vector<int> allowed = {0};
                vector<char> raises = {'A', 'B', 'C', 'D'};
                
                if (game.can_call(player)) {
                    allowed.push_back(1);
                    cout << ", 1=c";
                }
                for (int i = 2; i < N_ACTIONS; i++){
                    Action b = actions[i];
                    if (game.can_raise(player, b)) {
                        allowed.push_back(int(b));
                        cout << ", " << to_string(int(b)) << "=" << string(b);
                    }
                }
                cout << "\n";

                cin >> s;
                a = actions[stoi(s)];
            } else {
                string key = game.create_key(player);
                Node node = Node();
                if (tree.contains(key)) {
                    node = tree[key];
                }
                array<float, N_ACTIONS> strategy = node.get_average_strategy();
                string s = "Opponents strategy: ";
                for (int i = 0; i < N_ACTIONS; i++){
                    char buffer[5];  // maximum expected length of the float
                    snprintf(buffer, 5, "%.2f", strategy[i]);
                    s.append(std::string(buffer) + " | ");
                }
                cout << s << "\n";                
                a = game.sample_action(strategy, player);
                cout << "Playing action " << std::string(a) << "\n";    
            }
            game.take_action(a);

            if (round != game.get_round() && game.get_round() < Round::REVEAL){
                round = game.get_round();
                cout << "New round is: " << static_cast<int>(round) << "\n";
                const array<Card*, 3>& flop = game.get_flop();
                Card* turn = game.get_turn();
                Card* river = game.get_river();

                cout << "Cards on table are " << string(*flop[0])  
                                              << " & " << string(*flop[1]) 
                                              << " & " << string(*flop[2]) 
                                              << (game.get_round() >= Round::TURN ? " & " : "")
                                              << (game.get_round() >= Round::TURN ? string(*turn) : "")
                                              << (game.get_round() >= Round::RIVER ? " & " : "")
                                              << (game.get_round() >= Round::RIVER ? string(*river) : "")
                                              << ".\n";
            }
        }

        const array<Card*, 2>& opp_cards = game.get_player_cards(opponent);
        cout << "Opponent's cards " << string(*opp_cards[0]) << " & " << string(*opp_cards[1]) << ".\n";
        vector<int8_t> winners;
        if (game.get_round() == Round::REVEAL){
            winners = game.find_winner();
        } else {
            for (int i = 0; i < N_PLAYERS; i++){
                if (game.is_player_in_game(i)) {
                    winners.push_back(i);
                    break;
                }
            }
        }

        if (winners.size() > 1){
            cout << "Draw between: ";
            for (int w : winners){
                cout << to_string(w) << "->" << (hero == w ? "hero " : "opponent ");
             }
             cout << ".\n\n-----------------------------------\n\n";
        } else{
            cout << "Winner is: " << to_string(winners[0]) << "->" << (hero == winners[0] ? "hero" : "opponent") << ".\n\n-----------------------------------\n";
            int change = game.get_player_pot_contribution((winners[0] + 1) % 2); /* loosing player contribution */

            if (winners[0] == hero){
                chips[0] += change;
                chips[1] -= change;
            } else {
                chips[1] += change;
                chips[0] -= change;
            }
            cout << "Hero: " << to_string(chips[0]) << ", opponent: " << to_string(chips[1]) << "\n-----------------------------------\n\n";
            
        }

        /* Change traversal player */
        hero = (hero + 1) % N_PLAYERS;
        opponent = 1 - hero;
    }
}