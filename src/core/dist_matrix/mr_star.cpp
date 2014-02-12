/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "elemental-lite.hpp"

namespace elem {

template<typename T>
using GDM = GeneralDistMatrix<T,MR,STAR>;
template<typename T>
using DM = DistMatrix<T,MR,STAR>;

// Public section
// ##############

// Constructors and destructors
// ============================

template<typename T>
DM<T>::DistMatrix( const elem::Grid& g )
: GDM<T>(g)
{ this->SetShifts(); } 

template<typename T>
DM<T>::DistMatrix( Int height, Int width, const elem::Grid& g )
: GDM<T>(g)
{ this->SetShifts(); this->Resize(height,width); }

template<typename T>
DM<T>::DistMatrix( Int height, Int width, Int colAlign, const elem::Grid& g )
: GDM<T>(g)
{ this->Align(colAlign,0); this->Resize(height,width); }

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int colAlign, Int ldim, const elem::Grid& g )
: GDM<T>(g)
{ this->Align(colAlign,0); this->Resize(height,width,ldim); }

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int colAlign, const T* buffer, Int ldim,
  const elem::Grid& g )
: GDM<T>(g)
{ this->LockedAttach( height, width, colAlign, 0, buffer, ldim, g ); }

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int colAlign, T* buffer, Int ldim,
  const elem::Grid& g )
: GDM<T>(g)
{ this->Attach( height, width, colAlign, 0, buffer, ldim, g ); }

template<typename T>
DM<T>::DistMatrix( const DM<T>& A )
: GDM<T>(A.Grid())
{
    DEBUG_ONLY(CallStackEntry cse("[MR,* ]::DistMatrix"))
    this->SetShifts();
    if( &A != this )
        *this = A;
    else
        LogicError("Tried to construct [MR,* ] with itself");
}

template<typename T>
template<Dist U,Dist V>
DM<T>::DistMatrix( const DistMatrix<T,U,V>& A )
: GDM<T>(A.Grid())
{
    DEBUG_ONLY(CallStackEntry cse("[MR,* ]::DistMatrix"))
    this->SetShifts();
    if( MR != U || STAR != V || 
        reinterpret_cast<const DM<T>*>(&A) != this )
        *this = A;
    else
        LogicError("Tried to construct [MR,* ] with itself");
}

template<typename T> DM<T>::DistMatrix( DM<T>&& A ) : GDM<T>(std::move(A)) { }

template<typename T> DM<T>::~DistMatrix() { }

