#ifndef ARRAY_H_
#define ARRAY_H_

#include "datatypes/matrix.h"

/**
 * @brief \f$ n\times m \f$ Array type
 * @details Acts as array class which stores data in a simple C style array.
 *
 * Main functionality which the Array class offers is to supplement the fall backs of storing
 * information in a Matrix data structure.
 *
 * For example assigning a value to each element with the Matrix class is not directly possible
 * using the assignment operator=. This is because assignment with matrices is defined as
 * \f$ M = a = a*I \f$ where M is a general matrix, a is a scalar and I an identity matrix. The
 * result would only assign a to the diagonal elements.
 *
 * Unlike the Matrix object, the Array object assigns element wise. This allows filling the Array
 * with the assignment operator. Additionally element wise operations are useable as long as they
 * are defined for the Array type. For example the operation:
 *
 * \code
 * Array<n,m,double> m.random();
 * sin(m);
 * \endcode
 *
 * is defined, since the \f$sin\f$ function is defined for doubles.
 *
 * The above operation would not work for matrices, but with casting operations. Matrix::asArray and
 * Array::asMatrix one can take full advantage of element wise operations.
 *
 * @tparam n Number of rows
 * @tparam m Number of columns
 * @tparam T Array element type
 */
template <const int n, const int m, typename T>
class Array {
  public:
    static_assert(hila::is_complex_or_arithmetic<T>::value,
                  "Array requires Complex or arithmetic type");

    // Store Array contents in one dimensional C style array
    T c[n * m];

    // std incantation for field types
    using base_type = hila::scalar_type<T>;
    using argument_type = T;

    /**
     * @brief Construct a new Matrix object
     * @details The following ways of constructing a matrix are:
     *
     * __NOTE__: n,m are integers and MyType is a HILA [standard type](@ref standard) or Complex.
     *
     * __Default constructor__:
     *
     * Allocates undefined \f$ n\times m\f$ Array.
     *
     * \code{.cpp}
     * Array<n,m,MyType> A;
     * \endcode
     *
     *
     * __Scalar constructor__:
     *
     * Construct with given scalar which is assigned to all elements in array
     *
     * \code{.cpp}
     * MyType x = hila::random();
     * Array<n,m,MyType> A = x;
     * \endcode
     *
     * __Copy constructor__:
     *
     * Construction from previously defined Array.
     *
     * \code{.cpp}
     * Array<n,m,MyOtherType> A_0;
     * //
     * // A_0 is filled with content
     * //
     * Array<n,m,MyType> A = A_0;
     * \endcode
     *
     * __Initializer list__:
     *
     * Construction from c++ initializer list.
     *
     * \code{.cpp}
     * Array<2,2,int> A = {1, 0
     *                      0, 1};
     * \endcode
     */
    Array() = default;
    ~Array() = default;
    Array(const Array<n, m, T> &v) = default;

    // constructor from scalar - make this also explicit, consistency
    template <typename S, std::enable_if_t<hila::is_assignable<T &, S>::value, int> = 0>
    explicit inline Array(const S rhs) {
        for (int i = 0; i < n * m; i++) {
            this->c[i] = rhs;
        }
    }

    // and make non-explicit constructor from 0
#pragma hila loop_function
    inline Array(const std::nullptr_t &z) {
        for (int i = 0; i < n * m; i++)
            c[i] = static_cast<T>(0);
    }

    // Construct array automatically from right-size initializer list
    // This does not seem to be dangerous, so keep non-explicit

    template <typename S, std::enable_if_t<hila::is_assignable<T &, S>::value, int> = 0>
    inline Array(std::initializer_list<S> rhs) {
        assert(rhs.size() == n * m && "Array initializer list size must match variable size");
        int i = 0;
        for (auto it = rhs.begin(); it != rhs.end(); it++, i++) {
            c[i] = *it;
        }
    }

    /**
     * @brief Returns number of rows
     *
     * @return constexpr int
     */
    constexpr int rows() const {
        return n;
    }
    /**
     * @brief Returns number of columns
     *
     * @return constexpr int
     */
    constexpr int columns() const {
        return m;
    }

    /**
     * @brief Returns size of #Vector Array or square Array
     *
     * @tparam q Number of rows n
     * @tparam p Number of columns m
     * @return constexpr int
     */
    template <int q = n, int p = m, std::enable_if_t<q == 1, int> = 0>
    constexpr int size() const {
        return p;
    }

