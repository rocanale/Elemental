/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "elemental-lite.hpp"
#include "elemental/blas-like/level1/Axpy.hpp"
#include "elemental/blas-like/level1/Transpose.hpp"

namespace elem {

template<typename T>
using GDM = GeneralDistMatrix<T,MC,MR>;
template<typename T>
using DM = DistMatrix<T,MC,MR>;

// Public section
// ##############

// Constructors and destructors
// ============================

template<typename T>
DM<T>::DistMatrix( const elem::Grid& grid )
: GDM<T>(grid)
{ this->SetShifts(); }

template<typename T>
DM<T>::DistMatrix( Int height, Int width, const elem::Grid& grid )
: GDM<T>(grid)
{ this->SetShifts(); this->Resize( height, width ); }

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int colAlign, Int rowAlign, const elem::Grid& g )
: GDM<T>(g)
{ 
    this->Align( colAlign, rowAlign );
    this->Resize( height, width );
}

template<typename T>
DM<T>::DistMatrix
( Int height, Int width,
  Int colAlign, Int rowAlign, Int ldim, const elem::Grid& g )
: GDM<T>(g)
{ 
    this->Align( colAlign, rowAlign );
    this->Resize( height, width, ldim );
}

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int colAlign, Int rowAlign,
  const T* buffer, Int ldim, const elem::Grid& g )
: GDM<T>(g)
{ this->LockedAttach( height, width, colAlign, rowAlign, buffer, ldim, g ); }

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int colAlign, Int rowAlign,
  T* buffer, Int ldim, const elem::Grid& g )
: GDM<T>(g)
{ this->Attach( height, width, colAlign, rowAlign, buffer, ldim, g ); }

template<typename T>
DM<T>::DistMatrix( const DM<T>& A )
: GDM<T>(A.Grid())
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::DistMatrix"))
    this->SetShifts();
    if( &A != this )
        *this = A;
    else
        LogicError("Tried to construct [MC,MR] with itself");
}

template<typename T>
template<Dist U,Dist V>
DM<T>::DistMatrix( const DistMatrix<T,U,V>& A )
: GDM<T>(A.Grid())
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::DistMatrix"))
    this->SetShifts();
    if( MC != U || MR != V ||
        reinterpret_cast<const DM<T>*>(&A) != this )
        *this = A;
    else
        LogicError("Tried to construct [MC,MR] with itself");
}

template<typename T>
DM<T>::DistMatrix( DM<T>&& A ) : GDM<T>(std::move(A)) { }

template<typename T> DM<T>::~DistMatrix() { }

// Assignment and reconfiguration
// ==============================

template<typename T>
const DM<T>&
DM<T>::operator=( const DM<T>& A )
{
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR] = [MC,MR]");
        this->AssertNotLocked();
    )
    if( this->Grid() == A.Grid() )
    {
        this->AlignAndResize
        ( A.ColAlign(), A.RowAlign(), A.Height(), A.Width() );
        if( !this->Participating() && !A.Participating() )
            return *this;
        if( this->ColAlign() == A.ColAlign() &&
            this->RowAlign() == A.RowAlign() )
        {
            this->matrix_ = A.LockedMatrix();
        }
        else
        {
            const elem::Grid& g = this->Grid();
#ifdef UNALIGNED_WARNINGS
            if( g.Rank() == 0 )
                std::cerr << "Unaligned [MC,MR] <- [MC,MR]." << std::endl;
#endif
            const Int colRank = this->ColRank();
            const Int rowRank = this->RowRank();
            const Int colStride = this->ColStride();
            const Int rowStride = this->RowStride();
            const Int colAlign = this->ColAlign();
            const Int rowAlign = this->RowAlign();
            const Int colAlignA = A.ColAlign();
            const Int rowAlignA = A.RowAlign();
            const Int colDiff = colAlign - colAlignA;
            const Int rowDiff = rowAlign - rowAlignA;
            const Int sendRow = (colRank + colStride + colDiff) % colStride;
            const Int recvRow = (colRank + colStride - colDiff) % colStride;
            const Int sendCol = (rowRank + rowStride + rowDiff) % rowStride;
            const Int recvCol = (rowRank + rowStride - rowDiff) % rowStride;
            const Int sendRank = sendRow + sendCol*colStride;
            const Int recvRank = recvRow + recvCol*colStride;

            const Int localHeight = this->LocalHeight();
            const Int localWidth = this->LocalWidth();
            const Int localHeightA = A.LocalHeight();
            const Int localWidthA = A.LocalWidth();
            const Int sendSize = localHeightA*localWidthA;
            const Int recvSize = localHeight*localWidth;
            T* auxBuf = this->auxMemory_.Require( sendSize + recvSize );
            T* sendBuf = &auxBuf[0];
            T* recvBuf = &auxBuf[sendSize];

            // Pack
            const Int ALDim = A.LDim();
            const T* ABuffer = A.LockedBuffer();
            PARALLEL_FOR
            for( Int jLoc=0; jLoc<localWidthA; ++jLoc )
                MemCopy
                ( &sendBuf[jLoc*localHeightA], 
                  &ABuffer[jLoc*ALDim], localHeightA );

            // Communicate
            mpi::SendRecv
            ( sendBuf, sendSize, sendRank, 
              recvBuf, recvSize, recvRank, g.VCComm() );

            // Unpack
            T* buffer = this->Buffer();
            const Int ldim = this->LDim();
            PARALLEL_FOR
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
                MemCopy
                ( &buffer[jLoc*ldim], 
                  &recvBuf[jLoc*localHeight], localHeight );
            this->auxMemory_.Release();
        }
    }
    else // the grids don't match
    {
        CopyFromDifferentGrid( A );
    }
    return *this;
}

