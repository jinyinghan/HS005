#ifndef __JZ_MXU_CORE_INTERNAL_HPP__
#define __JZ_MXU_CORE_INTERNAL_HPP__

#include <vector>

#include "mxu_core.hpp"
#include "mxu_types_c.h"

#if defined WIN32 || defined _WIN32
#  ifndef WIN32
#    define WIN32
#  endif
#  ifndef _WIN32
#    define _WIN32
#  endif
#endif

#if !defined WIN32 && !defined WINCE
#  include <pthread.h>
#endif

#ifdef __BORLANDC__
#  ifndef WIN32
#    define WIN32
#  endif
#  ifndef _WIN32
#    define _WIN32
#  endif
#  define MXU_DLL
#  undef _MXU_ALWAYS_PROFILE_
#  define _MXU_ALWAYS_NO_PROFILE_
#endif

#ifndef FALSE
#  define FALSE 0
#endif
#ifndef TRUE
#  define TRUE 1
#endif

#ifndef JZ_MXU
#   define JZ_MXU 1
#endif

#if JZ_MXU
#include <mxu2.h>
#include <jzmxu128.h>
#endif

//#define __BEGIN__ __MXU_BEGIN__
//#define __END__ __MXU_END__
//#define EXIT __MXU_EXIT__


/* default image row align (in bytes) */
#define  MXU_DEFAULT_IMAGE_ROW_ALIGN  4

/* matrices are continuous by default */
#define  MXU_DEFAULT_MAT_ROW_ALIGN  1

/* maximum size of dynamic memory buffer.
   mxuAlloc reports an error if a larger block is requested. */
#define  MXU_MAX_ALLOC_SIZE    (((size_t)1 << (sizeof(size_t)*8-2)))

/* the alignment of all the allocated buffers */
#define  MXU_MALLOC_ALIGN    16

/* default alignment for dynamic data strucutures, resided in storages. */
#define  MXU_STRUCT_ALIGN    ((int)sizeof(double))

/* default storage block size */
#define  MXU_STORAGE_BLOCK_SIZE   ((1<<16) - 128)

/* default memory block for sparse array elements */
#define  MXU_SPARSE_MAT_BLOCK    (1<<12)

/* initial hash table size */
#define  MXU_SPARSE_HASH_SIZE0    (1<<10)

/* maximal average node_count/hash_size ratio beyond which hash table is resized */
#define  MXU_SPARSE_HASH_RATIO    3

/* max length of strings */
#define  MXU_MAX_STRLEN  1024

#if 0 /*def  MXU_CHECK_FOR_NANS*/
#  define MXU_CHECK_NANS( arr ) mxuCheckArray((arr))
#else
#  define MXU_CHECK_NANS( arr )
#endif

/****************************************************************************************\
*                                  Common declarations                                   *
\****************************************************************************************/

#ifdef __GNUC__
#  define MXU_DECL_ALIGNED(x) __attribute__ ((aligned (x)))
#elif defined _MSC_VER
#  define MXU_DECL_ALIGNED(x) __declspec(align(x))
#else
#  define MXU_DECL_ALIGNED(x)
#endif

#ifndef MXU_IMPL
#  define MXU_IMPL MXU_EXTERN_C
#endif

#define MXU_DBG_BREAK() { volatile int* crashMe = 0; *crashMe = 0; }

/* default step, set in case of continuous data
   to work around checks for valid step in some ipp functions */
#define  MXU_STUB_STEP     (1 << 30)

#define  MXU_SIZEOF_FLOAT ((int)sizeof(float))
#define  MXU_SIZEOF_SHORT ((int)sizeof(short))

#define  MXU_ORIGIN_TL  0
#define  MXU_ORIGIN_BL  1

/* IEEE754 constants and macros */
#define  MXU_POS_INF       0x7f800000
#define  MXU_NEG_INF       0x807fffff /* MXU_TOGGLE_FLT(0xff800000) */
#define  MXU_1F            0x3f800000
#define  MXU_TOGGLE_FLT(x) ((x)^((int)(x) < 0 ? 0x7fffffff : 0))
#define  MXU_TOGGLE_DBL(x) \
    ((x)^((int64)(x) < 0 ? MXU_BIG_INT(0x7fffffffffffffff) : 0))

#define  MXU_NOP(a)      (a)
#define  MXU_ADD(a, b)   ((a) + (b))
#define  MXU_SUB(a, b)   ((a) - (b))
#define  MXU_MUL(a, b)   ((a) * (b))
#define  MXU_AND(a, b)   ((a) & (b))
#define  MXU_OR(a, b)    ((a) | (b))
#define  MXU_XOR(a, b)   ((a) ^ (b))
#define  MXU_ANDN(a, b)  (~(a) & (b))
#define  MXU_ORN(a, b)   (~(a) | (b))
#define  MXU_SQR(a)      ((a) * (a))

#define  MXU_LT(a, b)    ((a) < (b))
#define  MXU_LE(a, b)    ((a) <= (b))
#define  MXU_EQ(a, b)    ((a) == (b))
#define  MXU_NE(a, b)    ((a) != (b))
#define  MXU_GT(a, b)    ((a) > (b))
#define  MXU_GE(a, b)    ((a) >= (b))