    template <int q = n, int p = m, std::enable_if_t<p == 1, int> = 0>
    constexpr int size() const {
        return q;
    }

    template <int q = n, int p = m, std::enable_if_t<q == p, int> = 0>
    constexpr int size() const {
        return q;
    }

    /**
     * @brief Standard array indexing operation for 2D and 1D Arrays
     *
     * @details Accessing singular elements is insufficient, but matrix elements are often quite
     * small.
     *
     * Exammple for 2D Array:
     * \code
     *  Array<n,m,MyType> A;
     *  MyType a = A.e(i,j); \\ i <= n, j <= m
     * \endcode
     *
     * Example for Vector Array:
     * \code {.cpp}
     *  Array1d<n,MyType> A;
     * MyType a = A.e(i) \\ i <= n
     * \endcode
     *
     * @param i row index
     * @param j column index
     * @return T matrix element type
     */
    inline T e(const int i, const int j) const {
        return c[i * m + j];
    }
    // standard access ops m.e(i,j) - assume T is small, as it should
    inline T &e(const int i, const int j) const_function {
        return c[i * m + j];
    }

    // declare single e here too in case we have a vector
    // (one size == 1)
    template <int q = n, int p = m, std::enable_if_t<(q == 1 || p == 1), int> = 0>
    inline T e(const int i) const {
        return c[i];
    }

    template <int q = n, int p = m, std::enable_if_t<(q == 1 || p == 1), int> = 0>
    inline T &e(const int i) const_function {
        return c[i];
    }

    /**
     * @brief Cast Array to Matrix
     *
     * @return Matrix<n, m, T>&
     */
    Matrix<n, m, T> &asMatrix() const_function {
        return *reinterpret_cast<Matrix<n, m, T> *>(this);
    }

    const Matrix<n, m, T> &asMatrix() const {
        return *reinterpret_cast<const Matrix<n, m, T> *>(this);
    }

    /**
     * @brief Cast Array1D to Vector
     * @details asMatrix will perform the same operation.
     *
     * @return Vector<n, T>&
     */
    Vector<n, T> &asVector() const_function {
        static_assert(1 == m, "asVector() only for column arrays");
        return *reinterpret_cast<Vector<n, T> *>(this);
    }

    const Vector<n, T> &asVector() const {
        static_assert(1 == m, "asVector() only for column arrays");
        return *reinterpret_cast<const Vector<n, T> *>(this);
    }


    // casting from one Array (number) type to another
    // TODO: CHECK AVX CONVERSIONS
    template <typename S, std::enable_if_t<hila::is_assignable<S &, T>::value, int> = 0>
    operator Array<n, m, S>() {
        Array<n, m, S> res;
        for (int i = 0; i < n * m; i++) {
            res.c[i] = c[i];
        }
        return res;
    }

    /**
     * @brief Unary - operator
     * @details Returns Array with the signs of all the elements in the Arrat flipped.
     *
     * @return Array<n,m,T>
     */
    inline Array<n, m, T> operator-() const {
        Array<n, m, T> res;
        for (int i = 0; i < n * m; i++) {
            res.c[i] = -c[i];
        }
        return res;
    }

    /**
     * @brief Unary + operator
     * @details Equivalent to identity operator meaning that Arrat stays as is.
     *
     * @return const Array<n, m, T>
     */
    inline Array<n, m, T> operator+() const {
        return *this;
    }