// Assignment and reconfiguration
// ==============================

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MC,MR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [MC,MR]"))
    const elem::Grid& g = this->Grid();
    std::unique_ptr<DistMatrix<T,VC,STAR>> A_VC_STAR
    ( new DistMatrix<T,VC,STAR>(g) );
    *A_VC_STAR = A;

    std::unique_ptr<DistMatrix<T,VR,STAR>> A_VR_STAR
    ( new DistMatrix<T,VR,STAR>(true,this->ColAlign(),g) );
    *A_VR_STAR = *A_VC_STAR;
    delete A_VC_STAR.release(); // lowers memory highwater

    *this = *A_VR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MC,STAR>& A )
{ 
    DEBUG_ONLY(
        CallStackEntry cse("[MR,* ] = [MC,* ]");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    const elem::Grid& g = this->Grid();
    if( !this->Participating() )
    {
        this->Resize( A.Height(), A.Width() );
        return *this;
    }

    if( A.Width() == 1 )
    {
        this->Resize( A.Height(), 1 );
        const Int r = g.Height();
        const Int c = g.Width();
        const Int p = g.Size();
        const Int myCol = g.Col();
        const Int rankCM = g.VCRank();
        const Int rankRM = g.VRRank();
        const Int colAlign = this->ColAlign();
        const Int colShift = this->ColShift();
        const Int colAlignOfA = A.ColAlign();
        const Int colShiftOfA = A.ColShift();

        const Int height = this->Height();
        const Int maxLocalVectorHeight = MaxLength(height,p);
        const Int portionSize = mpi::Pad( maxLocalVectorHeight );

        const Int colShiftVR = Shift(rankRM,colAlign,p);
        const Int colShiftVCOfA = Shift(rankCM,colAlignOfA,p);
        const Int sendRankRM = (rankRM+(p+colShiftVCOfA-colShiftVR)) % p;
        const Int recvRankCM = (rankCM+(p+colShiftVR-colShiftVCOfA)) % p;
        const Int recvRankRM = (recvRankCM/r)+c*(recvRankCM%r);

        T* buffer = this->auxMemory_.Require( (r+1)*portionSize );
        T* sendBuf = &buffer[0];
        T* recvBuf = &buffer[r*portionSize];

        // A[VC,* ] <- A[MC,* ]
        {
            const Int shift = Shift(rankCM,colAlignOfA,p);
            const Int offset = (shift-colShiftOfA) / r;
            const Int thisLocalHeight = Length(height,shift,p);

            const T* ABuffer = A.LockedBuffer();
            PARALLEL_FOR
            for( Int iLoc=0; iLoc<thisLocalHeight; ++iLoc )
                sendBuf[iLoc] = ABuffer[offset+iLoc*c];
        }

        // A[VR,* ] <- A[VC,* ]
        mpi::SendRecv
        ( sendBuf, portionSize, sendRankRM,
          recvBuf, portionSize, recvRankRM, g.VRComm() );

        // A[MR,* ] <- A[VR,* ]
        mpi::AllGather
        ( recvBuf, portionSize,
          sendBuf, portionSize, g.ColComm() );

        // Unpack
        T* thisBuffer = this->Buffer();
        PARALLEL_FOR
        for( Int k=0; k<r; ++k )
        {
            const T* data = &sendBuf[k*portionSize];
            const Int shift = Shift_(myCol+c*k,colAlign,p);
            const Int offset = (shift-colShift) / c;
            const Int thisLocalHeight = Length_(height,shift,p);
            for( Int iLoc=0; iLoc<thisLocalHeight; ++iLoc )
                thisBuffer[offset+iLoc*r] = data[iLoc];
        }
        this->auxMemory_.Release();
    }
    else
    {
        std::unique_ptr<DistMatrix<T,VC,STAR>> A_VC_STAR
        ( new DistMatrix<T,VC,STAR>(g) );
        *A_VC_STAR = A;

        std::unique_ptr<DistMatrix<T,VR,STAR>> A_VR_STAR
        ( new DistMatrix<T,VR,STAR>(true,this->ColAlign(),g) );
        *A_VR_STAR = *A_VC_STAR;
        delete A_VC_STAR.release(); // lowers memory highwater

        *this = *A_VR_STAR;
    }
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,MR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [* ,MR]"))
    const elem::Grid& g = this->Grid();
    std::unique_ptr<DistMatrix<T,MC,MR>> A_MC_MR( new DistMatrix<T,MC,MR>(A) );

    std::unique_ptr<DistMatrix<T,VC,STAR>> A_VC_STAR
    ( new DistMatrix<T,VC,STAR>(g) );
    *A_VC_STAR = *A_MC_MR;
    delete A_MC_MR.release(); // lowers memory highwater

    std::unique_ptr<DistMatrix<T,VR,STAR>> A_VR_STAR
    ( new DistMatrix<T,VR,STAR>(true,this->ColAlign(),g) );
    *A_VR_STAR = *A_VC_STAR;
    delete A_VC_STAR.release(); // lowers memory highwater

    *this = *A_VR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MD,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [MD,* ]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,MD>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [* ,MD]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MR,MC>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [MR,MC]"))
    A.RowAllGather( *this );
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DM<T>& A )
{
    DEBUG_ONLY(
        CallStackEntry cse("[MR,* ] = [MR,* ]");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    const elem::Grid& g = this->Grid();
    this->AlignColsAndResize( A.ColAlign(), A.Height(), A.Width() );
    if( !this->Participating() )
        return *this;

    if( this->ColAlign() == A.ColAlign() )
    {
        this->matrix_ = A.LockedMatrix();
    }
    else
    {
#ifdef UNALIGNED_WARNINGS
        if( g.Rank() == 0 )
            std::cerr << "Unaligned [MR,* ] <- [MR,* ]." << std::endl;
#endif
        const Int rank = g.Col();
        const Int c = g.Width();

        const Int colAlign = this->ColAlign();
        const Int colAlignOfA = A.ColAlign();

        const Int sendRank = (rank+c+colAlign-colAlignOfA) % c;
        const Int recvRank = (rank+c+colAlignOfA-colAlign) % c;

        const Int width = this->Width();
        const Int localHeight = this->LocalHeight();
        const Int localHeightOfA = A.LocalHeight();

        const Int sendSize = localHeightOfA * width;
        const Int recvSize = localHeight * width;

        T* buffer = this->auxMemory_.Require( sendSize + recvSize );
        T* sendBuf = &buffer[0];
        T* recvBuf = &buffer[sendSize];

        // Pack
        const Int ALDim = A.LDim();
        const T* ABuffer = A.LockedBuffer();
        PARALLEL_FOR
        for( Int j=0; j<width; ++j )
        {
            const T* ACol = &ABuffer[j*ALDim];
            T* sendBufCol = &sendBuf[j*localHeightOfA];
            MemCopy( sendBufCol, ACol, localHeightOfA );
        }

        // Communicate
        mpi::SendRecv
        ( sendBuf, sendSize, sendRank,
          recvBuf, recvSize, recvRank, g.RowComm() );

        // Unpack
        T* thisBuffer = this->Buffer();
        const Int thisLDim = this->LDim();
        PARALLEL_FOR
        for( Int j=0; j<width; ++j )
        {
            const T* recvBufCol = &recvBuf[j*localHeight];
            T* thisCol = &thisBuffer[j*thisLDim];
            MemCopy( thisCol, recvBufCol, localHeight );
        }

        this->auxMemory_.Release();
    }
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,MC>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [* ,MC]"))
    DistMatrix<T,MR,MC> A_MR_MC( A );
    *this = A_MR_MC;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,VC,STAR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [VC,* ]"))
    const elem::Grid& g = this->Grid();
    DistMatrix<T,VR,STAR> A_VR_STAR(true,this->ColAlign(),g);

    A_VR_STAR = A;
    *this = A_VR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,VC>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [* ,VC]"))
    DistMatrix<T,MR,MC> A_MR_MC( A );
    *this = A_MR_MC;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,VR,STAR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [VR,* ]"))
    A.PartialColAllGather( *this );
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,VR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [* ,VR]"))
    const elem::Grid& g = this->Grid();
    std::unique_ptr<DistMatrix<T,STAR,VC>> A_STAR_VC
    ( new DistMatrix<T,STAR,VC>(A) );

    std::unique_ptr<DistMatrix<T,MR,MC>> A_MR_MC
    ( new DistMatrix<T,MR,MC>(true,false,this->ColAlign(),0,g) );
    *A_MR_MC = *A_STAR_VC;
    delete A_STAR_VC.release(); // lowers memory highwater

    *this = *A_MR_MC;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [* ,* ]"))
    this->ColFilterFrom( A );
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,CIRC,CIRC>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MR,* ] = [o ,o ]"))
    DistMatrix<T,MR,MC> A_MR_MC( A.Grid() );
    A_MR_MC.AlignWith( *this );
    A_MR_MC = A;
    *this = A_MR_MC;
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
    DEBUG_ONLY(CallStackEntry cse("[MR,* ]::AlignWith"))
    this->SetGrid( *data.grid );
    if( data.colDist == MR )
        this->AlignCols( data.colAlign );
    else if( data.rowDist == MR )
        this->AlignCols( data.rowAlign );
    else if( data.colDist == VR )
        this->AlignCols( data.colAlign % this->ColStride() );
    else if( data.rowDist == VR )
        this->AlignCols( data.rowAlign % this->ColStride() );
    DEBUG_ONLY(else LogicError("Nonsensical alignment"))
}

template<typename T>
void
DM<T>::AlignColsWith( const elem::DistData& data )
{ this->AlignWith( data ); }

// Specialized redistributions
// ---------------------------

template<typename T>
void
DM<T>::TransposeFrom( const DistMatrix<T,MC,MR>& A, bool conjugate )
{ 
    DEBUG_ONLY(
        CallStackEntry cse("[MR,* ]::TransposeFrom");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    const elem::Grid& g = this->Grid();
    this->AlignColsAndResize( A.RowAlign(), A.Width(), A.Height() );
    if( !this->Participating() )
        return;

    if( this->ColAlign() == A.RowAlign() )
    {
        const Int r = g.Height();

        const Int width = this->Width();
        const Int localHeight = this->LocalHeight();
        const Int localHeightOfA = A.LocalHeight();
        const Int maxLocalWidth = MaxLength(width,r);
        const Int portionSize = mpi::Pad( localHeight*maxLocalWidth );

        T* buffer = this->auxMemory_.Require( (r+1)*portionSize );
        T* sendBuf = &buffer[0];
        T* recvBuf = &buffer[portionSize];

        // Pack 
        const Int ALDim = A.LDim();
        const T* ABuffer = A.LockedBuffer();
        if( conjugate )
        {
            PARALLEL_FOR
            for( Int jLoc=0; jLoc<localHeightOfA; ++jLoc )
            {
                T* destCol = &sendBuf[jLoc*localHeight];
                const T* sourceCol = &ABuffer[jLoc];
                for( Int iLoc=0; iLoc<localHeight; ++iLoc )
                    destCol[iLoc] = Conj( sourceCol[iLoc*ALDim] );
            }
        }
        else
        {
            PARALLEL_FOR
            for( Int jLoc=0; jLoc<localHeightOfA; ++jLoc )
            {
                T* destCol = &sendBuf[jLoc*localHeight];
                const T* sourceCol = &ABuffer[jLoc];
                for( Int iLoc=0; iLoc<localHeight; ++iLoc )
                    destCol[iLoc] = sourceCol[iLoc*ALDim];
            }
        }

        // Communicate
        mpi::AllGather
        ( sendBuf, portionSize,
          recvBuf, portionSize, g.ColComm() );

        // Unpack
        T* thisBuffer = this->Buffer();
        const Int thisLDim = this->LDim();
        const Int colAlignOfA = A.ColAlign();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<r; ++k )
        {
            const T* data = &recvBuf[k*portionSize];
            const Int rowShift = Shift_( k, colAlignOfA, r );
            const Int localWidth = Length_( width, rowShift, r );
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            {
                const T* dataCol = &data[jLoc*localHeight];
                T* thisCol = &thisBuffer[(rowShift+jLoc*r)*thisLDim];
                MemCopy( thisCol, dataCol, localHeight );
            }
        }
        this->auxMemory_.Release();
    }
    else
    {
#ifdef UNALIGNED_WARNINGS
        if( g.Rank() == 0 )
            std::cerr << "Unaligned [MR,* ]::TransposeFrom" << std::endl;
#endif
        const Int r = g.Height();
        const Int c = g.Width();
        const Int col = g.Col();

        const Int colAlign = this->ColAlign();
        const Int rowAlignOfA = A.RowAlign();
        const Int sendCol = (col+c+colAlign-rowAlignOfA) % c;
        const Int recvCol = (col+c+rowAlignOfA-colAlign) % c;

        const Int height = this->Height();
        const Int width = this->Width();
        const Int localHeight = this->LocalHeight();
        const Int localHeightOfA = A.LocalHeight();
        const Int localWidthOfA = A.LocalWidth();
        const Int maxLocalHeight = MaxLength(height,c);
        const Int maxLocalWidth = MaxLength(width,r);
        const Int portionSize = mpi::Pad( maxLocalHeight*maxLocalWidth );

        T* buffer = this->auxMemory_.Require( (r+1)*portionSize );
        T* firstBuf = &buffer[0];
        T* secondBuf = &buffer[portionSize];

        // Pack the currently owned local data of A into the second buffer
        const Int ALDim = A.LDim();
        const T* ABuffer = A.LockedBuffer();
        if( conjugate )
        {
            PARALLEL_FOR
            for( Int jLoc=0; jLoc<localHeightOfA; ++jLoc )
            {
                T* destCol = &secondBuf[jLoc*localWidthOfA];
                const T* sourceCol = &ABuffer[jLoc];
                for( Int iLoc=0; iLoc<localWidthOfA; ++iLoc )
                    destCol[iLoc] = Conj( sourceCol[iLoc*ALDim] );
            }
        }
        else
        {
            PARALLEL_FOR
            for( Int jLoc=0; jLoc<localHeightOfA; ++jLoc )
            {
                T* destCol = &secondBuf[jLoc*localWidthOfA];
                const T* sourceCol = &ABuffer[jLoc];
                for( Int iLoc=0; iLoc<localWidthOfA; ++iLoc )
                    destCol[iLoc] = sourceCol[iLoc*ALDim];
            }
        }

        // Perform the SendRecv: puts the new data into the first buffer
        mpi::SendRecv
        ( secondBuf, portionSize, sendCol,
          firstBuf,  portionSize, recvCol, g.RowComm() );

        // Use the output of the SendRecv as input to the AllGather
        mpi::AllGather
        ( firstBuf,  portionSize,
          secondBuf, portionSize, g.ColComm() );

        // Unpack the contents of each member of the process col
        T* thisBuffer = this->Buffer();
        const Int thisLDim = this->LDim();
        const Int colAlignOfA = A.ColAlign();
        OUTER_PARALLEL_FOR
        for( Int k=0; k<r; ++k )
        {
            const T* data = &secondBuf[k*portionSize];
            const Int rowShift = Shift_( k, colAlignOfA, r );
            const Int localWidth = Length_( width, rowShift, r );
            INNER_PARALLEL_FOR
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            {
                const T* dataCol = &data[jLoc*localHeight];
                T* thisCol = &thisBuffer[(rowShift+jLoc*r)*thisLDim];
                MemCopy( thisCol, dataCol, localHeight );
            }
        }
        this->auxMemory_.Release();
    }
}

template<typename T>
void
DM<T>::AdjointFrom( const DistMatrix<T,MC,MR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MR,* ]::AdjointFrom"))
    this->TransposeFrom( A, true );
}

