/********************************************************************
* Vector Class Template
*
* Author: Paul Thompson
*         prt@uakron.edu
*
* Purpose: Declaration of the Vector class, definition of some
* inline member functions, the = and [] overloads, constructors
* and destructor, and = != < > >= <= overloads.
*
********************************************************************/
#ifndef VECTOR_HPP
#define  VECTOR_HPP

#include "memory.hpp"
#include <iostream>
#include <cassert>

template <typename T>
class Vector {
    // base is the beginning of the vector, last is the
    // end of initialized memory, and limit is the end
    // of the vector's capacity.
    T *base, *last, *limit;
    using iterator = T*;
    using const_iterator = const T*;
public:
    Vector<T>():
            base(nullptr),
            last(base),
            limit()
    { }

    // Constructor used to initialize in the format Vector v{1,2,3};
    // Can accept any number of arguments. Reserves memory in the
    // amount of the list size.
    Vector<T>(std::initializer_list<T> list) :
            base(nullptr),
            last(base),
            limit(last)
    {
        reserve(list.size());
        for (T n : list)
            push_back(n);
    }

    // Copy constructor. Vector rhs = lhs; Reserves memory in rhs
    // in the amount of lhs's size, then copies each element.
    Vector<T> (Vector const& v):
            base(nullptr),
            last(base),
            limit(last)
    {
        reserve(v.size());
        for(int i = 0; i != v.size(); ++i) push_back(v[i]);
    }

    // TODO: You will need to add the following constructor to
    // make the program compile. Be sure to initialize your
    // member variables before calling reserve.
    Vector(int n)
    {
        // TODO: Consider using std::uninitialized_fill to make
        // this more efficient.
        while (n) {
            push_back(T());
            --n;
        }
    }

    // Destructor. Iterates through the Vector and first destroys
    // each object, then deallocates the memory for the base pointer.
    ~Vector(){
        for(iterator i = base; i!=last;++i) destroy(i);
        deallocate(base);
    }

    // Member functions to access the size, beginning, end, capacity
    // and is_empty state of the Vector.
    int size() const { return last-base;}
    iterator begin() {return base;}
    const_iterator begin() const {return base;}
    iterator end() {return last;}
    const_iterator end() const {return last;}
    int capacity() const {return limit - base;};
    bool is_empty() const {return size()==0;}

    // Add a new element to the end of the vector
    inline void push_back(T n) {
        if(!base) reserve(16);
        else if(last == limit) reserve((2*capacity()));
        construct(last++, n);
    }

    // Remove the last element from the vector
    inline void pop_back(){
        if(is_empty()) return;
        destroy(--last);
    }

    // Insert anywhere in the vector, before the given iterator
    inline iterator insert(iterator insertBefore, T const& val){
        // If the vector is empty, push_back the value and return base
        if(is_empty()) {
            push_back(val);
            return base;
        }

        // else: create a copy of the vector from insertBefore on
        iterator cBase = allocate<T>(this->size());
        iterator cLast = std::uninitialized_copy(insertBefore, last, cBase);

        // remove the elements of the original vector from last to
        // insertBefore
        for(iterator i = last; i != insertBefore; --i) pop_back();

        // push_back() value to insert NOTE: push_back() will allocate
        // more memory in the case of a full vector
        push_back(val);

        // push the original elements in range: insertBefore to last
        // onto the vector
        for(; cBase != cLast; ++cBase) push_back(*cBase);

        // clean up the memory from the copies
        initialized_destroy(cBase, cLast);
        return --insertBefore;
    }

    // Erase any element from the vector at the given iterator
    inline void erase(iterator eraseAt){
        // if the element we're asked to erase is at the end, pop_back()
        if(eraseAt == last) pop_back();

        // else: make a copy from eraseAt+1 (so the element eraseAt
        // doesn't get copied) to last. These values will be added
        // back to the vector after we remove the element at eraseAt.
        iterator cBase = allocate<T>(this->size());
        iterator cLast = std::uninitialized_copy(eraseAt+1, last, cBase);

        // pop_back() elements up to and including eraseAt
        for(iterator i = last; i != eraseAt; --i) pop_back();

        // push the remaining original values back on to the vector
        for(; cBase != cLast; ++cBase) push_back(*cBase);

        // clean up the copies
        initialized_destroy(cBase, cLast);

    }

    // Reserve memory for the vector. Reserves enough memory for
    // n number of elements in the vector through the allocate
    // function defined in memory.hpp.
    inline void reserve(int n)  {
        // If reserving for a previously un-reserved vector
        if(!base){
            base = allocate<T>(n);
            last = base;
            limit = base+n;
        }
        else {
            // If the requested reserve can be accommodated by
            // the current capacity, return.
            if(n < capacity()) return;

            // Create copies so as to retain the original data,
            // then reserve the requested amount. We clear the
            // original Vector and are left with a new one with
            // the requested reserve and original data.
            iterator p = allocate<T>(sizeof(n) + n);
            iterator q = std::uninitialized_copy(base, last, p);
            // Cleanup
            this->clear();
            base = p;
            last = q;
            limit = base+n;
            initialized_destroy(p,q);
        }
    }

    //Clears all elements from the vector without releasing the memory
    void clear() {initialized_destroy(base, last); last = base;}

    // Assignment operator overload. Will copy the contents of
    // Vector lhs into rhs when writing: rhs = lhs;
    Vector& operator = (Vector const& lhs) {
        // Check for self assignment. If so, return.
        if(base == lhs.begin()) return *this;
        reserve(lhs.size());
        for(int i = 0; i != lhs.size(); ++i) push_back(lhs[i]);
        return *this;
    }

    // Overloads of array notation []. Returns the value
    // located at the position within the square brackets
    T const& operator [](int index) const {
        // Assert that the requested index is smaller  or equal
        //  to the vector size
        assert(index <= size());
        return *(base + index);
    }

    T& operator [](int index) {
        assert(index <= size());
        return *(base+index);
    }
};

// Overload operators == != < <= => >. For strings and chars the comparison
// is done lexicographically.
template <typename T>
bool operator == (const Vector<T>& lhs, const Vector<T>& rhs){
    if(lhs.size() != rhs.size()) return false;
    for(int i = 0; i != lhs.size(); ++i) if(lhs[i] != rhs[i]) return false;
    return true;
}

template <typename T>
bool operator < (Vector<T>& lhs, Vector<T>& rhs){
    T *first1 = lhs.begin(), *last1 = lhs.end(), *first2 = rhs.begin(),
            *last2 = rhs.end();
    for(;(first1 != last1) && (first2 != last2);first1++, first2++){
        if(*first1 < *first2) return true;
        if(*first2 < *first1) return false;
    }
    return(first1 == last1) && (first2 != last2);
}

template <typename T>
bool operator != (const Vector<T>& lhs, const Vector<T>& rhs){
    return !(lhs == rhs);
}

template <typename T>
bool operator <= (Vector<T>& lhs, Vector<T>& rhs){
    return !(rhs < lhs);
}

template <typename T>
bool operator > (Vector<T>& lhs, Vector<T>& rhs){
    return !(lhs <= rhs);
}

template <typename T>
bool operator >= (Vector<T>& lhs, Vector<T>& rhs){
    return !(lhs < rhs);
}
#endif
