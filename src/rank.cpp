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

#include "rank.h"


Rank::Rank(std::array<Card*, 2> player, const std::array<Card*, 3>& flop)
{
    m_player_cards = player;
    m_value = 0xFFFF;
    m_board_cards.insert(m_board_cards.end(), flop.begin(), flop.end());

    int suit_hash = 0, suit_hash_board;

    
    // (1 << ((c % 4) * 3))
    suit_hash += flop[0]->get_suit_hash();
    suit_hash += flop[1]->get_suit_hash();
    suit_hash += flop[2]->get_suit_hash();

    int8_t suit_board = suits[suit_hash];

    suit_hash += player[0]->get_suit_hash();
    suit_hash += player[1]->get_suit_hash();

    int8_t suit = suits[suit_hash];
    m_suit = -1;

    if (suit > 0) {
        m_is_flush = 's';
        m_possible_flush = 'F';
        int suit_binary[4] = {0};
        m_suit = suit;

        suit_binary[player[0]->get_suit_int()] |= player[0]->get_value_hash();
        suit_binary[player[1]->get_suit_int()] |= player[1]->get_value_hash();
        suit_binary[flop[0]->get_suit_int()] |= flop[0]->get_value_hash();
        suit_binary[flop[1]->get_suit_int()] |= flop[1]->get_value_hash();
        suit_binary[flop[2]->get_suit_int()] |= flop[2]->get_value_hash();

        m_value = flush[suit_binary[suits[suit_hash] - 1]];
    } else if (suit == -1){
        m_possible_flush = '1';
    }  else if (suit == -2){
        m_possible_flush = '2';
    }  else {
        m_possible_flush = 'x';
    }

    if (suit_board > 0){
        m_board_flush = 'F';
    } else if (suit_board == -1){
        m_board_flush = '1';
    } else if (suit_board == -2){
        m_board_flush = '2';
    } else {
        m_board_flush = 'x';
    }

    uint8_t count[13] = {0};
    uint16_t possible_straight_hash = 0;
    uint16_t pairs_triplets_hash = 0;

    // Without player card to check what is on the board
    possible_straight_hash |= flop[0]->get_value_hash();
    possible_straight_hash |= flop[1]->get_value_hash();
    possible_straight_hash |= flop[2]->get_value_hash();
    m_board_straight = straights[possible_straight_hash];

    possible_straight_hash |= player[0]->get_value_hash();
    possible_straight_hash |= player[1]->get_value_hash();
    m_possible_straight = straights[possible_straight_hash];

    // Without player card to check what is on the board
    count[int(*flop[0])]++;
    count[int(*flop[1])]++;
    count[int(*flop[2])]++;

    for (int i = 0; i < 13; i++){
        pairs_triplets_hash += count_hash[count[i]];
    }

    m_four_of_a_kind_on_board = pairs[pairs_triplets_hash][0] == 1;
    m_three_of_a_kind_on_board = pairs[pairs_triplets_hash][1] == 1;
    m_pairs_on_board = pairs[pairs_triplets_hash][2];

    if (m_value == 0xFFFF){
        count[int(*player[0])]++;
        count[int(*player[1])]++;

        const int hash = hash_nonflush(count, 5);

        m_value = noflush5[hash];
    }
}