    /**
     * @brief Assignment operator = to assign values to Array
     * @details The following ways to assign an Array are:
     *
     *
     * __Assignment from Array__:
     *
     * \code {.cpp}
     * Array<n,m,MyType> A_0;
     * .
     * . A_0 has values assigned to it
     * .
     * Array<n,m,MyType> A; \\ undefined matrix
     * A = A_0; \\ Assignment from A_0
     * \endcode
     *
     * __Assignment from 0__:
     *
     * \code {.cpp}
     * Array<n,m,MyType> A;
     * A = 0; Zero matrix;
     * \endcode
     *
     * __Assignment from scalar__:
     *
     * Assignment from scalar assigns the scalar to the diagonal elements as \f$ A = I\cdot a\f$
     *
     * \code {.cpp}
     * MyType a = hila::random;
     * Array<n,m,MyType> A;
     * A = a; A = I*a
     * \endcode
     *
     *__Initializer list__:
     *
     * Assignment from c++ initializer list.
     *
     * \code{.cpp}
     * Array<2,2,int> A ;
     * A = {1, 0
     *      0, 1};
     * \endcode
     */
#pragma hila loop_function
    template <typename S, std::enable_if_t<hila::is_assignable<T &, S>::value, int> = 0>
    inline Array<n, m, T> &operator=(const S rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] = rhs;
        }
        return *this;
    }

    /**
     * @brief Compare equality of Arrays
     *
     * Two Arrays are equal iff Arrays are of same dimension and all elements compare to equal
     * Note: Complex == scalar if arithmetic value is equal
     *
     * @tparam S
     * @param rhs
     * @return true if equal
     */

    template <typename S, int n1, int m1>
    bool operator==(const Array<n1, m1, S> &rhs) const {
        if constexpr (n != n1 || m != m1)
            return false;

        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++) {
                if (e(i, j) != rhs.e(i, j))
                    return false;
            }
        return true;
    }

    /**
     * @brief Compare non-equality of two Arrays
     *
     * Negation of operator==()
     *
     * @tparam S
     * @tparam n1
     * @tparam m1
     * @param rhs
     * @return true
     * @return false
     */

    template <typename S, int n1, int m1>
    bool operator!=(const Array<n1, m1, S> &rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Add assign operator with Array or scalar
     * @details Add assign operator can be used in the following ways
     *
     * __Add assign Array__:
     *
     * Requires that Arrays have same dimensions
     *
     * \code {.cpp}
     * Array<n,m,MyType> A,B;
     * A = 1;
     * B = 1;
     * A += B; \\ A is uniformly 2
     * \endcode
     *
     * __Add assign scalar__:
     *
     * Adds scalar \f$ a \f$ to Array uniformly
     *
     * \code {.cpp}
     * Array<n,m,MyType> A = 1;
     * A += 1 ; \\ A is uniformly 2
     * \endcode
     *
     * @tparam S Element type of rhs
     * @param rhs Array to add
     * @return Array<n, m, T>
     */
#pragma hila loop_function
    template <typename S, std::enable_if_t<std::is_convertible<S, T>::value, int> = 0>
    Array<n, m, T> &operator+=(const Array<n, m, S> &rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] += rhs.c[i];
        }
        return *this;
    }

    /**
     * @brief Subtract assign operator with Array or scalar
     * @details Subtract assign operator can be used in the following ways
     *
     * __Subtract assign Array__:
     *
     * \code {.cpp}
     * Array<n,m,MyType> A,B;
     * A = 3;
     * B = 1;
     * A -= B; \\ A is uniformly 2
     * \endcode
     *
     * __Subtract assign scalar__:
     *
     * Subtract scalar uniformly from square matrix
     *
     * \code {.cpp}
     * A<n,m,MyType> A = 3;
     * A -= 1 ; \\ A is uniformly 2
     * \endcode
     * @tparam S
     * @param rhs
     * @return Array<n, m, T>&
     */
    template <typename S, std::enable_if_t<std::is_convertible<S, T>::value, int> = 0>
    Array<n, m, T> &operator-=(const Array<n, m, S> &rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] -= rhs.c[i];
        }
        return *this;
    }

    // add assign type T and convertible
    template <typename S,
              std::enable_if_t<std::is_convertible<hila::type_plus<T, S>, T>::value, int> = 0>
    Array<n, m, T> &operator+=(const S rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] += rhs;
        }
        return *this;
    }

    // subtract assign type T and convertible
    template <typename S,
              std::enable_if_t<std::is_convertible<hila::type_minus<T, S>, T>::value, int> = 0>
    Array<n, m, T> &operator-=(const S rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] -= rhs;
        }
        return *this;
    }

    /**
     * @brief Multiply assign scalar or array
     * @details Multiplication works element wise
     *
     * Multiply assign operator can be used in the following ways
     *
     * __Multiply assign Array__:
     *
     * \code {.cpp}
     * Array<n,m,MyType> A,B;
     * A = 2;
     * B = 2;
     * A *= B; \\ A is uniformly 4
     * \endcode
     *
     * __Multiply assign scalar__:
     *
     * \code {.cpp}
     * Array<n,m,MyType> A = 1;

     * A *= 2 ; \\ A is uniformly 2
     * \endcode
     * @tparam S
     * @param rhs
     * @return Array<n, m, T>&
     */
    template <typename S,
              std::enable_if_t<std::is_convertible<hila::type_mul<T, S>, T>::value, int> = 0>
    Array<n, m, T> &operator*=(const Array<n, m, S> &rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] *= rhs.c[i];
        }
        return *this;
    }

    /// multiply assign with scalar
    template <typename S,
              std::enable_if_t<std::is_convertible<hila::type_mul<T, S>, T>::value, int> = 0>
    Array<n, m, T> &operator*=(const S rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] *= rhs;
        }
        return *this;
    }

    /**
     * @brief Division assign with array or scalar
     * @details Division works element wise
     *
     * Division assign operator can be used in the following ways
     *
     * __Division assign Array__:
     *
     * \code {.cpp}
     * Array<n,m,MyType> A,B;
     * A = 2;
     * B = 2;
     * A /= B; \\ A is uniformly 1
     * \endcode
     *
     * __Division assign scalar__:
     *
     * \code {.cpp}
     * Array<n,m,MyType> A = 2;

     * A /= 2 ; \\ A is uniformly 1
     * \endcode
     * @tparam S
     * @param rhs
     * @return Array<n, m, T>&
     */
    template <typename S,
              std::enable_if_t<std::is_convertible<hila::type_div<T, S>, T>::value, int> = 0>
    Array<n, m, T> &operator/=(const Array<n, m, S> &rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] /= rhs.c[i];
        }
        return *this;
    }

    // divide assign with scalar
    template <typename S,
              std::enable_if_t<std::is_convertible<hila::type_div<T, S>, T>::value, int> = 0>
    Array<n, m, T> &operator/=(const S rhs) {
        for (int i = 0; i < n * m; i++) {
            c[i] /= rhs;
        }
        return *this;
    }

    /**
     * @brief Returns element wise Complex conjugate of Array
     *
     * @return Array<n, m, T>
     */
    inline Array<n, m, T> conj() const {
        Array<n, m, T> res;
        for (int i = 0; i < n * m; i++) {
            res.c[i] = ::conj(c[i]);
        }
        return res;
    }
    /**
     * @brief Returns real part of Array
     *
     * @return Array<n, m, T>
     */
    inline Array<n, m, hila::scalar_type<T>> real() const {
        Array<n, m, hila::scalar_type<T>> res;
        for (int i = 0; i < m * n; i++) {
            res.c[i] = ::real(c[i]);
        }
        return res;
    }

    /// Returns imaginary part of Array
    inline Array<n, m, hila::scalar_type<T>> imag() const {
        Array<n, m, hila::scalar_type<T>> res;
        for (int i = 0; i < m * n; i++) {
            res.c[i] = ::imag(c[i]);
        }
        return res;
    }

    /// calculate square norm - sum of squared elements
    hila::scalar_type<T> squarenorm() const {
        hila::scalar_type<T> result = 0;
        for (int i = 0; i < n * m; i++) {
            result += ::squarenorm(c[i]);
        }
        return result;
    }

    /**
     * @brief Fill Array with random elements
     *
     * @return Array<n, m, T>&
     */
    Array<n, m, T> &random() out_only {
        for (int i = 0; i < n * m; i++) {
            hila::random(c[i]);
        }
        return *this;
    }

    /**
     * @brief Fill Array with Gaussian random elements
     *
     * @param width
     * @return Array<n, m, T>&
     */
    Array<n, m, T> &gaussian_random(double width = 1.0) out_only {
        for (int i = 0; i < n * m; i++) {
            hila::gaussian_random(c[i], width);
        }
        return *this;
    }

    /// Convert to string for printing
    std::string str() const {
        return this->asMatrix().str();
    }
};