// PAUSED PASS HERE
// TODO: Remember what needs to be finished...

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MC,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [MC,* ]"))
    this->RowFilterFrom( A );
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,MR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [* ,MR]"))
    this->ColFilterFrom( A );
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MD,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [MD,* ]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,MD>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [* ,MD]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MR,MC>& A )
{ 
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR] = [MR,MC]");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    const elem::Grid& g = this->Grid();
    this->Resize( A.Height(), A.Width() );
    if( !this->Participating() )
        return *this;

    if( A.Width() == 1 )
    {
        const Int r = g.Height();
        const Int c = g.Width();
        const Int p = g.Size();
        const Int myRow = g.Row();
        const Int myCol = g.Col();
        const Int rankCM = g.VCRank();
        const Int rankRM = g.VRRank();
        const Int ownerCol = this->RowAlign();
        const Int ownerRow = A.RowAlign();
        const Int colAlign = this->ColAlign();
        const Int colAlignA = A.ColAlign();
        const Int colShift = this->ColShift();
        const Int colShiftA = A.ColShift();

        const Int height = A.Height();
        const Int maxLocalHeight = MaxLength(height,p);
        const Int portionSize = mpi::Pad( maxLocalHeight );

        const Int colShiftVC = Shift(rankCM,colAlign,p);
        const Int colShiftVRA = Shift(rankRM,colAlignA,p);
        const Int sendRankCM = (rankCM+(p+colShiftVRA-colShiftVC)) % p;
        const Int recvRankRM = (rankRM+(p+colShiftVC-colShiftVRA)) % p;
        const Int recvRankCM = (recvRankRM/c)+r*(recvRankRM%c);

        T* buffer = this->auxMemory_.Require( (r+c)*portionSize );
        T* sendBuf = &buffer[0];
        T* recvBuf = &buffer[c*portionSize];

        if( myRow == ownerRow )
        {
            // Pack
            const T* ABuffer = A.LockedBuffer();
            PARALLEL_FOR
            for( Int k=0; k<r; ++k )
            {
                T* data = &recvBuf[k*portionSize];

                const Int shift = Shift_(myCol+c*k,colAlignA,p);
                const Int offset = (shift-colShiftA) / c;
                const Int thisLocalHeight = Length_(height,shift,p);

                for( Int iLoc=0; iLoc<thisLocalHeight; ++iLoc )
                    data[iLoc] = ABuffer[offset+iLoc*r];
            }
        }

        // A[VR,* ] <- A[MR,MC]
        mpi::Scatter
        ( recvBuf, portionSize, sendBuf, portionSize, ownerRow, g.ColComm() );

        // A[VC,* ] <- A[VR,* ]
        mpi::SendRecv
        ( sendBuf, portionSize, sendRankCM,
          recvBuf, portionSize, recvRankCM, g.VCComm() );

        // A[MC,MR] <- A[VC,* ]
        mpi::Gather
        ( recvBuf, portionSize, sendBuf, portionSize, ownerCol, g.RowComm() );

        if( myCol == ownerCol )
        {
            // Unpack
            T* thisBuffer = this->Buffer();
            PARALLEL_FOR
            for( Int k=0; k<c; ++k )
            {
                const T* data = &sendBuf[k*portionSize];

                const Int shift = Shift_(myRow+r*k,colAlign,p);
                const Int offset = (shift-colShift) / r;
                const Int thisLocalHeight = Length_(height,shift,p);

                for( Int iLoc=0; iLoc<thisLocalHeight; ++iLoc )
                    thisBuffer[offset+iLoc*c] = data[iLoc];
            }
        }
        this->auxMemory_.Release();
    }
    else if( A.Height() == 1 )
    {
        const Int r = g.Height();
        const Int c = g.Width();
        const Int p = g.Size();
        const Int myRow = g.Row();
        const Int myCol = g.Col();
        const Int rankCM = g.VCRank();
        const Int rankRM = g.VRRank();
        const Int ownerRow = this->ColAlign();
        const Int ownerCol = A.ColAlign();
        const Int rowAlign = this->RowAlign();
        const Int rowAlignA = A.RowAlign();
        const Int rowShift = this->RowShift();
        const Int rowShiftA = A.RowShift();

        const Int width = A.Width();
        const Int maxLocalWidth = MaxLength(width,p);
        const Int portionSize = mpi::Pad( maxLocalWidth );

        const Int rowShiftVR = Shift(rankRM,rowAlign,p);
        const Int rowShiftVCA = Shift(rankCM,rowAlignA,p);
        const Int sendRankRM = (rankRM+(p+rowShiftVCA-rowShiftVR)) % p;
        const Int recvRankCM = (rankCM+(p+rowShiftVR-rowShiftVCA)) % p;
        const Int recvRankRM = (recvRankCM/r)+c*(recvRankCM%r);

        T* buffer = this->auxMemory_.Require( (r+c)*portionSize );
        T* sendBuf = &buffer[0];
        T* recvBuf = &buffer[r*portionSize];

        if( myCol == ownerCol )
        {
            // Pack
            const T* ABuffer = A.LockedBuffer();
            const Int ALDim = A.LDim();
            PARALLEL_FOR
            for( Int k=0; k<c; ++k )
            {
                T* data = &recvBuf[k*portionSize];

                const Int shift = Shift_(myRow+r*k,rowAlignA,p);
                const Int offset = (shift-rowShiftA) / r;
                const Int thisLocalWidth = Length_(width,shift,p);

                for( Int jLoc=0; jLoc<thisLocalWidth; ++jLoc )
                    data[jLoc] = ABuffer[(offset+jLoc*c)*ALDim];
            }
        }

        // A[* ,VC] <- A[MR,MC]
        mpi::Scatter
        ( recvBuf, portionSize, sendBuf, portionSize, ownerCol, g.RowComm() );

        // A[* ,VR] <- A[* ,VC]
        mpi::SendRecv
        ( sendBuf, portionSize, sendRankRM,
          recvBuf, portionSize, recvRankRM, g.VRComm() );

        // A[MC,MR] <- A[* ,VR]
        mpi::Gather
        ( recvBuf, portionSize, sendBuf, portionSize, ownerRow, g.ColComm() );
    
        if( myRow == ownerRow )
        {
            // Unpack
            T* thisBuffer = this->Buffer();
            const Int thisLDim = this->LDim();
            PARALLEL_FOR
            for( Int k=0; k<r; ++k )
            {
                const T* data = &sendBuf[k*portionSize];

                const Int shift = Shift_(myCol+c*k,rowAlign,p);
                const Int offset = (shift-rowShift) / c;
                const Int thisLocalWidth = Length_(width,shift,p);

                for( Int jLoc=0; jLoc<thisLocalWidth; ++jLoc )
                    thisBuffer[(offset+jLoc*r)*thisLDim] = data[jLoc];
            }
        }

        this->auxMemory_.Release();
    }
    else
    {
        if( A.Height() >= A.Width() )
        {
            std::unique_ptr<DistMatrix<T,VR,STAR>> A_VR_STAR
            ( new DistMatrix<T,VR,STAR>(g) );

            *A_VR_STAR = A;

            std::unique_ptr<DistMatrix<T,VC,STAR>> A_VC_STAR
            ( new DistMatrix<T,VC,STAR>(true,this->ColAlign(),g) );
            *A_VC_STAR = *A_VR_STAR;
            delete A_VR_STAR.release(); // lowers memory highwater

            *this = *A_VC_STAR;
        }
        else
        {
            std::unique_ptr<DistMatrix<T,STAR,VC>> A_STAR_VC
            ( new DistMatrix<T,STAR,VC>(g) );
            *A_STAR_VC = A;

            std::unique_ptr<DistMatrix<T,STAR,VR>> A_STAR_VR
            ( new DistMatrix<T,STAR,VR>(true,this->RowAlign(),g) );
            *A_STAR_VR = *A_STAR_VC;
            delete A_STAR_VC.release(); // lowers memory highwater

            *this = *A_STAR_VR;
            this->Resize( A_STAR_VR->Height(), A_STAR_VR->Width() );
        }
    }
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MR,STAR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [MR,* ]"))
    const Grid& g = A.Grid();
    std::unique_ptr<DistMatrix<T,VR,STAR>> A_VR_STAR
    ( new DistMatrix<T,VR,STAR>(A) );
    std::unique_ptr<DistMatrix<T,VC,STAR>> A_VC_STAR
    ( new DistMatrix<T,VC,STAR>(true,this->ColAlign(),g) );
    *A_VC_STAR = *A_VR_STAR;
    delete A_VR_STAR.release(); // lowers memory highwater
    *this = *A_VC_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,MC>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [* ,MC]"))
    const Grid& g = A.Grid();
    std::unique_ptr<DistMatrix<T,STAR,VC>> A_STAR_VC
    ( new DistMatrix<T,STAR,VC>(A) );
    std::unique_ptr<DistMatrix<T,STAR,VR>> A_STAR_VR
    ( new DistMatrix<T,STAR,VR>(true,this->RowAlign(),g) );
    *A_STAR_VR = *A_STAR_VC;
    delete A_STAR_VC.release(); // lowers memory highwater
    *this = *A_STAR_VR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,VC,STAR>& A )
{ 
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR] = [VC,* ]");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    const elem::Grid& g = this->Grid();
    this->AlignColsAndResize
    ( A.ColAlign()%g.Height(), A.Height(), A.Width() );
    if( !this->Participating() )
        return *this;

    if( this->ColAlign() == A.ColAlign() % g.Height() )
    {
        const Int r = g.Height();
        const Int c = g.Width();
        const Int p = r * c;
        const Int row = g.Row();
        const Int colShift = this->ColShift();
        const Int rowAlign = this->RowAlign();
        const Int colAlignA = A.ColAlign();

        const Int height = this->Height();
        const Int width = this->Width();
        const Int localWidth = this->LocalWidth();
        const Int localHeightA = A.LocalHeight();

        const Int maxHeight = MaxLength(height,p);
        const Int maxWidth = MaxLength(width,c);
        const Int portionSize = mpi::Pad( maxHeight*maxWidth );

        T* buffer = this->auxMemory_.Require( 2*c*portionSize );
        T* sendBuf = &buffer[0];
        T* recvBuf = &buffer[c*portionSize];

        // Pack
        const T* ABuffer = A.LockedBuffer();
        const Int ALDim = A.LDim();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<c; ++k )
        {
            T* data = &sendBuf[k*portionSize];
            const Int thisRowShift = Shift_(k,rowAlign,c);
            const Int thisLocalWidth = Length_(width,thisRowShift,c);
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<thisLocalWidth; ++jLoc )
                MemCopy
                ( &data[jLoc*localHeightA], 
                  &ABuffer[(thisRowShift+jLoc*c)*ALDim], localHeightA );
        }

        // Communicate
        mpi::AllToAll
        ( sendBuf, portionSize, recvBuf, portionSize, g.RowComm() );

        // Unpack
        T* thisBuffer = this->Buffer();
        const Int thisLDim = this->LDim();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<c; ++k )
        {
            const T* data = &recvBuf[k*portionSize];
            const Int thisRank = row+k*r;
            const Int thisColShift = Shift_(thisRank,colAlignA,p);
            const Int thisColOffset = (thisColShift-colShift) / r;
            const Int thisLocalHeight = Length_(height,thisColShift,p);
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            {
                T* destCol = &thisBuffer[thisColOffset+jLoc*thisLDim];
                const T* sourceCol = &data[jLoc*thisLocalHeight];
                for( Int iLoc=0; iLoc<thisLocalHeight; ++iLoc )
                    destCol[iLoc*c] = sourceCol[iLoc];
            }
        }
        this->auxMemory_.Release();
    }
    else
    {
#ifdef UNALIGNED_WARNINGS
        if( g.Rank() == 0 )
            std::cerr << "Unaligned [MC,MR] <- [VC,* ]." << std::endl;
#endif
        const Int r = g.Height();
        const Int c = g.Width();
        const Int p = r * c;
        const Int row = g.Row();
        const Int colShift = this->ColShift();
        const Int colAlign = this->ColAlign();
        const Int rowAlign = this->RowAlign();
        const Int colAlignA = A.ColAlign();

        const Int sendRow = (row+r+colAlign-(colAlignA%r)) % r;
        const Int recvRow = (row+r+(colAlignA%r)-colAlign) % r;

        const Int height = this->Height();
        const Int width = this->Width();
        const Int localWidth = this->LocalWidth();
        const Int localHeightA = A.LocalHeight();

        const Int maxHeight = MaxLength(height,p);
        const Int maxWidth = MaxLength(width,c);
        const Int portionSize = mpi::Pad( maxHeight*maxWidth );

        T* buffer = this->auxMemory_.Require( 2*c*portionSize );
        T* firstBuf = &buffer[0];
        T* secondBuf = &buffer[c*portionSize];

        // Pack
        const Int ALDim = A.LDim();
        const T* ABuffer = A.LockedBuffer();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<c; ++k )
        {
            T* data = &secondBuf[k*portionSize];
            const Int thisRowShift = Shift_(k,rowAlign,c);
            const Int thisLocalWidth = Length_(width,thisRowShift,c);
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<thisLocalWidth; ++jLoc )
                MemCopy
                ( &data[jLoc*localHeightA], 
                  &ABuffer[(thisRowShift+jLoc*c)*ALDim], localHeightA );
        }

        // SendRecv: properly align A[VC,*] via a trade in the column
        mpi::SendRecv
        ( secondBuf, c*portionSize, sendRow,
          firstBuf,  c*portionSize, recvRow, g.ColComm() );

        // AllToAll to gather all of the aligned A[VC,*] data into 
        // secondBuff.
        mpi::AllToAll
        ( firstBuf,  portionSize, secondBuf, portionSize, g.RowComm() );

        // Unpack
        T* thisBuffer = this->Buffer();
        const Int thisLDim = this->LDim();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<c; ++k )
        {
            const T* data = &secondBuf[k*portionSize];
            const Int thisRank = recvRow+k*r;
            const Int thisColShift = Shift_(thisRank,colAlignA,p);
            const Int thisColOffset = (thisColShift-colShift) / r;
            const Int thisLocalHeight = Length_(height,thisColShift,p);
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            {
                T* destCol = &thisBuffer[thisColOffset+jLoc*thisLDim];
                const T* sourceCol = &data[jLoc*thisLocalHeight];
                for( Int iLoc=0; iLoc<thisLocalHeight; ++iLoc )
                    destCol[iLoc*c] = sourceCol[iLoc];
            }
        }
        this->auxMemory_.Release();
    }
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,VC>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [* ,VC]"))
    const elem::Grid& g = this->Grid();
    DistMatrix<T,STAR,VR> A_STAR_VR(true,this->RowAlign(),g);
    A_STAR_VR = A;
    *this = A_STAR_VR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,VR,STAR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [VR,* ]"))
    const elem::Grid& g = this->Grid();
    DistMatrix<T,VC,STAR> A_VC_STAR(true,this->ColAlign(),g);
    A_VC_STAR = A;
    *this = A_VC_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,VR>& A )
{ 
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR] = [* ,VR]");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    const elem::Grid& g = this->Grid();
    this->AlignRowsAndResize
    ( A.RowAlign()%g.Width(), A.Height(), A.Width() );
    if( !this->Participating() )
        return *this;

    if( this->RowAlign() == A.RowAlign() % g.Width() )
    {
        const Int r = g.Height();
        const Int c = g.Width();
        const Int p = r * c;
        const Int col = g.Col();
        const Int rowShift = this->RowShift();
        const Int colAlign = this->ColAlign();
        const Int rowAlignA = A.RowAlign();
    
        const Int height = this->Height();
        const Int width = this->Width();
        const Int localHeight = this->LocalHeight();
        const Int localWidthA = A.LocalWidth();

        const Int maxHeight = MaxLength(height,r);
        const Int maxWidth = MaxLength(width,p);
        const Int portionSize = mpi::Pad( maxHeight*maxWidth );

        T* buffer = this->auxMemory_.Require( 2*r*portionSize );
        T* sendBuf = &buffer[0];
        T* recvBuf = &buffer[r*portionSize];

        // Pack
        const Int ALDim = A.LDim();
        const T* ABuffer = A.LockedBuffer();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<r; ++k )
        {
            T* data = &sendBuf[k*portionSize];
            const Int thisColShift = Shift_(k,colAlign,r);
            const Int thisLocalHeight = Length_(height,thisColShift,r);
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<localWidthA; ++jLoc )
            {
                T* destCol = &data[jLoc*thisLocalHeight];
                const T* sourceCol = &ABuffer[thisColShift+jLoc*ALDim];
                for( Int iLoc=0; iLoc<thisLocalHeight; ++iLoc )
                    destCol[iLoc] = sourceCol[iLoc*r];
            }
        }

        // Communicate
        mpi::AllToAll
        ( sendBuf, portionSize, recvBuf, portionSize, g.ColComm() );

        // Unpack
        T* thisBuffer = this->Buffer();
        const Int thisLDim = this->LDim();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<r; ++k )
        {
            const T* data = &recvBuf[k*portionSize];
            const Int thisRank = col+k*c;
            const Int thisRowShift = Shift_(thisRank,rowAlignA,p);
            const Int thisRowOffset = (thisRowShift-rowShift) / c;
            const Int thisLocalWidth = Length_(width,thisRowShift,p);
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<thisLocalWidth; ++jLoc )
                MemCopy
                ( &thisBuffer[(thisRowOffset+jLoc*r)*thisLDim], 
                  &data[jLoc*localHeight], localHeight );
        }
        this->auxMemory_.Release();
    }
    else
    {
#ifdef UNALIGNED_WARNINGS
        if( g.Rank() == 0 )
            std::cerr << "Unaligned [MC,MR] <- [* ,VR]." << std::endl;
#endif
        const Int r = g.Height();
        const Int c = g.Width();
        const Int p = r * c;
        const Int col = g.Col();
        const Int rowShift = this->RowShift();
        const Int colAlign = this->ColAlign();
        const Int rowAlign = this->RowAlign();
        const Int rowAlignA = A.RowAlign();

        const Int sendCol = (col+c+rowAlign-(rowAlignA%c)) % c;
        const Int recvCol = (col+c+(rowAlignA%c)-rowAlign) % c;

        const Int height = this->Height();
        const Int width = this->Width();
        const Int localHeight = this->LocalHeight();
        const Int localWidthA = A.LocalWidth();
            
        const Int maxHeight = MaxLength(height,r);
        const Int maxWidth = MaxLength(width,p);
        const Int portionSize = mpi::Pad( maxHeight*maxWidth );

        T* buffer = this->auxMemory_.Require( 2*r*portionSize );
        T* firstBuf = &buffer[0];
        T* secondBuf = &buffer[r*portionSize];

        // Pack
        const Int ALDim = A.LDim();
        const T* ABuffer = A.LockedBuffer();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<r; ++k )
        {
            T* data = &secondBuf[k*portionSize];
            const Int thisColShift = Shift_(k,colAlign,r);
            const Int thisLocalHeight = Length_(height,thisColShift,r);
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<localWidthA; ++jLoc )
            {
                T* destCol = &data[jLoc*thisLocalHeight];
                const T* sourceCol = &ABuffer[thisColShift+jLoc*ALDim];
                for( Int iLoc=0; iLoc<thisLocalHeight; ++iLoc )
                    destCol[iLoc] = sourceCol[iLoc*r];
            }
        }

        // SendRecv: properly align A[*,VR] via a trade in the column
        mpi::SendRecv
        ( secondBuf, r*portionSize, sendCol,
          firstBuf,  r*portionSize, recvCol, g.RowComm() );

        // AllToAll to gather all of the aligned [*,VR] data into 
        // secondBuf
        mpi::AllToAll
        ( firstBuf, portionSize, secondBuf, portionSize, g.ColComm() );

        // Unpack
        T* thisBuffer = this->Buffer();
        const Int thisLDim = this->LDim();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<r; ++k )
        {
            const T* data = &secondBuf[k*portionSize];
            const Int thisRank = recvCol+k*c;
            const Int thisRowShift = Shift_(thisRank,rowAlignA,p);
            const Int thisRowOffset = (thisRowShift-rowShift) / c;
            const Int thisLocalWidth = Length_(width,thisRowShift,p);
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<thisLocalWidth; ++jLoc )
                MemCopy
                ( &thisBuffer[(thisRowOffset+jLoc*r)*thisLDim], 
                  &data[jLoc*localHeight], localHeight );
        }
        this->auxMemory_.Release();
    }
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [* ,* ]"))
    this->FilterFrom( A );
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,CIRC,CIRC>& A )
{
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR] = [o ,o ]");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    const Grid& g = A.Grid();
    const Int m = A.Height();
    const Int n = A.Width();
    const Int colStride = this->ColStride();
    const Int rowStride = this->RowStride();
    const Int p = g.Size();
    this->Resize( m, n );

    const Int colAlign = this->ColAlign();
    const Int rowAlign = this->RowAlign();
    const Int mLocal = this->LocalHeight();
    const Int nLocal = this->LocalWidth();
    const Int pkgSize = mpi::Pad(MaxLength(m,colStride)*MaxLength(n,rowStride));
    const Int recvSize = pkgSize;
    const Int sendSize = p*pkgSize;
    T* recvBuf=0; // some compilers (falsely) warn otherwise
    if( A.Participating() )
    {
        T* buffer = this->auxMemory_.Require( sendSize + recvSize );
        T* sendBuf = &buffer[0];
        recvBuf = &buffer[sendSize];

        // Pack the send buffer
        const Int ALDim = A.LDim();
        const T* ABuffer = A.LockedBuffer();
        for( Int t=0; t<rowStride; ++t )
        {
            const Int tLocalWidth = Length( n, t, rowStride );
            const Int col = (rowAlign+t) % rowStride;
            for( Int s=0; s<colStride; ++s )
            {
                const Int sLocalHeight = Length( m, s, colStride );
                const Int row = (colAlign+s) % colStride;
                const Int q = row + col*colStride;
                for( Int jLoc=0; jLoc<tLocalWidth; ++jLoc ) 
                {
                    const Int j = t + jLoc*rowStride;
                    for( Int iLoc=0; iLoc<sLocalHeight; ++iLoc )
                    {
                        const Int i = s + iLoc*colStride;
                        sendBuf[q*pkgSize+iLoc+jLoc*sLocalHeight] = 
                            ABuffer[i+j*ALDim];
                    }
                }
            }
        }

        // Scatter from the root
        mpi::Scatter
        ( sendBuf, pkgSize, recvBuf, pkgSize, A.Root(), g.VCComm() );
    }
    else if( this->Participating() )
    {
        recvBuf = this->auxMemory_.Require( recvSize );

        // Perform the receiving portion of the scatter from the non-root
        mpi::Scatter
        ( static_cast<T*>(0), pkgSize, 
          recvBuf,            pkgSize, A.Root(), g.VCComm() );
    }

    if( this->Participating() )
    {
        // Unpack
        const Int ldim = this->LDim();
        T* buffer = this->Buffer();
        for( Int jLoc=0; jLoc<nLocal; ++jLoc )
            for( Int iLoc=0; iLoc<mLocal; ++iLoc )
                buffer[iLoc+jLoc*ldim] = recvBuf[iLoc+jLoc*mLocal];     
        this->auxMemory_.Release();
    }

    return *this;
}

