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

#include "node.h"
#include <cmath>
#include <math.h>

Node::Node() noexcept {
    for (int i = 0; i < N_ACTIONS; i++) {
        m_regret_sum[i] = 0.0;
        m_strategy[i] = 1 / N_ACTIONS_f;
        m_strategy_sum[i] = 0;
    }
    m_visits = 0;
    m_visits_2 = 0;
}

std::array<float, N_ACTIONS> Node::get_strategy() const noexcept {
    std::array<float, N_ACTIONS> strategy;
    float sum = 0;
    float valid_sum = 0;

    for (int i = 0; i < N_ACTIONS; i++) {
        if (m_regret_sum[i] > 0){
            strategy[i] = m_regret_sum[i] * m_valid_action_mask[i];
            sum += strategy[i];
        } else {
            strategy[i] = 0;
        }
        valid_sum += static_cast<float>(m_valid_action_mask[i]);
    }

    if (sum <= 0) {
        for (int i = 0; i < N_ACTIONS; i++) {
            strategy[i] = m_valid_action_mask[i] / valid_sum;
        }
    } else {
        for (int i = 0; i < N_ACTIONS; i++) {
            strategy[i] /= sum;
        }
    }
    return strategy;
}

void Node::update_avg_strategy(const std::array<float, N_ACTIONS>& strategy) noexcept {

    for (int i = 0; i < N_ACTIONS; i++) {
        m_strategy_sum[i] += strategy[i];
    }
}

std::array<float, N_ACTIONS> Node::get_average_strategy() const noexcept {
    float sum = 0.0;

    for (int i = 0; i < N_ACTIONS; i++) {
        sum += m_strategy_sum[i];// * m_valid_action_mask[i];
    }

    std::array<float, N_ACTIONS> strategy;
    if (sum > 0) {
        for (int i = 0; i < N_ACTIONS; i++)
            strategy[i] = m_strategy_sum[i] / sum;
    } else {
        for (int i = 0; i < N_ACTIONS; i++)
            strategy[i] = 1 / N_ACTIONS_f;
    }
    return strategy;
}