/**
 * @brief Return conjugate Array
 * @details Wrapper around Array::conj
 *
 * @tparam n Number of rows
 * @tparam m Number of columns
 * @tparam T Array element type
 * @param arg Array to be conjugated
 * @return Array<n, m, T>
 */
template <const int n, const int m, typename T>
inline Array<n, m, T> conj(const Array<n, m, T> &arg) {
    return arg.conj();
}
/**
 * @brief Return real part of Array
 * @details Wrapper around Array::real
 *
 * @tparam n Number of rows
 * @tparam m Number of columns
 * @tparam T Array element type
 * @param arg Array to return real part of
 * @return Array<n, m, T>
 */
template <const int n, const int m, typename T>
inline Array<n, m, hila::scalar_type<T>> real(const Array<n, m, T> &arg) {
    return arg.real();
}
/**
 * @brief Return imaginary part of Array
 * @details Wrapper around Array::imag
 *
 * @tparam n Number of rows
 * @tparam m Number of columns
 * @tparam T Array element type
 * @param arg Array to return real part of
 * @return Array<n, m, T>
 */
template <const int n, const int m, typename T>
inline Array<n, m, hila::scalar_type<T>> imag(const Array<n, m, T> &arg) {
    return arg.imag();
}

/**
 * @brief Addition operator
 * @details Defined for the following operations
 *
 * __Array + Array:__
 *
 * __NOTE__: Arrays must share same dimensionality
 *
 * \code {.cpp}
 * Array<n,m,MyType> A, B, C;
 * A = 1;
 * B = 1
 * C = A + B; // C is uniformly 2
 * \endcode
 *
 *
 * __Scalar + Array / Array + Scalar:__
 *
 * __NOTE__: Exact definition exist in overloaded functions that can be viewed in source code.
 * *
 * \code {.cpp}
 * Array<n,m,MyType> A,B;
 * A = 1;
 * B = A + 1; // S is B is uniformly 2
 * \endcode
 *
 * @tparam n Number of rows
 * @tparam m Number of columns
 * @tparam T Array element type
 * @param a
 * @param b
 * @return Array<n, m, T>
 */
