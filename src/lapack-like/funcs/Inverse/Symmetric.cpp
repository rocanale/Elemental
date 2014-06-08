/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El-lite.hpp"

namespace El {

// NOTE: This overwrites both triangles of the inverse.
template<typename F>
void SymmetricInverse
( UpperOrLower uplo, Matrix<F>& A, bool conjugate, LDLPivotType pivotType )
{
    DEBUG_ONLY(CallStackEntry cse("SymmetricInverse"))
    if( uplo == LOWER )
    {
        Matrix<Int> pPerm;
        Matrix<F> dSub;
        LDL( A, dSub, pPerm, conjugate, pivotType );
        TriangularInverse( LOWER, UNIT, A ); 
        Trdtrmm( LOWER, A, dSub, conjugate );

        // NOTE: Fill in both triangles of the inverse
        Matrix<Int> pInvPerm;
        InvertPermutation( pPerm, pInvPerm );
        MakeSymmetric( LOWER, A, conjugate );
        PermuteRows( A, pInvPerm, pPerm );
        PermuteCols( A, pInvPerm, pPerm ); 
    }
    else
        LogicError("This option is not yet supported");
}

template<typename F>
void SymmetricInverse
( UpperOrLower uplo, DistMatrix<F>& A, bool conjugate, LDLPivotType pivotType )
{
    DEBUG_ONLY(CallStackEntry cse("SymmetricInverse"))
    if( uplo == LOWER )
    {
        DistMatrix<Int,VC,STAR> pPerm( A.Grid() );
        DistMatrix<F,MD,STAR> dSub( A.Grid() );
        LDL( A, dSub, pPerm, conjugate, pivotType );
        TriangularInverse( LOWER, UNIT, A ); 
        Trdtrmm( LOWER, A, dSub, conjugate );

        // NOTE: Fill in both triangles of the inverse
        DistMatrix<Int,VC,STAR> pInvPerm(pPerm.Grid());
        InvertPermutation( pPerm, pInvPerm );
        MakeSymmetric( LOWER, A, conjugate );
        PermuteRows( A, pInvPerm, pPerm );
        PermuteCols( A, pInvPerm, pPerm );
    }
    else
        LogicError("This option is not yet supported");
}

template<typename F>
void LocalSymmetricInverse
( UpperOrLower uplo, DistMatrix<F,STAR,STAR>& A, bool conjugate, 
  LDLPivotType pivotType )
{
    DEBUG_ONLY(CallStackEntry cse("LocalSymmetricInverse"))
    SymmetricInverse( uplo, A.Matrix(), conjugate, pivotType );
}

#define PROTO(F) \
  template void SymmetricInverse \
  ( UpperOrLower uplo, Matrix<F>& A, bool conjugate, \
    LDLPivotType pivotType ); \
  template void SymmetricInverse \
  ( UpperOrLower uplo, DistMatrix<F>& A, bool conjugate, \
    LDLPivotType pivotType ); \
  template void LocalSymmetricInverse \
  ( UpperOrLower uplo, DistMatrix<F,STAR,STAR>& A, bool conjugate, \
    LDLPivotType pivotType );

PROTO(float)
PROTO(double)
PROTO(Complex<float>)
PROTO(Complex<double>)

} // namespace El
