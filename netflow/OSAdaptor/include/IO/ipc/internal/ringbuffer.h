#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>
#include <stdexcept>

template <typename T>
class RingBuffer {
public:
    RingBuffer(size_t capacity);
    bool push(const T& value);
    bool pop(T& value);
    bool isEmpty() const;
    bool isFull() const;
    size_t size() const;

private:
    std::vector<T> buffer;
    size_t writeIndex;
    size_t readIndex;
    size_t count;
};

template <typename T>
RingBuffer<T>::RingBuffer(size_t capacity)
    : buffer(capacity), writeIndex(0), readIndex(0), count(0) {}

template <typename T>
bool RingBuffer<T>::push(const T& value) {
    if (isFull()) {
        return false;
    }
    buffer[writeIndex] = value;
    writeIndex = (writeIndex + 1) % buffer.size();
    count++;
    return true;
}

template <typename T>
bool RingBuffer<T>::pop(T& value) {
    if (isEmpty()) {
        return false;
    }
    value = buffer[readIndex];
    readIndex = (readIndex + 1) % buffer.size();
    count--;
    return true;
}

template <typename T>
bool RingBuffer<T>::isEmpty() const {
    return count == 0;
}

template <typename T>
bool RingBuffer<T>::isFull() const {
    return count == buffer.size();
}

template <typename T>
size_t RingBuffer<T>::size() const {
    return count;
}

#endif // RINGBUFFER_H