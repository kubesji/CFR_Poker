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
 
#ifndef _ACTION_H
#define _ACTION_H

#include <string>

class Action{
public:
    Action() : m_str('?'),
               m_int(-1),
               m_value(-1){};
    Action(int numerical, char alphabetical, int value) : m_str(alphabetical),
                                                          m_int(numerical),
                                                          m_value(value){};
    operator std::string() const noexcept {return std::string(1, m_str);}
    operator char() const noexcept {return m_str;}
    operator int() const noexcept {return m_int;}
    inline int get_value() const noexcept{return m_value;}
private:
    char m_str;
    int m_int;
    int m_value;
};

#endif