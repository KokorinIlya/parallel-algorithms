#include <cstdint>
#include <new>
#include <type_traits>

template <typename T>
struct raw_array
{
public:
    raw_array(uint32_t _size) : size(_size),
                                ptr(static_cast<T*>(::operator new(sizeof(T) * _size))) 
    {
        static_assert(std::is_trivially_destructible<T>::value, "Type parameter should be trivially destructible");
    }

    raw_array(raw_array const& other) : size(other.size),
                                        ptr(static_cast<T*>(::operator new(sizeof(T) * other.size))) 
    {
        for (uint32_t i = 0; i < other.size; ++i)
        {
            *(ptr + i) = other[i];
        }
    }

    raw_array(raw_array&& other) :  size(other.size),
                                    ptr(other.ptr) 
    {
        other.ptr = nullptr;
    }

    T const& operator[](uint32_t idx) const 
    {
        return *(ptr + idx);
    }

    T& operator[](uint32_t idx) 
    {
        return *(ptr + idx);
    }

    uint32_t get_size() const
    {
        return size;
    }

    ~raw_array()
    {
        if (ptr != nullptr)
        {
            ::operator delete(ptr);
        }
    }
private:
    uint32_t size;
    T* ptr;
};
