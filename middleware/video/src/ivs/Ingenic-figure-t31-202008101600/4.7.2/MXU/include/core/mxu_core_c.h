#ifndef __JZ_MXU_CORE_C_H__
#define __JZ_MXU_CORE_C_H__

#include "mxu_types_c.h"

#ifdef __cplusplus
extern "C" {
#endif

MXUAPI(void) mxuPrintMat(MxuArr* _mat);
MXUAPI(int) mxuCompareMat(void *mat1,void *mat2);
MXUAPI(void) mxuUseMxu(bool flag);
MXUAPI(bool) mxuCheckMxu(void);
MXUAPI(int64)  mxuGetTickCount(void);
MXUAPI(double) mxuGetTickFrequency(void);
/****************************************************************************************\
*          Array allocation, deallocation, initialization and access to elements         *
\****************************************************************************************/

/* <malloc> wrapper.
   If there is no enough memory, the function
   (as well as other OpenMXU functions that call mxuAlloc)
   raises an error. */
MXUAPI(void*)  mxuAlloc( size_t size );

/* <free> wrapper.
   Here and further all the memory releasing functions
   (that all call mxuFree) take double pointer in order to
   to clear pointer to the data after releasing it.
   Passing pointer to NULL pointer is Ok: nothing happens in this case
*/
MXUAPI(void)   mxuFree_( void* ptr );
#define mxuFree(ptr) (mxuFree_(*(ptr)), *(ptr)=0)

/* Allocates and initializes MxuMat header */
MXUAPI(MxuMat*)  mxuCreateMatHeader( int rows, int cols, int type );

#define MXU_AUTOSTEP  0x7fffffff

/* Initializes MxuMat header */
MXUAPI(MxuMat*) mxuInitMatHeader( MxuMat* mat, int rows, int cols,
                              int type, void* data MXU_DEFAULT(NULL),
                              int step MXU_DEFAULT(MXU_AUTOSTEP) );

/* Allocates and initializes MxuMat header and allocates data */
MXUAPI(MxuMat*)  mxuCreateMat( int rows, int cols, int type );

/* Releases MxuMat header and deallocates matrix data
   (reference counting is used for data) */
MXUAPI(void)  mxuReleaseMat( MxuMat** mat );

/* Decrements MxuMat data reference counter and deallocates the data if
   it reaches 0 */
MXU_INLINE  void  mxuDecRefData( MxuArr* arr )
{
    if( MXU_IS_MAT( arr ))
    {
        MxuMat* mat = (MxuMat*)arr;
        mat->data.ptr = NULL;
        if( mat->refcount != NULL && --*mat->refcount == 0 )
            mxuFree( &mat->refcount );
        mat->refcount = NULL;
    }
    else if( MXU_IS_MATND( arr ))
    {
        MxuMatND* mat = (MxuMatND*)arr;
        mat->data.ptr = NULL;
        if( mat->refcount != NULL && --*mat->refcount == 0 )
            mxuFree( &mat->refcount );
        mat->refcount = NULL;
    }
}

/* Increments MxuMat data reference counter */
MXU_INLINE  int  mxuIncRefData( MxuArr* arr )
{
    int refcount = 0;
    if( MXU_IS_MAT( arr ))
    {
        MxuMat* mat = (MxuMat*)arr;
        if( mat->refcount != NULL )
            refcount = ++*mat->refcount;
    }
    else if( MXU_IS_MATND( arr ))
    {
        MxuMatND* mat = (MxuMatND*)arr;
        if( mat->refcount != NULL )
            refcount = ++*mat->refcount;
    }
    return refcount;
}


/* Creates an exact copy of the input matrix (except, may be, step value) */
MXUAPI(MxuMat*) mxuCloneMat( const MxuMat* mat );


/* Makes a new matrix from <rect> subrectangle of input array.
   No data is copied */
MXUAPI(MxuMat*) mxuGetSubRect( const MxuArr* arr, MxuMat* submat, MxuRect rect );
#define mxuGetSubArr mxuGetSubRect

/* Selects row span of the input array: arr(start_row:delta_row:end_row,:)
    (end_row is not included into the span). */
MXUAPI(MxuMat*) mxuGetRows( const MxuArr* arr, MxuMat* submat,
                        int start_row, int end_row,
                        int delta_row MXU_DEFAULT(1));

MXU_INLINE  MxuMat*  mxuGetRow( const MxuArr* arr, MxuMat* submat, int row )
{
    return mxuGetRows( arr, submat, row, row + 1, 1 );
}


/* Selects column span of the input array: arr(:,start_col:end_col)
   (end_col is not included into the span) */
MXUAPI(MxuMat*) mxuGetCols( const MxuArr* arr, MxuMat* submat,
                        int start_col, int end_col );

MXU_INLINE  MxuMat*  mxuGetCol( const MxuArr* arr, MxuMat* submat, int col )
{
    return mxuGetCols( arr, submat, col, col + 1 );
}

/* Select a diagonal of the input array.
   (diag = 0 means the main diagonal, >0 means a diagonal above the main one,
   <0 - below the main one).
   The diagonal will be represented as a column (nx1 matrix). */
MXUAPI(MxuMat*) mxuGetDiag( const MxuArr* arr, MxuMat* submat,
                            int diag MXU_DEFAULT(0));



/**************** matrix iterator: used for n-ary operations on dense arrays *********/

#define MXU_MAX_ARR 10

typedef struct MxuNArrayIterator
{
    int count; /* number of arrays */
    int dims; /* number of dimensions to iterate */
    MxuSize size; /* maximal common linear size: { width = size, height = 1 } */
    uchar* ptr[MXU_MAX_ARR]; /* pointers to the array slices */
    int stack[MXU_MAX_DIM]; /* for internal use */
    MxuMatND* hdr[MXU_MAX_ARR]; /* pointers to the headers of the
                                 matrices that are processed */
}
MxuNArrayIterator;

#define MXU_NO_DEPTH_CHECK     1
#define MXU_NO_CN_CHECK        2
#define MXU_NO_SIZE_CHECK      4

/* initializes iterator that traverses through several arrays simulteneously
   (the function together with mxuNextArraySlice is used for
    N-ari element-wise operations) */
MXUAPI(int) mxuInitNArrayIterator( int count, MxuArr** arrs,
                                 const MxuArr* mask, MxuMatND* stubs,
                                 MxuNArrayIterator* array_iterator,
                                 int flags MXU_DEFAULT(0) );

/* returns zero value if iteration is finished, non-zero (slice length) otherwise */
MXUAPI(int) mxuNextNArraySlice( MxuNArrayIterator* array_iterator );


/* Returns type of array elements:
   MXU_8UC1 ... MXU_64FC4 ... */
MXUAPI(int) mxuGetElemType( const MxuArr* arr );

/* Retrieves number of an array dimensions and
   optionally sizes of the dimensions */
MXUAPI(int) mxuGetDims( const MxuArr* arr, int* sizes MXU_DEFAULT(NULL) );


/* Retrieves size of a particular array dimension.
   For 2d arrays mxuGetDimSize(arr,0) returns number of rows (image height)
   and mxuGetDimSize(arr,1) returns number of columns (image width) */
MXUAPI(int) mxuGetDimSize( const MxuArr* arr, int index );


/* Converts MxuArr (IplImage or MxuMat,...) to MxuMat.
   If the last parameter is non-zero, function can
   convert multi(>2)-dimensional array to MxuMat as long as
   the last array's dimension is continous. The resultant
   matrix will be have appropriate (a huge) number of rows */
MXUAPI(MxuMat*) mxuGetMat( const MxuArr* arr, MxuMat* header,int* coi MXU_DEFAULT(NULL),int allowND MXU_DEFAULT(0));

#define mxuReshapeND( arr, header, new_cn, new_dims, new_sizes )   \
      mxuReshapeMatND( (arr), sizeof(*(header)), (header),         \
                      (new_cn), (new_dims), (new_sizes))

MXUAPI(MxuMat*) mxuReshape( const MxuArr* arr, MxuMat* header,
                        int new_cn, int new_rows MXU_DEFAULT(0) );

/* Repeats source 2d array several times in both horizontal and
   vertical direction to fill destination array */
MXUAPI(void) mxuRepeat( const MxuArr* src, MxuArr* dst );

/* Allocates array data */
MXUAPI(void)  mxuCreateData( MxuArr* arr );

/* Releases array data */
MXUAPI(void)  mxuReleaseData( MxuArr* arr );

/* Attaches user data to the array header. The step is reffered to
   the pre-last dimension. That is, all the planes of the array
   must be joint (w/o gaps) */
MXUAPI(void)  mxuSetData( MxuArr* arr, void* data, int step );

/* Retrieves raw data of MxuMat, IftImage or MxuMatND.
   In the latter case the function raises an error if
   the array can not be represented as a matrix */
MXUAPI(void) mxuGetRawData( const MxuArr* arr, uchar** data,
                         int* step MXU_DEFAULT(NULL),
                         MxuSize* roi_size MXU_DEFAULT(NULL));

/* Returns width and height of array in elements */
MXUAPI(MxuSize) mxuGetSize( const MxuArr* arr );

/* Copies source array to destination array */
MXUAPI(void)  mxuCopy( const MxuArr* src, MxuArr* dst,
                     const MxuArr* mask MXU_DEFAULT(NULL) );

/* Sets all or "masked" elements of input array
   to the same value*/
MXUAPI(void)  mxuSet( MxuArr* arr, MxuScalar value,
                    const MxuArr* mask MXU_DEFAULT(NULL) );

/* Clears all the array elements (sets them to 0) */
MXUAPI(void)  mxuSetZero( MxuArr* arr );
#define mxuZero  mxuSetZero

/* checks termination criteria validity and
   sets eps to default_eps (if it is not set),
   max_iter to default_max_iters (if it is not set)
*/
MXUAPI(MxuTermCriteria) mxuCheckTermCriteria( MxuTermCriteria criteria,
                                           double default_eps,
                                           int default_max_iters );

/****************************************************************************************\
*                   Arithmetic, logic and comparison operations                          *
\****************************************************************************************/
/* dst(x,y,c) = abs(src1(x,y,c) - src2(x,y,c)) */
MXUAPI(void) mxuAbsDiff( const MxuArr* src1, const MxuArr* src2, MxuArr* dst );


/****************************************************************************************\
*                                Matrix operations                                       *
\****************************************************************************************/
/* Fills matrix with given range of numbers */
MXUAPI(MxuArr*)  mxuRange( MxuArr* mat, double start, double end );

/****************************************************************************************\
 * *                              Dynamic data structures                                   *
 * \****************************************************************************************/
MXUAPI(void)  mxuCreateSeqBlock( MxuSeqWriter* writer );

/* Updates sequence header. May be useful to get access to some of previously
 *    written elements via mxuGetSeqElem or sequence reader */
MXUAPI(void)   mxuFlushSeqWriter( MxuSeqWriter* writer );

/* Creates new empty sequence that will reside in the specified storage */
MXUAPI(MxuSeq*)  mxuCreateSeq( int seq_flags, size_t header_size,
                                size_t elem_size, MxuMemStorage* storage );


/* Adds new element to the end of sequence. Returns pointer to the element */
MXUAPI(schar*)  mxuSeqPush( MxuSeq* seq, const void* element MXU_DEFAULT(NULL));


/* Returns current sequence reader position (currently observed sequence element) */
MXUAPI(int)  mxuGetSeqReaderPos( MxuSeqReader* reader );

/* Removes the last element from sequence and optionally saves it */
MXUAPI(void)  mxuSeqPop( MxuSeq* seq, void* element MXU_DEFAULT(NULL));

/* Releases memory storage. All the children of a parent must be released before
   the parent. A child storage returns all the blocks to parent when it is released */
MXUAPI(void)  mxuReleaseMemStorage( MxuMemStorage** storage );

/* Retrieves pointer to specified sequence element.
   Negative indices are supported and mean counting from the end
   (e.g -1 means the last sequence element) */
MXUAPI(schar*)  mxuGetSeqElem( const MxuSeq* seq, int index );

/* Combination of mxuCreateSeq and mxuStartAppendToSeq */
MXUAPI(void)  mxuStartWriteSeq( int seq_flags, int header_size,
                              int elem_size, MxuMemStorage* storage,
                              MxuSeqWriter* writer );



/* Removes all the elements from the sequence. The freed memory
   can be reused later only by the same sequence unless mxuClearMemStorage
   or mxuRestoreMemStoragePos is called */
MXUAPI(void)  mxuClearSeq( MxuSeq* seq );

/* Removes several elements from the end of sequence and optionally saves them */
MXUAPI(void)  mxuSeqPopMulti( MxuSeq* seq, void* elements,
                                int count, int in_front MXU_DEFAULT(0) );

/* Allocates continuous buffer of the specified size in the storage */
MXUAPI(void*) mxuMemStorageAlloc( MxuMemStorage* storage, size_t size );

/* Initializes sequence writer. The new elements will be added to the end of sequence */
MXUAPI(void)  mxuStartAppendToSeq( MxuSeq* seq, MxuSeqWriter* writer );

/* Closes sequence writer, updates sequence header and returns pointer
   to the resultant sequence
   (which may be useful if the sequence was created using mxuStartWriteSeq))
*/
MXUAPI(MxuSeq*)  mxuEndWriteSeq( MxuSeqWriter* writer );


/* Creates sequence header for array.
   After that all the operations on sequences that do not alter the content
   can be applied to the resultant sequence */
MXUAPI(MxuSeq*) mxuMakeSeqHeaderForArray( int seq_type, int header_size,
                                       int elem_size, void* elements, int total,
                                       MxuSeq* seq, MxuSeqBlock* block );

/* Copies sequence content to a continuous piece of memory */
MXUAPI(void*)  mxuCvtSeqToArray( const MxuSeq* seq, void* elements,
                                   MxuSlice slice MXU_DEFAULT(MXU_WHOLE_SEQ) );


/* Changes sequence reader position. It may seek to an absolute or
 *    to relative to the current position */
MXUAPI(void)   mxuSetSeqReaderPos( MxuSeqReader* reader, int index,
                                     int is_relative MXU_DEFAULT(0));




/* Creates a memory storage that will borrow memory blocks from parent storage */
MXUAPI(MxuMemStorage*)  mxuCreateChildMemStorage( MxuMemStorage* parent );

/************ Internal sequence functions ************/
MXUAPI(void)  mxuChangeSeqBlock( void* reader, int direction );


/* Initializes sequence reader.
 *    The sequence can be read in forward or backward direction */
MXUAPI(void) mxuStartReadSeq( const MxuSeq* seq, MxuSeqReader* reader,
                               int reverse MXU_DEFAULT(0) );


/****************************************************************************************\
*                                    Array Statistics                                    *
\****************************************************************************************/

/* Calculates number of non-zero pixels */
MXUAPI(int)  mxuCountNonZero( const MxuArr* arr );
MXUAPI(int)  mxuCountNonZero_Reset(MxuArr* imgarr );

/* types of array norm */
#define MXU_C            1
#define MXU_L1           2
#define MXU_L2           4
#define MXU_NORM_MASK    7
#define MXU_RELATIVE     8
#define MXU_DIFF         16
#define MXU_MINMAX       32

#define MXU_DIFF_C       (MXU_DIFF | MXU_C)
#define MXU_DIFF_L1      (MXU_DIFF | MXU_L1)
#define MXU_DIFF_L2      (MXU_DIFF | MXU_L2)
#define MXU_RELATIVE_C   (MXU_RELATIVE | MXU_C)
#define MXU_RELATIVE_L1  (MXU_RELATIVE | MXU_L1)
#define MXU_RELATIVE_L2  (MXU_RELATIVE | MXU_L2)

/* Finds norm, difference norm or relative difference norm for an array (or two arrays) */
MXUAPI(double)  mxuNorm( const MxuArr* arr1, const MxuArr* arr2 MXU_DEFAULT(NULL),
                       int norm_type MXU_DEFAULT(MXU_L2),
                       const MxuArr* mask MXU_DEFAULT(NULL) );


/****************************************************************************************\
*                      Discrete Linear Transforms and Related Functions                  *
\****************************************************************************************/

#define MXU_DXT_FORWARD  0
#define MXU_DXT_INVERSE  1
#define MXU_DXT_SCALE    2 /* divide result by size of array */
#define MXU_DXT_INV_SCALE (MXU_DXT_INVERSE + MXU_DXT_SCALE)
#define MXU_DXT_INVERSE_SCALE MXU_DXT_INV_SCALE
#define MXU_DXT_ROWS     4 /* transform each row individually */
#define MXU_DXT_MUL_CONJ 8 /* conjugate the second argument of mxuMulSpectrums */


/****************************************************************************************\
*                                     Drawing                                            *
\****************************************************************************************/
#define MXU_AA 16
/* Fills convex or monotonous polygon. */
MXUAPI(void)  mxuFillConvexPoly( MxuArr* img, const MxuPoint* pts, int npts, MxuScalar color,
                               int line_type MXU_DEFAULT(8), int shift MXU_DEFAULT(0));

MXUAPI(void) mxuCartToPolar( const MxuArr* xarr, const MxuArr* yarr,
                             MxuArr* magarr, MxuArr* anglearr, int angle_in_degrees );

//add by cfwang
MXUAPI(void) mxuAdd(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuAddS(const MxuArr *_src1,const MxuScalar value,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuSub(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuSubS(const MxuArr *_src1,const MxuScalar value,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuSubRS(const MxuArr *_src1,const MxuScalar value,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuMax(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst);
MXUAPI(void) mxuMaxS(const MxuArr *_src1,double value,MxuArr*_dst);
MXUAPI(void) mxuMin(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst);
MXUAPI(void) mxuMinS(const MxuArr *_src1,double value,MxuArr*_dst);
MXUAPI(void) mxuAbsDiff(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst);
MXUAPI(void) mxuAbsDiffS(const MxuArr *_src1,MxuScalar value,MxuArr*_dst);
MXUAPI(void) mxuAnd(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuAndS(const MxuArr *_src1,MxuScalar value,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuOr(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuOrS(const MxuArr *_src1,MxuScalar value,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuXor(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuXorS(const MxuArr *_src1,MxuScalar value,MxuArr*_dst,MxuArr* _mask=NULL);
MXUAPI(void) mxuNot(const MxuArr *_src1,MxuArr*_dst);
MXUAPI(void) mxuCmp(const MxuArr *_src1,const MxuArr* _src2,MxuArr*_dst,int cmp_op);
MXUAPI(void) mxuCmpS(const MxuArr *_src1,double value,MxuArr*_dst,int cmp_op);

#ifdef __cplusplus
}
#endif

#endif
