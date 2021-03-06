//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_OBJECT_HPP
#define BOOST_JSON_OBJECT_HPP

#include <boost/json/config.hpp>
#include <boost/json/kind.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/detail/object_impl.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

class value;

/** A dynamically sized associative container of JSON key/value pairs.

    This is an associative container whose elements
    are key/value pairs with unique keys.

    <br>

    The elements are stored contiguously, which means that
    elements can be accessed not only through iterators, but
    also using offsets to regular pointers to elements. A
    pointer to an element of an @ref object may be passed to
    any function that expects a pointer to
    @ref key_value_pair.

    <br>

    The container also maintains an internal index to speed
    up find operations, reducing the average complexity
    for most lookups and insertions

    <br>

    Reallocations are usually costly operations in terms of
    performance, as elements are copied and the internal
    index must be rebuilt. The @ref reserve function can
    be used to eliminate reallocations if the number of
    elements is known beforehand.

    @par Storage

    All elements stored in the container, and their
    children if any, will use the same storage that
    was used to construct the container.

    @par Thread Safety

    Non-const member functions may not be called
    concurrently with any other member functions.

    @par Satisfies

    Meets the requirements of
        <em>Container</em>,
        <em>ReversibleContainer</em>,
        <em>SequenceContainer</em>, and
        <em>UnorderedAssociativeContainer</em>.
*/
class object
{
    using object_impl =
        detail::object_impl;

    storage_ptr sp_;    // must come first
    kind k_ =           // must come second
        kind::object;
    object_impl impl_;

    struct undo_construct;
    class undo_insert;

    template<class T>
    using is_inputit = typename std::enable_if<
        std::is_constructible<key_value_pair,
        typename std::iterator_traits<T>::value_type
            >::value>::type;

    static
    constexpr
    double
    max_load_factor() noexcept
    {
        return 1.0;
    }

public:
    /** The type of keys.

        The function @ref string::max_size returns the
        maximum allowed size of strings used as keys.
    */
    using key_type = string_view;

    /// The type of mapped values
    using mapped_type = value;

    /// The element type
    using value_type = key_value_pair;

    /// The type used to represent unsigned integers
    using size_type = std::size_t;

    /// The type used to represent signed integers
    using difference_type = std::ptrdiff_t;

    /// A reference to an element
    using reference = value_type&;

    /// A const reference to an element
    using const_reference = value_type const&;

    /// A pointer to an element
    using pointer = value_type*;

    /// A const pointer to an element
    using const_pointer = value_type const*;

    /// A random access iterator to an element
    using iterator = value_type*;

    /// A const random access iterator to an element
    using const_iterator = value_type const*;

    /// A reverse random access iterator to an element
    using reverse_iterator =
        std::reverse_iterator<iterator>;

    /// A const reverse random access iterator to an element
    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;

    /// The type of initializer lists
    using init_list = std::initializer_list<
        std::pair<key_type, value>>;

    //------------------------------------------------------

    /** Destructor.

        The destructor for each element is called if needed,
        any used memory is deallocated, and shared ownership
        of the @ref storage is released.

        @par Complexity

        Constant, or linear in @ref size().
    */
    inline
    ~object()
    {
        impl_.destroy(sp_);
    }

#ifndef GENERATING_DOCUMENTATION
    // private
    BOOST_JSON_DECL
    explicit
    object(detail::unchecked_object&& uo);
#endif

    //------------------------------------------------------

    /** Default constructor.

        The constructed object is empty with zero
        capacity, using the default storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.
    */
    object() = default;

    /** Constructor.

        The constructed object is empty with zero
        capacity, using the specified storage.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    explicit
    object(storage_ptr sp) noexcept;

    /** Constructor.

        The constructed object is empty with capacity
        equal to the specified minimum capacity,
        using the specified storage.

        @par Complexity

        Constant.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param min_capacity The minimum number
        of elements for which capacity is guaranteed
        without a subsequent reallocation.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    object(
        std::size_t min_capacity,
        storage_ptr sp = {});

    /** Constructor.

        The object is constructed with the elements
        in the range `{first, last)`, preserving order,
        using the specified storage.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Constraints

        @code
        std::is_constructible_v<
            value_type,
            std::iterator_traits<InputIt>::value_type>
        @endcode

        @par Complexity

        Linear in `std::distance(first, last)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param first An input iterator pointing to the
        first element to insert, or pointing to the end
        of the range.

        @param last An input iterator pointing to the end
        of the range.

        @param min_capacity The minimum number
        of elements for which capacity is guaranteed
        without a subsequent reallocation.
        Upon construction, @ref capacity() will be greater
        than or equal to this number.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    object(
        InputIt first,
        InputIt last,
        std::size_t min_capacity = 0,
        storage_ptr sp = {});

    /** Move constructor.

        The object is constructed by acquiring ownership of
        the contents of `other` and shared ownership of
        the storage of `other`.

        @note

        After construction, the moved-from object behaves
        as if newly constructed with its current storage.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The object to move.
    */
    BOOST_JSON_DECL
    object(object&& other) noexcept;

