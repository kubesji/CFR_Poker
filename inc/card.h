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

#ifndef _CARD_H
#define _CARD_H

#include <string>
#include <cstdint>

class Card{
public:
    Card();
    Card(int code);
    operator std::string() const {std::string s(2, m_str); s[1] = m_suit; return s;}
    operator int() const { return m_int; }

    inline char get_suit() const noexcept {return m_suit;};
    inline uint8_t get_suit_int() const noexcept {return m_suit_int;};

    inline char get_value_str() const noexcept {return m_str;};
    inline char get_value() const noexcept {return m_int + 2;};

    inline uint32_t get_suit_hash() const noexcept {return m_suit_hash;};
    inline uint32_t get_value_hash() const noexcept {return m_value_hash;};

    bool operator<(const Card& other) const { return m_int < other.m_int; }
    bool operator<=(const Card& other) const { return m_int <= other.m_int; }
    bool operator>(const Card& other) const { return m_int > other.m_int; }
    bool operator>=(const Card& other) const { return m_int >= other.m_int; }
    bool operator==(const Card& other) const { return m_int == other.m_int; }
    bool operator!=(const Card& other) const { return m_int != other.m_int; }
private:
    char int_to_str(uint8_t value) noexcept;

    char m_str;
    uint8_t m_int;
    char m_suit;
    uint8_t m_suit_int;
    uint8_t m_code;

    uint32_t m_suit_hash;
    uint32_t m_value_hash;
    //CardCode m_code;
};
#endif