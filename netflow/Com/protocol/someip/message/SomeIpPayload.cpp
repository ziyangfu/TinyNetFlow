//
// Created by fzy on 23-8-28.
//

#include "SomeIpPayload.h"

using namespace netflow::net::someip;

SomeIpPayload::SomeIpPayload()
    : data_()
{
}

SomeIpPayload::~SomeIpPayload() noexcept {
}

void SomeIpPayload::setPayload(const std::vector<uint8_t> &data) {
    data_ = data;
}

void SomeIpPayload::setPayload(const std::string &data) {

}

void SomeIpPayload::setPayload(const uint8_t *data, int length) {
    data_.assign(data, data + length);
}

const uint8_t *SomeIpPayload::getPayload() {
    return data_.data();
}

std::string &SomeIpPayload::getPayload(bool isString) {

}

void SomeIpPayload::getPayload(std::vector<uint8_t> &data) {

}

size_t SomeIpPayload::getLength() {
    return data_.size();
}

bool SomeIpPayload::operator==(const Payload &other) {
    bool isEqual{true};
    try {
        const SomeIpPayload& other_ = dynamic_cast<const SomeIpPayload&>(other);
        if (data_ == other_.data_) {
            isEqual = true;
        }
        else {
            isEqual = false;
        }
    }
    catch (...) {
        isEqual = false;
    }
    return isEqual;
}


void SomeIpPayload::setCapacity(size_t capacity) {
    data_.reserve(capacity);
}