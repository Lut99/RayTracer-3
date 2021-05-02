/* ARRAY.hpp
 *   by Lut99
 *
 * Created:
 *   12/22/2020, 5:00:01 PM
 * Last edited:
 *   1/14/2021, 3:11:16 PM
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Our own Array class, which is optimizing for largely remaining the
 *   same size, but with options to change if needed.
**/

#ifndef TOOLS_ARRAY_HPP
#define TOOLS_ARRAY_HPP

/***** HEADER *****/
#include <cstddef>
#include <vector>
#include <initializer_list>
#include <limits>
#include <cstdio>

namespace Tools {
    /* The Array class, which can be used as a container for many things. It's optimized to work for containers that rarely (but still sometimes) have to resize. */
    template <class T, bool D = std::is_default_constructible<T>::value, bool C = std::is_copy_constructible<T>::value, bool M = std::is_move_constructible<T>::value>
    class Array {
    protected:
        /* The internal data. */
        T* elements;
        /* The number of internal elements. */
        size_t length;
        /* The maximum number of elements we allocated for. */
        size_t max_length;

    public:
        /* Default constructor for the Array class, which initializes it to zero. */
        Array();
        /* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
        Array(size_t initial_size);
        /* Constructor for the Array class, which takes an initializer_list to initialize the Array with. Makes use of the element's copy constructor. */
        Array(const std::initializer_list<T>& list);
        /* Constructor for the Array class, which takes a raw C-style vector to copy elements from and its size. Note that the Array's element type must have a copy custructor defined. */
        Array(T* list, size_t list_size);
        /* Constructor for the Array class, which takes a C++-style vector. Note that the Array's element type must have a copy custructor defined. */
        Array(const std::vector<T>& list);
        /* Copy constructor for the Array class. Note that this only works if the Array's element has a copy constructor defined. */
        Array(const Array& other);
        /* Move constructor for the Array class. */
        Array(Array&& other);
        /* Destructor for the Array class. */
        virtual ~Array();

        /* Adds a new element of type T to the array, copying it. Note that this requires the element to be copy constructible. */
        void push_back(const T& elem);
        /* Adds a new element of type T to the array, leaving it in an usused state (moving it). Note that this requires the element to be move constructible. */
        void push_back(T&& elem);
        /* Removes the last element from the array. */
        void pop_back();
        
        /* Erases an element with the given index from the array. Does nothing if the index is out-of-bounds. */
        void erase(size_t index);
        /* Erases multiple elements in the given (inclusive) range from the array. Does nothing if the any index is out-of-bounds or if the start_index is larger than the stop_index. */
        void erase(size_t start_index, size_t stop_index);
        /* Erases everything from the array, even removing the internal allocated array. */
        void clear();

        /* Re-allocates the internal array to the given size. Any leftover elements will be left unitialized, and elements that won't fit will be deallocated. */
        void reserve(size_t new_size);
        /* Resizes the array to the given size. Any leftover elements will be initialized with their default constructor, and elements that won't fit will be deallocated. */
        void resize(size_t new_size);

        /* Returns a muteable reference to the element at the given index. Does not perform any in-of-bounds checking. */
        inline T& operator[](size_t index) { return this->elements[index]; }
        /* Returns a constant reference to the element at the given index. Does not perform any in-of-bounds checking. */
        inline const T& operator[](size_t index) const { return this->elements[index]; }
        /* Returns a muteable reference to the element at the given index. Performs in-of-bounds checks before accessing the element. */
        T& at(size_t index);
        /* Returns a constant reference to the element at the given index. Performs in-of-bounds checks before accessing the element. */
        const T& at(size_t index) const;