Rank::Rank(std::array<Card*, 2> player, const std::array<Card*, 3>& flop, Card* turn)
{
    m_player_cards = player;
    m_value = 0xFFFF;
    m_board_cards.insert(m_board_cards.end(), flop.begin(), flop.end());
    m_board_cards.insert(m_board_cards.end(), turn);

    int suit_hash = 0, suit_hash_board;
    
    // (1 << ((c % 4) * 3))
    suit_hash += flop[0]->get_suit_hash();
    suit_hash += flop[1]->get_suit_hash();
    suit_hash += flop[2]->get_suit_hash();
    suit_hash += turn->get_suit_hash();

    int8_t suit_board = suits[suit_hash];

    suit_hash += player[0]->get_suit_hash();
    suit_hash += player[1]->get_suit_hash();

    int8_t suit = suits[suit_hash];
    m_suit = -1;

    if (suit > 0) {
        m_is_flush = 's';
        m_possible_flush = 'F';
        int suit_binary[4] = {0};
        m_suit = suit;

        suit_binary[player[0]->get_suit_int()] |= player[0]->get_value_hash();
        suit_binary[player[1]->get_suit_int()] |= player[1]->get_value_hash();
        suit_binary[flop[0]->get_suit_int()] |= flop[0]->get_value_hash();
        suit_binary[flop[1]->get_suit_int()] |= flop[1]->get_value_hash();
        suit_binary[flop[2]->get_suit_int()] |= flop[2]->get_value_hash();
        suit_binary[turn->get_suit_int()] |= turn->get_value_hash();

        m_value = flush[suit_binary[suits[suit_hash] - 1]];
    } else if (suit == -1){
        m_possible_flush = '1';
    } else {
        m_possible_flush = 'x';
    }

    if (suit_board > 0){
        m_board_flush = 'F';
    } else if (suit_board == -1){
        m_board_flush = '1';
    } else if (suit_board == -2){
        m_board_flush = '2';
    } else {
        m_board_flush = 'x';
    }

    uint8_t count[13] = {0};
    uint16_t possible_straight_hash = 0;
    uint16_t pairs_triplets_hash = 0;

    // Without player card to check what is on the board
    possible_straight_hash |= flop[0]->get_value_hash();
    possible_straight_hash |= flop[1]->get_value_hash();
    possible_straight_hash |= flop[2]->get_value_hash();
    possible_straight_hash |= turn->get_value_hash();
    m_board_straight = straights[possible_straight_hash];

    possible_straight_hash |= player[0]->get_value_hash();
    possible_straight_hash |= player[1]->get_value_hash();
    m_possible_straight = straights[possible_straight_hash];

    // Without player card to check what is on the board
    count[int(*flop[0])]++;
    count[int(*flop[1])]++;
    count[int(*flop[2])]++;
    count[int(*turn)]++;

    for (int i = 0; i < 13; i++){
        pairs_triplets_hash += count_hash[count[i]];
    }

    m_four_of_a_kind_on_board = pairs[pairs_triplets_hash][0] == 1;
    m_three_of_a_kind_on_board = pairs[pairs_triplets_hash][1] == 1;
    m_pairs_on_board = pairs[pairs_triplets_hash][2];

    if (m_value == 0xFFFF){
        count[int(*player[0])]++;
        count[int(*player[1])]++;

        const int hash = hash_nonflush(count, 6);

        m_value = noflush6[hash];
    }
}

