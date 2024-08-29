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

#ifndef _DECK_H
#define _DECK_H

#include <string>
#include <array>

#include "card.h"

class Deck
{
public:
    Deck();
    inline Card* draw() noexcept { return &m_cards[m_pointer_to_deck++]; };  
    inline Card* draw(int idx) noexcept { return &m_cards[idx]; };
    void shuffle() noexcept;
private:
    int m_pointer_to_deck;
    std::array<Card, 52> m_cards;
};

#endif