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

#include "card.h"

Card::Card()
    : m_int(-1)
    , m_str('?')
    , m_suit('?')
    , m_code(-1)
{};

Card::Card(int code)
{
    m_code = code;
    m_int = code >> 2;
    m_str = int_to_str(m_int);
    m_suit_int = static_cast<uint8_t>(code % 4);

    m_suit_hash = 1 << ((code % 4) * 3);
    m_value_hash = 1 << m_int;

    switch (m_suit_int) {
    case 0:
        m_suit = 'c';
        break;
    case 1:
        m_suit = 'd';
        break;
    case 2:
        m_suit = 'h';
        break;
    case 3:
        m_suit = 's';
        break;
    default:
        m_suit = '?';
        break;
    }
};

char Card::int_to_str(uint8_t value) noexcept {
    if (value <= 7) {
        return '0' + value + 2;
    } else if (value == 8) {
        return 'T';
    } else if (value == 9) {
        return 'J';
    } else if (value == 10) {
        return 'Q';
    } else if (value == 11) {
        return 'K';
    } else if (value == 12) {
        return 'A';
    }
    return '?';
};