        /* Returns a muteable pointer to the internal data struct. Use this to fill the array using C-libraries, but beware that the array needs to have enough space reserved. Also note that object put here will still be deallocated by the Array using ~T(). The optional new_size parameter is used to update the size() value of the array, so it knows what is initialized and what is not. Leave it at numeric_limits<size_t>::max() to leave the array size unchanged. */
        T* const wdata(size_t new_size = std::numeric_limits<size_t>::max());
        /* Returns a constant pointer to the internal data struct. Use this to read from the array using C-libraries, but beware that the array needs to have enough space reserved. */
        inline const T* const rdata() const { return this->elements; }
        /* Returns true if there are no elements in this array, or false otherwise. */
        inline bool empty() const { return this->length == 0; }
        /* Returns the number of elements stored in this Array. */
        inline size_t size() const { return this->length; }
        /* Returns the number of elements this Array can store before resizing. */
        inline size_t capacity() const { return this->max_length; }

        /* Copy assignment operator for the Array class. Depends on Array's copy constructor, and therefore requires the Array's type to be copy constructible. */
        inline Array<T, D, C, M>& operator=(const Array<T, D, C, M>& other) { return *this = Array<T, D, C, M>(other); }
        /* Move assignment operator for the Array class. */
        Array<T, D, C, M>& operator=(Array<T, D, C, M>&& other);
        /* Swap operator for the Array class. */
        friend void swap(Array<T, D, C, M>& a1, Array<T, D, C, M>& a2) {
            using std::swap;

            swap(a1.elements, a2.elements);
            swap(a1.length, a2.length);
            swap(a1.max_length, a2.max_length);
        }
    };

    

    /* The Array class, which can be used as a container for many things. It's optimized to work for containers that rarely (but still sometimes) have to resize.
     * This class specialization is for when the contained type is copy constructible and move constructible, but not default constructible.
     */
    template <class T>
    class Array<T, false, true, true>: public Array<T, true, true, true> {
    public:
        /* Default constructor for the Array class, which initializes it to zero. */
        Array(): Array<T, true, true, true>() {}
        /* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
        Array(size_t initial_size): Array<T, true, true, true>(initial_size) {}
        /* Constructor for the Array class, which takes an initializer_list to initialize the Array with. Makes use of the element's copy constructor. */
        Array(const std::initializer_list<T>& list): Array<T, true, true, true>(list) {}
        /* Constructor for the Array class, which takes a raw C-style vector to copy elements from and its size. Note that the Array's element type must have a copy custructor defined. */
        Array(T* list, size_t list_size): Array<T, true, true, true>(list, list_size) {}
        /* Constructor for the Array class, which takes a C++-style vector. Note that the Array's element type must have a copy custructor defined. */
        Array(const std::vector<T>& list): Array<T, true, true, true>(list) {}
        /* Copy constructor for the Array class. Note that this only works if the Array's element has a copy constructor defined. */
        Array(const Array& other): Array<T, true, true>(other) {}
        /* Move constructor for the Array class. */
        Array(Array&& other): Array<T, true, true, true>(std::move(other)) {};
        /* Destructor for the Array class. */
        ~Array() {}

        /* Resizes the array to the given size. Any leftover elements will be initialized with their default constructor, and elements that won't fit will be deallocated. */
        void resize(size_t new_size) = delete;

        /* Copy assignment operator for the Array class. Depends on Array's copy constructor, and therefore requires the Array's type to be copy constructible. */
        inline Array& operator=(const Array& other) { return *this = Array(other); }
        /* Move assignment operator for the Array class. */
        inline Array& operator=(Array&& other) { if (this != &other) { swap(*this, other); }; return *this; }
        /* Swap operator for the Array class. */
        friend inline void swap(Array& a1, Array& a2) { return swap((Array<T, true, true, true>&) a1, (Array<T, true, true, true>&) a2); }
    };
    
