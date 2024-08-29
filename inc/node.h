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

#ifndef _NODE_H
#define _NODE_H

#include <array>
#include "settings.h"
#include <string>
#include <cstdint>

class Node{
public:
    Node() noexcept;

    // inline const std::array<float, N_ACTIONS>& get_strategy() const noexcept{return m_strategy;}
    std::array<float, N_ACTIONS> get_strategy() const noexcept;

    void update_avg_strategy(const std::array<float, N_ACTIONS>& strategy) noexcept;

    std::array<float, N_ACTIONS> get_average_strategy() const noexcept;

    inline operator std::string() const noexcept{
        std::string s;
        for (int i = 0; i < N_ACTIONS; i++){
            char buffer[10];  // maximum expected length of the float
            std::snprintf(buffer, 10, "%.2e", m_regret_sum[i]);
            s.append(std::string(buffer) + "  |");
        }
        s.append(" |");
        std::array<float, N_ACTIONS> avg = get_average_strategy();
        for (int i = 0; i < N_ACTIONS; i++){
            char buffer[5];  // maximum expected length of the float
            std::snprintf(buffer, 5, "%.2f", avg[i]);
            s.append(std::string(buffer) + " | ");
        }
        s.append("|  visits: " + std::to_string(m_visits));
        s.append(", visits2: " + std::to_string(m_visits_2));
        return s;
    }

    inline void inc_visits() noexcept {m_visits++;}
    inline int get_visits() noexcept {return m_visits;}
    inline void inc_visits2() noexcept {m_visits_2++;}
    inline int get_visits2() noexcept {return m_visits_2;}

    inline void update_regret_sum(int idx, float f) noexcept {m_regret_sum[idx] += f * m_valid_action_mask[idx];};
    inline std::array<float, N_ACTIONS>  get_regrets() const noexcept {return m_regret_sum;};
    inline void set_mask(const std::array<uint8_t, N_ACTIONS>& mask) noexcept {m_valid_action_mask = mask;};
    inline std::array<uint8_t, N_ACTIONS>  get_valid_actions() const noexcept {return m_valid_action_mask;};

private:
    std::array<float, N_ACTIONS> m_regret_sum;
    std::array<float, N_ACTIONS> m_strategy;
    std::array<float, N_ACTIONS> m_strategy_sum;
    int m_visits, m_visits_2;
    std::array<uint8_t, N_ACTIONS> m_valid_action_mask;
};
#endif