// Basic queries
// =============

template<typename T>
elem::DistData DM<T>::DistData() const { return elem::DistData(*this); }

template<typename T>
mpi::Comm DM<T>::DistComm() const { return this->grid_->MRComm(); }
template<typename T>
mpi::Comm DM<T>::CrossComm() const { return mpi::COMM_SELF; }
template<typename T>
mpi::Comm DM<T>::RedundantComm() const { return this->grid_->MCComm(); }
template<typename T>
mpi::Comm DM<T>::ColComm() const { return this->grid_->MRComm(); }
template<typename T>
mpi::Comm DM<T>::RowComm() const { return mpi::COMM_SELF; }

template<typename T>
Int DM<T>::ColStride() const { return this->grid_->Width(); }
template<typename T>
Int DM<T>::RowStride() const { return 1; }

// Instantiate {Int,Real,Complex<Real>} for each Real in {float,double}
// ####################################################################

#define PROTO(T) template class DistMatrix<T,MR,STAR>
#define COPY(T,U,V) \
  template DistMatrix<T,MR,STAR>::DistMatrix( const DistMatrix<T,U,V>& A )
#define FULL(T) \
  PROTO(T); \
  COPY(T,CIRC,CIRC); \
  COPY(T,MC,  MR); \
  COPY(T,MC,  STAR); \
  COPY(T,MD,  STAR); \
  COPY(T,MR,  MC  ); \
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