    /* The Array class, which can be used as a container for many things. It's optimized to work for containers that rarely (but still sometimes) have to resize.
     * This class specialization is for when the contained type is default constructible and move constructible, but not copy constructible.
     */
    template <class T>
    class Array<T, true, false, true>: public Array<T, true, true, true> {
    public:
        /* Default constructor for the Array class, which initializes it to zero. */
        Array(): Array<T, true, true, true>() {}
        /* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
        Array(size_t initial_size): Array<T, true, true, true>(initial_size) {}
        /* Copy constructor for the Array class. Deleted, since the chosen type does not support copy constructing. */
        Array(const Array& other) = delete;
        /* Move constructor for the Array class. */
        Array(Array&& other): Array<T, true, true, true>(std::move(other)) {};
        /* Destructor for the Array class. */
        ~Array() {}
        
        /* Adds a new element of type T to the array, copying it. Note that this requires the element to be copy constructible. */
        void push_back(const T& elem) = delete;
        /* Adds a new element of type T to the array, leaving it in an usused state (moving it). Note that this requires the element to be move constructible. */
        void push_back(T&& elem) { return Array<T, true, true, true>::push_back(std::move(elem)); }

        /* Copy assignment operator for the Array class. Depends on Array's copy constructor, and therefore requires the Array's type to be copy constructible. */
        inline Array& operator=(const Array& other) = delete;
        /* Move assignment operator for the Array class. */
        inline Array& operator=(Array&& other) { if (this != &other) { swap(*this, other); }; return *this; }
        /* Swap operator for the Array class. */
        friend inline void swap(Array& a1, Array& a2) { swap((Array<T, true, true, true>&) a1, (Array<T, true, true, true>&) a2); }
    };
    
    /* The Array class, which can be used as a container for many things. It's optimized to work for containers that rarely (but still sometimes) have to resize.
     * This class specialization is for when the contained type is default constructible and copy constructible, but not move constructible.
     */
    template <class T>
    class Array<T, true, true, false>: public Array<T, true, true, true> {
    public:
        /* Default constructor for the Array class, which initializes it to zero. */
        Array(): Array<T, true, true, true>() {}
        /* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
        Array(size_t initial_size): Array<T, true, true, true>(initial_size) {}
        /* Constructor for the Array class, which takes an initializer_list to initialize the Array with. Makes use of the element's copy constructor. */
        Array(const std::initializer_list<T>& list): Array<T, true, true, true>(list) {}
        /* Constructor for the Array class, which takes a raw C-style vector to copy elements from and its size. Note that the Array's element type must have a copy custructor defined. */
        Array(T* list, size_t list_size): Array<T, true, true, true>(list, list_size) {}
        /* Constructor for the Array class, which takes a C++-style vector. Note that the Array's element type must have a copy custructor defined. */
        Array(const std::vector<T>& list): Array<T, true, true, true>(list) {}
        /* Copy constructor for the Array class. Note that this only works if the Array's element has a copy constructor defined. */
        Array(const Array& other): Array<T, true, true>(other) {}
        /* Move constructor for the Array class. */
        Array(Array&& other): Array<T, true, true, true>(std::move(other)) {};
        /* Destructor for the Array class. */
        ~Array() {}

        /* Adds a new element of type T to the array, copying it. Note that this requires the element to be copy constructible. */
        void push_back(const T& elem) { return Array<T, true, true, true>::push_back(elem); }
        /* Adds a new element of type T to the array, leaving it in an usused state (moving it). Note that this requires the element to be move constructible. */
        void push_back(T&& elem) = delete;

        /* Copy assignment operator for the Array class. Depends on Array's copy constructor, and therefore requires the Array's type to be copy constructible. */
        inline Array& operator=(const Array& other) { return *this = Array(other); }
        /* Move assignment operator for the Array class. */
        inline Array& operator=(Array&& other) { if (this != &other) { swap(*this, other); }; return *this; }
        /* Swap operator for the Array class. */
        friend inline void swap(Array& a1, Array& a2) { swap((Array<T, true, true, true>&) a1, (Array<T, true, true, true>&) a2); }
    };
    
