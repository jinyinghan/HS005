#ifndef __JZ_MXU_CORE_HPP__
#define __JZ_MXU_CORE_HPP__


#include "mxu_types_c.h"

#ifdef __cplusplus

#ifndef SKIP_INCLUDES
#include <limits.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <complex>
#include <map>
#include <new>
#include <string>
#include <vector>
#include <sstream>
#endif // SKIP_INCLUDES

/*! \namespace mxu
    Namespace where all the C++ JZMXU functionality resides
*/
namespace mxu
{
#undef abs
#undef min
#undef max
#undef Complex




using std::vector;
using std::string;
using std::ptrdiff_t;

template<typename _Tp> class Size_;
template<typename _Tp> class Point_;
template<typename _Tp> class Rect_;
template<typename _Tp, int cn> class Vec;
template<typename _Tp, int m, int n> class Matx;

typedef std::string String;

class Mat;
class SparseMat;
//typedef Mat MatND;

template<typename _Tp> class Mat_;

//template<typename _Tp> class MatCommaInitializer_;

// matrix decomposition types
enum { DECOMP_LU=0, DECOMP_SVD=1, DECOMP_EIG=2, DECOMP_CHOLESKY=3, DECOMP_QR=4, DECOMP_NORMAL=16 };
enum { NORM_INF=1, NORM_L1=2, NORM_L2=4, NORM_L2SQR=5, NORM_HAMMING=6, NORM_HAMMING2=7, NORM_TYPE_MASK=7, NORM_RELATIVE=8, NORM_MINMAX=32 };
enum { CMP_EQ=0, CMP_GT=1, CMP_GE=2, CMP_LT=3, CMP_LE=4, CMP_NE=5 };
enum { GEMM_1_T=1, GEMM_2_T=2, GEMM_3_T=4 };
enum { DFT_INVERSE=1, DFT_SCALE=2, DFT_ROWS=4, DFT_COMPLEX_OUTPUT=16, DFT_REAL_OUTPUT=32,
    DCT_INVERSE = DFT_INVERSE, DCT_ROWS=DFT_ROWS };

#if defined __GNUC__
#define MXU_Func __func__
#elif defined _MSC_VER
#define MXU_Func __FUNCTION__
#else
#define MXU_Func ""
#endif


MXU_EXPORTS void* fastMalloc(size_t bufSize);


MXU_EXPORTS void fastFree(void* ptr);

template<typename _Tp> static inline _Tp* allocate(size_t n)
{
    return new _Tp[n];
}

template<typename _Tp> static inline void deallocate(_Tp* ptr, size_t)
{
    delete[] ptr;
}


template<typename _Tp> static inline _Tp* alignPtr(_Tp* ptr, int n=(int)sizeof(_Tp))
{
    return (_Tp*)(((size_t)ptr + n-1) & -n);
}


static inline size_t alignSize(size_t sz, int n)
{
    assert((n & (n - 1)) == 0); // n is a power of 2
    return (sz + n-1) & -n;
}



template<typename _Tp> class Allocator
{
public:
    typedef _Tp value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template<typename U> class rebind { typedef Allocator<U> other; };

    explicit Allocator() {}
    ~Allocator() {}
    explicit Allocator(Allocator const&) {}
    template<typename U>
    explicit Allocator(Allocator<U> const&) {}

    // address
    pointer address(reference r) { return &r; }
    const_pointer address(const_reference r) { return &r; }

    pointer allocate(size_type count, const void* =0)
    { return reinterpret_cast<pointer>(fastMalloc(count * sizeof (_Tp))); }

    void deallocate(pointer p, size_type) {fastFree(p); }

    size_type max_size() const
    { return max(static_cast<_Tp>(-1)/sizeof(_Tp), 1); }

    void construct(pointer p, const _Tp& v) { new(static_cast<void*>(p)) _Tp(v); }
    void destroy(pointer p) { p->~_Tp(); }
};

/////////////////////// Vec (used as element of multi-channel images /////////////////////


template<typename _Tp> class DataDepth {};

template<> class DataDepth<bool> { public: enum { value = MXU_8U, fmt=(int)'u' }; };
template<> class DataDepth<uchar> { public: enum { value = MXU_8U, fmt=(int)'u' }; };
template<> class DataDepth<schar> { public: enum { value = MXU_8S, fmt=(int)'c' }; };
template<> class DataDepth<char> { public: enum { value = MXU_8S, fmt=(int)'c' }; };
template<> class DataDepth<ushort> { public: enum { value = MXU_16U, fmt=(int)'w' }; };
template<> class DataDepth<short> { public: enum { value = MXU_16S, fmt=(int)'s' }; };
template<> class DataDepth<int> { public: enum { value = MXU_32S, fmt=(int)'i' }; };
// this is temporary solution to support 32-bit unsigned integers
template<> class DataDepth<unsigned> { public: enum { value = MXU_32S, fmt=(int)'i' }; };
template<> class DataDepth<float> { public: enum { value = MXU_32F, fmt=(int)'f' }; };
template<> class DataDepth<double> { public: enum { value = MXU_64F, fmt=(int)'d' }; };
template<typename _Tp> class DataDepth<_Tp*> { public: enum { value = MXU_USRTYPE1, fmt=(int)'r' }; };


////////////////////////////// Small Matrix ///////////////////////////

struct MXU_EXPORTS Matx_AddOp {};
struct MXU_EXPORTS Matx_SubOp {};
struct MXU_EXPORTS Matx_ScaleOp {};
struct MXU_EXPORTS Matx_MulOp {};
struct MXU_EXPORTS Matx_MatMulOp {};
struct MXU_EXPORTS Matx_TOp {};

template<typename _Tp, int m, int n> class Matx
{
public:
    typedef _Tp value_type;
    typedef Matx<_Tp, (m < n ? m : n), 1> diag_type;
    typedef Matx<_Tp, m, n> mat_type;
    enum { depth = DataDepth<_Tp>::value, rows = m, cols = n, channels = rows*cols,
           type = MXU_MAKETYPE(depth, channels) };

    //! default constructor
    Matx();

    Matx(_Tp v0); //!< 1x1 matrix
    Matx(_Tp v0, _Tp v1); //!< 1x2 or 2x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2); //!< 1x3 or 3x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3); //!< 1x4, 2x2 or 4x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4); //!< 1x5 or 5x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5); //!< 1x6, 2x3, 3x2 or 6x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6); //!< 1x7 or 7x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7); //!< 1x8, 2x4, 4x2 or 8x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8); //!< 1x9, 3x3 or 9x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8, _Tp v9); //!< 1x10, 2x5 or 5x2 or 10x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3,
         _Tp v4, _Tp v5, _Tp v6, _Tp v7,
         _Tp v8, _Tp v9, _Tp v10, _Tp v11); //!< 1x12, 2x6, 3x4, 4x3, 6x2 or 12x1 matrix
    Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3,
         _Tp v4, _Tp v5, _Tp v6, _Tp v7,
         _Tp v8, _Tp v9, _Tp v10, _Tp v11,
         _Tp v12, _Tp v13, _Tp v14, _Tp v15); //!< 1x16, 4x4 or 16x1 matrix
    explicit Matx(const _Tp* vals); //!< initialize from a plain array

    static Matx all(_Tp alpha);
    static Matx zeros();
    static Matx ones();
    static Matx eye();
    static Matx diag(const diag_type& d);
    //static Matx randu(_Tp a, _Tp b);
    //static Matx randn(_Tp a, _Tp b);

    //! dot product computed with the default precision
    _Tp dot(const Matx<_Tp, m, n>& v) const;

    //! dot product computed in double-precision arithmetics
    double ddot(const Matx<_Tp, m, n>& v) const;

    //! conversion to another data type
    template<typename T2> operator Matx<T2, m, n>() const;

    //! change the matrix shape
    template<int m1, int n1> Matx<_Tp, m1, n1> reshape() const;

    //! extract part of the matrix
    template<int m1, int n1> Matx<_Tp, m1, n1> get_minor(int i, int j) const;

    //! extract the matrix row
    Matx<_Tp, 1, n> row(int i) const;

    //! extract the matrix column
    Matx<_Tp, m, 1> col(int i) const;

    //! extract the matrix diagonal
    diag_type diag() const;

    //! transpose the matrix
    Matx<_Tp, n, m> t() const;

    //! invert matrix the matrix
    Matx<_Tp, n, m> inv(int method=DECOMP_LU) const;

    //! solve linear system
    template<int l> Matx<_Tp, n, l> solve(const Matx<_Tp, m, l>& rhs, int flags=DECOMP_LU) const;
    Vec<_Tp, n> solve(const Vec<_Tp, m>& rhs, int method) const;

    //! multiply two matrices element-wise
    Matx<_Tp, m, n> mul(const Matx<_Tp, m, n>& a) const;

    //! element access
    const _Tp& operator ()(int i, int j) const;
    _Tp& operator ()(int i, int j);

    //! 1D element access
    const _Tp& operator ()(int i) const;
    _Tp& operator ()(int i);

    Matx(const Matx<_Tp, m, n>& a, const Matx<_Tp, m, n>& b, Matx_AddOp);
    Matx(const Matx<_Tp, m, n>& a, const Matx<_Tp, m, n>& b, Matx_SubOp);
    template<typename _T2> Matx(const Matx<_Tp, m, n>& a, _T2 alpha, Matx_ScaleOp);
    Matx(const Matx<_Tp, m, n>& a, const Matx<_Tp, m, n>& b, Matx_MulOp);
    template<int l> Matx(const Matx<_Tp, m, l>& a, const Matx<_Tp, l, n>& b, Matx_MatMulOp);
    Matx(const Matx<_Tp, n, m>& a, Matx_TOp);

    _Tp val[m*n]; //< matrix elements
};