template<typename T>
DM<T>&
DM<T>::operator=( DM<T>&& A )
{
    GDM<T>::operator=( std::move(A) );
    return *this;
}

// Realignment
// -----------

template<typename T>
void
DM<T>::AlignWith( const elem::DistData& data )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::AlignWith"))
    this->SetGrid( *data.grid );
    if( data.colDist == MC && data.rowDist == MR )
        this->Align( data.colAlign, data.rowAlign );
    else if( data.colDist == MC && data.rowDist == STAR )
        this->AlignCols( data.colAlign );
    else if( data.colDist == MR && data.rowDist == MC )
        this->Align( data.rowAlign, data.colAlign );
    else if( data.colDist == MR && data.rowDist == STAR )
        this->AlignRows( data.colAlign );
    else if( data.colDist == STAR && data.rowDist == MC )
        this->AlignCols( data.rowAlign );
    else if( data.colDist == STAR && data.rowDist == MR )
        this->AlignRows( data.rowAlign );
    else if( data.colDist == STAR && data.rowDist == VC )
        this->AlignCols( data.rowAlign % this->ColStride() );
    else if( data.colDist == STAR && data.rowDist == VR )
        this->AlignRows( data.rowAlign % this->RowStride() );
    else if( data.colDist == VC && data.rowDist == STAR )
        this->AlignCols( data.colAlign % this->ColStride() );
    else if( data.colDist == VR && data.rowDist == STAR )
        this->AlignRows( data.colAlign % this->RowStride() );
    DEBUG_ONLY(else LogicError("Nonsensical alignment"))
}

