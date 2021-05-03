/* ARRAY.cpp
 *   by Lut99
 *
 * Created:
 *   12/22/2020, 4:59:25 PM
 * Last edited:
 *   1/14/2021, 12:36:20 PM
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Our own Array class, which is optimizing for largely remaining the
 *   same size, but with options to change if needed.
**/

#include <stdexcept>
#include <type_traits>
#include <cstring>
#include <algorithm>

#include "tools/Array.hpp"

using namespace std;
using namespace Tools;


/***** ARRAY CLASS ****/
/* Default constructor for the Array class, which initializes it to zero. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>::Array() :
    elements(nullptr),
    length(0),
    max_length(0)
{}

/* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>::Array(size_t initial_size) :
    length(0),
    max_length(initial_size)
{
    // Allocate enough space
    this->elements = (T*) malloc(sizeof(T) * this->max_length);
    if (this->elements == nullptr) { throw std::bad_alloc(); }
}

/* Constructor for the Array class, which takes an initializer_list to initialize the Array with. Makes use of the element's copy constructor. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>::Array(const std::initializer_list<T>& list) :
    Array(list.size())
{
    // Overwrite the length to the list's size
    this->length = list.size();

    // Copy all the elements over
    size_t i = 0;
    for (const T& elem : list) {
        // Use the placement new to call T's copy constructor
        new(this->elements + i) T(elem);
        i++;
    }
}

/* Constructor for the Array class, which takes a raw C-style vector to copy elements from and its size. Note that the Array's element type must have a copy custructor defined. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>::Array(T* list, size_t list_size) :
    Array(list_size)
{
    // Overwrite the length to the list's size
    this->length = list_size;

    // Copy all the elements over
    for (size_t i = 0; i < list_size; i++) {
        // Use the placement new to call T's copy constructor
        new(this->elements + i) T(list[i]);
    }
}

/* Constructor for the Array class, which takes a C++-style vector. Note that the Array's element type must have a copy custructor defined. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>::Array(const std::vector<T>& list) :
    Array(list.size())
{
    // Overwrite the length to the list's size
    this->length = list.size();

    // Copy all the elements over
    for (size_t i = 0; i < list.size(); i++) {
        // Use the placement new to call T's copy constructor
        new(this->elements + i) T(list[i]);
    }
}

/* Copy constructor for the Array class. Note that this only works if the Array's element has a copy constructor defined. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>::Array(const Array& other) :
    length(other.length),
    max_length(other.max_length)
{
    // Allocate a new list of T's
    this->elements = (T*) malloc(sizeof(T) * other.max_length);
    if (this->elements == nullptr) { throw std::bad_alloc(); }

    // Copy each element over
    for (size_t i = 0; i < other.length; i++) {
        new(this->elements + i) T(other.elements[i]);
    }
}

/* Move constructor for the Array class. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>::Array(Array&& other) :
    elements(other.elements),
    length(other.length),
    max_length(other.max_length)
{
    // Tell the other one that it's over
    other.elements = nullptr;
}

/* Destructor for the Array class. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>::~Array() {
    // Only deallocate everything if not a nullptr
    if (this->elements != nullptr) {
        // First deallocate all elements if the element needs that
        if (std::is_destructible<T>::value) {
            for (size_t i = 0; i < this->length; i++) {
                this->elements[i].~T();
            }
        }
        // Now, free the list itself
        free(this->elements);
    }
}



/* Adds a new element of type T to the array, copying it. Note that this requires the element to be copy constructible. */
template<class T, bool D, bool C, bool M> void Array<T, D, C, M>::push_back(const T& elem) {
    // Make sure that the array has enough size
    if (this->length >= this->max_length) {
        this->reserve(this->length + 1);
    }

    // Add the element at the end of the array
    new(this->elements + this->length++) T(elem);
}

/* Adds a new element of type T to the array, leaving it in an usused state (moving it). Note that this requires the element to be move constructible. */
template<class T, bool D, bool C, bool M> void Array<T, D, C, M>::push_back(T&& elem) {
    // Make sure that the array has enough size
    if (this->length >= this->max_length) {
        this->reserve(this->length + 1);
    }

    // Add the element at the end of the array
    new(this->elements + this->length++) T(std::move(elem));
}

/* Removes the last element from the array. */
template<class T, bool D, bool C, bool M> void Array<T, D, C, M>::pop_back() {
    // Check if there are any elements
    if (this->length == 0) { return; }

    // Delete the last element if needed
    if (std::is_destructible<T>::value) {
        this->elements[this->length - 1].~T();
    }

    // Decrement the length
    --this->length;
}



/* Erases an element with the given index from the array. Does nothing if the index is out-of-bounds. */
template<class T, bool D, bool C, bool M> void Array<T, D, C, M>::erase(size_t index) {
    // Check if in bounds
    if (index >= this->length) { return; }

    // Otherwise, delete the element if needed
    if (std::is_destructible<T>::value) {
        this->elements[index].~T();
    }

    // Move all elements following it one back
    memmove(this->elements + index, this->elements + index + 1, sizeof(T) * (this->length - 1 - index));

    // Decrease the length
    --this->length;
}

