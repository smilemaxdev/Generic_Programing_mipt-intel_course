#pragma once
#include <stdexcept>
#include <cstring>
#include <iterator.hpp>
namespace my {

template<typename CharT,
         typename TraitsT = std::char_traits<CharT>,
         typename Allocator = std::allocator<CharT>>
class base_string{

    using it       = StringIterator<CharT>;
    using const_it = StringIterator<const CharT>;

    size_t size_ = 0;
    union{
        struct{
            CharT* data = nullptr;
            size_t cap  = 0;
        } large;
        char small[sizeof (large)];
    };

    void create(const CharT* data, size_t n){
        if(n < sizeof (large)){
            std::memset(&small[0], '\0', sizeof (large));
            std::memcpy(&small[0], data, n);
            size_ = 0;
        }
        else{
            Allocator alloc;
            large.data = alloc.allocate(2 * n);
            std::memcpy(large.data, data, n);
            size_ = n;
            large.cap = 2 * n;
            large.data[n] = '\0';
        }
    }

    CharT* choose(){
        CharT* it = nullptr;
        if(size_ < sizeof (large)) it = &small[0];
        else                       it = large.data;
        return it;
    }

    CharT* choose(const base_string& str) const{
        CharT* it = nullptr;
        if(str.size_ < sizeof (large)) it = &str.small[0];
        else                           it = str.large.data;
        return it;
    }

public:

    base_string() = default;

    base_string(const CharT* data, size_t n){
        create(data, n);
    }

    base_string(const CharT* data){
        size_t n = std::strlen(data);
        create(data, n);
    }

    template<size_t N>
    base_string(const CharT (&data)[N]){
        create(data, N);
    }

    base_string(const base_string& str){
        create(choose(str), str.size_);
    }

    base_string(base_string&& str){
        CharT* it = choose(str);
        create(it, str.size_);
        if(it == str.large.data){
            Allocator alloc;
            alloc.deallocate(str.large.data, str.size_);
            str.large.data = nullptr;
        }
        str.size_ = 0;
        str.large.cap = 0;
    }

    base_string& operator=(const base_string& str){
        CharT* it = choose(str);
        create(it, str.size_);
        return *this;
    }

    base_string& operator=(base_string&& str){
        CharT* it = choose(str);
        create(it, str.size_);
        if(it == str.large.data){
            Allocator alloc;
            alloc.deallocate(str.large.data, str.size_);
            str.large.data = nullptr;
        }
        str.size_ = 0;
        str.large.cap = 0;
        return *this;
    }

    template<typename CharU,
             typename TraitsU,
             typename AllocatorU,
             std::enable_if_t<std::is_convertible_v<CharU, CharT>>>
    bool operator==(const base_string<CharU, TraitsU, AllocatorU>& str) const{
        if(size_ != str.size_) return false;
        CharT* lhs = choose();
        CharT* rhs = choose(str);
        for(size_t i = 0; i < size_; ++i){
            if(!TraitsT::eq(*lhs, *rhs)) return false;
            lhs++; rhs++;
        }

        return true;

    }

    template<typename CharU,
             typename TraitsU,
             typename AllocatorU,
             std::enable_if_t<std::is_constructible_v<CharU, CharT>>>
    bool operator<(const base_string<CharU, TraitsU, AllocatorU>& str) const{
        if(size_ < str.size_) return true;
        if(size_ > str.size_) return false;

        CharT* lhs = choose();
        CharT* rhs = choose(str);

        for(size_t i = 0; i < size_; ++i){
            if(!TraitsT::lt(*lhs, *rhs)) return false;
            lhs++; rhs++;
        }

        return true;

    }

    CharT& operator[](size_t idx){
        CharT* it = choose();
        return it[idx];
    }

    CharT operator[](size_t idx) const{
        CharT* it = choose();

        return it[idx];
    }

    CharT& at(size_t idx){

        if(idx > size_ - 2) throw std::out_of_range("");
        CharT* it = choose();
        return it[idx];
    }

    CharT at(size_t idx) const{

        if(idx > size_ - 2) throw std::out_of_range("");
        CharT* it = choose();
        return it[idx];
    }

    const CharT* c_str() const{
        CharT* it = choose();
        return it;
    }

    it begin(){
        CharT* it = choose();
        return it(it);
    }

    it end(){
        CharT* it = choose();

        return it(it + size_);
    }

    const_it cbegin() const{
        CharT* it = choose();
        return const_it(it);
    }

    const_it cend() const{
        CharT* it = choose();

        return const_it(it + size_);
    }

    size_t size() const{
        return size_ - 1;
    }

    it front(){
        CharT* it = choose();
        return it(it);
    }

    const_it front() const{
        CharT* it = choose();
        return const_it(it);
    }

    it back(){
        CharT* it = choose();
        return it(it);
    }

    const_it back() const{
        CharT* it = choose();
        return const_it(it);
    }

    base_string copy() const{
        CharT* it = choose();
        return base_string(it, size_);
    }

    void assign(const_it beg, const_it end){
        size_t size = end - beg + 1;
        Allocator alloc;
        if(size < sizeof (large)){
            if(large.data){
                alloc.deallocate(large.cap);
                large.cap = 0;
            }
            std::memcpy(&small[0], &(*beg), size - 1);
        }
        else{
            if(large.cap <= size){
                alloc.deallocate(large.data, large.cap);
                large.data = alloc.allocate(2 * size);
                large.cap = 2 * size;
            }
            std::memcpy(large.data, &(*beg), size - 1);
        }
        size_ = size;
        large.data[size] = '\0';
    }

    base_string substr(size_t beg, size_t end) const{
        if(beg == end) return base_string();
        if(end - beg == size_ - 1) return copy();
        base_string str;
        str.assign(beg, end);
        return str;
    }


    it find(CharT v){
        CharT* it = choose();
        for(size_t idx = 0; idx < size_ - 1; ++idx)
            if(TraitsT::eq(v, it[idx])) return &it[idx];
        return end();
    }

    const_it find(CharT v) const{
        CharT* it = choose();
        for(size_t idx = 0; idx < size_ - 1; ++idx)
            if(TraitsT::eq(v, it[idx])) return &it[idx];
        return cend();
    }


    it find(it beg, it end, CharT v){
        if(beg == end) return it();
        for(auto it = beg; it != end; ++it)
            if(TraitsT::eq(v, *it)) return it;
        return end();
    }

    const_it find(const_it beg, const_it end, CharT v) const{
        if(beg == end) return it();
        for(auto it = beg; it != end; it = std::next(it))
            if(TraitsT::eq(v, *it)) return it;
        return cend();
    }

    template<typename F>
    it find_if(F pred){
        CharT* it = choose();
        for(size_t idx = 0; idx < size_ - 1; ++idx)
            if(pred(it[idx])) return &it[idx];
        return end();
    }

    template<typename F>
    const_it find_if(F pred) const{
        CharT* it = choose();
        for(size_t idx = 0; idx < size_ - 1; ++idx)
            if(pred(it[idx])) return &it[idx];
        return cend();
    }

    size_t count(CharT v) const{
        size_t res = 0;
        CharT* it = choose();
        for(size_t idx = 0; idx < size_ - 1; ++idx) if(TraitsT::eq(v, it[idx])) res++;
        return res;
    }

};
}

template<typename CharU,
         typename TraitsU,
         typename AllocatorU>
std::basic_ostream<CharU, TraitsU>& operator<<(std::basic_ostream<CharU, TraitsU>& os, const my::base_string<CharU, TraitsU, AllocatorU>& str){
    os << str.c_str();
    return os;
}

using string = my::base_string<char>;