    /** Move constructor.

        The object is constructed with the contents of
        `other` by move semantics, using the specified
        storage:

        @li If `*other.storage() == *sp`, ownership of
        the underlying memory is transferred in constant
        time, with no possibility of exceptions.
        After construction, the moved-from object behaves
        as if newly constructed with its current storage
        pointer.

        @li If `*other.storage() != *sp`, an
        element-wise copy is performed, which may throw.
        In this case, the moved-from object is not
        changed.
        
        @par Complexity

        Constant or linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The object to move.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    object(
        object&& other,
        storage_ptr sp);

    /** Pilfer constructor.

        The object is constructed by acquiring ownership of
        the contents of `other` using pilfer semantics.

        @note

        After construction, the moved-from object may only
        be destroyed.
        
        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param other The object to pilfer.

        @see
        
        Pilfering constructors are described in
        <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0308r0.html">Valueless Variants Considered Harmful</a>, by Peter Dimov.
    */
    BOOST_JSON_DECL
    object(pilfered<object> other) noexcept;

    /** Copy constructor.

        The object is constructed with a copy of the
        contents of `other`, using the storage of `other`.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The object to copy.
    */
    BOOST_JSON_DECL
    object(
        object const& other);

    /** Copy constructor.

        The object is constructed with a copy of the
        contents of `other`, using the specified storage.

        @par Complexity

        Linear in `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The object to copy.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    object(
        object const& other,
        storage_ptr sp);

    /** Constructor.

        The object is constructed with a copy of the values
        in the initializer-list in order, using the
        specified storage.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to insert.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    object(
        init_list init,
        storage_ptr sp = {})
        : object(init, 0, std::move(sp))
    {
    }

    /** Constructor.

        Storage for at least `min_capacity` elements is
        reserved, and then
        the object is constructed with a copy of the values
        in the initializer-list in order, using the
        specified storage.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Complexity

        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to insert.

        @param min_capacity The minimum number
        of elements for which capacity is guaranteed
        without a subsequent reallocation.
        Upon construction, @ref capacity() will be greater
        than or equal to this number.

        @param sp A pointer to the @ref storage
        to use. The container will acquire shared
        ownership of the storage object.
    */
    BOOST_JSON_DECL
    object(
        init_list init,
        std::size_t min_capacity,
        storage_ptr sp = {});

    //------------------------------------------------------

    /** Move assignment.

        The contents of the object are replaced with the
        contents of `other` using move semantics:

        @li If `*other.storage() == *sp`, ownership of
        the underlying memory is transferred in constant
        time, with no possibility of exceptions.
        After assignment, the moved-from object behaves
        as if newly constructed with its current storage
        pointer.

        @li If `*other.storage() != *sp`, an
        element-wise copy is performed, which may throw.
        In this case, the moved-from object is not
        changed.

        @par Complexity

        Constant or linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The object to move.
    */
    BOOST_JSON_DECL
    object&
    operator=(object&& other);

    /** Copy assignment.

        The contents of the object are replaced with an
        element-wise copy of `other`.

        @par Complexity

        Linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The object to copy.
    */
    BOOST_JSON_DECL
    object&
    operator=(object const& other);

    /** Assignment.

        Replaces the contents with the contents of an
        initializer list.

        @par Complexity

        Linear in @ref size() plus
        average case linear in `init.size()`,
        worst case quadratic in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param init The initializer list to copy.
    */
    BOOST_JSON_DECL
    object&
    operator=(init_list init);

    //------------------------------------------------------

    /** Return the storage used by the object.

        This returns the storage used by the object
        for all elements and all internal allocations.

        @par Complexity

        Constant.
    */
    storage_ptr const&
    storage() const noexcept
    {
        return sp_;
    }

    //------------------------------------------------------
    //
    // Iterators
    //
    //------------------------------------------------------

