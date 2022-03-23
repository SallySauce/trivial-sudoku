#pragma once

template <typename T>
struct Slice {
    T *data;
    size_t length;

    Slice(T *v, size_t offset, size_t width) {
        data = v + offset;
        length = width;
    }

    Slice() {
        data = 0;
        length = 0;
    }

    T& operator[](size_t index) {
        return data[index];
    }

    size_t size() const {
        return length;
    }

    T *begin() const {
        return data;
    }

    T *end() const {
        return &data[length];
    }
};