/* Erases multiple elements in the given (inclusive) range from the array. Does nothing if the any index is out-of-bounds or if the start_index is larger than the stop_index. */
template<class T, bool D, bool C, bool M> void Array<T, D, C, M>::erase(size_t start_index, size_t stop_index) {
    // Check if in bounds
    if (start_index >= this->length || stop_index >= this->length || start_index > stop_index) { return; }

    // Otherwise, delete the elements if needed
    if (std::is_destructible<T>::value) {
        for (size_t i = start_index; i <= stop_index; i++) {
            this->elements[i].~T();
        }
    }

    // Move all elements following it back
    memmove(this->elements + start_index, this->elements + stop_index + 1, sizeof(T) * (this->length - 1 - stop_index));

    // Decrease the length
    this->length -= 1 + stop_index - start_index;
}

/* Erases everything from the array, even removing the internal allocated array. */
template<class T, bool D, bool C, bool M> void Array<T, D, C, M>::clear() {
    // Delete everything currently in the array if needed
    if (std::is_destructible<T>::value) {
        for (size_t i = 0; i < this->length; i++) {
            this->elements[i].~T();
        }
    }
    free(this->elements);

    // Set the new values
    this->elements = nullptr;
    this->length = 0;
    this->max_length = 0;
}



/* Re-allocates the internal array to the given size. Any leftover elements will be left unitialized, and elements that won't fit will be deallocated. */
template<class T, bool D, bool C, bool M> void Array<T, D, C, M>::reserve(size_t new_size) {
    // Do only something if the size is different from what it is now
    if (new_size == this->max_length) {
        // Do nothing
        return;
    }

    // Start by allocating space for a new array
    T* new_elements = (T*) malloc(sizeof(T) * new_size);
    if (new_elements == nullptr) { throw std::bad_alloc(); }

    // Determine how many elements to move over; either new_size or the length of the array, whichever is smaller
    size_t n_to_copy = std::min(new_size, this->length);
    memmove(new_elements, this->elements, sizeof(T) * n_to_copy);
    
    // Deallocate any elements that are leftover (if needed), then delete the old array
    if (std::is_destructible<T>::value) {
        for (size_t i = new_size; i < this->length; i++) {
            this->elements[i].~T();
        }
    }
    free(this->elements);

    // Re-populate the struct with the correct elements
    this->elements = new_elements;
    this->length = n_to_copy;
    this->max_length = new_size;
}

/* Resizes the array to the given size. Any leftover elements will be initialized with their default constructor, and elements that won't fit will be deallocated. If the same size is given as the vector, can be used to initialize a whole array to default constructor. */
template<class T, bool D, bool C, bool M> void Array<T, D, C, M>::resize(size_t new_size) {
    // Simply reserve the required space
    this->reserve(new_size);

    // Loop for the non-reserved elements to initialize them
    for (size_t i = this->length; i < this->max_length; i++) {
        new(this->elements + i) T();
    }

    // Update the length parameter, and we're done
    this->length = this->max_length;
}



/* Returns a muteable reference to the element at the given index. Performs in-of-bounds checks before accessing the element. */
template<class T, bool D, bool C, bool M> T& Array<T, D, C, M>::at(size_t index) {
    if (index >= this->length) { throw std::out_of_range("Index " + std::to_string(index) + " is out-of-bounds for Array with size " + std::to_string(this->length)); }
    return this->elements[index];
}

/* Returns a constant reference to the element at the given index. Performs in-of-bounds checks before accessing the element. */
template<class T, bool D, bool C, bool M> const T& Array<T, D, C, M>::at(size_t index) const {
    if (index >= this->length) { throw std::out_of_range("Index " + std::to_string(index) + " is out-of-bounds for Array with size " + std::to_string(this->length)); }
    return this->elements[index];
}



/* Returns a muteable pointer to the internal data struct. Use this to fill the array using C-libraries, but beware that the array needs to have enough space reserved. Also note that object put here will still be deallocated by the Array using ~T(). The optional new_size parameter is used to update the size() value of the array, so it knows what is initialized and what is not. Leave it at numeric_limits<size_t>::max() to leave the array size unchanged. */
template<class T, bool D, bool C, bool M> T* Array<T, D, C, M>::wdata(size_t new_size) {
    // Update the size if it's not the max already
    if (new_size != std::numeric_limits<size_t>::max()) { this->length = new_size; }
    // Return the pointer
    return this->elements;
}



/* Move assignment operator for the Array class. */
template<class T, bool D, bool C, bool M> Array<T, D, C, M>& Array<T, D, C, M>::operator=(Array<T, D, C, M>&& other) {
    if (this != &other) { swap(*this, other); }
    return *this;
}
