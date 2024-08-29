/*
 *  Copyright 2016 Henry Lee
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

#ifndef HASH_TABLES_H
#define HASH_TABLES_H

#include <cstdint>
#include <string>

// extern const uint16_t bit_of_mod_4_x_3[52];
// extern const uint16_t bit_of_div_4[52];

extern const uint16_t flush[8192];
extern const uint16_t noflush5[6175];
extern const uint16_t noflush6[18395];
extern const uint16_t noflush7[49205];

extern const int8_t suits[3585];
extern const int8_t straights[16384];
extern const uint8_t pairs[592][3];
extern const uint16_t count_hash[5];

extern const uint32_t dp[5][14][10];

extern const std::string rank_description[7463][2];
#endif  // TABLES_H
