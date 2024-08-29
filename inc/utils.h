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

#include <unordered_map>
#include "node.h"
#include "settings.h"
#include "game.h"

int get_ram_usage();

std::string pad_string(const std::string& str, int length);

void saveModel(std::unordered_map<std::string, Node>& tree);
std::unordered_map<std::string, Node> loadModel();

void store_card_combination_key(std::string key, std::vector<std::string> &keys);
void save_card_combination_keys(std::vector<std::string> &keys);