typedef Matx<float, 1, 2> Matx12f;
typedef Matx<double, 1, 2> Matx12d;
typedef Matx<float, 1, 3> Matx13f;
typedef Matx<double, 1, 3> Matx13d;
typedef Matx<float, 1, 4> Matx14f;
typedef Matx<double, 1, 4> Matx14d;
typedef Matx<float, 1, 6> Matx16f;
typedef Matx<double, 1, 6> Matx16d;

typedef Matx<float, 2, 1> Matx21f;
typedef Matx<double, 2, 1> Matx21d;
typedef Matx<float, 3, 1> Matx31f;
typedef Matx<double, 3, 1> Matx31d;
typedef Matx<float, 4, 1> Matx41f;
typedef Matx<double, 4, 1> Matx41d;
typedef Matx<float, 6, 1> Matx61f;
typedef Matx<double, 6, 1> Matx61d;

typedef Matx<float, 2, 2> Matx22f;
typedef Matx<double, 2, 2> Matx22d;
typedef Matx<float, 2, 3> Matx23f;
typedef Matx<double, 2, 3> Matx23d;
typedef Matx<float, 3, 2> Matx32f;
typedef Matx<double, 3, 2> Matx32d;

typedef Matx<float, 3, 3> Matx33f;
typedef Matx<double, 3, 3> Matx33d;

typedef Matx<float, 3, 4> Matx34f;
typedef Matx<double, 3, 4> Matx34d;
typedef Matx<float, 4, 3> Matx43f;
typedef Matx<double, 4, 3> Matx43d;

typedef Matx<float, 4, 4> Matx44f;
typedef Matx<double, 4, 4> Matx44d;
typedef Matx<float, 6, 6> Matx66f;
typedef Matx<double, 6, 6> Matx66d;


/*!
  A short numerical vector.
*/
template<typename _Tp, int cn> class Vec : public Matx<_Tp, cn, 1>
{
public:
    typedef _Tp value_type;
    enum { depth = DataDepth<_Tp>::value, channels = cn, type = MXU_MAKETYPE(depth, channels) };

    //! default constructor
    Vec();

    Vec(_Tp v0); //!< 1-element vector constructor
    Vec(_Tp v0, _Tp v1); //!< 2-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2); //!< 3-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3); //!< 4-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4); //!< 5-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5); //!< 6-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6); //!< 7-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7); //!< 8-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8); //!< 9-element vector constructor
    Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8, _Tp v9); //!< 10-element vector constructor
    explicit Vec(const _Tp* values);

    Vec(const Vec<_Tp, cn>& v);

    static Vec all(_Tp alpha);

    //! per-element multiplication
    Vec mul(const Vec<_Tp, cn>& v) const;

    //! conjugation (makes sense for complex numbers and quaternions)
    Vec conj() const;

    /*!
      cross product of the two 3D vectors.

      For other dimensionalities the exception is raised
    */
    Vec cross(const Vec& v) const;
    //! conversion to another data type
    template<typename T2> operator Vec<T2, cn>() const;
    //! conversion to 4-element MxuScalar.
    operator MxuScalar() const;

    /*! element access */
    const _Tp& operator [](int i) const;
    _Tp& operator[](int i);
    const _Tp& operator ()(int i) const;
    _Tp& operator ()(int i);

    Vec(const Matx<_Tp, cn, 1>& a, const Matx<_Tp, cn, 1>& b, Matx_AddOp);
    Vec(const Matx<_Tp, cn, 1>& a, const Matx<_Tp, cn, 1>& b, Matx_SubOp);
    template<typename _T2> Vec(const Matx<_Tp, cn, 1>& a, _T2 alpha, Matx_ScaleOp);
};


/* \typedef

   Shorter aliases for the most popular specializations of Vec<T,n>
*/
typedef Vec<uchar, 2> Vec2b;
typedef Vec<uchar, 3> Vec3b;
typedef Vec<uchar, 4> Vec4b;

typedef Vec<short, 2> Vec2s;
typedef Vec<short, 3> Vec3s;
typedef Vec<short, 4> Vec4s;

typedef Vec<ushort, 2> Vec2w;
typedef Vec<ushort, 3> Vec3w;
typedef Vec<ushort, 4> Vec4w;

typedef Vec<int, 2> Vec2i;
typedef Vec<int, 3> Vec3i;
typedef Vec<int, 4> Vec4i;
typedef Vec<int, 6> Vec6i;
typedef Vec<int, 8> Vec8i;

typedef Vec<float, 2> Vec2f;
typedef Vec<float, 3> Vec3f;
typedef Vec<float, 4> Vec4f;
typedef Vec<float, 6> Vec6f;

typedef Vec<double, 2> Vec2d;
typedef Vec<double, 3> Vec3d;
typedef Vec<double, 4> Vec4d;
typedef Vec<double, 6> Vec6d;

//////////////////////////////// Complex //////////////////////////////

/*!
  A complex number class.

  The template class is similar and compatible with std::complex, however it provides slightly
  more convenient access to the real and imaginary parts using through the simple field access, as opposite
  to std::complex::real() and std::complex::imag().
*/
template<typename _Tp> class Complex
{
public:

    //! constructors
    Complex();
    Complex( _Tp _re, _Tp _im=0 );
    Complex( const std::complex<_Tp>& c );

    //! conversion to another data type
    template<typename T2> operator Complex<T2>() const;
    //! conjugation
    Complex conj() const;
    //! conversion to std::complex
    operator std::complex<_Tp>() const;

    _Tp re, im; //< the real and the imaginary parts
};


/*!
  \typedef
*/
typedef Complex<float> Complexf;
typedef Complex<double> Complexd;


//////////////////////////////// Point_ ////////////////////////////////

/*!
  template 2D point class.
*/
template<typename _Tp> class Point_
{
public:
    typedef _Tp value_type;

    // various constructors
    Point_();
    Point_(_Tp _x, _Tp _y);
    Point_(const Point_& pt);
    Point_(const MxuPoint& pt);
    Point_(const MxuPoint2D32f& pt);
    Point_(const Size_<_Tp>& sz);
    Point_(const Vec<_Tp, 2>& v);

    Point_& operator = (const Point_& pt);
    //! conversion to another data type
    template<typename _Tp2> operator Point_<_Tp2>() const;

    //! conversion to the old-style C structures
    operator MxuPoint() const;
    operator MxuPoint2D32f() const;
    operator Vec<_Tp, 2>() const;

    //! dot product
    _Tp dot(const Point_& pt) const;
    //! dot product computed in double-precision arithmetics
    double ddot(const Point_& pt) const;
    //! cross-product
    double cross(const Point_& pt) const;
    //! checks whether the point is inside the specified rectangle
    bool inside(const Rect_<_Tp>& r) const;

    _Tp x, y; //< the point coordinates
};

/*!
  template 3D point class.
*/
template<typename _Tp> class Point3_
{
public:
    typedef _Tp value_type;

    // various constructors
    Point3_();
    Point3_(_Tp _x, _Tp _y, _Tp _z);
    Point3_(const Point3_& pt);
    explicit Point3_(const Point_<_Tp>& pt);
    Point3_(const MxuPoint3D32f& pt);
    Point3_(const Vec<_Tp, 3>& v);

    Point3_& operator = (const Point3_& pt);
    //! conversion to another data type
    template<typename _Tp2> operator Point3_<_Tp2>() const;
    //! conversion to the old-style MxuPoint...
    operator MxuPoint3D32f() const;
    //! conversion to mxu::Vec<>
    operator Vec<_Tp, 3>() const;

    //! dot product
    _Tp dot(const Point3_& pt) const;
    //! dot product computed in double-precision arithmetics
    double ddot(const Point3_& pt) const;
    //! cross product of the 2 3D points
    Point3_ cross(const Point3_& pt) const;

    _Tp x, y, z; //< the point coordinates
};

//////////////////////////////// Size_ ////////////////////////////////

/*!
  The 2D size class
*/
template<typename _Tp> class Size_
{
public:
    typedef _Tp value_type;

    //! various constructors
    Size_();
    Size_(_Tp _width, _Tp _height);
    Size_(const Size_& sz);
    Size_(const MxuSize& sz);
    Size_(const MxuSize2D32f& sz);
    Size_(const Point_<_Tp>& pt);

    Size_& operator = (const Size_& sz);
    //! the area (width*height)
    _Tp area() const;

    //! conversion of another data type.
    template<typename _Tp2> operator Size_<_Tp2>() const;

    //! conversion to the old-style JZMXU types
    operator MxuSize() const;
    operator MxuSize2D32f() const;

    _Tp width, height; // the width and the height
};

//////////////////////////////// Rect_ ////////////////////////////////

