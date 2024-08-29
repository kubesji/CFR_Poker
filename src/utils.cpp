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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>

#include "sys/types.h"
#include "sys/sysinfo.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int get_ram_usage(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

std::string pad_string(const std::string& str, int length){
    std::stringstream ss;
    ss << std::left << std::setw(length) << std::setfill(' ') << str;
    return ss.str();
}

void saveModel(std::unordered_map<std::string, Node>& tree){
    std::cout << "Saving model. ";
    std::unordered_map<std::string, Node>::iterator iter;
    FILE *f = fopen("tree", "wb");
    FILE *f_text = fopen("tree.txt", "w");

    // std::map<std::string, Node> ordered(tree.begin(), tree.end());

    // for(auto iter = ordered.begin(); iter != ordered.end(); ++iter){
    for(iter=tree.begin(); iter!=tree.end(); iter++){
        if ((iter->second).get_visits() == 0) continue;
        fwrite((iter->first).c_str(), sizeof(char), KEY_LENGTH, f);
        fwrite(&(iter->second), sizeof(Node), 1, f);
        
        if ((iter->second).get_visits() < 100) continue;
        std::string text = iter->first;
        text.append(":  ").append(std::string(iter->second)).append("\n");
        fwrite(text.c_str(), text.length(), 1, f_text);
    }
    fclose(f);
    fclose(f_text);
    std::cout << "Done!\n";
}

std::unordered_map<std::string, Node> loadModel(){
    std::unordered_map<std::string, Node> tree = {};
    FILE *f = fopen("tree", "rb");
    char key[KEY_LENGTH + 1] = {0};
    Node node;
    while(fread(key, sizeof(char), KEY_LENGTH, f)){
        fread(&node, sizeof(Node), 1, f);
        std::string k = std::string(key);
        tree.insert({k, node});
    }
    fclose(f);
    return tree;
}

void store_card_combination_key(std::string key, std::vector<std::string> &keys){
    size_t pos = key.find("-");
    if (pos != std::string::npos){
        std::string k = key.substr(0, pos);
        if(std::find(keys.begin(), keys.end(), k) == keys.end())
            keys.push_back(k);
    }
}

void save_card_combination_keys(std::vector<std::string> &keys){
    std::sort(keys.begin(), keys.end());
    FILE *f_text = fopen("keys.txt", "w");

    for(std::string k : keys){       
        std::string txt = k.append("\n"); 
        fwrite(txt.c_str(), txt.length(), 1, f_text);
    }

    fclose(f_text);
}