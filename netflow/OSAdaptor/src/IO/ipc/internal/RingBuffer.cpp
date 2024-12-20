//
// Created by fzy on 2024/12/18.
//
#include "IO/ipc/internal/RingBuffer.h"

using namespace osadaptor::ipc::internal;

RingBuffer::RingBuffer(const char* shmName, size_t capacity)
        : m_cb(nullptr),
          m_fd(-1),
          m_capacity(capacity)
{
}

RingBuffer::~RingBuffer() {
    if (m_fd != -1) {
        close(m_fd);
    }
}

bool RingBuffer::write(const void* data, size_t dataSize) {
    if (dataSize > m_cb->capacity || isFull()) {
        return false;
    }

    size_t freeSpace = (m_cb->head >= m_cb->tail) ?
                       m_cb->capacity - m_cb->head + m_cb->tail :
                       m_cb->tail - m_cb->head;

    if (dataSize > freeSpace) {
        return false;
    }

    if (m_cb->head + dataSize <= m_cb->capacity) {
        memcpy(m_cb->buffer + m_cb->head, data, dataSize);
        m_cb->head = (m_cb->head + dataSize) % m_cb->capacity;
    } else {
        size_t firstPart = m_cb->capacity - m_cb->head;
        memcpy(m_cb->buffer + m_cb->head, data, firstPart);
        memcpy(m_cb->buffer, (char*)data + firstPart, dataSize - firstPart);
        m_cb->head = dataSize - firstPart;
    }

    return true;
}

bool RingBuffer::read(void* data, size_t dataSize) {
    if (dataSize > m_cb->capacity || isEmpty()) {
        return false;
    }

    size_t availableData = (m_cb->head >= m_cb->tail) ?
                           m_cb->head - m_cb->tail :
                           m_cb->capacity - m_cb->tail + m_cb->head;

    if (dataSize > availableData) {
        return false;
    }

    if (m_cb->tail + dataSize <= m_cb->capacity) {
        memcpy(data, m_cb->buffer + m_cb->tail, dataSize);
        m_cb->tail = (m_cb->tail + dataSize) % m_cb->capacity;
    } else {
        size_t firstPart = m_cb->capacity - m_cb->tail;
        memcpy(data, m_cb->buffer + m_cb->tail, firstPart);
        memcpy((char*)data + firstPart, m_cb->buffer, dataSize - firstPart);
        m_cb->tail = dataSize - firstPart;
    }

    return true;
}

bool RingBuffer::isEmpty() const {
    return m_cb->head == m_cb->tail;
}

bool RingBuffer::isFull() const {
    return (m_cb->head + 1) % m_cb->capacity == m_cb->tail;
}