    /* The Array class, which can be used as a container for many things. It's optimized to work for containers that rarely (but still sometimes) have to resize.
     * This class specialization is for when the contained type is neither default constructible nor copy constructible, but is move constructible.
     */
    template <class T>
    class Array<T, false, false, true>: public Array<T, true, true, true> {
    public:
        /* Default constructor for the Array class, which initializes it to zero. */
        Array(): Array<T, true, true, true>() {}
        /* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
        Array(size_t initial_size): Array<T, true, true, true>(initial_size) {}
        /* Copy constructor for the Array class. Note that this only works if the Array's element has a copy constructor defined. */
        Array(const Array& other) = delete;
        /* Move constructor for the Array class. */
        Array(Array&& other): Array<T, true, true, true>(std::move(other)) {};
        /* Destructor for the Array class. */
        ~Array() {}

        /* Adds a new element of type T to the array, copying it. Note that this requires the element to be copy constructible. */
        void push_back(const T& elem) = delete;
        /* Adds a new element of type T to the array, leaving it in an usused state (moving it). Note that this requires the element to be move constructible. */
        void push_back(T&& elem) { return Array<T, true, true, true>::push_back(std::move(elem)); }

        /* Resizes the array to the given size. Any leftover elements will be initialized with their default constructor, and elements that won't fit will be deallocated. */
        void resize(size_t new_size) = delete;

        /* Copy assignment operator for the Array class. Depends on Array's copy constructor, and therefore requires the Array's type to be copy constructible. */
        inline Array& operator=(const Array& other) = delete;
        /* Move assignment operator for the Array class. */
        inline Array& operator=(Array&& other) { if (this != &other) { swap(*this, other); }; return *this; }
        /* Swap operator for the Array class. */
        friend inline void swap(Array& a1, Array& a2) { swap((Array<T, true, true, true>&) a1, (Array<T, true, true, true>&) a2); }
    };

    /* The Array class, which can be used as a container for many things. It's optimized to work for containers that rarely (but still sometimes) have to resize.
     * This class specialization is for when the contained type is neither default constructible nor move constructible, but is copy constructible.
     */
    template <class T>
    class Array<T, false, true, false>: public Array<T, true, true, true> {
    public:
        /* Default constructor for the Array class, which initializes it to zero. */
        Array(): Array<T, true, true, true>() {}
        /* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
        Array(size_t initial_size): Array<T, true, true, true>(initial_size) {}
        /* Constructor for the Array class, which takes an initializer_list to initialize the Array with. Makes use of the element's copy constructor. */
        Array(const std::initializer_list<T>& list): Array<T, true, true, true>(list) {}
        /* Constructor for the Array class, which takes a raw C-style vector to copy elements from and its size. Note that the Array's element type must have a copy custructor defined. */
        Array(T* list, size_t list_size): Array<T, true, true, true>(list, list_size) {}
        /* Constructor for the Array class, which takes a C++-style vector. Note that the Array's element type must have a copy custructor defined. */
        Array(const std::vector<T>& list): Array<T, true, true, true>(list) {}
        /* Copy constructor for the Array class. Note that this only works if the Array's element has a copy constructor defined. */
        Array(const Array& other): Array<T, true, true>(other) {}
        /* Move constructor for the Array class. */
        Array(Array&& other): Array<T, true, true, true>(std::move(other)) {};
        /* Destructor for the Array class. */
        ~Array() {}

        /* Adds a new element of type T to the array, copying it. Note that this requires the element to be copy constructible. */
        inline void push_back(const T& elem) { return Array<T, true, true, true>::push_back(elem); }
        /* Adds a new element of type T to the array, leaving it in an usused state (moving it). Note that this requires the element to be move constructible. */
        void push_back(T&& elem) = delete;

        /* Resizes the array to the given size. Any leftover elements will be initialized with their default constructor, and elements that won't fit will be deallocated. */
        void resize(size_t new_size) = delete;

        /* Copy assignment operator for the Array class. Depends on Array's copy constructor, and therefore requires the Array's type to be copy constructible. */
        inline Array& operator=(const Array& other) { return *this = Array(other); }
        /* Move assignment operator for the Array class. */
        inline Array& operator=(Array&& other) { if (this != &other) { swap(*this, other); }; return *this; }
        /* Swap operator for the Array class. */
        friend inline void swap(Array& a1, Array& a2) { swap((Array<T, true, true, true>&) a1, (Array<T, true, true, true>&) a2); }
    };

