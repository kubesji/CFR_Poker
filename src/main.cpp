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

#include <thread>   
#include <iostream>
#include <vector>

#include "train.h"

using namespace std;

/* Use this for training */
int main(){
    unsigned int processor_count = thread::hardware_concurrency();
    if (processor_count == 0) {
        cout << "Could not detect number of processors. Using 1 thread.\n";
        processor_count = 1;
    } else {
        cout << "Detected " << processor_count << " processors. ";
        processor_count -= 1;
        cout << "Using " << processor_count << " processors for training, 1 for monitoring.\n";
    }

    vector<thread> threads;

    for (int i = 0; i < processor_count; i++) {
        threads.push_back(thread(train));
    }
    threads.push_back(thread(monitor));

    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    }

    return 0;
}

/* Use this for debugging */
// int main(){
//     train();
//     return 0;
// }