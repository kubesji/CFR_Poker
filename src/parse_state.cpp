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

#include "utils.h"
#include <map>
#include <string>
#include <iostream>
#include <algorithm>

int main(){
    std::unordered_map<std::string, Node> model = loadModel();  
    std::map<std::string, Node> ordered(model.begin(), model.end());
    std::map<std::string, Node>::iterator iter;
    std::vector<std::string> key_list;
    std::array<int, 10> n_combos;
    n_combos.fill(0);

    // std::map<std::string, Node> ordered(tree.begin(), tree.end());

    for(iter = ordered.begin(); iter != ordered.end(); ++iter){
        std::string key = iter->first;

        /* Full state */
        size_t pos = key.find("-");
        if (pos != std::string::npos){
            key = key.substr(0, pos);
            if(std::find(key_list.begin(), key_list.end(), key) == key_list.end()){
                key_list.push_back(key);
            }else{
                continue;
            }
        }
        /* Combination */
        pos = key.find(".");
        if (pos != std::string::npos){
            key = key.substr(0, pos);
            if ( key == "HC" ) n_combos[0]++;
            else if (key == "1P") n_combos[1]++;
            else if (key == "2P") n_combos[2]++;
            else if (key == "TR") n_combos[3]++;
            else if (key == "ST") n_combos[4]++;
            else if (key == "FL") n_combos[5]++;
            else if (key == "FH") n_combos[6]++;
            else if (key == "PK") n_combos[7]++;
            else if (key == "SF") n_combos[8]++;
            else n_combos[9]++;            
        }
    }

    std::cout << "HC: " << std::to_string(n_combos[0]) << "\n"
              << "1P: " << std::to_string(n_combos[1]) << "\n"
              << "2P: " << std::to_string(n_combos[2]) << "\n"
              << "TR: " << std::to_string(n_combos[3]) << "\n"
              << "ST: " << std::to_string(n_combos[4]) << "\n"
              << "FL: " << std::to_string(n_combos[5]) << "\n"
              << "FH: " << std::to_string(n_combos[6]) << "\n"
              << "PK: " << std::to_string(n_combos[7]) << "\n"
              << "SF: " << std::to_string(n_combos[8]) << "\n"
              << "??: " << std::to_string(n_combos[9]) << "\n";

    save_card_combination_keys(key_list);
}