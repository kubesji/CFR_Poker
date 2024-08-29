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
#include <chrono>
#include <iostream>
#include <random>

#include "deck.h"

Deck::Deck() {
    m_pointer_to_deck = 0;
    int code, suit;
    for (int i = 0; i < 52; i++) {
        m_cards[i] = Card(i);
    }
};

void Deck::shuffle() noexcept {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    m_pointer_to_deck = 0;
    std::shuffle(m_cards.begin(), m_cards.end(), rng);
};
