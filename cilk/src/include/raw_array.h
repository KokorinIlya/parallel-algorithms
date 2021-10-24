#ifndef RAW_ARRAY_H
#define RAW_ARRAY_H

#include <cstdint>
#include <new>
#include <type_traits>

template <typename T>
struct raw_array
{
public:
    raw_array(uint32_t array_size) : _size(array_size),
                                     _ptr(nullptr) 
    {
        static_assert(std::is_trivially_destructible<T>::value, "Type parameter should be trivially destructible");
        if (_size > 0)
        {
            _ptr = static_cast<T*>(::operator new(sizeof(T) * _size));
        }
    }

    raw_array(raw_array<T> const& other) : _size(other._size),
                                           _ptr(nullptr) 
    {
        if (_size > 0)
        {
            _ptr = static_cast<T*>(::operator new(sizeof(T) * other._size));
            for (uint32_t i = 0; i < other._size; ++i)
            {
                *(_ptr + i) = other[i];
            }
        }
    }

    raw_array(raw_array<T>&& other) noexcept : 
        _size(other._size),
        _ptr(other._ptr) 
    {
        other._ptr = nullptr;
        other._size = 0;
    }

    T* get_raw_ptr()
    {
        return _ptr;
    }

    T const* get_raw_ptr() const
    {
        return _ptr;
    }

    T const& operator[](uint32_t idx) const 
    {
        return *(_ptr + idx);
    }

    T& operator[](uint32_t idx) 
    {
        return *(_ptr + idx);
    }

    uint32_t size() const
    {
        return _size;
    }

    ~raw_array()
    {
        if (_ptr != nullptr)
        {
            ::operator delete(_ptr);
        }
    }
private:
    uint32_t _size;
    T*       _ptr;
};

#endif