    /** Return an iterator to the first element.

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    inline
    iterator
    begin() noexcept;

    /** Return a const iterator to the first element.

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    inline
    const_iterator
    begin() const noexcept;

    /** Return a const iterator to the first element.

        If the container is empty, the returned iterator
        will be equal to @ref end().

        @par Complexity

        Constant.
    */
    inline
    const_iterator
    cbegin() const noexcept;

    /** Return an iterator to the element following the last element.

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    iterator
    end() noexcept;

    /** Return a const iterator to the element following the last element.

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    const_iterator
    end() const noexcept;

    /** Return a const iterator to the element following the last element.

        The element acts as a placeholder; attempting to
        access it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    const_iterator
    cend() const noexcept;

    /** Return a reverse iterator to the first element of the reversed container.

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    inline
    reverse_iterator
    rbegin() noexcept;

    /** Return a const reverse iterator to the first element of the reversed container.

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    inline
    const_reverse_iterator
    rbegin() const noexcept;

    /** Return a const reverse iterator to the first element of the reversed container.

        The pointed-to element corresponds to the last element
        of the non-reversed container. If the container is empty,
        the returned iterator is equal to @ref rend()

        @par Complexity

        Constant.
    */
    inline
    const_reverse_iterator
    crbegin() const noexcept;

    /** Return a reverse iterator to the element following the last element of the reversed container.

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    reverse_iterator
    rend() noexcept;

    /** Return a const reverse iterator to the element following the last element of the reversed container.

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    const_reverse_iterator
    rend() const noexcept;

    /** Return a const reverse iterator to the element following the last element of the reversed container.

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting to access
        it results in undefined behavior.

        @par Complexity

        Constant.
    */
    inline
    const_reverse_iterator
    crend() const noexcept;

    //------------------------------------------------------
    //
    // Capacity
    //
    //------------------------------------------------------

    /** Return whether there are no elements.

        Returns `true` if there are no elements in
        the container, i.e. @ref size() returns 0.

        @par Complexity

        Constant.
    */
    inline
    bool
    empty() const noexcept;

    /** Return the number of elements.

        This returns the number of elements in the container.

        @par Complexity

        Constant.
    */
    inline
    std::size_t
    size() const noexcept;

    /** Return the maximum number of elements the container can hold

        The maximum is an implementation-defined number dependent
        on system or library implementation. This value is a
        theoretical limit; at runtime, the actual maximum size
        may be less due to resource limits.

        @par Complexity

        Constant.
    */
    static
    constexpr
    std::size_t
    max_size() noexcept
    {
        return BOOST_JSON_MAX_OBJECT_SIZE;
    }

    /** Return the number of elements that can be held in currently allocated memory

        This number may be larger than the value returned
        by @ref size().

        @par Complexity

        Constant.
    */
    inline
    std::size_t
    capacity() const noexcept;

    /** Increase the capacity to at least a certain amount.

        This inserts an element into the container.

        @par Complexity

        Constant or average case linear in
        @ref size(), worst case quadratic.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param new_capacity The new minimum capacity.

        @throw std::length_error `new_capacity > max_size()`
    */
    inline
    void
    reserve(std::size_t new_capacity);

    //------------------------------------------------------
    //
    // Modifiers
    //
    //------------------------------------------------------

    /** Erase all elements.

        Erases all elements from the container without
        changing the capacity.
        After this call, @ref size() returns zero.
        All references, pointers, and iterators are
        invalidated.

        @par Complexity

        Linear in @ref size().

        @par Exception Safety

        No-throw guarantee.
    */
    BOOST_JSON_DECL
    void
    clear() noexcept;

    /** Insert elements.

        Inserts `p`, from which @ref value_type must
        be constructible.

        @par Constraints

        @code
        std::is_constructible_v<value_type, P>
        @endcode

        @par Complexity

        Average case amortized constant,
        worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        
        @param p The value to insert.

        @throw std::length_error key is too long.

        @throw std::length_error @ref size() >= max_size().

        @return A pair where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` if
        the assignment took place.
    */
     template<class P
#ifndef GENERATING_DOCUMENTATION
        ,class = typename std::enable_if<
            std::is_constructible<
                value_type, P, storage_ptr>::value>::type
#endif
    >
    std::pair<iterator, bool>
    insert(P&& p);