template<typename T>
void
DM<T>::AlignColsWith( const elem::DistData& data )
{
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR]::AlignColsWith");
        // Consider the case where the row alignment is larger than that
        // permitted by the new grid
        if( *this->grid_ != *data.grid )
            LogicError("Grids do not match");
    )
    if( data.colDist == MC )
        this->AlignCols( data.colAlign );
    else if( data.rowDist == MC )
        this->AlignCols( data.rowAlign );
    else if( data.colDist == VC )
        this->AlignCols( data.colAlign % this->ColStride() );
    else if( data.rowDist == VC )
        this->AlignCols( data.rowAlign % this->ColStride() );
    DEBUG_ONLY(else LogicError("Nonsensical alignment"))
}

template<typename T>
void
DM<T>::AlignRowsWith( const elem::DistData& data )
{
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR]::AlignRowsWith");
        if( *this->grid_ != *data.grid )
            LogicError("Grids do not match");
    )
    if( data.colDist == MR )
        this->AlignRows( data.colAlign );
    else if( data.rowDist == MR )
        this->AlignRows( data.rowAlign );
    else if( data.colDist == VR )
        this->AlignRows( data.colAlign % this->RowStride() );
    else if( data.rowDist == VR )
        this->AlignRows( data.rowAlign % this->RowStride() );
    DEBUG_ONLY(else LogicError("Nonsensical alignment"))
}