    /* The Array class, which can be used as a container for many things. It's optimized to work for containers that rarely (but still sometimes) have to resize.
     * This class specialization is for when the contained type is neither copy constructible nor move constructible, but is default constructible.
     */
    template <class T>
    class Array<T, true, false, false>: public Array<T, true, true, true> {
    public:
        /* Default constructor for the Array class, which initializes it to zero. */
        Array(): Array<T, true, true, true>() {}
        /* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
        Array(size_t initial_size): Array<T, true, true, true>(initial_size) {}
        /* Copy constructor for the Array class. Note that this only works if the Array's element has a copy constructor defined. */
        Array(const Array& other) = delete;
        /* Move constructor for the Array class. */
        Array(Array&& other): Array<T, true, true, true>(std::move(other)) {}
        /* Destructor for the Array class. */
        ~Array() {}

        /* Adds a new element of type T to the array, copying it. Note that this requires the element to be copy constructible. */
        void push_back(const T& elem) = delete;
        /* Adds a new element of type T to the array, leaving it in an usused state (moving it). Note that this requires the element to be move constructible. */
        void push_back(T&& elem) = delete;

        /* Copy assignment operator for the Array class. Depends on Array's copy constructor, and therefore requires the Array's type to be copy constructible. */
        inline Array& operator=(const Array& other) = delete;
        /* Move assignment operator for the Array class. */
        inline Array& operator=(Array&& other) { if (this != &other) { swap(*this, other); }; return *this; }
        /* Swap operator for the Array class. */
        friend inline void swap(Array& a1, Array& a2) { swap((Array<T, true, true, true>&) a1, (Array<T, true, true, true>&) a2); }
    };

    /* The Array class, which can be used as a container for many things. It's optimized to work for containers that rarely (but still sometimes) have to resize.
     * This class specialization is for when the contained type is neither default constructible, copy constructible nor move constructibl.
     */
    template <class T>
    class Array<T, false, false, false>: public Array<T, true, true, true> {
    public:
        /* Default constructor for the Array class, which initializes it to zero. */
        Array(): Array<T, true, true, true>() {}
        /* Constructor for the Array class, which takes an initial amount to size to. Each element will thus be uninitialized. */
        Array(size_t initial_size): Array<T, true, true, true>(initial_size) {}
        /* Copy constructor for the Array class. Note that this only works if the Array's element has a copy constructor defined. */
        Array(const Array& other) = delete;
        /* Move constructor for the Array class. */
        Array(Array&& other): Array<T, true, true, true>(std::move(other)) {}
        /* Destructor for the Array class. */
        ~Array() {}

        /* Adds a new element of type T to the array, copying it. Note that this requires the element to be copy constructible. */
        void push_back(const T& elem) = delete;
        /* Adds a new element of type T to the array, leaving it in an usused state (moving it). Note that this requires the element to be move constructible. */
        void push_back(T&& elem) = delete;

        /* Resizes the array to the given size. Any leftover elements will be initialized with their default constructor, and elements that won't fit will be deallocated. */
        void resize(size_t new_size) = delete;

        /* Copy assignment operator for the Array class. Depends on Array's copy constructor, and therefore requires the Array's type to be copy constructible. */
        inline Array& operator=(const Array& other) = delete;
        /* Move assignment operator for the Array class. */
        inline Array& operator=(Array&& other) { if (this != &other) { swap(*this, other); }; return *this; }
        /* Swap operator for the Array class. */
        friend inline void swap(Array& a1, Array& a2) { swap((Array<T, true, true, true>&) a1, (Array<T, true, true, true>&) a2); }
    };
}





/***** IMPLEMENTATIONS *****/
#include "tools/Array.cpp"

#endif
