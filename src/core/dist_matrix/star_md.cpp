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
using GDM = GeneralDistMatrix<T,STAR,MD>;
template<typename T>
using DM = DistMatrix<T,STAR,MD>;

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
DM<T>::DistMatrix
( Int height, Int width, Int rowAlign, Int root, const elem::Grid& g )
: GDM<T>(g)
{ 
    this->root_ = root;
    this->Align(0,rowAlign); 
    this->Resize(height,width); 
}

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int rowAlign, Int root, Int ldim, const elem::Grid& g )
: GDM<T>(g)
{ 
    this->root_ = root;
    this->Align(0,rowAlign);
    this->Resize(height,width,ldim);
}

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int rowAlign, Int root, const T* buffer, Int ldim,
  const elem::Grid& g )
: GDM<T>(g)
{ this->LockedAttach(height,width,0,rowAlign,buffer,ldim,g,root); }

template<typename T>
DM<T>::DistMatrix
( Int height, Int width, Int rowAlign, Int root, T* buffer, Int ldim,
  const elem::Grid& g )
: GDM<T>(g)
{ this->Attach(height,width,0,rowAlign,buffer,ldim,g,root); }

template<typename T>
DM<T>::DistMatrix( const DM<T>& A )
: GDM<T>(A.Grid())
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD]::DistMatrix"))
    this->SetShifts();
    if( &A != this )
        *this = A;
    else
        LogicError("Tried to construct [* ,MD] with itself");
}

template<typename T>
template<Dist U,Dist V>
DM<T>::DistMatrix( const DistMatrix<T,U,V>& A )
: GDM<T>(A.Grid())
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD]::DistMatrix"))
    this->SetShifts();
    if( STAR != U || MD != V || 
        reinterpret_cast<const DM<T>*>(&A) != this )
        *this = A;
    else
        LogicError("Tried to construct [* ,MD] with itself");
}

template<typename T> DM<T>::DistMatrix( DM<T>&& A ) : GDM<T>(std::move(A)) { }

template<typename T> DM<T>::~DistMatrix() { }

// Assignment and reconfiguration
// ==============================

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MC,MR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [MC,MR]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MC,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [MC,* ]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,MR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [* ,MR]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MD,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [MD,* ]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DM<T>& A )
{
    DEBUG_ONLY(
        CallStackEntry cse("[* ,MD] = [* ,MD]");
        this->AssertNotLocked();
        this->AssertSameGrid( A.Grid() );
    )
    if( !this->Viewing() && !this->RowConstrained() )
    {
        this->SetRoot( A.root_ );
        this->AlignRows( A.rowAlign_ );
    }
    this->Resize( A.Height(), A.Width() );

    if( this->root_ == A.root_ && 
        this->rowAlign_ == A.rowAlign_ )
    {
        this->matrix_ = A.LockedMatrix();
    }
    else
    {
#ifdef UNALIGNED_WARNINGS
        if( this->Grid().Rank() == 0 )
            std::cerr << "Unaligned [* ,MD] <- [* ,MD]." << std::endl;
#endif
        // TODO: More efficient implementation?
        DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
        *this = A_STAR_STAR;
    }
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MR,MC>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [MR,MC]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,MR,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [MR,* ]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,MC>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [* ,MC]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,VC,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [VC,* ]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,VC>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [* ,VC]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,VR,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [VR,* ]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,VR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [* ,VR]"))
    // TODO: More efficient implementation?
    DistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,STAR,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [* ,* ]"))
    this->RowFilterFrom( A );
    return *this;
}

template<typename T>
const DM<T>&
DM<T>::operator=( const DistMatrix<T,CIRC,CIRC>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[* ,MD] = [o ,o ]"))
    DistMatrix<T,MC,MR> A_MC_MR( A.Grid() );
    A_MC_MR.AlignWith( *this );
    A_MC_MR = A;
    *this = A_MC_MR;
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
    DEBUG_ONLY(CallStackEntry cse("[* ,MD]::AlignWith"))
    this->SetGrid( *data.grid );

    if( data.colDist == MD && data.rowDist == STAR )
    {
        this->SetRoot( data.root );
        this->AlignRows( data.colAlign );
    }
    else if( data.colDist == STAR && data.rowDist == MD )
    {
        this->SetRoot( data.root );
        this->AlignRows( data.rowAlign );
    }
    DEBUG_ONLY(else LogicError("Invalid alignment"))
}

template<typename T>
void
DM<T>::AlignRowsWith( const elem::DistData& data )
{ this->AlignWith( data ); }

// Basic queries
// =============

template<typename T>
elem::DistData DM<T>::DistData() const { return elem::DistData(*this); }

template<typename T>
mpi::Comm DM<T>::DistComm() const { return this->grid_->MDComm(); }
template<typename T>
mpi::Comm DM<T>::CrossComm() const { return this->grid_->MDPerpComm(); }
template<typename T>
mpi::Comm DM<T>::RedundantComm() const { return mpi::COMM_SELF; }
template<typename T>
mpi::Comm DM<T>::ColComm() const { return mpi::COMM_SELF; }
template<typename T>
mpi::Comm DM<T>::RowComm() const { return this->grid_->MDComm(); }

template<typename T>
Int DM<T>::ColStride() const { return 1; }
template<typename T>
Int DM<T>::RowStride() const { return this->grid_->LCM(); }

// Private section
// ###############

// Exchange metadata with another matrix
// =====================================

template<typename T>
void DM<T>::ShallowSwap( DM<T>& A ) { GDM<T>::ShallowSwap( A ); }

// Instantiate {Int,Real,Complex<Real>} for each Real in {float,double}
// ####################################################################

#define PROTO(T) template class DistMatrix<T,STAR,MD>
#define COPY(T,U,V) \
  template DistMatrix<T,STAR,MD>::DistMatrix( const DistMatrix<T,U,V>& A )
#define FULL(T) \
  PROTO(T); \
  COPY(T,CIRC,CIRC); \
  COPY(T,MC,  MR  ); \
  COPY(T,MC,  STAR); \
  COPY(T,MD,  STAR); \
  COPY(T,MR,  MC  ); \
  COPY(T,MR,  STAR); \
  COPY(T,STAR,MC  ); \
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