/*!
  The 2D up-right rectangle class
*/
template<typename _Tp> class Rect_
{
public:
    typedef _Tp value_type;

    //! various constructors
    Rect_();
    Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
    Rect_(const Rect_& r);
    Rect_(const MxuRect& r);
    Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz);
    Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2);

    Rect_& operator = ( const Rect_& r );
    //! the top-left corner
    Point_<_Tp> tl() const;
    //! the bottom-right corner
    Point_<_Tp> br() const;

    //! size (width, height) of the rectangle
    Size_<_Tp> size() const;
    //! area (width*height) of the rectangle
    _Tp area() const;

    //! conversion to another data type
    template<typename _Tp2> operator Rect_<_Tp2>() const;
    //! conversion to the old-style MxuRect
    operator MxuRect() const;

    //! checks whether the rectangle contains the point
    bool contains(const Point_<_Tp>& pt) const;

    _Tp x, y, width, height; //< the top-left corner, as well as width and height of the rectangle
};


/*!
  \typedef

  shorter aliases for the most popular mxu::Point_<>, mxu::Size_<> and mxu::Rect_<> specializations
*/
typedef Point_<int> Point2i;
typedef Point2i Point;
typedef Size_<int> Size2i;
typedef Size_<double> Size2d;
typedef Size2i Size;
typedef Rect_<int> Rect;
typedef Rect_<double> Rect2d;
typedef Rect_<float> Rect2f;   //add by klyu
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
typedef Size_<float> Size2f;
typedef Point3_<int> Point3i;
typedef Point3_<float> Point3f;
typedef Point3_<double> Point3d;

/*!
  The rotated 2D rectangle.
*/
class MXU_EXPORTS RotatedRect
{
public:
    //! various constructors
    RotatedRect();
    RotatedRect(const Point2f& center, const Size2f& size, float angle);
    RotatedRect(const MxuBox2D& box);

    //! returns 4 vertices of the rectangle
    void points(Point2f pts[]) const;
    //! returns the minimal up-right rectangle containing the rotated rectangle
    Rect boundingRect() const;
    //! conversion to the old-style MxuBox2D structure
    operator MxuBox2D() const;

    Point2f center; //< the rectangle mass center
    Size2f size;    //< width and height of the rectangle
    float angle;    //< the rotation angle. When the angle is 0, 90, 180, 270 etc., the rectangle becomes an up-right rectangle.
};

//////////////////////////////// Scalar_ ///////////////////////////////

/*!
   The template scalar class.

   This is partially specialized mxu::Vec class with the number of elements = 4, i.e. a short vector of four elements.
   Normally, mxu::Scalar ~ mxu::Scalar_<double> is used.
*/
template<typename _Tp> class Scalar_ : public Vec<_Tp, 4>
{
public:
    //! various constructors
    Scalar_();
    Scalar_(_Tp v0, _Tp v1, _Tp v2=0, _Tp v3=0);
    Scalar_(const MxuScalar& s);
    Scalar_(_Tp v0);

    //! returns a scalar with all elements set to v0
    static Scalar_<_Tp> all(_Tp v0);
    //! conversion to the old-style MxuScalar
    operator MxuScalar() const;

    //! conversion to another data type
    template<typename T2> operator Scalar_<T2>() const;

    //! per-element product
    Scalar_<_Tp> mul(const Scalar_<_Tp>& t, double scale=1 ) const;

    // returns (v0, -v1, -v2, -v3)
    Scalar_<_Tp> conj() const;

    // returns true iff v1 == v2 == v3 == 0
    bool isReal() const;
};

typedef Scalar_<double> Scalar;

MXU_EXPORTS void scalarToRawData(const Scalar& s, void* buf, int type, int unroll_to=0);

//////////////////////////////// Range /////////////////////////////////

/*!
   The 2D range class

   This is the class used to specify a continuous subsequence, i.e. part of a contour, or a column span in a matrix.
*/
class MXU_EXPORTS Range
{
public:
    Range();
    Range(int _start, int _end);
    Range(const MxuSlice& slice);
    int size() const;
    bool empty() const;
    static Range all();
    operator MxuSlice() const;

    int start, end;
};


/////////////////////////////// DataType ////////////////////////////////