// Specialized redistribution/update routines
// ------------------------------------------

template<typename T>
void
DM<T>::TransposeFrom( const DistMatrix<T,STAR,MC>& A, bool conjugate )
{
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR]::TransposeFrom");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    this->AlignColsAndResize( A.RowAlign(), A.Width(), A.Height() );
    if( !this->Participating() )
        return;

    if( this->ColAlign() == A.RowAlign() )
    {
        const Int rowStride = this->RowStride();
        const Int rowShift = this->RowShift();
        const Int localHeight = this->LocalHeight();
        const Int localWidth = this->LocalWidth();
        const T* ABuffer = A.LockedBuffer();
        const Int ALDim = A.LDim();
        T* buffer = this->Buffer();
        const Int ldim = this->LDim();
        PARALLEL_FOR
        for( Int jLoc=0; jLoc<localWidth; ++jLoc )
        {
            T* destCol = &buffer[jLoc*ldim];
            const T* sourceCol = &ABuffer[rowShift+jLoc*rowStride];
            for( Int iLoc=0; iLoc<localHeight; ++iLoc )
                destCol[iLoc] = ( conjugate ? Conj(sourceCol[iLoc*ALDim])
                                            : sourceCol[iLoc*ALDim] );
        }
    }
    else
    {
        const Grid& g = this->Grid();
#ifdef UNALIGNED_WARNINGS
        if( g.Rank() == 0 )
            std::cerr << "Unaligned [MC,MR]::TransposeFrom." << std::endl;
#endif
        const Int colStride = this->ColStride();
        const Int rowStride = this->RowStride();
        const Int colRank = this->ColRank();
        const Int rowShift = this->RowShift();
        const Int colAlign = this->ColAlign();
        const Int rowAlignA = A.RowAlign();
        const Int sendRank = (colRank+colStride+colAlign-rowAlignA) % colStride;
        const Int recvRank = (colRank+colStride+rowAlignA-colAlign) % colStride;

        const Int localHeight = this->LocalHeight();
        const Int localWidth = this->LocalWidth();
        const Int localWidthA = A.LocalWidth();
        const Int sendSize = localWidthA*localWidth;
        const Int recvSize = localHeight*localWidth;
        T* auxBuf = this->auxMemory_.Require( sendSize + recvSize );
        T* sendBuf = &auxBuf[0];
        T* recvBuf = &auxBuf[sendSize];

        // Pack
        const Int ALDim = A.LDim();
        const T* ABuffer = A.LockedBuffer();
        PARALLEL_FOR
        for( Int jLoc=0; jLoc<localWidth; ++jLoc )
        {
            T* destCol = &sendBuf[jLoc*localWidth];
            const T* sourceCol = &ABuffer[rowShift+jLoc*rowStride];
            for( Int iLoc=0; iLoc<localWidthA; ++iLoc )
                destCol[iLoc] = ( conjugate ? Conj(sourceCol[iLoc*ALDim]) 
                                            : sourceCol[iLoc*ALDim] );
        }

        // Communicate
        mpi::SendRecv
        ( sendBuf, sendSize, sendRank, 
          recvBuf, recvSize, recvRank, g.ColComm() );

        // Unpack
        T* buffer = this->Buffer();
        const Int ldim = this->LDim();
        PARALLEL_FOR
        for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            MemCopy
            ( &buffer[jLoc*ldim], &recvBuf[jLoc*localHeight], localHeight );
        this->auxMemory_.Release();
    }
}

