/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_LAPACK_RQ_HOUSEHOLDER_HPP
#define ELEM_LAPACK_RQ_HOUSEHOLDER_HPP

#include "elemental/lapack-like/RQ/ApplyQ.hpp"
#include "elemental/lapack-like/RQ/PanelHouseholder.hpp"

namespace elem {
namespace rq {

template<typename F> 
inline void
Householder( Matrix<F>& A, Matrix<F>& t )
{
#ifndef RELEASE
    CallStackEntry entry("rq::Householder");
#endif
    const Int m = A.Height();
    const Int n = A.Width();
    const Int minDim = Min(m,n);
    t.ResizeTo( minDim, 1 );

    const Int iOff = ( n>=m ? 0   : m-n ); 
    const Int jOff = ( n>=m ? n-m : 0   );
 
    const Int bsize = Blocksize();
    const Int kLast = LastOffset( minDim, bsize );
    for( Int k=kLast; k>=0; k-=bsize )
    {
        const Int nb = Min(bsize,minDim-k);
        const Int ki = k + iOff;
        const Int kj = k + jOff;
        auto ATopPan    = View( A, 0,  0, ki, kj+nb ); 
        auto ABottomPan = View( A, ki, 0, nb, kj+nb );
        auto t1 = View( t, k, 0, nb, 1 );

        PanelHouseholder( ABottomPan, t1 );
        ApplyQ( RIGHT, ADJOINT, ABottomPan, t1, ATopPan );
    }
}

template<typename F> 
inline void
Householder( Matrix<F>& A )
{
#ifndef RELEASE
    CallStackEntry entry("rq::Householder");
#endif
    Matrix<F> t;
    Householder( A, t );
}

template<typename F> 
inline void
Householder( DistMatrix<F>& A, DistMatrix<F,MD,STAR>& t )
{
#ifndef RELEASE
    CallStackEntry entry("rq::Householder");
    if( A.Grid() != t.Grid() )
        LogicError("{A,s} must be distributed over the same grid");
#endif
    const Int m = A.Height();
    const Int n = A.Width();
    const Int minDim = Min(m,n);
    const Int offset = n-m;
    if( t.Viewing() )
    {
        if( !t.AlignedWithDiagonal( A, offset ) ) 
            LogicError("t was not aligned with A");
    }
    else
    {
        t.AlignWithDiagonal( A, offset );
    }
    t.ResizeTo( minDim, 1 );

    const Int iOff = ( n>=m ? 0   : m-n );
    const Int jOff = ( n>=m ? n-m : 0   );

    const Int bsize = Blocksize();
    const Int kLast = LastOffset( minDim, bsize );
    for( Int k=kLast; k>=0; k-=bsize )
    {
        const Int nb = Min(bsize,minDim-k);
        const Int ki = k + iOff;
        const Int kj = k + jOff;
        auto ATopPan    = View( A, 0,  0, ki, kj+nb ); 
        auto ABottomPan = View( A, ki, 0, nb, kj+nb );
        auto t1 = View( t, k, 0, nb, 1 );

        PanelHouseholder( ABottomPan, t1 );
        ApplyQ( RIGHT, ADJOINT, ABottomPan, t1, ATopPan );
    }
}

template<typename F> 
inline void
Householder( DistMatrix<F>& A )
{
#ifndef RELEASE
    CallStackEntry entry("rq::Householder");
#endif
    DistMatrix<F,MD,STAR> t(A.Grid());
    Householder( A, t );
}

} // namespace rq
} // namespace elem

#endif // ifndef ELEM_LAPACK_RQ_HOUSEHOLDER_HPP