#define  MXU_NONZERO(a)      ((a) != 0)
#define  MXU_NONZERO_FLT(a)  (((a)+(a)) != 0)

/* general-purpose saturation macros */
#define  MXU_CAST_8U(t)  (uchar)(!((t) & ~255) ? (t) : (t) > 0 ? 255 : 0)
#define  MXU_CAST_8S(t)  (schar)(!(((t)+128) & ~255) ? (t) : (t) > 0 ? 127 : -128)
#define  MXU_CAST_16U(t) (ushort)(!((t) & ~65535) ? (t) : (t) > 0 ? 65535 : 0)
#define  MXU_CAST_16S(t) (short)(!(((t)+32768) & ~65535) ? (t) : (t) > 0 ? 32767 : -32768)
#define  MXU_CAST_32S(t) (int)(t)
#define  MXU_CAST_64S(t) (int64)(t)
#define  MXU_CAST_32F(t) (float)(t)
#define  MXU_CAST_64F(t) (double)(t)

#define  MXU_PASTE2(a,b) a##b
#define  MXU_PASTE(a,b)  MXU_PASTE2(a,b)

#define  MXU_EMPTY
#define  MXU_MAKE_STR(a) #a

#define  MXU_ZERO_OBJ(x) memset((x), 0, sizeof(*(x)))

#define  MXU_DIM(static_array) ((int)(sizeof(static_array)/sizeof((static_array)[0])))

#define  mxuUnsupportedFormat "Unsupported format"

MXU_INLINE void* mxuAlignPtr( const void* ptr, int align MXU_DEFAULT(32) )
{
    assert( (align & (align-1)) == 0 );
    return (void*)( ((size_t)ptr + align - 1) & ~(size_t)(align-1) );
}

MXU_INLINE int mxuAlign( int size, int align )
{
    assert( (align & (align-1)) == 0 && size < INT_MAX );
    return (size + align - 1) & -align;
}

MXU_INLINE  MxuSize  mxuGetMatSize( const MxuMat* mat )
{
    MxuSize size;
    size.width = mat->cols;
    size.height = mat->rows;
    return size;
}

#define  MXU_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))
#define  MXU_FLT_TO_FIX(x,n)  mxuRound((x)*(1<<(n)))



/****************************************************************************************\

  Generic implementation of QuickSort algorithm.
  ----------------------------------------------
  Using this macro user can declare customized sort function that can be much faster
  than built-in qsort function because of lower overhead on elements
  comparison and exchange. The macro takes less_than (or LT) argument - a macro or function
  that takes 2 arguments returns non-zero if the first argument should be before the second
  one in the sorted sequence and zero otherwise.

  Example:

    Suppose that the task is to sort points by ascending of y coordinates and if
    y's are equal x's should ascend.

    The code is:
    ------------------------------------------------------------------------------
           #define cmp_pts( pt1, pt2 ) \
               ((pt1).y < (pt2).y || ((pt1).y < (pt2).y && (pt1).x < (pt2).x))

           [static] MXU_IMPLEMENT_QSORT( imxuSortPoints, MxuPoint, cmp_pts )
    ------------------------------------------------------------------------------

    After that the function "void imxuSortPoints( MxuPoint* array, size_t total, int aux );"
    is available to user.

  aux is an additional parameter, which can be used when comparing elements.
  The current implementation was derived from *BSD system qsort():

    * Copyright (c) 1992, 1993
    *  The Regents of the University of California.  All rights reserved.
    *
    * Redistribution and use in source and binary forms, with or without
    * modification, are permitted provided that the following conditions
    * are met:
    * 1. Redistributions of source code must retain the above copyright
    *    notice, this list of conditions and the following disclaimer.
    * 2. Redistributions in binary form must reproduce the above copyright
    *    notice, this list of conditions and the following disclaimer in the
    *    documentation and/or other materials provided with the distribution.
    * 3. All advertising materials mentioning features or use of this software
    *    must display the following acknowledgement:
    *  This product includes software developed by the University of
    *  California, Berkeley and its contributors.
    * 4. Neither the name of the University nor the names of its contributors
    *    may be used to endorse or promote products derived from this software
    *    without specific prior written permission.
    *
    * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
    * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
    * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    * SUCH DAMAGE.

\****************************************************************************************/