template<typename T>
void
DM<T>::TransposeFrom( const DistMatrix<T,MR,STAR>& A, bool conjugate )
{
    DEBUG_ONLY(
        CallStackEntry cse("[MC,MR]::TransposeFrom");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    this->AlignRowsAndResize( A.ColAlign(), A.Width(), A.Height(), true );
    if( !this->Participating() )
        return;

    const Int colStride = this->ColStride();
    const Int colShift = this->ColShift();
    const Int localHeight = this->LocalHeight();
    const Int localWidth = this->LocalWidth();
    const T* ABuffer = A.LockedBuffer();
    const Int ALDim = A.LDim();
    T* buffer = this->Buffer();
    const Int ldim = this->LDim();
    PARALLEL_FOR
    for( Int jLoc=0; jLoc<localWidth; ++jLoc )
    {
        T* destCol = &buffer[jLoc*ldim];
        const T* sourceCol = &ABuffer[jLoc+colShift*ALDim];
        for( Int iLoc=0; iLoc<localHeight; ++iLoc )
            destCol[iLoc] = 
                ( conjugate ? Conj(sourceCol[iLoc*colStride*ALDim])
                            : sourceCol[iLoc*colStride*ALDim] );
    }
}

template<typename T>
void
DM<T>::TransposeSumScatterFrom
( const DistMatrix<T,MR,STAR>& ATrans_MR_STAR, bool conjugate )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::TransposeSumScatterFrom"))
    const Grid& g = ATrans_MR_STAR.Grid();
    DistMatrix<T,MR,MC> ATrans( g );
    if( this->Viewing() )
        ATrans.AlignWith( *this );
    ATrans.RowSumScatterFrom( ATrans_MR_STAR );
    Transpose( ATrans, *this, conjugate );
}