    /** Insert elements.

        The elements in the range `{first, last)` are
        appended to the end, in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Precondition

        `first` and `last` are not iterators into `*this`.

        @par Constraints

        @code
        std::is_constructible_v<value_type, std::iterator_traits<InputIt>::value_type>
        @endcode

        @par Complexity
        
        Linear in `std::distance(first, last)`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        
        @param first An input iterator pointing to the first
        element to insert, or pointing to the end of the range.

        @param last An input iterator pointing to the end
        of the range.

        @tparam InputIt a type meeting the requirements of
        __InputIterator__.
    */
    template<
        class InputIt
    #ifndef GENERATING_DOCUMENTATION
        ,class = is_inputit<InputIt>
    #endif
    >
    void
    insert(InputIt first, InputIt last)
    {
        insert_range(first, last, 0);
    }

    /** Insert elements.

        The elements in the initializer list are
        inserted at the end, in order.
        If multiple elements in the range have keys that
        compare equivalent, only the first occurring key
        will be inserted.

        @par Complexity
        
        Linear in `init.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        
        @param init The initializer list to insert
    */
    BOOST_JSON_DECL
    void
    insert(init_list init);

    /** Insert an element or assign to the current element if the key already exists.

        If the key equivalent to `key` already exists in the
        container. assigns `std::forward<M>(obj)` to the
        `mapped type` corresponding to the key. Otherwise,
        inserts the new value at the end as if by insert,
        constructing it from
        `value_type(key, std::forward<M>(obj))`.

        If the insertion occurs and results in a rehashing
        of the container, all iterators are invalidated.
        Otherwise, iterators are not affected.
        References are not invalidated.
        Rehashing occurs only if the new number of elements
        is greater than @ref capacity().

        @par Complexity

        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param key The key used for lookup and insertion

        @param m The value to insert or assign

        @throw std::length_error if key is too long

        @return A `std::pair` where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` if
        the assignment took place.
    */
    template<class M>
    std::pair<iterator, bool>
    insert_or_assign(
        key_type key, M&& m);

    /** Construct an element in-place.

        Inserts a new element into the container constructed
        in-place with the given argument if there is no
        element with the key in the container.
        The element is inserted after all the existing
        elements.

        If the insertion occurs and results in a rehashing
        of the container, all iterators are invalidated.
        Otherwise, iterators are not affected.
        References are not invalidated.
        Rehashing occurs only if the new number of elements
        is greater than @ref capacity().

        @par Complexity

        Amortized constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param key The key used for lookup and insertion

        @param arg The argument used to construct the value.
        This will be passed as `std::forward<Arg>(arg)` to
        the @ref value constructor.

        @throw std::length_error if key is too long

        @return A `std::pair` where `first` is an iterator
        to the existing or inserted element, and `second`
        is `true` if the insertion took place or `false` if
        the assignment took place.
    */
    template<class Arg>
    std::pair<iterator, bool>
    emplace(key_type key, Arg&& arg);

    /** Erase an element

        Remove the element pointed to by `pos`, which must
        be valid and dereferenceable. Thus the @ref end()
        iterator (which is valid but cannot be dereferenced)
        cannot be used as a value for `pos`.
        References and iterators to the erased element are
        invalidated. Other iterators and references are not
        invalidated.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param pos An iterator pointing to the element to be
        removed.

        @return The number of elements removed, which can
        be either 0 or 1.
    */
    BOOST_JSON_DECL
    iterator
    erase(const_iterator pos) noexcept;
    
    /** Erase an element

        Remove the element which matches `key`, if it exists.
        References and iterators to the erased element are
        invalidated. Other iterators and references are not
        invalidated.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @return The number of elements removed, which can
        be either 0 or 1.
    */
    BOOST_JSON_DECL
    std::size_t
    erase(key_type key) noexcept;

    /** Swap the contents.

        Exchanges the contents of this object with another
        object. Ownership of the respective @ref storage
        objects is not transferred.

        @li If `*other.storage() == *sp`, ownership of the
        underlying memory is swapped in constant time, with
        no possibility of exceptions. All iterators and
        references remain valid.

        @li If `*other.storage() != *sp`, the contents are
        logically swapped by making copies, which can throw.
        In this case all iterators and references are invalidated.

        @par Complexity

        Constant or linear in @ref size() plus `other.size()`.

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.

        @param other The object to swap with.
    */
    BOOST_JSON_DECL
    void
    swap(object& other);

    //------------------------------------------------------
    //
    // Lookup
    //
    //------------------------------------------------------

    /** Access the specified element, with bounds checking.

        Returns a reference to the mapped value of the element
        that matches `key`, otherwise throws.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @param key The key of the element to find

        @throw std::out_of_range if no such element exists.
    */
    BOOST_JSON_DECL
    value&
    at(key_type key);