Rank::Rank(std::array<Card*, 2> player, const std::array<Card*, 3>& flop, Card* turn, Card* river)
{
    m_player_cards = player;
    m_value = 0xFFFF;
    m_board_cards.insert(m_board_cards.end(), flop.begin(), flop.end());
    m_board_cards.insert(m_board_cards.end(), turn);
    m_board_cards.insert(m_board_cards.end(), river);
    // evaluate_7(int(*player[0]), int(*player[1]), int(*flop[0]), int(*flop[1]), int(*flop[2]), int(*turn), int(*river));
    int suit_hash = 0, suit_hash_board;
    
    // (1 << ((c % 4) * 3))
    suit_hash += flop[0]->get_suit_hash();
    suit_hash += flop[1]->get_suit_hash();
    suit_hash += flop[2]->get_suit_hash();
    suit_hash += turn->get_suit_hash();
    suit_hash += river->get_suit_hash();

    int8_t suit_board = suits[suit_hash];

    suit_hash += player[0]->get_suit_hash();
    suit_hash += player[1]->get_suit_hash();

    int8_t suit = suits[suit_hash];
    m_suit = -1;

    if (suit > 0) {
        m_is_flush = 's';
        m_possible_flush = 'F';
        int suit_binary[4] = {0};
        m_suit = suit;

        suit_binary[player[0]->get_suit_int()] |= player[0]->get_value_hash();
        suit_binary[player[1]->get_suit_int()] |= player[1]->get_value_hash();
        suit_binary[flop[0]->get_suit_int()] |= flop[0]->get_value_hash();
        suit_binary[flop[1]->get_suit_int()] |= flop[1]->get_value_hash();
        suit_binary[flop[2]->get_suit_int()] |= flop[2]->get_value_hash();
        suit_binary[turn->get_suit_int()] |= turn->get_value_hash();
        suit_binary[river->get_suit_int()] |= river->get_value_hash();

        m_value = flush[suit_binary[suits[suit_hash] - 1]];
    } else {
        m_possible_flush = 'x';
    }

    if (suit_board > 0){
        m_board_flush = 'F';
    } else if (suit_board == -1){
        m_board_flush = '1';
    } else if (suit_board == -2){
        m_board_flush = '2';
    } else {
        m_board_flush = 'x';
    }

    uint8_t count[13] = {0};
    uint16_t possible_straight_hash = 0;
    uint16_t pairs_triplets_hash = 0;

    // Without player card to check what is on the board
    possible_straight_hash |= flop[0]->get_value_hash();
    possible_straight_hash |= flop[1]->get_value_hash();
    possible_straight_hash |= flop[2]->get_value_hash();
    possible_straight_hash |= turn->get_value_hash();
    possible_straight_hash |= river->get_value_hash();
    m_board_straight = straights[possible_straight_hash];

    /* If we do not have straight already, it is not possible anymore */
    m_possible_straight = 'x';

    // Without player card to check what is on the board
    count[int(*flop[0])]++;
    count[int(*flop[1])]++;
    count[int(*flop[2])]++;
    count[int(*turn)]++;
    count[int(*river)]++;

    for (int i = 0; i < 13; i++){
        pairs_triplets_hash += count_hash[count[i]];
    }

    m_four_of_a_kind_on_board = pairs[pairs_triplets_hash][0] == 1;
    m_three_of_a_kind_on_board = pairs[pairs_triplets_hash][1] == 1;
    m_pairs_on_board = pairs[pairs_triplets_hash][2];

    if (m_value == 0xFFFF){
        count[int(*player[0])]++;
        count[int(*player[1])]++;

        const int hash = hash_nonflush(count, 7);

        m_value = noflush7[hash];
    }
}

RankCategory Rank::get_rank_category() const noexcept {
    if (m_value > 6185) return RankCategory::HighCard;
    if (m_value > 3325) return RankCategory::OnePair; 
    if (m_value > 2467) return RankCategory::TwoPairs;
    if (m_value > 1609) return RankCategory::ThreeOfAKind;
    if (m_value > 1599) return RankCategory::Straight;
    if (m_value > 322) return RankCategory::Flush;
    if (m_value > 166) return RankCategory::FullHouse; 
    if (m_value > 10) return RankCategory::FourOfAKind;
    return RankCategory::StraightFlush;
}

int Rank::hash_nonflush(const uint8_t q[], int k) {
    int sum = 0;

    for (int i = 0; i < 13; i++) {
        sum += dp[q[i]][12 - i][k];

        k -= q[i];

        if (k <= 0) {
            break;
        }
    }

    return sum;
}
char Rank::bin_card(const Card &c) const {
    if (c.get_value() >= 9) {
        return c.get_value_str();
    } else if (c.get_value() >= 6) {
        return 'L';
    } else {
        return 'l';
    }
}

char Rank::bin_card(char card) const {
    if (card < '6') {
        return 'l';
    } else if (card < '9') {
        return 'L';
    } else {
        return card;
    }
}

