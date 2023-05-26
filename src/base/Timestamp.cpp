//
// Created by fzy on 23-5-26.
//

#include "Timestamp.h"

using namespace netflow::base;

Timestamp::Timestamp() {

}

Timestamp::~Timestamp() {

}

int Timestamp::now() {
    auto now = std::chrono::system_clock::now();
    return now;
}