template<typename T>
void
DM<T>::TransposeSumScatterUpdate
( T alpha, const DistMatrix<T,MR,STAR>& ATrans_MR_STAR, bool conjugate )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::TransposeSumScatterUpdate"))
    const Grid& g = ATrans_MR_STAR.Grid();
    DistMatrix<T,MR,MC> ATrans( g );
    ATrans.RowSumScatterFrom( ATrans_MR_STAR );
    DM<T> A( g );
    if( this->Viewing() )
        A.AlignWith( *this );
    Transpose( ATrans, A, conjugate );
    Axpy( alpha, A, *this );
}

template<typename T>
void
DM<T>::AdjointFrom( const DistMatrix<T,STAR,MC>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::AdjointFrom"))
    this->TransposeFrom( A, true );
}

template<typename T>
void
DM<T>::AdjointFrom( const DistMatrix<T,MR,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::AdjointFrom"))
    this->TransposeFrom( A, true );
}

template<typename T>
void
DM<T>::AdjointSumScatterFrom( const DistMatrix<T,MR,STAR>& AAdj_MR_STAR )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::AdjointSumScatterFrom"))
    this->TransposeSumScatterFrom( AAdj_MR_STAR, true );
}

template<typename T>
void
DM<T>::AdjointSumScatterUpdate
( T alpha, const DistMatrix<T,MR,STAR>& AAdj_MR_STAR )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::AdjointSumScatterUpdate"))
    this->TransposeSumScatterUpdate( alpha, AAdj_MR_STAR, true );
}

// Basic queries
// =============

template<typename T>
elem::DistData DM<T>::DistData() const { return elem::DistData(*this); }

template<typename T>
mpi::Comm DM<T>::DistComm() const { return this->grid_->VCComm(); }
template<typename T>
mpi::Comm DM<T>::CrossComm() const { return mpi::COMM_SELF; }
template<typename T>
mpi::Comm DM<T>::RedundantComm() const { return mpi::COMM_SELF; }
template<typename T>
mpi::Comm DM<T>::ColComm() const { return this->grid_->MCComm(); }
template<typename T>
mpi::Comm DM<T>::RowComm() const { return this->grid_->MRComm(); }

template<typename T>
Int DM<T>::ColStride() const { return this->grid_->Height(); }
template<typename T>
Int DM<T>::RowStride() const { return this->grid_->Width(); }

// Private section
// ###############

// Redistribute from a different process grid
// ==========================================