/*!
   Informative template class for JZMXU "scalars".

   The class is specialized for each primitive numerical type supported by JZMXU (such as unsigned char or float),
   as well as for more complex types, like mxu::Complex<>, std::complex<>, mxu::Vec<> etc.
   The common property of all such types (called "scalars", do not confuse it with mxu::Scalar_)
   is that each of them is basically a tuple of numbers of the same type. Each "scalar" can be represented
   by the depth id (MXU_8U ... MXU_64F) and the number of channels.
   JZMXU matrices, 2D or nD, dense or sparse, can store "scalars",
   as long as the number of channels does not exceed MXU_CN_MAX.
*/
template<typename _Tp> class DataType
{
public:
    typedef _Tp value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 1, depth = -1, channels = 1, fmt=0,
        type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<bool>
{
public:
    typedef bool value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<uchar>
{
public:
    typedef uchar value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<schar>
{
public:
    typedef schar value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<char>
{
public:
    typedef schar value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<ushort>
{
public:
    typedef ushort value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<short>
{
public:
    typedef short value_type;
    typedef int work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<int>
{
public:
    typedef int value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<float>
{
public:
    typedef float value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<> class DataType<double>
{
public:
    typedef double value_type;
    typedef value_type work_type;
    typedef value_type channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<typename _Tp, int m, int n> class DataType<Matx<_Tp, m, n> >
{
public:
    typedef Matx<_Tp, m, n> value_type;
    typedef Matx<typename DataType<_Tp>::work_type, m, n> work_type;
    typedef _Tp channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = m*n,
        fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
        type = MXU_MAKETYPE(depth, channels) };
};

template<typename _Tp, int cn> class DataType<Vec<_Tp, cn> >
{
public:
    typedef Vec<_Tp, cn> value_type;
    typedef Vec<typename DataType<_Tp>::work_type, cn> work_type;
    typedef _Tp channel_type;
    typedef value_type vec_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = cn,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
};

template<typename _Tp> class DataType<std::complex<_Tp> >
{
public:
    typedef std::complex<_Tp> value_type;
    typedef value_type work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Complex<_Tp> >
{
public:
    typedef Complex<_Tp> value_type;
    typedef value_type work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Point_<_Tp> >
{
public:
    typedef Point_<_Tp> value_type;
    typedef Point_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Point3_<_Tp> >
{
public:
    typedef Point3_<_Tp> value_type;
    typedef Point3_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 3,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Size_<_Tp> >
{
public:
    typedef Size_<_Tp> value_type;
    typedef Size_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Rect_<_Tp> >
{
public:
    typedef Rect_<_Tp> value_type;
    typedef Rect_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 4,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<typename _Tp> class DataType<Scalar_<_Tp> >
{
public:
    typedef Scalar_<_Tp> value_type;
    typedef Scalar_<typename DataType<_Tp>::work_type> work_type;
    typedef _Tp channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 4,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

template<> class DataType<Range>
{
public:
    typedef Range value_type;
    typedef value_type work_type;
    typedef int channel_type;
    enum { generic_type = 0, depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = MXU_MAKETYPE(depth, channels) };
    typedef Vec<channel_type, channels> vec_type;
};

//////////////////// generic_type ref-counting pointer class for C/C++ objects ////////////////////////

/*!
  Smart pointer to dynamically allocated objects.
*/
template<typename _Tp> class Ptr
{
public:
    //! empty constructor
    Ptr();
    //! take ownership of the pointer. The associated reference counter is allocated and set to 1
    Ptr(_Tp* _obj);
    //! calls release()
    ~Ptr();
    //! copy constructor. Copies the members and calls addref()
    Ptr(const Ptr& ptr);
    template<typename _Tp2> Ptr(const Ptr<_Tp2>& ptr);
    //! copy operator. Calls ptr.addref() and release() before copying the members
    Ptr& operator = (const Ptr& ptr);
    //! increments the reference counter
    void addref();
    //! decrements the reference counter. If it reaches 0, delete_obj() is called
    void release();
    //! deletes the object. Override if needed
    void delete_obj();
    //! returns true iff obj==NULL
    bool empty() const;

    //! cast pointer to another type
    template<typename _Tp2> Ptr<_Tp2> ptr();
    template<typename _Tp2> const Ptr<_Tp2> ptr() const;

    //! helper operators making "Ptr<T> ptr" use very similar to "T* ptr".
    _Tp* operator -> ();
    const _Tp* operator -> () const;

    operator _Tp* ();
    operator const _Tp*() const;

    _Tp* obj; //< the object pointer.
    int* refcount; //< the associated reference counter
};

typedef Ptr<MxuMemStorage> MemStorage;
//////////////////////// Input/Output Array Arguments /////////////////////////////////

/*!
 Proxy datatype for passing Mat's and vector<>'s as input parameters
 */
class MXU_EXPORTS _InputArray
{
public:
    enum {
        KIND_SHIFT = 16,
        FIXED_TYPE = 0x8000 << KIND_SHIFT,
        FIXED_SIZE = 0x4000 << KIND_SHIFT,
        KIND_MASK = ~(FIXED_TYPE|FIXED_SIZE) - (1 << KIND_SHIFT) + 1,

        NONE              = 0 << KIND_SHIFT,
        MAT               = 1 << KIND_SHIFT,
        MATX              = 2 << KIND_SHIFT,
        STD_VECTOR        = 3 << KIND_SHIFT,
        STD_VECTOR_VECTOR = 4 << KIND_SHIFT,
        STD_VECTOR_MAT    = 5 << KIND_SHIFT,
        EXPR              = 6 << KIND_SHIFT,
        OPENGL_BUFFER     = 7 << KIND_SHIFT,
        OPENGL_TEXTURE    = 8 << KIND_SHIFT,
        GPU_MAT           = 9 << KIND_SHIFT,
        OCL_MAT           =10 << KIND_SHIFT,
        UMAT              =10 << KIND_SHIFT,
        STD_VECTOR_UMAT   =11 << KIND_SHIFT,
        STD_BOOL_VECTOR   =12 << KIND_SHIFT
    };
    _InputArray();

    _InputArray(const Mat& m);
    //_InputArray(const MatExpr& expr);
    template<typename _Tp> _InputArray(const _Tp* vec, int n);
    template<typename _Tp> _InputArray(const vector<_Tp>& vec);
    template<typename _Tp> _InputArray(const vector<vector<_Tp> >& vec);
    _InputArray(const vector<Mat>& vec);
    template<typename _Tp> _InputArray(const vector<Mat_<_Tp> >& vec);
    template<typename _Tp> _InputArray(const Mat_<_Tp>& m);
    template<typename _Tp, int m, int n> _InputArray(const Matx<_Tp, m, n>& matx);
    _InputArray(const Scalar& s);
    _InputArray(const double& val);
    // < Deprecated
    //_InputArray(const GlBuffer& buf);
    //_InputArray(const GlTexture& tex);
    // >
    //_InputArray(const gpu::GpuMat& d_mat);
    //_InputArray(const ogl::Buffer& buf);
    //_InputArray(const ogl::Texture2D& tex);

    virtual Mat getMat(int i=-1) const;
    virtual void getMatVector(vector<Mat>& mv) const;
    // < Deprecated
    //virtual GlBuffer getGlBuffer() const;
    //virtual GlTexture getGlTexture() const;
    // >
    //virtual gpu::GpuMat getGpuMat() const;
    /*virtual*/ //ogl::Buffer getOGlBuffer() const;
    /*virtual*/ //ogl::Texture2D getOGlTexture2D() const;

    virtual int kind() const;
    virtual Size size(int i=-1) const;
    virtual size_t total(int i=-1) const;
    virtual int type(int i=-1) const;
    virtual int depth(int i=-1) const;
    virtual int channels(int i=-1) const;
    virtual bool empty() const;

    int flags;
    void* obj;
    Size sz;
};


enum
{
    DEPTH_MASK_8U = 1 << MXU_8U,
    DEPTH_MASK_8S = 1 << MXU_8S,
    DEPTH_MASK_16U = 1 << MXU_16U,
    DEPTH_MASK_16S = 1 << MXU_16S,
    DEPTH_MASK_32S = 1 << MXU_32S,
    DEPTH_MASK_32F = 1 << MXU_32F,
    DEPTH_MASK_64F = 1 << MXU_64F,
    DEPTH_MASK_ALL = (DEPTH_MASK_64F<<1)-1,
    DEPTH_MASK_ALL_BUT_8S = DEPTH_MASK_ALL & ~DEPTH_MASK_8S,
    DEPTH_MASK_FLT = DEPTH_MASK_32F + DEPTH_MASK_64F
};


/*!
 Proxy datatype for passing Mat's and vector<>'s as input parameters
 */
class MXU_EXPORTS _OutputArray : public _InputArray
{
public:
    _OutputArray();

    _OutputArray(Mat& m);
    template<typename _Tp> _OutputArray(vector<_Tp>& vec);
    template<typename _Tp> _OutputArray(vector<vector<_Tp> >& vec);
    _OutputArray(vector<Mat>& vec);
    template<typename _Tp> _OutputArray(vector<Mat_<_Tp> >& vec);
    template<typename _Tp> _OutputArray(Mat_<_Tp>& m);
    template<typename _Tp, int m, int n> _OutputArray(Matx<_Tp, m, n>& matx);
    template<typename _Tp> _OutputArray(_Tp* vec, int n);
   // _OutputArray(gpu::GpuMat& d_mat);
    //_OutputArray(ogl::Buffer& buf);
    //_OutputArray(ogl::Texture2D& tex);

    _OutputArray(const Mat& m);
    template<typename _Tp> _OutputArray(const vector<_Tp>& vec);
    template<typename _Tp> _OutputArray(const vector<vector<_Tp> >& vec);
    _OutputArray(const vector<Mat>& vec);
    template<typename _Tp> _OutputArray(const vector<Mat_<_Tp> >& vec);
    template<typename _Tp> _OutputArray(const Mat_<_Tp>& m);
    template<typename _Tp, int m, int n> _OutputArray(const Matx<_Tp, m, n>& matx);
    template<typename _Tp> _OutputArray(const _Tp* vec, int n);
    //_OutputArray(const gpu::GpuMat& d_mat);
    //_OutputArray(const ogl::Buffer& buf);
    //_OutputArray(const ogl::Texture2D& tex);

    virtual bool fixedSize() const;
    virtual bool fixedType() const;
    virtual bool needed() const;
    virtual Mat& getMatRef(int i=-1) const;
    /*virtual*/ //gpu::GpuMat& getGpuMatRef() const;
    /*virtual*/ //ogl::Buffer& getOGlBufferRef() const;
    /*virtual*/ //ogl::Texture2D& getOGlTexture2DRef() const;
    virtual void create(Size sz, int type, int i=-1, bool allowTransposed=false, int fixedDepthMask=0) const;
    virtual void create(int rows, int cols, int type, int i=-1, bool allowTransposed=false, int fixedDepthMask=0) const;
    virtual void create(int dims, const int* size, int type, int i=-1, bool allowTransposed=false, int fixedDepthMask=0) const;
    virtual void release() const;
    virtual void clear() const;


};

typedef const _InputArray& InputArray;
typedef InputArray InputArrayOfArrays;
typedef const _OutputArray& OutputArray;
typedef OutputArray OutputArrayOfArrays;
typedef OutputArray InputOutputArray;
typedef OutputArray InputOutputArrayOfArrays;

MXU_EXPORTS OutputArray noArray();










//MXU_EXPORTS_W void add(const Mat& a, const Mat& b, Mat& c);
//MXU_EXPORTS_W void subtract(const Mat& a, const Mat& b, Mat& c);
MXU_EXPORTS_W void gemm(const Mat& a, const Mat& b, Mat& c);

//! implements generalized matrix product algorithm GEMM from BLAS
MXU_EXPORTS_W void gemm(InputArray src1, InputArray src2, double alpha,
                       InputArray src3, double gamma, OutputArray dst, int flags=0);

////! multiplies matrix by its transposition from the left or from the right
MXU_EXPORTS_W void mulTransposed( InputArray src, OutputArray dst, bool aTa,
                                 InputArray delta=noArray(),
                                 double scale=1, int dtype=-1 );
//! transposes the matrix
MXU_EXPORTS_W void transpose(InputArray src, OutputArray dst);

MXU_EXPORTS_W void divide(const Mat& a, const Mat& b, Mat& c);

//! computes weighted sum of two arrays (dst = alpha*src1 + beta*src2 + gamma)
MXU_EXPORTS_W void addWeighted(InputArray src1, double alpha, InputArray src2,
                              double beta, double gamma, OutputArray dst, int dtype=-1);






/////////////////////////////////////// Mat ///////////////////////////////////////////

enum { MAGIC_MASK=0xFFFF0000, TYPE_MASK=0x00000FFF, DEPTH_MASK=7 };

static inline size_t getElemSize(int type) { return MXU_ELEM_SIZE(type); }

/*!
   Custom array allocator

*/
class MXU_EXPORTS MatAllocator
{
public:
    MatAllocator() {}
    virtual ~MatAllocator() {}
    virtual void allocate(int dims, const int* sizes, int type, int*& refcount,
                          uchar*& datastart, uchar*& data, size_t* step) = 0;
    virtual void deallocate(int* refcount, uchar* datastart, uchar* data) = 0;
};

class MXU_EXPORTS Mat
{
public:
    //! default constructor
    Mat();
    //! constructs 2D matrix of the specified size and type
    // (_type is MXU_8UC1, MXU_64FC3, MXU_32SC(12) etc.)
    Mat(int rows, int cols, int type);
    Mat(Size size, int type);
    //! constucts 2D matrix and fills it with the specified value _s.
    Mat(int rows, int cols, int type, const Scalar& s);
    Mat(Size size, int type, const Scalar& s);

    //! constructs n-dimensional matrix
    Mat(int ndims, const int* sizes, int type);
    Mat(int ndims, const int* sizes, int type, const Scalar& s);

    //! copy constructor
    Mat(const Mat& m);
    //! constructor for matrix headers pointing to user-allocated data
    Mat(int rows, int cols, int type, void* data, size_t step=AUTO_STEP);
    Mat(Size size, int type, void* data, size_t step=AUTO_STEP);
    Mat(int ndims, const int* sizes, int type, void* data, const size_t* steps=0);

    //! creates a matrix header for a part of the bigger matrix
    Mat(const Mat& m, const Range& rowRange, const Range& colRange=Range::all());
    Mat(const Mat& m, const Rect& roi);
    Mat(const Mat& m, const Range* ranges);

    /** @overload
    @param vec STL vector whose elements form the matrix. The matrix has a single column and the number
    of rows equal to the number of vector elements. Type of the matrix matches the type of vector
    elements. The constructor can handle arbitrary types, for which there is a properly declared
    DataType . This means that the vector elements must be primitive numbers or uni-type numerical
    tuples of numbers. Mixed-type structures are not supported. The corresponding constructor is
    explicit. Since STL vectors are not automatically converted to Mat instances, you should write
    Mat(vec) explicitly. Unless you copy the data into the matrix ( copyData=true ), no new elements
    will be added to the vector because it can potentially yield vector data reallocation, and, thus,
    the matrix data pointer will be invalid.
    @param copyData Flag to specify whether the underlying data of the STL vector should be copied
    to (true) or shared with (false) the newly constructed matrix. When the data is copied, the
    allocated buffer is managed using Mat reference counting mechanism. While the data is shared,
    the reference counter is NULL, and you should not deallocate the data until the matrix is not
    destructed.
    */
    template<typename _Tp> explicit Mat(const std::vector<_Tp>& vec, bool copyData=false);

    //! converts old-style MxuMat to the new matrix; the data is not copied by default
    Mat(const MxuMat* m, bool copyData=false);

    /*! builds matrix from std::vector with or without copying the data
    template<typename _Tp> explicit Mat(const vector<_Tp>& vec, bool copyData=false);
    //! builds matrix from mxu::Vec; the data is copied by default
    template<typename _Tp, int n> explicit Mat(const Vec<_Tp, n>& vec, bool copyData=true);
    //! builds matrix from mxu::Matx; the data is copied by default
    template<typename _Tp, int m, int n> explicit Mat(const Matx<_Tp, m, n>& mtx, bool copyData=true);
    //! builds matrix from a 2D point
    template<typename _Tp> explicit Mat(const Point_<_Tp>& pt, bool copyData=true);
    //! builds matrix from a 3D point
    template<typename _Tp> explicit Mat(const Point3_<_Tp>& pt, bool copyData=true);
*/
    //! destructor - calls release()
    ~Mat();
    //! assignment operators
    Mat& operator = (const Mat& m);
    //Mat& operator = (const MatExpr& expr);

    //! returns a new matrix header for the specified row
    Mat row(int y) const;
    //! returns a new matrix header for the specified column
    Mat col(int x) const;
    //! ... for the specified row span
    Mat rowRange(int startrow, int endrow) const;
    Mat rowRange(const Range& r) const;
    //! ... for the specified column span
    Mat colRange(int startcol, int endcol) const;
    Mat colRange(const Range& r) const;
    //! ... for the specified diagonal
    // (d=0 - the main diagonal,
    //  >0 - a diagonal from the lower half,
    //  <0 - a diagonal from the upper half)
    Mat diag(int d=0) const;
    //! constructs a square diagonal matrix which main diagonal is vector "d"
    static Mat diag(const Mat& d);

    //! returns deep copy of the matrix, i.e. the data is copied
    Mat clone() const;
    //! copies the matrix content to "m".
    // It calls m.create(this->size(), this->type()).
    void copyTo( OutputArray m ) const;
    //! copies those matrix elements to "m" that are marked with non-zero mask elements.
    void copyTo( OutputArray m, InputArray mask ) const;
    //! converts matrix to another datatype with optional scalng. See mxuConvertScale.
    void convertTo( OutputArray m, int rtype, double alpha=1, double beta=0 ) const;

    void assignTo( Mat& m, int type=-1 ) const;

    //! sets every matrix element to s
    Mat& operator = (const Scalar& s);
    //! sets some of the matrix elements to s, according to the mask
    Mat& setTo(InputArray value, InputArray mask=noArray());
    //! creates alternative matrix header for the same data, with different
    // number of channels and/or different number of rows. see mxuReshape.
    Mat reshape(int cn, int rows=0) const;
    Mat reshape(int cn, int newndims, const int* newsz) const;

    //! matrix transposition by means of matrix expressions
    //MatExpr t() const;
    //! matrix inversion by means of matrix expressions
    //MatExpr inv(int method=DECOMP_LU) const;
    //! per-element matrix multiplication by means of matrix expressions
    //MatExpr mul(InputArray m, double scale=1) const;

    //! computes cross-product of 2 3D vectors
    //Mat cross(InputArray m) const;
    //! computes dot-product
    double dot(InputArray m) const;

    /*! Matlab-style matrix initialization
    static MatExpr zeros(int rows, int cols, int type);
    static MatExpr zeros(Size size, int type);
    static MatExpr zeros(int ndims, const int* sz, int type);
    static MatExpr ones(int rows, int cols, int type);
    static MatExpr ones(Size size, int type);
    static MatExpr ones(int ndims, const int* sz, int type);
    static MatExpr eye(int rows, int cols, int type);
    static MatExpr eye(Size size, int type);
    */
    //! allocates new matrix data unless the matrix already has specified size and type.
    // previous data is unreferenced if needed.
    void create(int rows, int cols, int type);
    void create(Size size, int type);
    void create(int ndims, const int* sizes, int type);

    //! increases the reference counter; use with care to avoid memleaks
    void addref();
    //! decreases reference counter;
    // deallocates the data when reference counter reaches 0.
    void release();

    //! deallocates the matrix data
    void deallocate();
    //! internal use function; properly re-allocates _size, _step arrays
    void copySize(const Mat& m);

    //! reserves enough space to fit sz hyper-planes
    void reserve(size_t sz);
    //! resizes matrix to the specified number of hyper-planes
    void resize(size_t sz);
    //! resizes matrix to the specified number of hyper-planes; initializes the newly added elements
    void resize(size_t sz, const Scalar& s);
    //! internal function
    void push_back_(const void* elem);
    //! adds element to the end of 1d matrix (or possibly multiple elements when _Tp=Mat)
    template<typename _Tp> void push_back(const _Tp& elem);
    //template<typename _Tp> void push_back(const Mat_<_Tp>& elem);
    void push_back(const Mat& m);
    //! removes several hyper-planes from bottom of the matrix
    void pop_back(size_t nelems=1);

    //! locates matrix header within a parent matrix. See below
    void locateROI( Size& wholeSize, Point& ofs ) const;
    //! moves/resizes the current matrix ROI inside the parent matrix.
    Mat& adjustROI( int dtop, int dbottom, int dleft, int dright );
    //! extracts a rectangular sub-matrix
    // (this is a generalized form of row, rowRange etc.)
    Mat operator()( Range rowRange, Range colRange ) const;
    Mat operator()( const Rect& roi ) const;
    Mat operator()( const Range* ranges ) const;

    //! converts header to MxuMat; no data is copied
    operator MxuMat() const;
    //! converts header to MxuMatND; no data is copied
    //operator MxuMatND() const;
    //! converts header to IftImage; no data is copied
    //operator IftImage() const;

    //template<typename _Tp> operator vector<_Tp>() const;
    //template<typename _Tp, int n> operator Vec<_Tp, n>() const;
    //template<typename _Tp, int m, int n> operator Matx<_Tp, m, n>() const;

    //! returns true iff the matrix data is continuous
    // (i.e. when there are no gaps between successive rows).
    // similar to MXU_IS_MAT_CONT(mxumat->type)
    bool isContinuous() const;

    //! returns true if the matrix is a submatrix of another matrix
    bool isSubmatrix() const;

    //! returns element size in bytes,
    // similar to MXU_ELEM_SIZE(mxumat->type)
    size_t elemSize() const;
    //! returns the size of element channel in bytes.
    size_t elemSize1() const;
    //! returns element type, similar to MXU_MAT_TYPE(mxumat->type)
    int type() const;
    //! returns element type, similar to MXU_MAT_DEPTH(mxumat->type)
    int depth() const;
    //! returns element type, similar to MXU_MAT_CN(mxumat->type)
    int channels() const;
    //! returns step/elemSize1()
    size_t step1(int i=0) const;
    //! returns true if matrix data is NULL
    bool empty() const;
    //! returns the total number of matrix elements
    size_t total() const;

    //! returns N if the matrix is 1-channel (N x ptdim) or ptdim-channel (1 x N) or (N x 1); negative number otherwise
    int checkVector(int elemChannels, int depth=-1, bool requireContinuous=true) const;

    //! returns pointer to i0-th submatrix along the dimension #0
    uchar* ptr(int i0=0);
    const uchar* ptr(int i0=0) const;

    //! returns pointer to (i0,i1) submatrix along the dimensions #0 and #1
    uchar* ptr(int i0, int i1);
    const uchar* ptr(int i0, int i1) const;

    //! returns pointer to (i0,i1,i3) submatrix along the dimensions #0, #1, #2
    uchar* ptr(int i0, int i1, int i2);
    const uchar* ptr(int i0, int i1, int i2) const;

    //! returns pointer to the matrix element
    uchar* ptr(const int* idx);
    //! returns read-only pointer to the matrix element
    const uchar* ptr(const int* idx) const;

    template<int n> uchar* ptr(const Vec<int, n>& idx);
    template<int n> const uchar* ptr(const Vec<int, n>& idx) const;

    //! template version of the above method
    template<typename _Tp> _Tp* ptr(int i0=0);
    template<typename _Tp> const _Tp* ptr(int i0=0) const;

    template<typename _Tp> _Tp* ptr(int i0, int i1);
    template<typename _Tp> const _Tp* ptr(int i0, int i1) const;

    template<typename _Tp> _Tp* ptr(int i0, int i1, int i2);
    template<typename _Tp> const _Tp* ptr(int i0, int i1, int i2) const;

    template<typename _Tp> _Tp* ptr(const int* idx);
    template<typename _Tp> const _Tp* ptr(const int* idx) const;

    template<typename _Tp, int n> _Tp* ptr(const Vec<int, n>& idx);
    template<typename _Tp, int n> const _Tp* ptr(const Vec<int, n>& idx) const;

    //! the same as above, with the pointer dereferencing
    template<typename _Tp> _Tp& at(int i0=0);
    template<typename _Tp> const _Tp& at(int i0=0) const;

    template<typename _Tp> _Tp& at(int i0, int i1);
    template<typename _Tp> const _Tp& at(int i0, int i1) const;

    template<typename _Tp> _Tp& at(int i0, int i1, int i2);
    template<typename _Tp> const _Tp& at(int i0, int i1, int i2) const;

    template<typename _Tp> _Tp& at(const int* idx);
    template<typename _Tp> const _Tp& at(const int* idx) const;

    template<typename _Tp, int n> _Tp& at(const Vec<int, n>& idx);
    template<typename _Tp, int n> const _Tp& at(const Vec<int, n>& idx) const;

    //! special versions for 2D arrays (especially convenient for referencing image pixels)
    template<typename _Tp> _Tp& at(Point pt);
    template<typename _Tp> const _Tp& at(Point pt) const;

    //! template methods for iteration over matrix elements.
    // the iterators take care of skipping gaps in the end of rows (if any)
    //template<typename _Tp> MatIterator_<_Tp> begin();
    //template<typename _Tp> MatIterator_<_Tp> end();
    //template<typename _Tp> MatConstIterator_<_Tp> begin() const;
    //template<typename _Tp> MatConstIterator_<_Tp> end() const;

    enum { MAGIC_VAL=0x42FF0000, AUTO_STEP=0, CONTINUOUS_FLAG=MXU_MAT_CONT_FLAG, SUBMATRIX_FLAG=MXU_SUBMAT_FLAG };

    /*! includes several bit-fields:
         - the magic signature
         - continuity flag
         - depth
         - number of channels
     */
    int flags;
    //! the matrix dimensionality, >= 2
    int dims;
    //! the number of rows and columns or (-1, -1) when the matrix has more than 2 dimensions
    int rows, cols;
    //! pointer to the data
    uchar* data;

    //! pointer to the reference counter;
    // when matrix points to user-allocated data, the pointer is NULL
    int* refcount;

    //! helper fields used in locateROI and adjustROI
    uchar* datastart;
    uchar* dataend;
    uchar* datalimit;

    //! custom allocator
    MatAllocator* allocator;

    struct MXU_EXPORTS MSize
    {
        MSize(int* _p);
        Size operator()() const;
        const int& operator[](int i) const;
        int& operator[](int i);
        operator const int*() const;
        bool operator == (const MSize& sz) const;
        bool operator != (const MSize& sz) const;

        int* p;
    };

    struct MXU_EXPORTS MStep
    {
        MStep();
        MStep(size_t s);
        const size_t& operator[](int i) const;
        size_t& operator[](int i);
        operator size_t() const;
        MStep& operator = (size_t s);

        size_t* p;
        size_t buf[2];
    protected:
        MStep& operator = (const MStep&);
    };

    MSize size;
    MStep step;

protected:
    void initEmpty();
};


/*!
   Random Number Generator

   The class implements RNG using Multiply-with-Carry algorithm
*/
class MXU_EXPORTS RNG
{
public:
    enum { UNIFORM=0, NORMAL=1 };

    RNG();
    RNG(uint64 state);
    //! updates the state and returns the next 32-bit unsigned integer random number
    unsigned next();

    operator uchar();
    operator schar();
    operator ushort();
    operator short();
    operator unsigned();
    //! returns a random integer sampled uniformly from [0, N).
    unsigned operator ()(unsigned N);
    unsigned operator ()();
    operator int();
    operator float();
    operator double();
    //! returns uniformly distributed integer random number from [a,b) range
    int uniform(int a, int b);
    //! returns uniformly distributed floating-point random number from [a,b) range
    float uniform(float a, float b);
    //! returns uniformly distributed double-precision floating-point random number from [a,b) range
    double uniform(double a, double b);
    void fill( InputOutputArray mat, int distType, InputArray a, InputArray b, bool saturateRange=false );
    //! returns Gaussian random variate with mean zero.
    double gaussian(double sigma);

    uint64 state;
};

/*!
   Random Number Generator - MT

   The class implements RNG using the Mersenne Twister algorithm
*/
class MXU_EXPORTS RNG_MT19937
{
public:
    RNG_MT19937();
    RNG_MT19937(unsigned s);
    void seed(unsigned s);

    unsigned next();

    operator int();
    operator unsigned();
    operator float();
    operator double();

    unsigned operator ()(unsigned N);
    unsigned operator ()();

    //! returns uniformly distributed integer random number from [a,b) range
    int uniform(int a, int b);
    //! returns uniformly distributed floating-point random number from [a,b) range
    float uniform(float a, float b);
    //! returns uniformly distributed double-precision floating-point random number from [a,b) range
    double uniform(double a, double b);

private:
    enum PeriodParameters {N = 624, M = 397};
    unsigned state[N];
    int mti;
};

/*!
 Termination criteria in iterative algorithms
 */
class MXU_EXPORTS TermCriteria
{
public:
    enum
    {
        COUNT=1, //!< the maximum number of iterations or elements to compute
        MAX_ITER=COUNT, //!< ditto
        EPS=2 //!< the desired accuracy or change in parameters at which the iterative algorithm stops
    };

    //! default constructor
    TermCriteria();
    //! full constructor
    TermCriteria(int type, int maxCount, double epsilon);
    //! conversion from MxuTermCriteria
    TermCriteria(const MxuTermCriteria& criteria);
    //! conversion to MxuTermCriteria
    operator MxuTermCriteria() const;

    int type; //!< the type of termination criteria: COUNT, EPS or COUNT + EPS
    int maxCount; // the maximum number of iterations/elements
    double epsilon; // the desired accuracy
};




typedef void (*BinaryFunc)(const uchar* src1, size_t step1,
                           const uchar* src2, size_t step2,
                           uchar* dst, size_t step, Size sz,
                           void*);

MXU_EXPORTS BinaryFunc getConvertFunc(int sdepth, int ddepth);
MXU_EXPORTS BinaryFunc getConvertScaleFunc(int sdepth, int ddepth);
MXU_EXPORTS BinaryFunc getCopyMaskFunc(size_t esz);

//! swaps two matrices
MXU_EXPORTS void swap(Mat& a, Mat& b);
//! converts array (MxuMat or IftImage) to mxu::Mat
MXU_EXPORTS Mat mxuarrToMat(const MxuArr* arr, bool copyData=false,
                          bool allowND=true, int coiMode=0);


//! scales array elements, computes absolute values and converts the results to 8-bit unsigned integers: dst(i)=saturate_cast<uchar>abs(src(i)*alpha+beta)
MXU_EXPORTS_W void convertScaleAbs(InputArray src, OutputArray dst,
                                  double alpha=1, double beta=0);
//! computes the number of nonzero array elements
MXU_EXPORTS_W int CountNonZero( InputArray src );
MXU_EXPORTS_W int CountNonZero_Reset( InputArray _src );

//! computes mean value of selected array elements
MXU_EXPORTS_W Scalar mean(InputArray src, InputArray mask=noArray());
//! computes mean value and standard deviation of all or selected array elements
MXU_EXPORTS_W void meanStdDev(InputArray src, OutputArray mean, OutputArray stddev,
                             InputArray mask=noArray());

//! computes norm of the selected array part
MXU_EXPORTS_W double norm(InputArray src1, int normType=NORM_L2, InputArray mask=noArray());
//! computes norm of selected part of the difference between two arrays
MXU_EXPORTS_W double norm(InputArray src1, InputArray src2,
			  int normType=NORM_L2, InputArray mask=noArray());

//! finds global minimum and maximum array elements and returns their values and their locations
MXU_EXPORTS_W void minMaxLoc(InputArray src, double* minVal,
			     double* maxVal=0, Point* minLoc=0,
			     Point* maxLoc=0, InputArray mask=noArray());
MXU_EXPORTS void minMaxIdx(InputArray src, double* minVal, double* maxVal,
                          int* minIdx=0, int* maxIdx=0, InputArray mask=noArray());

//! makes multi-channel array out of several single-channel arrays
MXU_EXPORTS void merge(const Mat* mv, size_t count, OutputArray dst);
MXU_EXPORTS void merge(const vector<Mat>& mv, OutputArray dst );

//! makes multi-channel array out of several single-channel arrays
MXU_EXPORTS_W void merge(InputArrayOfArrays mv, OutputArray dst);

//! copies each plane of a multi-channel array to a dedicated array
MXU_EXPORTS void split(const Mat& src, Mat* mvbegin);
MXU_EXPORTS void split(const Mat& m, vector<Mat>& mv );

//! copies each plane of a multi-channel array to a dedicated array
MXU_EXPORTS_W void split(InputArray m, OutputArrayOfArrays mv);

//! copies selected channels from the input arrays to the selected channels of the output arrays
MXU_EXPORTS void mixChannels(const Mat* src, size_t nsrcs, Mat* dst, size_t ndsts,
                            const int* fromTo, size_t npairs);
MXU_EXPORTS void mixChannels(const vector<Mat>& src, vector<Mat>& dst,
                            const int* fromTo, size_t npairs);
MXU_EXPORTS_W void mixChannels(InputArrayOfArrays src, InputArrayOfArrays dst,
                              const vector<int>& fromTo);

//! extracts a single channel from src (coi is 0-based index)
MXU_EXPORTS_W void extractChannel(InputArray src, OutputArray dst, int coi);

//! inserts a single channel to dst (coi is 0-based index)
MXU_EXPORTS_W void insertChannel(InputArray src, InputOutputArray dst, int coi);

//! reverses the order of the rows, columns or both in a matrix
MXU_EXPORTS_W void flip(InputArray src, OutputArray dst, int flipCode);

//! computes element-wise absolute difference of two arrays (dst = abs(src1 - src2))
MXU_EXPORTS_W void absdiff(InputArray src1, InputArray src2, OutputArray dst);

//! computes exponent of each matrix element (dst = e**src)
MXU_EXPORTS_W void exp(InputArray src, OutputArray dst);

MXU_EXPORTS void exp(const float* src, float* dst, int n);

//! draws a filled convex polygon in the image
MXU_EXPORTS void fillConvexPoly(Mat& img, const Point* pts, int npts,
                               const Scalar& color, int lineType=8,
                               int shift=0);
MXU_EXPORTS_W void fillConvexPoly(InputOutputArray img, InputArray points,
                                 const Scalar& color, int lineType=8,
                                 int shift=0);
/*!
   Line iterator class
*/

class MXU_EXPORTS LineIterator
{
public:
    //! intializes the iterator
    LineIterator( const Mat& img, Point pt1, Point pt2,
                  int connectivity=8, bool leftToRight=false );
    //! returns pointer to the current pixel
    uchar* operator *();
    //! prefix increment operator (++it). shifts iterator to the next pixel
    LineIterator& operator ++();
    //! postfix increment operator (it++). shifts iterator to the next pixel
    LineIterator operator ++(int);
    //! returns coordinates of the current pixel
    Point pos() const;

    uchar* ptr;
    const uchar* ptr0;
    int step, elemSize;
    int err, count;
    int minusDelta, plusDelta;
    int minusStep, plusStep;
};

///////////////////////////////// Mat_<_Tp> ////////////////////////////////////

/*!
 Template matrix class derived from Mat
*/
template<typename _Tp> class Mat_ : public Mat
{
public:
    typedef _Tp value_type;
    typedef typename DataType<_Tp>::channel_type channel_type;
    //typedef MatIterator_<_Tp> iterator;
    //typedef MatConstIterator_<_Tp> const_iterator;

    //! default constructor
    Mat_();
    //! equivalent to Mat(_rows, _cols, DataType<_Tp>::type)
    Mat_(int _rows, int _cols);
    //! constructor that sets each matrix element to specified value
    Mat_(int _rows, int _cols, const _Tp& value);
    //! equivalent to Mat(_size, DataType<_Tp>::type)
    explicit Mat_(Size _size);
    //! constructor that sets each matrix element to specified value
    Mat_(Size _size, const _Tp& value);
    //! n-dim array constructor
    Mat_(int _ndims, const int* _sizes);
    //! n-dim array constructor that sets each matrix element to specified value
    Mat_(int _ndims, const int* _sizes, const _Tp& value);
    //! copy/conversion contructor. If m is of different type, it's converted
    Mat_(const Mat& m);
    //! copy constructor
    Mat_(const Mat_& m);
    //! constructs a matrix on top of user-allocated data. step is in bytes(!!!), regardless of the type
    Mat_(int _rows, int _cols, _Tp* _data, size_t _step=AUTO_STEP);
    //! constructs n-dim matrix on top of user-allocated data. steps are in bytes(!!!), regardless of the type
    Mat_(int _ndims, const int* _sizes, _Tp* _data, const size_t* _steps=0);
    //! selects a submatrix
    Mat_(const Mat_& m, const Range& rowRange, const Range& colRange=Range::all());
    //! selects a submatrix
    Mat_(const Mat_& m, const Rect& roi);
    //! selects a submatrix, n-dim version
    Mat_(const Mat_& m, const Range* ranges);
    //! from a matrix expression
    //explicit Mat_(const MatExpr& e);
    //! makes a matrix out of Vec, std::vector, Point_ or Point3_. The matrix will have a single column
    explicit Mat_(const vector<_Tp>& vec, bool copyData=false);
    template<int n> explicit Mat_(const Vec<typename DataType<_Tp>::channel_type, n>& vec, bool copyData=true);
    template<int m, int n> explicit Mat_(const Matx<typename DataType<_Tp>::channel_type, m, n>& mtx, bool copyData=true);
    explicit Mat_(const Point_<typename DataType<_Tp>::channel_type>& pt, bool copyData=true);
    explicit Mat_(const Point3_<typename DataType<_Tp>::channel_type>& pt, bool copyData=true);
    //explicit Mat_(const MatCommaInitializer_<_Tp>& commaInitializer);

    Mat_& operator = (const Mat& m);
    Mat_& operator = (const Mat_& m);
    //! set all the elements to s.
    Mat_& operator = (const _Tp& s);
    //! assign a matrix expression
    //Mat_& operator = (const MatExpr& e);

    //! iterators; they are smart enough to skip gaps in the end of rows
    //iterator begin();
    //iterator end();
    //const_iterator begin() const;
    //const_iterator end() const;

    //! equivalent to Mat::create(_rows, _cols, DataType<_Tp>::type)
    void create(int _rows, int _cols);
    //! equivalent to Mat::create(_size, DataType<_Tp>::type)
    void create(Size _size);
    //! equivalent to Mat::create(_ndims, _sizes, DatType<_Tp>::type)
    void create(int _ndims, const int* _sizes);
    //! cross-product
    Mat_ cross(const Mat_& m) const;
    //! data type conversion
    template<typename T2> operator Mat_<T2>() const;
    //! overridden forms of Mat::row() etc.
    Mat_ row(int y) const;
    Mat_ col(int x) const;
    Mat_ diag(int d=0) const;
    Mat_ clone() const;

    //! overridden forms of Mat::elemSize() etc.
    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t step1(int i=0) const;
    //! returns step()/sizeof(_Tp)
    size_t stepT(int i=0) const;

    /*! overridden forms of Mat::zeros() etc. Data type is omitted, of course
    static MatExpr zeros(int rows, int cols);
    static MatExpr zeros(Size size);
    static MatExpr zeros(int _ndims, const int* _sizes);
    static MatExpr ones(int rows, int cols);
    static MatExpr ones(Size size);
    static MatExpr ones(int _ndims, const int* _sizes);
    static MatExpr eye(int rows, int cols);
    static MatExpr eye(Size size);
    */
    //! some more overriden methods
    Mat_& adjustROI( int dtop, int dbottom, int dleft, int dright );
    Mat_ operator()( const Range& rowRange, const Range& colRange ) const;
    Mat_ operator()( const Rect& roi ) const;
    Mat_ operator()( const Range* ranges ) const;

    //! more convenient forms of row and element access operators
    _Tp* operator [](int y);
    const _Tp* operator [](int y) const;

    //! returns reference to the specified element
    _Tp& operator ()(const int* idx);
    //! returns read-only reference to the specified element
    const _Tp& operator ()(const int* idx) const;

    //! returns reference to the specified element
    template<int n> _Tp& operator ()(const Vec<int, n>& idx);
    //! returns read-only reference to the specified element
    template<int n> const _Tp& operator ()(const Vec<int, n>& idx) const;

    //! returns reference to the specified element (1D case)
    _Tp& operator ()(int idx0);
    //! returns read-only reference to the specified element (1D case)
    const _Tp& operator ()(int idx0) const;
    //! returns reference to the specified element (2D case)
    _Tp& operator ()(int idx0, int idx1);
    //! returns read-only reference to the specified element (2D case)
    const _Tp& operator ()(int idx0, int idx1) const;
    //! returns reference to the specified element (3D case)
    _Tp& operator ()(int idx0, int idx1, int idx2);
    //! returns read-only reference to the specified element (3D case)
    const _Tp& operator ()(int idx0, int idx1, int idx2) const;

    _Tp& operator ()(Point pt);
    const _Tp& operator ()(Point pt) const;

    //! conversion to vector.
    operator vector<_Tp>() const;
    //! conversion to Vec
    template<int n> operator Vec<typename DataType<_Tp>::channel_type, n>() const;
    //! conversion to Matx
    template<int m, int n> operator Matx<typename DataType<_Tp>::channel_type, m, n>() const;
};

typedef Mat_<uchar> Mat1b;
typedef Mat_<Vec2b> Mat2b;
typedef Mat_<Vec3b> Mat3b;
typedef Mat_<Vec4b> Mat4b;

typedef Mat_<short> Mat1s;
typedef Mat_<Vec2s> Mat2s;
typedef Mat_<Vec3s> Mat3s;
typedef Mat_<Vec4s> Mat4s;

typedef Mat_<ushort> Mat1w;
typedef Mat_<Vec2w> Mat2w;
typedef Mat_<Vec3w> Mat3w;
typedef Mat_<Vec4w> Mat4w;

typedef Mat_<int>   Mat1i;
typedef Mat_<Vec2i> Mat2i;
typedef Mat_<Vec3i> Mat3i;
typedef Mat_<Vec4i> Mat4i;

typedef Mat_<float> Mat1f;
typedef Mat_<Vec2f> Mat2f;
typedef Mat_<Vec3f> Mat3f;
typedef Mat_<Vec4f> Mat4f;

typedef Mat_<double> Mat1d;
typedef Mat_<Vec2d> Mat2d;
typedef Mat_<Vec3d> Mat3d;
typedef Mat_<Vec4d> Mat4d;




/*!
 Comma-separated Matrix Initializer
*/
/*
template<typename _Tp> class MatCommaInitializer_
{
public:
    //! the constructor, created by "matrix << firstValue" operator, where matrix is mxu::Mat
    MatCommaInitializer_(Mat_<_Tp>* _m);
    //! the operator that takes the next value and put it to the matrix
    template<typename T2> MatCommaInitializer_<_Tp>& operator , (T2 v);
    //! another form of conversion operator
    Mat_<_Tp> operator *() const;
    operator Mat_<_Tp>() const;
protected:
    //MatIterator_<_Tp> it;
};
*/

template<typename _Tp, int m, int n> class MatxCommaInitializer
{
public:
    MatxCommaInitializer(Matx<_Tp, m, n>* _mtx);
    template<typename T2> MatxCommaInitializer<_Tp, m, n>& operator , (T2 val);
    Matx<_Tp, m, n> operator *() const;

    Matx<_Tp, m, n>* dst;
    int idx;
};

template<typename _Tp, int m> class VecCommaInitializer : public MatxCommaInitializer<_Tp, m, 1>
{
public:
    VecCommaInitializer(Vec<_Tp, m>* _vec);
    template<typename T2> VecCommaInitializer<_Tp, m>& operator , (T2 val);
    Vec<_Tp, m> operator *() const;
};

/*!
 Automatically Allocated Buffer Class

*/
template<typename _Tp, size_t fixed_size=4096/sizeof(_Tp)+8> class AutoBuffer
{
public:
    typedef _Tp value_type;
    enum { buffer_padding = (int)((16 + sizeof(_Tp) - 1)/sizeof(_Tp)) };

    //! the default contructor
    AutoBuffer();
    //! constructor taking the real buffer size
    AutoBuffer(size_t _size);
    //! destructor. calls deallocate()
    ~AutoBuffer();

    //! allocates the new buffer of size _size. if the _size is small enough, stack-allocated buffer is used
    void allocate(size_t _size);
    //! deallocates the buffer if it was dynamically allocated
    void deallocate();
    //! returns pointer to the real buffer, stack-allocated or head-allocated
    operator _Tp* ();
    //! returns read-only pointer to the real buffer, stack-allocated or head-allocated
    operator const _Tp* () const;

protected:
    //! pointer to the real buffer, can point to buf if the buffer is small enough
    _Tp* ptr;
    //! size of the real buffer
    size_t size;
    //! pre-allocated buffer
    _Tp buf[fixed_size+buffer_padding];
};



/////////////////////////// multi-dimensional dense matrix //////////////////////////

/*!
 n-Dimensional Dense Matrix Iterator Class.
*/
class MXU_EXPORTS NAryMatIterator
{
public:
    //! the default constructor
    NAryMatIterator();
    //! the full constructor taking arbitrary number of n-dim matrices
    NAryMatIterator(const Mat** arrays, uchar** ptrs, int narrays=-1);
    //! the full constructor taking arbitrary number of n-dim matrices
    NAryMatIterator(const Mat** arrays, Mat* planes, int narrays=-1);
    //! the separate iterator initialization method
    void init(const Mat** arrays, Mat* planes, uchar** ptrs, int narrays=-1);

    //! proceeds to the next plane of every iterated matrix
    NAryMatIterator& operator ++();
    //! proceeds to the next plane of every iterated matrix (postfix increment operator)
    NAryMatIterator operator ++(int);

    //! the iterated arrays
    const Mat** arrays;
    //! the current planes
    Mat* planes;
    //! data pointers
    uchar** ptrs;
    //! the number of arrays
    int narrays;
    //! the number of hyper-planes that the iterator steps through
    size_t nplanes;
    //! the size of each segment (in elements)
    size_t size;
protected:
    int iterdepth;
    size_t idx;
};

//typedef NAryMatIterator NAryMatNDIterator;





/////////////////TIAOSHI///////////
MXU_EXPORTS_W int comparemat(void *mat1,void *mat2);
MXU_EXPORTS_W void printmat(Mat mat);
/////////////////set_mxu/////////
MXU_EXPORTS_W void useMxu(bool flag);
MXU_EXPORTS_W bool checkMxu();
MXU_EXPORTS_W int64 getTickCount();
MXU_EXPORTS_W double getTickFrequency();

//add by cfwang
MXU_EXPORTS_W void add(InputArray _src1,InputArray _src2,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void adds(InputArray _src1,mxu::Scalar value,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void subtract(InputArray _src1,InputArray _src2,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void subs(InputArray _src1,mxu::Scalar value,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void subrs(InputArray _src1,mxu::Scalar value,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void max_(InputArray _src1,InputArray _src2,OutputArray _dst);
MXU_EXPORTS_W void maxs(InputArray _src1,double value,OutputArray _dst);
MXU_EXPORTS_W void min_(InputArray _src1,InputArray _src2,OutputArray _dst);
MXU_EXPORTS_W void mins(InputArray _src1,double value,OutputArray _dst);
MXU_EXPORTS_W void absdiff(InputArray _src1,InputArray _src2,OutputArray _dst);
MXU_EXPORTS_W void absdiffs(InputArray _src1,mxu::Scalar value,OutputArray _dst);
MXU_EXPORTS_W void bitwise_and(InputArray _src1,InputArray _src2,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void bitwise_ands(InputArray _src1,mxu::Scalar value,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void bitwise_or(InputArray _src1,InputArray _src2,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void bitwise_ors(InputArray _src1,mxu::Scalar value,InputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void bitwise_xor(InputArray _src1,InputArray _src2,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void bitwise_xors(InputArray _src1,mxu::Scalar value,OutputArray _dst,InputArray _mask=noArray());
MXU_EXPORTS_W void bitwise_not(InputArray _src1,OutputArray _dst);
MXU_EXPORTS_W void compare(InputArray _src1,InputArray _src2,OutputArray _dst,int cmp_op);
MXU_EXPORTS_W void compares(InputArray _src,double value,OutputArray _dst,int cmp_op);

//! performs forward or inverse 1D or 2D Discrete Fourier Transformation
MXU_EXPORTS_W void dft(InputArray src, OutputArray dst, int flags=0, int nonzeroRows=0);
//! performs inverse 1D or 2D Discrete Fourier Transformation
MXU_EXPORTS_W void idft(InputArray src, OutputArray dst, int flags=0, int nonzeroRows=0);
//! performs forward or inverse 1D or 2D Discrete Cosine Transformation
MXU_EXPORTS_W void dct(InputArray src, OutputArray dst, int flags=0);
//! performs inverse 1D or 2D Discrete Cosine Transformation
MXU_EXPORTS_W void idct(InputArray src, OutputArray dst, int flags=0);
//! computes element-wise product of the two Fourier spectrums. The second spectrum can optionally be conjugated before the multiplication
MXU_EXPORTS_W void mulSpectrums(InputArray a, InputArray b, OutputArray c,
                               int flags, bool conjB=false);
//! computes the minimal vector size vecsize1 >= vecsize so that the dft() of the vector of length vecsize1 can be computed efficiently
MXU_EXPORTS_W int getOptimalDFTSize(int vecsize);



MXU_EXPORTS_W void cartToPolar(InputArray src1, InputArray src2,
                  OutputArray dst1, OutputArray dst2, bool angleInDegrees);
MXU_EXPORTS_W bool mxu_solve(InputArray _src, InputArray _src2arg, OutputArray _dst, int method);
}



#endif // __cplusplus


#include "mxu_operations.hpp"
#include "mxu_mat.hpp"
//#include "/home/lzwang/function/common/include/core/mxu_operations.hpp"
//#include "/home/lzwang/function/common/include/core/mxu_mat.hpp"

#endif /*__JZ_MXU_CORE_HPP__*/