std::string Rank::get_string_representation() const noexcept {
    std::string state;

    std::string hand = cards_in_combo();
    // hand.push_back(m_is_flush);
    // return hand;
    int n_highest_in_hand = 0;
    int n_in_hand = 0;

    switch (get_rank_category()){
    case RankCategory::HighCard:
        state = "HC.";
        if (*m_player_cards[0] > *m_player_cards[1]) {
            state += bin_card(*m_player_cards[0]);
            state += bin_card(*m_player_cards[1]);
        } else {
            state += bin_card(*m_player_cards[1]);
            state += bin_card(*m_player_cards[0]);
        }
        state.push_back('.');
        state.push_back(m_possible_flush);
        state.push_back(m_board_flush);
        state.push_back(m_possible_straight);
        state.push_back(m_board_straight);
        return state;

    case RankCategory::OnePair:
        state.append("1P.");
        /* Pair value */
        state.push_back(bin_card(hand[0]));
        /* Number of cards held from pair */
        state.append(std::to_string(count_player_cards(hand[0])));
        /* Kicker */
        state.push_back(bin_card(hand[2]));
        /* Kicker in hand */
        state.push_back(bool_to_str(count_player_cards(hand[2]) > 0));
        
        state.push_back('.');
        state.push_back(m_possible_flush);
        state.push_back(m_board_flush);
        state.push_back(m_possible_straight);
        state.push_back(m_board_straight);
        return state;

    case RankCategory::TwoPairs:
        state = "2P.";
        /* Higher pair value */
        state.push_back(bin_card(hand[0]));
        /* Number of cards held from pair */
        state.append(std::to_string(count_player_cards(hand[0])));
        /* Lower pair value */
        state.push_back(bin_card(hand[2]));
        /* Number of cards held from pair */
        state.append(std::to_string(count_player_cards(hand[2])));
        /* Kicker */
        // state.push_back(bin_card(hand[4]));
        /* Kicker in hand */
        // state.push_back(bool_to_str(count_player_cards(hand[4]) > 0));
        
        state.push_back('.');
        state.push_back(m_possible_flush);
        state.push_back(m_board_flush);
        state.push_back(m_possible_straight);
        state.push_back(m_board_straight);
        return state;

    case RankCategory::ThreeOfAKind:
        state = "TR.";
        /* Triplet value */
        state.push_back(bin_card(hand[0]));
        /* Number of cards held from triplet */
        n_in_hand = count_player_cards(hand[0]);
        state.append(std::to_string(n_in_hand));
        /* Kicker */
        if (n_in_hand != 2){
            if (count_player_cards(hand[3]) > 0){
                /* Highest kicker in hand */
                state.push_back(bin_card(hand[3]));
                state.push_back('t');
            }
            else if (count_player_cards(hand[4]) > 0){
                /* Highest kicker on board, second in hand */
                state.push_back(bin_card(hand[4]));
                state.push_back('f');
            }
        } else {
            /* No kicker in hand */
            state.push_back('x');
        }
        state.push_back('.');
        state.push_back(m_possible_flush);
        state.push_back(m_board_flush);
        state.push_back(m_possible_straight);
        state.push_back(m_board_straight);
        return state;
    
    case RankCategory::Straight:
        state = "ST.";
        /* Straight value */
        state.push_back(bin_card(hand[0]));
        /* Find how many highest cards is in hand */
        if (count_player_cards(hand[0]) > 0) {
            if (count_player_cards(hand[1]) > 0)
                n_highest_in_hand = 2;
            else
                n_highest_in_hand = 1;
        }
        state.append(std::to_string(n_highest_in_hand));
        
        state.push_back('.');
        state.push_back(m_possible_flush);
        state.push_back(m_board_flush);
        state.push_back(bool_to_str(m_three_of_a_kind_on_board));
        state.append(std::to_string(m_pairs_on_board));
        return state;
    
    case RankCategory::Flush:
        state = "FL.";
        state.push_back(bin_card(hand[0]));
        if (count_player_cards(hand[0], m_suit) > 0) {      /* suit-1 because lookup table uses 0 as none and suits */
            if (count_player_cards(hand[1], m_suit) > 0)    /* are represented 1-4 while they are 0-3 in Card */
                n_highest_in_hand = 2;
            else
                n_highest_in_hand = 1;
        }
        state.append(std::to_string(n_highest_in_hand));
        
        state.push_back('.');
        state.push_back(bool_to_str(m_three_of_a_kind_on_board));
        state.append(std::to_string(m_pairs_on_board));
        return state;
    
    case RankCategory::FullHouse:
        state = "FH.";
        /* Triplet value */
        /* Number of cards held from triplet */
        state.append(std::to_string(count_player_cards(hand[0])));
        /* Pair value */
        /* Number of cards held from pair */
        state.append(std::to_string(count_player_cards(hand[3])));
        state.push_back('.');
        state.append(std::to_string(m_pairs_on_board));
        state.push_back(bool_to_str(compare_cards(hand[0], hand[3])));
        return state;
    
    case RankCategory::FourOfAKind:
        state = "PK.";
        /* Find how many highest cards is in hand */
        n_in_hand = count_player_cards(hand[0]);
        if (n_in_hand > 0) {
            /* If at least one card from poker in hand, return count*/
            state.append(std::to_string(n_in_hand));
            /* Flag that there might be higher four of a kind */
            if (n_in_hand == 1){
                /* One in hand, three on board */
                if (m_pairs_on_board == 1){
                    state.push_back(bool_to_str(compare_cards(hand[4], find_non_poker_pair(hand[0]))));
                } else {
                    state.push_back(bool_to_str(false));
                }
            } else if(n_in_hand == 2) {
                /* Two in hand, two on board -> there might be another pair or triplet*/
                if (m_pairs_on_board == 2){
                    state.push_back(bool_to_str(compare_cards(hand[4], find_non_poker_pair(hand[0]))));
                } else if (m_three_of_a_kind_on_board){
                    state.push_back(bool_to_str(compare_cards(hand[4], hand[0])));
                } else {
                    state.push_back(bool_to_str(false));
                }
            }
        } else {
            /* Else return value of the kicker*/
            state.push_back(bin_card(hand[4]));
            /* Kicker in hand */
            state.push_back(bool_to_str(count_player_cards(hand[4]) > 0));
        }
        return state;
    
    case RankCategory::StraightFlush:
        state = "SF.";
        state.push_back(bin_card(hand[0]));        
        state.push_back(bool_to_str(count_player_cards(hand[0], m_suit) > 0));
        return state;    
    }

    return "???";
}