template <int n, int m, typename T>
inline Array<n, m, T> operator+(Array<n, m, T> a, const Array<n, m, T> &b) {
    a += b;
    return a;
}

/**
 * @brief Subtraction operator
 * @details Defined for the following operations
 *
 * __Array - Array:__
 *
 * __NOTE__: Arrays must share same dimensionality
 *
 * \code {.cpp}
 * Array<n,m,MyType> A, B, C;
 * A = 2;
 * B = 1
 * C = A - B; // C is uniformly 2
 * \endcode
 *
 *
 * __Scalar - Array / Array - Scalar:__
 *
 * __NOTE__: Exact definition exist in overloaded functions that can be viewed in source code.
 * *
 * \code {.cpp}
 * Array<n,m,MyType> A,B;
 * A = 2;
 * B = A - 1; // S is B is uniformly 2
 * \endcode
 *
 * @tparam n Number of rows
 * @tparam m Number of columns
 * @tparam T Array element type
 * @param a
 * @param b
 * @return Array<n, m, T>
 */
template <int n, int m, typename T>
inline Array<n, m, T> operator-(Array<n, m, T> a, const Array<n, m, T> &b) {
    a -= b;
    return a;
}

// Array + scalar
template <int n, int m, typename T, typename S,
          std::enable_if_t<std::is_convertible<hila::type_plus<T, S>, T>::value, int> = 0>
inline Array<n, m, T> operator+(Array<n, m, T> a, const S b) {
    a += b;
    return a;
}

// scalar + Array
template <int n, int m, typename T, typename S,
          std::enable_if_t<std::is_convertible<hila::type_plus<T, S>, T>::value, int> = 0>
inline Array<n, m, T> operator+(const S b, Array<n, m, T> a) {
    a += b;
    return a;
}

// Array - scalar
template <int n, int m, typename T, typename S,
          std::enable_if_t<std::is_convertible<hila::type_minus<T, S>, T>::value, int> = 0>
Array<n, m, T> operator-(Array<n, m, T> a, const S b) {
    a -= b;
    return a;
}

// scalar - Array
template <int n, int m, typename T, typename S,
          std::enable_if_t<std::is_convertible<hila::type_minus<S, T>, T>::value, int> = 0>
inline Array<n, m, T> operator-(const S b, Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = static_cast<T>(b) - a.c[i];
    return a;
}

/// and Array*Array
template <int n, int m, typename T>
inline Array<n, m, T> operator*(Array<n, m, T> a, const Array<n, m, T> &b) {
    a *= b;
    return a;
}

/// and Array/Array
template <int n, int m, typename T>
inline Array<n, m, T> operator/(Array<n, m, T> a, const Array<n, m, T> &b) {
    a /= b;
    return a;
}

/// Array * scalar
template <int n, int m, typename T, typename S,
          std::enable_if_t<std::is_convertible<hila::type_mul<T, S>, T>::value, int> = 0>
inline Array<n, m, T> operator*(Array<n, m, T> a, const S b) {
    a *= b;
    return a;
}

/// scalar * Array
template <int n, int m, typename T, typename S,
          std::enable_if_t<std::is_convertible<hila::type_mul<T, S>, T>::value, int> = 0>
inline Array<n, m, T> operator*(const S b, Array<n, m, T> a) {
    a *= b;
    return a;
}