#define MXU_IMPLEMENT_QSORT_EX( func_name, T, LT, user_data_type )                   \
void func_name( T *array, size_t total, user_data_type aux )                        \
{                                                                                   \
    int isort_thresh = 7;                                                           \
    T t;                                                                            \
    int sp = 0;                                                                     \
                                                                                    \
    struct                                                                          \
    {                                                                               \
        T *lb;                                                                      \
        T *ub;                                                                      \
    }                                                                               \
    stack[48];                                                                      \
                                                                                    \
    aux = aux;                                                                      \
                                                                                    \
    if( total <= 1 )                                                                \
        return;                                                                     \
                                                                                    \
    stack[0].lb = array;                                                            \
    stack[0].ub = array + (total - 1);                                              \
                                                                                    \
    while( sp >= 0 )                                                                \
    {                                                                               \
        T* left = stack[sp].lb;                                                     \
        T* right = stack[sp--].ub;                                                  \
                                                                                    \
        for(;;)                                                                     \
        {                                                                           \
            int i, n = (int)(right - left) + 1, m;                                  \
            T* ptr;                                                                 \
            T* ptr2;                                                                \
                                                                                    \
            if( n <= isort_thresh )                                                 \
            {                                                                       \
            insert_sort:                                                            \
                for( ptr = left + 1; ptr <= right; ptr++ )                          \
                {                                                                   \
                    for( ptr2 = ptr; ptr2 > left && LT(ptr2[0],ptr2[-1]); ptr2--)   \
                        MXU_SWAP( ptr2[0], ptr2[-1], t );                            \
                }                                                                   \
                break;                                                              \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                T* left0;                                                           \
                T* left1;                                                           \
                T* right0;                                                          \
                T* right1;                                                          \
                T* pivot;                                                           \
                T* a;                                                               \
                T* b;                                                               \
                T* c;                                                               \
                int swap_cnt = 0;                                                   \
                                                                                    \
                left0 = left;                                                       \
                right0 = right;                                                     \
                pivot = left + (n/2);                                               \
                                                                                    \
                if( n > 40 )                                                        \
                {                                                                   \
                    int d = n / 8;                                                  \
                    a = left, b = left + d, c = left + 2*d;                         \
                    left = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))     \
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));    \
                                                                                    \
                    a = pivot - d, b = pivot, c = pivot + d;                        \
                    pivot = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))    \
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));    \
                                                                                    \
                    a = right - 2*d, b = right - d, c = right;                      \
                    right = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))    \
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));    \
                }                                                                   \
                                                                                    \
                a = left, b = pivot, c = right;                                     \
                pivot = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))        \
                                   : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));       \
                if( pivot != left0 )                                                \
                {                                                                   \
                    MXU_SWAP( *pivot, *left0, t );                                   \
                    pivot = left0;                                                  \
                }                                                                   \
                left = left1 = left0 + 1;                                           \
                right = right1 = right0;                                            \
                                                                                    \
                for(;;)                                                             \
                {                                                                   \
                    while( left <= right && !LT(*pivot, *left) )                    \
                    {                                                               \
                        if( !LT(*left, *pivot) )                                    \
                        {                                                           \
                            if( left > left1 )                                      \
                                MXU_SWAP( *left1, *left, t );                        \
                            swap_cnt = 1;                                           \
                            left1++;                                                \
                        }                                                           \
                        left++;                                                     \
                    }                                                               \
                                                                                    \
                    while( left <= right && !LT(*right, *pivot) )                   \
                    {                                                               \
                        if( !LT(*pivot, *right) )                                   \
                        {                                                           \
                            if( right < right1 )                                    \
                                MXU_SWAP( *right1, *right, t );                      \
                            swap_cnt = 1;                                           \
                            right1--;                                               \
                        }                                                           \
                        right--;                                                    \
                    }                                                               \
                                                                                    \
                    if( left > right )                                              \
                        break;                                                      \
                    MXU_SWAP( *left, *right, t );                                    \
                    swap_cnt = 1;                                                   \
                    left++;                                                         \
                    right--;                                                        \
                }                                                                   \
                                                                                    \
                if( swap_cnt == 0 )                                                 \
                {                                                                   \
                    left = left0, right = right0;                                   \
                    goto insert_sort;                                               \
                }                                                                   \
                                                                                    \
                n = MIN( (int)(left1 - left0), (int)(left - left1) );               \
                for( i = 0; i < n; i++ )                                            \
                    MXU_SWAP( left0[i], left[i-n], t );                              \
                                                                                    \
                n = MIN( (int)(right0 - right1), (int)(right1 - right) );           \
                for( i = 0; i < n; i++ )                                            \
                    MXU_SWAP( left[i], right0[i-n+1], t );                           \
                n = (int)(left - left1);                                            \
                m = (int)(right1 - right);                                          \
                if( n > 1 )                                                         \
                {                                                                   \
                    if( m > 1 )                                                     \
                    {                                                               \
                        if( n > m )                                                 \
                        {                                                           \
                            stack[++sp].lb = left0;                                 \
                            stack[sp].ub = left0 + n - 1;                           \
                            left = right0 - m + 1, right = right0;                  \
                        }                                                           \
                        else                                                        \
                        {                                                           \
                            stack[++sp].lb = right0 - m + 1;                        \
                            stack[sp].ub = right0;                                  \
                            left = left0, right = left0 + n - 1;                    \
                        }                                                           \
                    }                                                               \
                    else                                                            \
                        left = left0, right = left0 + n - 1;                        \
                }                                                                   \
                else if( m > 1 )                                                    \
                    left = right0 - m + 1, right = right0;                          \
                else                                                                \
                    break;                                                          \
            }                                                                       \
        }                                                                           \
    }                                                                               \
}

#define MXU_IMPLEMENT_QSORT( func_name, T, cmp )  \
    MXU_IMPLEMENT_QSORT_EX( func_name, T, cmp, int )





#endif