template<typename T>
void DM<T>::CopyFromDifferentGrid( const DM<T>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR]::CopyFromDifferentGrid"))
    this->Resize( A.Height(), A.Width() ); 
    // Just need to ensure that each viewing comm contains the other team's
    // owning comm. Congruence is too strong.

    // Compute the number of process rows and columns that each process 
    // needs to send to.
    const Int colStride = this->ColStride();
    const Int rowStride = this->RowStride();
    const Int colRank = this->ColRank();
    const Int rowRank = this->RowRank();
    const Int colStrideA = A.ColStride();
    const Int rowStrideA = A.RowStride();
    const Int colRankA = A.ColRank();
    const Int rowRankA = A.RowRank();
    const Int colGCD = GCD( colStride, colStrideA );
    const Int rowGCD = GCD( rowStride, rowStrideA );
    const Int colLCM = colStride*colStrideA / colGCD;
    const Int rowLCM = rowStride*rowStrideA / rowGCD;
    const Int numColSends = colStride / colGCD;
    const Int numRowSends = rowStride / rowGCD;
    const Int localColStride = colLCM / colStride;
    const Int localRowStride = rowLCM / rowStride;
    const Int localColStrideA = numColSends;
    const Int localRowStrideA = numRowSends;

    const Int colAlign = this->ColAlign();
    const Int rowAlign = this->RowAlign();
    const Int colAlignA = A.ColAlign();
    const Int rowAlignA = A.RowAlign();

    const bool inThisGrid = this->Participating();
    const bool inAGrid = A.Participating();
    if( !inThisGrid && !inAGrid )
        return;

    const Int maxSendSize = 
        (A.Height()/(colStrideA*localColStrideA)+1) * 
        (A.Width()/(rowStrideA*localRowStrideA)+1);

    // Translate the ranks from A's VC communicator to this's viewing so that
    // we can match send/recv communicators
    const int sizeA = A.Grid().Size();
    std::vector<int> rankMap(sizeA), ranks(sizeA);
    for( int j=0; j<sizeA; ++j )
        ranks[j] = j;
    mpi::Group viewingGroup;
    mpi::CommGroup( this->Grid().ViewingComm(), viewingGroup );
    mpi::GroupTranslateRanks
    ( A.Grid().OwningGroup(), sizeA, &ranks[0], viewingGroup, &rankMap[0] );

    // Have each member of A's grid individually send to all numRow x numCol
    // processes in order, while the members of this grid receive from all 
    // necessary processes at each step.
    Int requiredMemory = 0;
    if( inAGrid )
        requiredMemory += maxSendSize;
    if( inThisGrid )
        requiredMemory += maxSendSize;
    T* auxBuf = this->auxMemory_.Require( requiredMemory );
    Int offset = 0;
    T* sendBuf = &auxBuf[offset];
    if( inAGrid )
        offset += maxSendSize;
    T* recvBuf = &auxBuf[offset];

    Int recvRow = 0; // avoid compiler warnings...
    if( inAGrid )
        recvRow = (((colRankA+colStrideA-colAlignA)%colStrideA)+colAlign) % 
                  colStride;
    for( Int colSend=0; colSend<numColSends; ++colSend )
    {
        Int recvCol = 0; // avoid compiler warnings...
        if( inAGrid )
            recvCol = (((rowRankA+rowStrideA-rowAlignA)%rowStrideA)+rowAlign) % 
                      rowStride;
        for( Int rowSend=0; rowSend<numRowSends; ++rowSend )
        {
            mpi::Request sendRequest;
            // Fire off this round of non-blocking sends
            if( inAGrid )
            {
                // Pack the data
                Int sendHeight = Length(A.LocalHeight(),colSend,numColSends);
                Int sendWidth = Length(A.LocalWidth(),rowSend,numRowSends);
                const T* ABuffer = A.LockedBuffer();
                const Int ALDim = A.LDim();
                PARALLEL_FOR
                for( Int jLoc=0; jLoc<sendWidth; ++jLoc )
                {
                    const Int j = rowSend+jLoc*localRowStrideA;
                    for( Int iLoc=0; iLoc<sendHeight; ++iLoc )
                    {
                        const Int i = colSend+iLoc*localColStrideA;
                        sendBuf[iLoc+jLoc*sendHeight] = ABuffer[i+j*ALDim];
                    }
                }
                // Send data
                const Int recvVCRank = recvRow + recvCol*colStride;
                const Int recvViewingRank = 
                    this->Grid().VCToViewingMap( recvVCRank );
                mpi::ISend
                ( sendBuf, sendHeight*sendWidth, recvViewingRank,
                  this->Grid().ViewingComm(), sendRequest );
            }
            // Perform this round of recv's
            if( inThisGrid )
            {
                const Int sendColOffset = (colSend*colStrideA+colAlignA) % colStrideA;
                const Int recvColOffset = (colSend*colStrideA+colAlign) % colStride;
                const Int sendRowOffset = (rowSend*rowStrideA+rowAlignA) % rowStrideA;
                const Int recvRowOffset = (rowSend*rowStrideA+rowAlign) % rowStride;

                const Int firstSendRow = (((colRank+colStride-recvColOffset)%colStride)+sendColOffset)%colStrideA;
                const Int firstSendCol = (((rowRank+rowStride-recvRowOffset)%rowStride)+sendRowOffset)%rowStrideA;

                const Int colShift = (colRank+colStride-recvColOffset)%colStride;
                const Int rowShift = (rowRank+rowStride-recvRowOffset)%rowStride;
                const Int numColRecvs = Length( colStrideA, colShift, colStride ); 
                const Int numRowRecvs = Length( rowStrideA, rowShift, rowStride );

                // Recv data
                // For now, simply receive sequentially. Until we switch to 
                // nonblocking recv's, we won't be using much of the 
                // recvBuf
                Int sendRow = firstSendRow;
                for( Int colRecv=0; colRecv<numColRecvs; ++colRecv )
                {
                    const Int sendColShift = Shift( sendRow, colAlignA, colStrideA ) + colSend*colStrideA;
                    const Int sendHeight = Length( A.Height(), sendColShift, colLCM );
                    const Int localColOffset = (sendColShift-this->ColShift()) / colStride;

                    Int sendCol = firstSendCol;
                    for( Int rowRecv=0; rowRecv<numRowRecvs; ++rowRecv )
                    {
                        const Int sendRowShift = Shift( sendCol, rowAlignA, rowStrideA ) + rowSend*rowStrideA;
                        const Int sendWidth = Length( A.Width(), sendRowShift, rowLCM );
                        const Int localRowOffset = (sendRowShift-this->RowShift()) / rowStride;

                        const Int sendVCRank = sendRow+sendCol*colStrideA;
                        mpi::Recv
                        ( recvBuf, sendHeight*sendWidth, rankMap[sendVCRank],
                          this->Grid().ViewingComm() );
                        
                        // Unpack the data
                        T* buffer = this->Buffer();
                        const Int ldim = this->LDim();
                        PARALLEL_FOR
                        for( Int jLoc=0; jLoc<sendWidth; ++jLoc )
                        {
                            const Int j = localRowOffset+jLoc*localRowStride;
                            for( Int iLoc=0; iLoc<sendHeight; ++iLoc )
                            {
                                const Int i = localColOffset+iLoc*localColStride;
                                buffer[i+j*ldim] = recvBuf[iLoc+jLoc*sendHeight];
                            }
                        }
                        // Set up the next send col
                        sendCol = (sendCol + rowStride) % rowStrideA;
                    }
                    // Set up the next send row
                    sendRow = (sendRow + colStride) % colStrideA;
                }
            }
            // Ensure that this round of non-blocking sends completes
            if( inAGrid )
            {
                mpi::Wait( sendRequest );
                recvCol = (recvCol + rowStrideA) % rowStride;
            }
        }
        if( inAGrid )
            recvRow = (recvRow + colStrideA) % colStride;
    }
    this->auxMemory_.Release();
}

// Instantiate {Int,Real,Complex<Real>} for each Real in {float,double}
// ####################################################################

#define PROTO(T) template class DistMatrix<T,MC,MR>
#define COPY(T,U,V) \
  template DistMatrix<T,MC,MR>::DistMatrix( const DistMatrix<T,U,V>& A )
#define FULL(T) \
  PROTO(T); \
  COPY(T,CIRC,CIRC); \
  COPY(T,MC,  STAR); \
  COPY(T,MD,  STAR); \
  COPY(T,MR,  MC  ); \
  COPY(T,MR,  STAR); \
  COPY(T,STAR,MC  ); \
  COPY(T,STAR,MD  ); \
  COPY(T,STAR,MR  ); \
  COPY(T,STAR,STAR); \
  COPY(T,STAR,VC  ); \
  COPY(T,STAR,VR  ); \
  COPY(T,VC,  STAR); \
  COPY(T,VR,  STAR);

FULL(Int);
#ifndef DISABLE_FLOAT
FULL(float);
#endif
FULL(double);

#ifndef DISABLE_COMPLEX
#ifndef DISABLE_FLOAT
FULL(Complex<float>);
#endif
FULL(Complex<double>);
#endif 

} // namespace elem