/// Array / scalar
template <int n, int m, typename T, typename S,
          std::enable_if_t<std::is_convertible<hila::type_div<T, S>, T>::value, int> = 0>
inline Array<n, m, T> operator/(Array<n, m, T> a, const S b) {
    a /= b;
    return a;
}

/// scalar / Array
template <int n, int m, typename T, typename S,
          std::enable_if_t<std::is_convertible<hila::type_div<S, T>, T>::value, int> = 0>
inline Array<n, m, T> operator/(const S b, Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = b / a.c[i];
    return a;
}

/// Stream operator
template <int n, int m, typename T>
std::ostream &operator<<(std::ostream &strm, const Array<n, m, T> &A) {
    return operator<<(strm, A.asMatrix());
}

namespace hila {

template <int n, int m, typename T>
std::string to_string(const Array<n, m, T> &A, int prec = 8, char separator = ' ') {
    return to_string(A.asMatrix(), prec, separator);
}

template <int n, int m, typename T>
std::string prettyprint(const Array<n, m, T> &A, int prec = 8) {
    return prettyprint(A.asMatrix(), prec);
}

} // namespace hila


/// Norm squared function
template <int n, int m, typename T>
inline hila::scalar_type<T> squarenorm(const Array<n, m, T> &rhs) {
    return rhs.squarenorm();
}


////////////////////////////////////////////////////////////////////////////////
/// Standard arithmetic functions - do element by element
////////////////////////////////////////////////////////////////////////////////

template <int n, int m, typename T>
inline Array<n, m, T> sqrt(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = sqrt(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> cbrt(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = cbrt(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> exp(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = exp(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> log(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = log(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> sin(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = sin(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> cos(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = cos(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> tan(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = tan(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> asin(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = asin(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> acos(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = acos(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> atan(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = atan(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> sinh(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = sinh(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> cosh(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = cosh(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> tanh(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = tanh(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> asinh(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = asinh(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> acosh(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = acosh(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> atanh(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = atanh(a.c[i]);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> pow(Array<n, m, T> a, int b) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = pow(a.c[i], b);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> pow(Array<n, m, T> a, T b) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = pow(a.c[i], b);
    return a;
}

template <int n, int m, typename T>
inline Array<n, m, T> pow(Array<n, m, T> a, const Array<n, m, T> &b) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = pow(a.c[i], b.c[i]);
    return a;
}

template <int n, int m, typename T, std::enable_if_t<hila::is_arithmetic<T>::value, int> = 0>
inline Array<n, m, T> round(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = round(a.c[i]);
    return a;
}

template <int n, int m, typename T, std::enable_if_t<hila::is_arithmetic<T>::value, int> = 0>
inline Array<n, m, T> floor(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = floor(a.c[i]);
    return a;
}

template <int n, int m, typename T, std::enable_if_t<hila::is_arithmetic<T>::value, int> = 0>
inline Array<n, m, T> ceil(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = ceil(a.c[i]);
    return a;
}

template <int n, int m, typename T, std::enable_if_t<hila::is_arithmetic<T>::value, int> = 0>
inline Array<n, m, T> trunc(Array<n, m, T> a) {
    for (int i = 0; i < n * m; i++)
        a.c[i] = trunc(a.c[i]);
    return a;
}


// Cast operators to different number or Complex type
// cast_to<double>(a);
// Cast from number->number, number->Complex, Complex->Complex OK,
//     Complex->number not.

template <typename Ntype, typename T, int n, int m,
          std::enable_if_t<hila::is_arithmetic<T>::value, int> = 0>
Array<n, m, Ntype> cast_to(const Array<n, m, T> &mat) {
    Array<n, m, Ntype> res;
    for (int i = 0; i < n * m; i++)
        res.c[i] = mat.c[i];
    return res;
}

template <typename Ntype, typename T, int n, int m,
          std::enable_if_t<hila::is_complex<T>::value, int> = 0>
Array<n, m, Ntype> cast_to(const Array<n, m, T> &mat) {
    Array<n, m, Ntype> res;
    for (int i = 0; i < n * m; i++)
        res.c[i] = cast_to<Ntype>(mat.c[i]);
    return res;
}


/// Array1d and Array2d are just aliased to Array
template <int n, typename T = double>
using Array1d = Array<n, 1, T>;

template <int n, int m, typename T = double>
using Array2d = Array<n, m, T>;

#endif