int Rank::count_player_cards(char card) const noexcept{
    int count = card == m_player_cards[0]->get_value_str() ? 1 : 0;
    if (card == m_player_cards[1]->get_value_str()){
        count++;
    }
    return count;
}

int Rank::count_player_cards(char card, int8_t suit) const noexcept{
    if (suit < 0) return 0;

    int count = 0;
    /* suit-1 because lookup table uses 0 as none and suits */
    /* are represented 1-4 while they are 0-3 in Card */
    if (card == m_player_cards[0]->get_value_str() && (suit-1) == m_player_cards[0]->get_suit_int()){
        count = 1;
    }
    if (card == m_player_cards[1]->get_value_str() && (suit-1) == m_player_cards[1]->get_suit_int()){
        count++;
    }
    return count;
}

char Rank::find_non_poker_pair(char poker_card) const noexcept{
    char a = 'x', b = 'x';

    for (Card* c : m_board_cards){
        if (char(*c) != poker_card){
            if (a == 'x') {
                a = char(*c);
            } else if (a == char(*c)){
                return a;
            } else if (b == 'x') {
                b = char(*c);
            } else if (b == char(*c)) {
                return b;
            } else {
                return 'x';
            }
        }
    }
    return 'x';
}

bool Rank::compare_cards(char a, char b) const noexcept
{
    uint8_t A, B;
    switch (a){
        case 'J': A = '9' + 1; break;
        case 'Q': A = '9' + 2; break;
        case 'K': A = '9' + 3; break;
        case 'A': A = '9' + 4; break;
        default: A = a;
    }
    switch (b){
        case 'J': B = '9' + 1; break;
        case 'Q': B = '9' + 2; break;
        case 'K': B = '9' + 3; break;
        case 'A': B = '9' + 4; break;
        default: B = b;
    }
    return A > B;
}