    /** Access the specified element, with bounds checking.

        Returns a constant reference to the mapped value of
        the element that matches `key`, otherwise throws.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @param key The key of the element to find

        @throw std::out_of_range if no such element exists.
    */
    BOOST_JSON_DECL
    value const&
    at(key_type key) const;

    /** Access or insert the specified element

        Returns a reference to the value that is mapped
        to a key equivalent to key, performing an insertion
        of a null value if such key does not already exist.
        <br>
        If an insertion occurs and results in a rehashing of
        the container, all iterators are invalidated. Otherwise
        iterators are not affected. References are not
        invalidated. Rehashing occurs only if the new
        number of elements is greater than
        @ref capacity().

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        Strong guarantee.
        Calls to @ref storage::allocate may throw.
        If an exception is thrown by any operation, the
        insertion has no effect.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    value&
    operator[](key_type key);

    /** Count the number of elements with a specific key

        This function returns the count of the number of
        elements match `key`. The only possible return values
        are 0 and 1.

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    std::size_t
    count(key_type key) const noexcept;

    /** Find an element with a specific key

        This function returns an iterator to the element
        matching `key` if it exists, otherwise returns
        @ref end().

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    iterator
    find(key_type key) noexcept;

    /** Find an element with a specific key

        This function returns a constant iterator to
        the element matching `key` if it exists,
        otherwise returns @ref end().

        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param key The key of the element to find
    */
    BOOST_JSON_DECL
    const_iterator
    find(key_type key) const noexcept;

    /** Check if the container contains an element with a specific key

        This function returns `true` if an element with the
        specified key exists, otherwise returns `false`.
        @par Complexity

        Constant on average, worst case linear in @ref size().

        @par Exception Safety

        No-throw guarantee.

        @param key The key of the element to find

        @see find.
    */
    BOOST_JSON_DECL
    bool
    contains(key_type key) const noexcept;

private:
    struct place_one;
    struct place_range;

    template<class It>
    using iter_cat = typename
        std::iterator_traits<It>::iterator_category;

    static
    inline
    value_type*&
    next(value_type& e) noexcept;

    static
    inline
    value_type const*
    next(value_type const& e) noexcept;

    BOOST_JSON_DECL
    std::pair<value_type*, std::size_t>
    find_impl(key_type key) const noexcept;

    BOOST_JSON_DECL
    void
    rehash(std::size_t new_capacity);

    BOOST_JSON_DECL
    std::pair<iterator, bool>
    emplace_impl(
        key_type key, place_one& f);

    BOOST_JSON_DECL
    std::pair<iterator, bool>
    insert_impl(place_one& f);

    BOOST_JSON_DECL
    iterator
    insert_impl(
        std::size_t hash,
        place_one& f);

    BOOST_JSON_DECL
    std::pair<iterator, bool>
    insert_or_assign_impl(
        key_type key, place_one& f);

    BOOST_JSON_DECL
    void
    insert_range_impl(
        std::size_t min_capacity,
        place_range& f);

    template<class InputIt>
    void
    insert_range(
        InputIt first,
        InputIt last,
        std::size_t min_capacity,
        std::input_iterator_tag);

    template<class InputIt>
    void
    insert_range(
        InputIt first,
        InputIt last,
        std::size_t min_capacity,
        std::forward_iterator_tag);

    template<class InputIt>
    void
    insert_range(
        InputIt first,
        InputIt last,
        std::size_t min_capacity)
    {
        insert_range(
            first, last,
            min_capacity,
            iter_cat<InputIt>{});
    }
};

//------------------------------------------------------

/** Exchange the given values.

    Exchanges the contents of the object `lhs` with
    another object `rhs`. Ownership of the respective
    @ref storage objects is not transferred.

    @li If `*lhs.storage() == *rhs.storage()`,
    ownership of the underlying memory is swapped in
    constant time, with no possibility of exceptions.
    All iterators and references remain valid.

    @li If `*lhs.storage() != *rhs.storage()`,
    the contents are logically swapped by making a copy,
    which can throw. In this case all iterators and
    references are invalidated.

    @par Preconditions

    `&lhs != &rhs`
        
    @par Complexity

    Constant or linear in `lhs.size() + rhs.size()`.

    @par Exception Safety

    Strong guarantee.
    Calls to @ref storage::allocate may throw.

    @param lhs The object to exchange.

    @param rhs The object to exchange.
*/
inline
void
swap(object& lhs, object& rhs)
{
    lhs.swap(rhs);
}

} // json
} // boost

// Must be included here for this file to stand alone
#include <boost/json/value.hpp>

// includes are at the bottom of <boost/json/value.hpp>

#endif
