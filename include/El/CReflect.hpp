/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_CREFLECT_C_HPP
#define EL_CREFLECT_C_HPP

#define EL_CATCH \
  catch( std::bad_alloc& e ) \
  { El::ReportException(e); return EL_ALLOC_ERROR; } \
  catch( El::ArgException& e ) \
  { El::ReportException(e); return EL_ARG_ERROR; } \
  catch( std::logic_error& e ) \
  { El::ReportException(e); return EL_LOGIC_ERROR; } \
  catch( std::runtime_error& e ) \
  { El::ReportException(e); return EL_RUNTIME_ERROR; } \
  catch( std::exception& e ) \
  { El::ReportException(e); return EL_ERROR; }

#define EL_TRY(payload) \
  try { payload; } EL_CATCH \
  return EL_SUCCESS;

#define EL_RC(TYPE,INPUT) reinterpret_cast<TYPE>(INPUT)

namespace El {

template<typename T>
struct CReflectType { typedef T type; };

// ElInt and Int are typedef's
/*
template<> struct CReflectType<ElInt> { typedef Int type; };
template<> struct CReflectType<Int> { typedef ElInt type; };
*/

template<> struct CReflectType<complex_float> 
{ typedef Complex<float> type; };
template<> struct CReflectType<complex_double> 
{ typedef Complex<double> type; };

template<> struct CReflectType<Complex<float>> 
{ typedef complex_float type; };
template<> struct CReflectType<Complex<double>> 
{ typedef complex_double type; };

#define CREFLECT(T) typename CReflectType<T>::type

template<typename T>
inline void DynamicCastCheck( T* A )
{ if( A == nullptr ) RuntimeError("Dynamic cast failed"); }

inline std::string CReflect( const char* name )
{ return std::string(name); }
// NOTE: This creates a deep copy and the pointer should be deleted later
inline char* CReflect( const std::string& name ) 
{
    const auto size = name.size();
    char* buffer = new char[size+1];
    memcpy( buffer, name.c_str(), size+1 );
    return buffer;
}

inline Orientation CReflect( ElOrientation orient ) 
{ return static_cast<Orientation>(orient); }
inline ElOrientation CReflect( Orientation orient )
{ return static_cast<ElOrientation>(orient); }

inline LeftOrRight CReflect( ElLeftOrRight side )
{ return static_cast<LeftOrRight>(side); }
inline ElLeftOrRight CReflect( LeftOrRight side )
{ return static_cast<ElLeftOrRight>(side); }

inline UpperOrLower CReflect( ElUpperOrLower uplo )
{ return static_cast<UpperOrLower>(uplo); }
inline ElUpperOrLower CReflect( UpperOrLower uplo )
{ return static_cast<ElUpperOrLower>(uplo); }

inline UnitOrNonUnit CReflect( ElUnitOrNonUnit diag )
{ return static_cast<UnitOrNonUnit>(diag); }
inline ElUnitOrNonUnit CReflect( UnitOrNonUnit diag )
{ return static_cast<ElUnitOrNonUnit>(diag); }

inline VerticalOrHorizontal CReflect( ElVerticalOrHorizontal dir )
{ return static_cast<VerticalOrHorizontal>(dir); }
inline ElVerticalOrHorizontal CReflect( VerticalOrHorizontal dir )
{ return static_cast<ElVerticalOrHorizontal>(dir); }

inline ForwardOrBackward CReflect( ElForwardOrBackward order )
{ return static_cast<ForwardOrBackward>(order); }
inline ElForwardOrBackward CReflect( ForwardOrBackward order )
{ return static_cast<ElForwardOrBackward>(order); }

inline Conjugation CReflect( ElConjugation conjugation )
{ return static_cast<Conjugation>(conjugation); }
inline ElConjugation CReflect( Conjugation conjugation )
{ return static_cast<ElConjugation>(conjugation); }

// Dist
// ----
inline Dist   CReflect( ElDist dist ) { return static_cast<  Dist>(dist); }
inline ElDist CReflect(   Dist dist ) { return static_cast<ElDist>(dist); }

// Grid
// ----
inline   GridOrder     CReflect( ElGridOrderType order )
{ return static_cast<  GridOrder    >(order); }
inline ElGridOrderType CReflect(   GridOrder     order )
{ return static_cast<ElGridOrderType>(order); }

inline Grid* CReflect( ElGrid grid )
{ return EL_RC(Grid*,grid); }
inline ElGrid CReflect( Grid* grid )
{ return (ElGrid)EL_RC(struct ElGrid_sDummy*,grid); }

inline const Grid* CReflect( ElConstGrid grid )
{ return EL_RC(const Grid*,grid); }
inline ElConstGrid CReflect( const Grid* grid )
{ return (ElConstGrid)EL_RC(const struct ElGrid_sDummy*,grid); }

// Complex<T>
// ----------
inline complex_float* CReflect( Complex<float>* buffer )
{ return EL_RC(complex_float*,buffer); }

inline complex_double* CReflect( Complex<double>* buffer )
{ return EL_RC(complex_double*,buffer); }

inline const complex_float* CReflect( const Complex<float>* buffer )
{ return EL_RC(const complex_float*,buffer); }

inline const complex_double* CReflect( const Complex<double>* buffer )
{ return EL_RC(const complex_double*,buffer); }

inline Complex<float>* CReflect( complex_float* buffer )
{ return EL_RC(Complex<float>*,buffer); }

inline Complex<double>* CReflect( complex_double* buffer )
{ return EL_RC(Complex<double>*,buffer); }

inline const Complex<float>* CReflect( const complex_float* buffer )
{ return EL_RC(const Complex<float>*,buffer); }

inline const Complex<double>* CReflect( const complex_double* buffer )
{ return EL_RC(const Complex<double>*,buffer); }

inline Complex<float> CReflect( complex_float alpha )
{ return Complex<float>(alpha.real,alpha.imag); }

inline Complex<double> CReflect( complex_double alpha )
{ return Complex<double>(alpha.real,alpha.imag); }

inline complex_float CReflect( Complex<float> alpha )
{ complex_float beta; beta.real = alpha.real(); beta.imag = alpha.imag();
  return beta; }

inline complex_double CReflect( Complex<double> alpha )
{ complex_double beta; beta.real = alpha.real(); beta.imag = alpha.imag();
  return beta; }

// Analogues for real variables and integers
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
inline Int CReflect( Int alpha ) { return alpha; }
inline Int* CReflect( Int* buffer ) { return buffer; }
inline const Int* CReflect( const Int* buffer ) { return buffer; }
/*
inline ElInt CReflect( Int alpha ) { return alpha; }

inline ElInt* CReflect( Int*   buffer ) { return buffer; }
inline Int*   CReflect( ElInt* buffer ) { return buffer; }

inline const ElInt* CReflect( const Int*   buffer ) { return buffer; }
inline const Int*   CReflect( const ElInt* buffer ) { return buffer; }
*/

inline float CReflect( float alpha) { return alpha; }
inline double CReflect( double alpha ) { return alpha; }

inline float* CReflect( float* buffer ) { return buffer; }
inline double* CReflect( double* buffer ) { return buffer; }

inline const float* CReflect( const float* buffer ) { return buffer; }
inline const double* CReflect( const double* buffer ) { return buffer; }

inline ValueInt<Int> CReflect( ElValueInt_i entryC )
{ return {entryC.value,entryC.index}; }
inline ElValueInt_i CReflect( ValueInt<Int> entry )
{ return {entry.value,entry.index}; }

inline ValueInt<Int>* CReflect( ElValueInt_i* entryC )
{ return EL_RC(ValueInt<Int>*,entryC); }
inline ElValueInt_i* CReflect( ValueInt<Int>* entryC )
{ return EL_RC(ElValueInt_i*,entryC); }

inline ValueInt<float> CReflect( ElValueInt_s entryC )
{ return {entryC.value,entryC.index}; }
inline ElValueInt_s CReflect( ValueInt<float> entry )
{ return {entry.value,entry.index}; }

inline ValueInt<float>* CReflect( ElValueInt_s* entryC )
{ return EL_RC(ValueInt<float>*,entryC); }
inline ElValueInt_s* CReflect( ValueInt<float>* entryC )
{ return EL_RC(ElValueInt_s*,entryC); }

inline ValueInt<double> CReflect( ElValueInt_d entryC )
{ return {entryC.value,entryC.index}; }
inline ElValueInt_d CReflect( ValueInt<double> entry )
{ return {entry.value,entry.index}; }

inline ValueInt<double>* CReflect( ElValueInt_d* entryC )
{ return EL_RC(ValueInt<double>*,entryC); }
inline ElValueInt_d* CReflect( ValueInt<double>* entryC )
{ return EL_RC(ElValueInt_d*,entryC); }

inline ValueInt<Complex<float>> CReflect( ElValueInt_c entryC )
{ return {CReflect(entryC.value),entryC.index}; }
inline ElValueInt_c CReflect( ValueInt<Complex<float>> entry )
{ return {CReflect(entry.value),entry.index}; }

inline ValueInt<Complex<float>>* CReflect( ElValueInt_c* entryC )
{ return EL_RC(ValueInt<Complex<float>>*,entryC); }
inline ElValueInt_c* CReflect( ValueInt<Complex<float>>* entryC )
{ return EL_RC(ElValueInt_c*,entryC); }

inline ValueInt<Complex<double>> CReflect( ElValueInt_z entryC )
{ return {CReflect(entryC.value),entryC.index}; }
inline ElValueInt_z CReflect( ValueInt<Complex<double>> entry )
{ return {CReflect(entry.value),entry.index}; }

inline ValueInt<Complex<double>>* CReflect( ElValueInt_z* entryC )
{ return EL_RC(ValueInt<Complex<double>>*,entryC); }
inline ElValueInt_z* CReflect( ValueInt<Complex<double>>* entryC )
{ return EL_RC(ElValueInt_z*,entryC); }

inline ValueIntPair<Int> CReflect( ElValueIntPair_i entryC )
{ return {entryC.value,{entryC.indices[0],entryC.indices[1]}}; }
inline ElValueIntPair_i CReflect( ValueIntPair<Int> entry )
{ return {entry.value,{entry.indices[0],entry.indices[1]}}; }

inline ValueIntPair<float> CReflect( ElValueIntPair_s entryC )
{ return {entryC.value,{entryC.indices[0],entryC.indices[1]}}; }
inline ElValueIntPair_s CReflect( ValueIntPair<float> entry )
{ return {entry.value,{entry.indices[0],entry.indices[1]}}; }

inline ValueIntPair<double> CReflect( ElValueIntPair_d entryC )
{ return {entryC.value,{entryC.indices[0],entryC.indices[1]}}; }
inline ElValueIntPair_d CReflect( ValueIntPair<double> entry )
{ return {entry.value,{entry.indices[0],entry.indices[1]}}; }

inline ValueIntPair<Complex<float>> CReflect( ElValueIntPair_c entryC ) { return {CReflect(entryC.value),{entryC.indices[0],entryC.indices[1]}}; }
inline ElValueIntPair_c CReflect( ValueIntPair<Complex<float>> entry )
{ return {CReflect(entry.value),{entry.indices[0],entry.indices[1]}}; }

inline ValueIntPair<Complex<double>> CReflect( ElValueIntPair_z entryC )
{ return {CReflect(entryC.value),{entryC.indices[0],entryC.indices[1]}}; }
inline ElValueIntPair_z CReflect( ValueIntPair<Complex<double>> entry )
{ return {CReflect(entry.value),{entry.indices[0],entry.indices[1]}}; }

// Matrix
// ------
inline Matrix<Int>* CReflect( ElMatrix_i A )
{ return EL_RC(Matrix<Int>*,A); }

inline Matrix<float>* CReflect( ElMatrix_s A )
{ return EL_RC(Matrix<float>*,A); }

inline Matrix<double>* CReflect( ElMatrix_d A )
{ return EL_RC(Matrix<double>*,A); }

inline Matrix<Complex<float>>* CReflect( ElMatrix_c A )
{ return EL_RC(Matrix<Complex<float>>*,A); }

inline Matrix<Complex<double>>* CReflect( ElMatrix_z A )
{ return EL_RC(Matrix<Complex<double>>*,A); }

inline const Matrix<Int>* CReflect( ElConstMatrix_i A )
{ return EL_RC(const Matrix<Int>*,A); }

inline const Matrix<float>* CReflect( ElConstMatrix_s A )
{ return EL_RC(const Matrix<float>*,A); }

inline const Matrix<double>* CReflect( ElConstMatrix_d A )
{ return EL_RC(const Matrix<double>*,A); }

inline const Matrix<Complex<float>>* CReflect( ElConstMatrix_c A )
{ return EL_RC(const Matrix<Complex<float>>*,A); }

inline const Matrix<Complex<double>>* CReflect( ElConstMatrix_z A )
{ return EL_RC(const Matrix<Complex<double>>*,A); }

inline ElMatrix_i CReflect( Matrix<Int>* A )
{ return (ElMatrix_i)EL_RC(struct ElMatrix_iDummy*,A); }

inline ElMatrix_s CReflect( Matrix<float>* A )
{ return (ElMatrix_s)EL_RC(struct ElMatrix_sDummy*,A); }

inline ElMatrix_d CReflect( Matrix<double>* A )
{ return (ElMatrix_d)EL_RC(struct ElMatrix_dDummy*,A); }

inline ElMatrix_c CReflect( Matrix<Complex<float>>* A )
{ return (ElMatrix_c)EL_RC(struct ElMatrix_cDummy*,A); }

inline ElMatrix_z CReflect( Matrix<Complex<double>>* A )
{ return (ElMatrix_z)EL_RC(struct ElMatrix_zDummy*,A); }

inline ElConstMatrix_i CReflect( const Matrix<Int>* A )
{ return (ElConstMatrix_i)EL_RC(const struct ElMatrix_iDummy*,A); }

inline ElConstMatrix_s CReflect( const Matrix<float>* A )
{ return (ElConstMatrix_s)EL_RC(const struct ElMatrix_sDummy*,A); }

inline ElConstMatrix_d CReflect( const Matrix<double>* A )
{ return (ElConstMatrix_d)EL_RC(const struct ElMatrix_dDummy*,A); }

inline ElConstMatrix_c CReflect( const Matrix<Complex<float>>* A )
{ return (ElConstMatrix_c)EL_RC(const struct ElMatrix_cDummy*,A); }

inline ElConstMatrix_z CReflect( const Matrix<Complex<double>>* A )
{ return (ElConstMatrix_z)EL_RC(const struct ElMatrix_zDummy*,A); }

// AbstractDistMatrix
// ------------------
inline AbstractDistMatrix<Int>* 
CReflect( ElDistMatrix_i A )
{ return EL_RC(AbstractDistMatrix<Int>*,A); }

inline AbstractDistMatrix<float>* 
CReflect( ElDistMatrix_s A )
{ return EL_RC(AbstractDistMatrix<float>*,A); }

inline AbstractDistMatrix<double>* 
CReflect( ElDistMatrix_d A )
{ return EL_RC(AbstractDistMatrix<double>*,A); }

inline AbstractDistMatrix<Complex<float>>* 
CReflect( ElDistMatrix_c A )
{ return EL_RC(AbstractDistMatrix<Complex<float>>*,A); }

inline AbstractDistMatrix<Complex<double>>* 
CReflect( ElDistMatrix_z A )
{ return EL_RC(AbstractDistMatrix<Complex<double>>*,A); }

inline const AbstractDistMatrix<Int>* 
CReflect( ElConstDistMatrix_i A )
{ return EL_RC(const AbstractDistMatrix<Int>*,A); }

inline const AbstractDistMatrix<float>* 
CReflect( ElConstDistMatrix_s A )
{ return EL_RC(const AbstractDistMatrix<float>*,A); }

inline const AbstractDistMatrix<double>* 
CReflect( ElConstDistMatrix_d A )
{ return EL_RC(const AbstractDistMatrix<double>*,A); }

inline const AbstractDistMatrix<Complex<float>>* 
CReflect( ElConstDistMatrix_c A )
{ return EL_RC(const AbstractDistMatrix<Complex<float>>*,A); }

inline const AbstractDistMatrix<Complex<double>>* 
CReflect( ElConstDistMatrix_z A )
{ return EL_RC(const AbstractDistMatrix<Complex<double>>*,A); }

inline ElDistMatrix_i
CReflect( AbstractDistMatrix<Int>* A )
{ return (ElDistMatrix_i)EL_RC(struct ElDistMatrix_iDummy*,A); }

inline ElDistMatrix_s 
CReflect( AbstractDistMatrix<float>* A )
{ return (ElDistMatrix_s)EL_RC(struct ElDistMatrix_sDummy*,A); }

inline ElDistMatrix_d 
CReflect( AbstractDistMatrix<double>* A )
{ return (ElDistMatrix_d)EL_RC(struct ElDistMatrix_dDummy*,A); }

inline ElDistMatrix_c 
CReflect( AbstractDistMatrix<Complex<float>>* A )
{ return (ElDistMatrix_c)EL_RC(struct ElDistMatrix_cDummy*,A); }

inline ElDistMatrix_z 
CReflect( AbstractDistMatrix<Complex<double>>* A )
{ return (ElDistMatrix_z)EL_RC(struct ElDistMatrix_zDummy*,A); }

inline ElConstDistMatrix_i
CReflect( const AbstractDistMatrix<Int>* A )
{ return (ElConstDistMatrix_i)EL_RC(const struct ElDistMatrix_iDummy*,A); }

inline ElConstDistMatrix_s 
CReflect( const AbstractDistMatrix<float>* A )
{ return (ElConstDistMatrix_s)EL_RC(const struct ElDistMatrix_sDummy*,A); }

inline ElConstDistMatrix_d 
CReflect( const AbstractDistMatrix<double>* A )
{ return (ElConstDistMatrix_d)EL_RC(const struct ElDistMatrix_dDummy*,A); }

inline ElConstDistMatrix_c 
CReflect( const AbstractDistMatrix<Complex<float>>* A )
{ return (ElConstDistMatrix_c)EL_RC(const struct ElDistMatrix_cDummy*,A); }

inline ElConstDistMatrix_z 
CReflect( const AbstractDistMatrix<Complex<double>>* A )
{ return (ElConstDistMatrix_z)EL_RC(const struct ElDistMatrix_zDummy*,A); }

inline ElDistData CReflect( const DistData& data )
{
    ElDistData distData;
    distData.colDist = CReflect(data.colDist);
    distData.rowDist = CReflect(data.rowDist);
    distData.colAlign = data.colAlign;
    distData.rowAlign = data.rowAlign;
    distData.root = data.root;
    distData.grid = CReflect(data.grid);
    return distData;
}

inline DistData CReflect( const ElDistData& distData )
{
    DistData data;
    data.colDist = CReflect(distData.colDist);
    data.rowDist = CReflect(distData.rowDist);
    data.colAlign = distData.colAlign;
    data.rowAlign = distData.rowAlign;
    data.root = distData.root;
    data.grid = CReflect(distData.grid);
    return data;
}

inline ElSafeProduct_s CReflect( const SafeProduct<float>& prod )
{ 
    ElSafeProduct_s prodC;    
    prodC.rho = prod.rho;
    prodC.kappa = prod.kappa;
    prodC.n = prod.n;
    return prodC;
}
inline ElSafeProduct_d CReflect( const SafeProduct<double>& prod )
{ 
    ElSafeProduct_d prodC;    
    prodC.rho = prod.rho;
    prodC.kappa = prod.kappa;
    prodC.n = prod.n;
    return prodC;
}
inline ElSafeProduct_c CReflect( const SafeProduct<Complex<float>>& prod )
{ 
    ElSafeProduct_c prodC;    
    prodC.rho = CReflect(prod.rho);
    prodC.kappa = prod.kappa;
    prodC.n = prod.n;
    return prodC;
}
inline ElSafeProduct_z CReflect( const SafeProduct<Complex<double>>& prod )
{ 
    ElSafeProduct_z prodC;    
    prodC.rho = CReflect(prod.rho);
    prodC.kappa = prod.kappa;
    prodC.n = prod.n;
    return prodC;
}

inline SafeProduct<float> CReflect( const ElSafeProduct_s& prodC )
{ 
    SafeProduct<float> prod( prodC.n );
    prod.rho = prodC.rho;
    prod.kappa = prodC.kappa;
    return prod;
}
inline SafeProduct<double> CReflect( const ElSafeProduct_d& prodC )
{ 
    SafeProduct<double> prod( prodC.n );
    prod.rho = prodC.rho;
    prod.kappa = prodC.kappa;
    return prod;
}
inline SafeProduct<Complex<float>> CReflect( const ElSafeProduct_c& prodC )
{ 
    SafeProduct<Complex<float>> prod( prodC.n );
    prod.rho = CReflect(prodC.rho);
    prod.kappa = prodC.kappa;
    return prod;
}
inline SafeProduct<Complex<double>> CReflect( const ElSafeProduct_z& prodC )
{ 
    SafeProduct<Complex<double>> prod( prodC.n );
    prod.rho = CReflect(prodC.rho);
    prod.kappa = prodC.kappa;
    return prod;
}

// Input/Output
// ------------
inline ElFileFormat CReflect( FileFormat format )
{ return static_cast<ElFileFormat>(format); }
inline FileFormat CReflect( ElFileFormat format )
{ return static_cast<FileFormat>(format); }

inline ElColorMap CReflect( ColorMap map )
{ return static_cast<ElColorMap>(map); }
inline ColorMap CReflect( ElColorMap map )
{ return static_cast<ColorMap>(map); }

// BLAS-like
// ---------
inline ElGemmAlgorithm CReflect( GemmAlgorithm alg )
{ return static_cast<ElGemmAlgorithm>(alg); }
inline GemmAlgorithm CReflect( ElGemmAlgorithm alg )
{ return static_cast<GemmAlgorithm>(alg); }

// LAPACK-like
// -----------

inline ElSortType CReflect( SortType type )
{ return static_cast<ElSortType>(type); }

inline SortType CReflect( ElSortType type )
{ return static_cast<SortType>(type); }

// Permutations
// ^^^^^^^^^^^^

inline ElPermutationMeta CReflect( const PermutationMeta& meta )
{
    ElPermutationMeta metaC;    

    metaC.align = meta.align;
    metaC.comm = meta.comm.comm;

    const Int commSize = mpi::Size( meta.comm );
    metaC.sendCounts = new int[commSize];
    metaC.sendDispls = new int[commSize];
    metaC.recvCounts = new int[commSize];
    metaC.recvDispls = new int[commSize];
    MemCopy( metaC.sendCounts, meta.sendCounts.data(), commSize );
    MemCopy( metaC.sendDispls, meta.sendDispls.data(), commSize ); 
    MemCopy( metaC.recvCounts, meta.recvCounts.data(), commSize );
    MemCopy( metaC.recvDispls, meta.recvDispls.data(), commSize );

    metaC.numSendIdx = meta.sendIdx.size();
    metaC.numRecvIdx = meta.recvIdx.size();
    metaC.sendIdx   = new int[metaC.numSendIdx];
    metaC.sendRanks = new int[metaC.numSendIdx];
    metaC.recvIdx   = new int[metaC.numRecvIdx];
    metaC.recvRanks = new int[metaC.numRecvIdx];
    MemCopy( metaC.sendIdx,   meta.sendIdx.data(),   metaC.numSendIdx );
    MemCopy( metaC.sendRanks, meta.sendRanks.data(), metaC.numSendIdx );
    MemCopy( metaC.recvIdx,   meta.recvIdx.data(),   metaC.numRecvIdx );
    MemCopy( metaC.recvRanks, meta.recvRanks.data(), metaC.numRecvIdx );

    return metaC;
}

inline PermutationMeta CReflect( const ElPermutationMeta& metaC )
{
    PermutationMeta meta;

    meta.align = metaC.align;
    meta.comm = metaC.comm;

    int commSize;
    MPI_Comm_size( metaC.comm, &commSize );
    meta.sendCounts = 
        std::vector<int>( metaC.sendCounts, metaC.sendCounts+commSize );
    meta.sendDispls = 
        std::vector<int>( metaC.sendDispls, metaC.sendDispls+commSize );
    meta.recvCounts =
        std::vector<int>( metaC.recvCounts, metaC.recvCounts+commSize );
    meta.recvDispls =
        std::vector<int>( metaC.recvDispls, metaC.recvDispls+commSize );

    meta.sendIdx = 
        std::vector<int>( metaC.sendIdx, metaC.sendIdx+metaC.numSendIdx );
    meta.sendRanks =
        std::vector<int>( metaC.sendRanks, metaC.sendRanks+metaC.numSendIdx );
    meta.recvIdx =
        std::vector<int>( metaC.recvIdx, metaC.recvIdx+metaC.numRecvIdx );
    meta.recvRanks =
        std::vector<int>( metaC.recvRanks, metaC.recvRanks+metaC.numRecvIdx );

    return meta;
}

// Condensed form
// ^^^^^^^^^^^^^^
inline ElHermitianTridiagApproach 
CReflect( HermitianTridiagApproach approach )
{ return static_cast<ElHermitianTridiagApproach>( approach ); }

inline HermitianTridiagApproach 
CReflect( ElHermitianTridiagApproach approach )
{ return static_cast<HermitianTridiagApproach>( approach ); }

inline ElHermitianTridiagCtrl
CReflect( const HermitianTridiagCtrl& ctrl )
{ 
    ElHermitianTridiagCtrl ctrlC;
    ctrlC.approach = CReflect(ctrl.approach);
    ctrlC.order = CReflect(ctrl.order);
    return ctrlC;
}

inline HermitianTridiagCtrl
CReflect( const ElHermitianTridiagCtrl& ctrlC )
{ 
    HermitianTridiagCtrl ctrl;
    ctrl.approach = CReflect(ctrlC.approach);
    ctrl.order = CReflect(ctrlC.order);
    return ctrl;
}

// Decompositions
// ^^^^^^^^^^^^^^

/* Pencil */
inline ElPencil CReflect( Pencil pencil )
{ return static_cast<ElPencil>(pencil); }

inline Pencil CReflect( ElPencil pencil )
{ return static_cast<Pencil>(pencil); }

/* HermitianSDCCtrl */
inline ElHermitianSDCCtrl_s CReflect( const HermitianSDCCtrl<float>& ctrl )
{
    ElHermitianSDCCtrl_s ctrlC;
    ctrlC.cutoff = ctrl.cutoff;
    ctrlC.maxInnerIts = ctrl.maxInnerIts;
    ctrlC.maxOuterIts = ctrl.maxOuterIts;
    ctrlC.tol = ctrl.tol;
    ctrlC.spreadFactor = ctrl.spreadFactor;
    ctrlC.progress = ctrl.progress;
    return ctrlC;
}
inline ElHermitianSDCCtrl_d CReflect( const HermitianSDCCtrl<double>& ctrl )
{
    ElHermitianSDCCtrl_d ctrlC;
    ctrlC.cutoff = ctrl.cutoff;
    ctrlC.maxInnerIts = ctrl.maxInnerIts;
    ctrlC.maxOuterIts = ctrl.maxOuterIts;
    ctrlC.tol = ctrl.tol;
    ctrlC.spreadFactor = ctrl.spreadFactor;
    ctrlC.progress = ctrl.progress;
    return ctrlC;
}

inline HermitianSDCCtrl<float> CReflect( const ElHermitianSDCCtrl_s& ctrlC )
{
    HermitianSDCCtrl<float> ctrl;
    ctrl.cutoff = ctrlC.cutoff;
    ctrl.maxInnerIts = ctrlC.maxInnerIts;
    ctrl.maxOuterIts = ctrlC.maxOuterIts;
    ctrl.tol = ctrlC.tol;
    ctrl.spreadFactor = ctrlC.spreadFactor;
    ctrl.progress = ctrlC.progress;
    return ctrl;
}
inline HermitianSDCCtrl<double> CReflect( const ElHermitianSDCCtrl_d& ctrlC )
{
    HermitianSDCCtrl<double> ctrl;
    ctrl.cutoff = ctrlC.cutoff;
    ctrl.maxInnerIts = ctrlC.maxInnerIts;
    ctrl.maxOuterIts = ctrlC.maxOuterIts;
    ctrl.tol = ctrlC.tol;
    ctrl.spreadFactor = ctrlC.spreadFactor;
    ctrl.progress = ctrlC.progress;
    return ctrl;
}

/* HermitianEigSubset */
inline ElHermitianEigSubset_s CReflect
( const HermitianEigSubset<float>& subset )
{
    ElHermitianEigSubset_s subsetC;
    subsetC.indexSubset = subset.indexSubset;
    subsetC.lowerIndex = subset.lowerIndex;
    subsetC.upperIndex = subset.upperIndex;
    subsetC.rangeSubset = subset.rangeSubset;
    subsetC.lowerBound = subset.lowerBound;
    subsetC.upperBound = subset.upperBound;
    return subsetC;
}
inline ElHermitianEigSubset_d CReflect
( const HermitianEigSubset<double>& subset )
{
    ElHermitianEigSubset_d subsetC;
    subsetC.indexSubset = subset.indexSubset;
    subsetC.lowerIndex = subset.lowerIndex;
    subsetC.upperIndex = subset.upperIndex;
    subsetC.rangeSubset = subset.rangeSubset;
    subsetC.lowerBound = subset.lowerBound;
    subsetC.upperBound = subset.upperBound;
    return subsetC;
}

inline HermitianEigSubset<float> CReflect
( const ElHermitianEigSubset_s& subsetC )
{
    HermitianEigSubset<float> subset;
    subset.indexSubset = subsetC.indexSubset;
    subset.lowerIndex = subsetC.lowerIndex;
    subset.upperIndex = subsetC.upperIndex;
    subset.rangeSubset = subsetC.rangeSubset;
    subset.lowerBound = subsetC.lowerBound;
    subset.upperBound = subsetC.upperBound;
    return subset;
}
inline HermitianEigSubset<double> CReflect
( const ElHermitianEigSubset_d& subsetC )
{
    HermitianEigSubset<double> subset;
    subset.indexSubset = subsetC.indexSubset;
    subset.lowerIndex = subsetC.lowerIndex;
    subset.upperIndex = subsetC.upperIndex;
    subset.rangeSubset = subsetC.rangeSubset;
    subset.lowerBound = subsetC.lowerBound;
    subset.upperBound = subsetC.upperBound;
    return subset;
}

/* HermitianEigCtrl */
inline ElHermitianEigCtrl_s CReflect( const HermitianEigCtrl<float>& ctrl )
{
    ElHermitianEigCtrl_s ctrlC;
    ctrlC.tridiagCtrl = CReflect( ctrl.tridiagCtrl );
    ctrlC.sdcCtrl = CReflect( ctrl.sdcCtrl );
    ctrlC.useSDC = ctrl.useSDC;
    return ctrlC;
}
inline ElHermitianEigCtrl_d CReflect( const HermitianEigCtrl<double>& ctrl )
{
    ElHermitianEigCtrl_d ctrlC;
    ctrlC.tridiagCtrl = CReflect( ctrl.tridiagCtrl );
    ctrlC.sdcCtrl = CReflect( ctrl.sdcCtrl );
    ctrlC.useSDC = ctrl.useSDC;
    return ctrlC;
}

inline HermitianEigCtrl<float> CReflect( const ElHermitianEigCtrl_s& ctrlC )
{
    HermitianEigCtrl<float> ctrl;
    ctrl.tridiagCtrl = CReflect( ctrlC.tridiagCtrl );
    ctrl.sdcCtrl = CReflect( ctrlC.sdcCtrl );
    ctrl.useSDC = ctrlC.useSDC;
    return ctrl;
}
inline HermitianEigCtrl<double> CReflect( const ElHermitianEigCtrl_d& ctrlC )
{
    HermitianEigCtrl<double> ctrl;
    ctrl.tridiagCtrl = CReflect( ctrlC.tridiagCtrl );
    ctrl.sdcCtrl = CReflect( ctrlC.sdcCtrl );
    ctrl.useSDC = ctrlC.useSDC;
    return ctrl;
}

/* PolarCtrl */
inline ElPolarCtrl CReflect( const PolarCtrl& ctrl )
{
    ElPolarCtrl ctrlC;
    ctrlC.qdwh = ctrl.qdwh;
    ctrlC.colPiv = ctrl.colPiv;
    ctrlC.maxIts = ctrl.maxIts;
    ctrlC.numIts = ctrl.numIts;
    return ctrlC;
}

inline PolarCtrl CReflect( const ElPolarCtrl& ctrlC )
{
    PolarCtrl ctrl;
    ctrl.qdwh = ctrlC.qdwh;
    ctrl.colPiv = ctrlC.colPiv;
    ctrl.maxIts = ctrlC.maxIts;
    ctrl.numIts = ctrlC.numIts;
    return ctrl;
}

/* SVDCtrl */
inline SVDCtrl<float> CReflect( const ElSVDCtrl_s& ctrlC )
{
    SVDCtrl<float> ctrl;
    ctrl.seqQR = ctrlC.seqQR;
    ctrl.valChanRatio = ctrlC.valChanRatio;
    ctrl.fullChanRatio = ctrlC.fullChanRatio;
    ctrl.thresholded = ctrlC.thresholded;
    ctrl.relative = ctrlC.relative;
    ctrl.tol = ctrlC.tol;
    return ctrl;
}

inline SVDCtrl<double> CReflect( const ElSVDCtrl_d& ctrlC )
{
    SVDCtrl<double> ctrl;
    ctrl.seqQR = ctrlC.seqQR;
    ctrl.valChanRatio = ctrlC.valChanRatio;
    ctrl.fullChanRatio = ctrlC.fullChanRatio;
    ctrl.thresholded = ctrlC.thresholded;
    ctrl.relative = ctrlC.relative;
    ctrl.tol = ctrlC.tol;
    return ctrl;
}

inline ElSVDCtrl_s CReflect( const SVDCtrl<float>& ctrl )
{
    ElSVDCtrl_s ctrlC;
    ctrlC.seqQR = ctrl.seqQR;
    ctrlC.valChanRatio = ctrl.valChanRatio;
    ctrlC.fullChanRatio = ctrl.fullChanRatio;
    ctrlC.thresholded = ctrl.thresholded;
    ctrlC.relative = ctrl.relative;
    ctrlC.tol = ctrl.tol;
    return ctrlC;
}

inline ElSVDCtrl_d CReflect( const SVDCtrl<double>& ctrl )
{
    ElSVDCtrl_d ctrlC;
    ctrlC.seqQR = ctrl.seqQR;
    ctrlC.valChanRatio = ctrl.valChanRatio;
    ctrlC.fullChanRatio = ctrl.fullChanRatio;
    ctrlC.thresholded = ctrl.thresholded;
    ctrlC.relative = ctrl.relative;
    ctrlC.tol = ctrl.tol;
    return ctrlC;
}

/* HessQRCtrl */
inline ElHessQRCtrl CReflect( const HessQRCtrl& ctrl )
{
    ElHessQRCtrl ctrlC;
    ctrlC.distAED = ctrl.distAED;
    ctrlC.blockHeight = ctrl.blockHeight;
    ctrlC.blockWidth = ctrl.blockWidth;
    return ctrlC;
}

inline HessQRCtrl CReflect( const ElHessQRCtrl& ctrlC )
{
    HessQRCtrl ctrl;
    ctrl.distAED = ctrlC.distAED;
    ctrl.blockHeight = ctrlC.blockHeight;
    ctrl.blockWidth = ctrlC.blockWidth;
    return ctrl;
}

/* SDCCtrl */
inline ElSDCCtrl_s CReflect( const SDCCtrl<float>& ctrl )
{
    ElSDCCtrl_s ctrlC;    
    ctrlC.cutoff = ctrl.cutoff;
    ctrlC.maxInnerIts = ctrl.maxInnerIts;
    ctrlC.maxOuterIts = ctrl.maxOuterIts;
    ctrlC.tol = ctrl.tol;
    ctrlC.spreadFactor = ctrl.spreadFactor;
    ctrlC.random = ctrl.random;
    ctrlC.progress = ctrl.progress;
    return ctrlC;
}
inline ElSDCCtrl_d CReflect( const SDCCtrl<double>& ctrl )
{
    ElSDCCtrl_d ctrlC;    
    ctrlC.cutoff = ctrl.cutoff;
    ctrlC.maxInnerIts = ctrl.maxInnerIts;
    ctrlC.maxOuterIts = ctrl.maxOuterIts;
    ctrlC.tol = ctrl.tol;
    ctrlC.spreadFactor = ctrl.spreadFactor;
    ctrlC.random = ctrl.random;
    ctrlC.progress = ctrl.progress;
    return ctrlC;
}

inline SDCCtrl<float> CReflect( const ElSDCCtrl_s& ctrlC )
{
    SDCCtrl<float> ctrl;
    ctrl.cutoff = ctrlC.cutoff;
    ctrl.maxInnerIts = ctrlC.maxInnerIts;
    ctrl.maxOuterIts = ctrlC.maxOuterIts;
    ctrl.tol = ctrlC.tol;
    ctrl.spreadFactor = ctrlC.spreadFactor;
    ctrl.random = ctrlC.random;
    ctrl.progress = ctrlC.progress;
    return ctrl;
}
inline SDCCtrl<double> CReflect( const ElSDCCtrl_d& ctrlC )
{
    SDCCtrl<double> ctrl;
    ctrl.cutoff = ctrlC.cutoff;
    ctrl.maxInnerIts = ctrlC.maxInnerIts;
    ctrl.maxOuterIts = ctrlC.maxOuterIts;
    ctrl.tol = ctrlC.tol;
    ctrl.spreadFactor = ctrlC.spreadFactor;
    ctrl.random = ctrlC.random;
    ctrl.progress = ctrlC.progress;
    return ctrl;
}

/* SchurCtrl */
inline ElSchurCtrl_s CReflect( const SchurCtrl<float>& ctrl )
{
    ElSchurCtrl_s ctrlC;
    ctrlC.useSDC = ctrl.useSDC;
    ctrlC.qrCtrl = CReflect( ctrl.qrCtrl );
    ctrlC.sdcCtrl = CReflect( ctrl.sdcCtrl );
    return ctrlC;
}
inline ElSchurCtrl_d CReflect( const SchurCtrl<double>& ctrl )
{
    ElSchurCtrl_d ctrlC;
    ctrlC.useSDC = ctrl.useSDC;
    ctrlC.qrCtrl = CReflect( ctrl.qrCtrl );
    ctrlC.sdcCtrl = CReflect( ctrl.sdcCtrl );
    return ctrlC;
}

inline SchurCtrl<float> CReflect( const ElSchurCtrl_s& ctrlC )
{
    SchurCtrl<float> ctrl;
    ctrl.useSDC = ctrlC.useSDC;
    ctrl.qrCtrl = CReflect( ctrlC.qrCtrl );
    ctrl.sdcCtrl = CReflect( ctrlC.sdcCtrl );
    return ctrl;
}
inline SchurCtrl<double> CReflect( const ElSchurCtrl_d& ctrlC )
{
    SchurCtrl<double> ctrl;
    ctrl.useSDC = ctrlC.useSDC;
    ctrl.qrCtrl = CReflect( ctrlC.qrCtrl );
    ctrl.sdcCtrl = CReflect( ctrlC.sdcCtrl );
    return ctrl;
}

// Factorizations
// ^^^^^^^^^^^^^^
inline ElLDLPivotType CReflect( LDLPivotType pivotType )
{ return static_cast<ElLDLPivotType>( pivotType ); }

inline LDLPivotType CReflect( ElLDLPivotType pivotType )
{ return static_cast<LDLPivotType>( pivotType ); }

inline ElLDLPivot CReflect( const LDLPivot& pivot )
{
    ElLDLPivot pivotC;
    pivotC.nb = pivot.nb;
    pivotC.from[0] = pivot.from[0];
    pivotC.from[1] = pivot.from[1];
    return pivotC;
}

inline LDLPivot CReflect( const ElLDLPivot& pivotC )
{
    LDLPivot pivot;
    pivot.nb = pivotC.nb;
    pivot.from[0] = pivotC.from[0];
    pivot.from[1] = pivotC.from[1];
    return pivot;
}

inline ElInertiaType CReflect( const InertiaType& inertia )
{ 
    ElInertiaType inertiaC;
    inertiaC.numPositive = inertia.numPositive;
    inertiaC.numNegative = inertia.numNegative;
    inertiaC.numZero = inertia.numZero;
    return inertiaC;
}

inline InertiaType CReflect( const ElInertiaType& inertiaC )
{ 
    InertiaType inertia;
    inertia.numPositive = inertiaC.numPositive;
    inertia.numNegative = inertiaC.numNegative;
    inertia.numZero = inertiaC.numZero;
    return inertia;
}

inline ElQRCtrl_s CReflect( const QRCtrl<float>& ctrl )
{ 
    ElQRCtrl_s ctrlC;
    ctrlC.colPiv = ctrl.colPiv;
    ctrlC.boundRank = ctrl.boundRank;
    ctrlC.maxRank = ctrl.maxRank;
    ctrlC.adaptive = ctrl.adaptive;
    ctrlC.tol = ctrl.tol;
    ctrlC.alwaysRecomputeNorms = ctrl.alwaysRecomputeNorms;
    return ctrlC;
}
inline ElQRCtrl_d CReflect( const QRCtrl<double>& ctrl )
{ 
    ElQRCtrl_d ctrlC;
    ctrlC.colPiv = ctrl.colPiv;
    ctrlC.boundRank = ctrl.boundRank;
    ctrlC.maxRank = ctrl.maxRank;
    ctrlC.adaptive = ctrl.adaptive;
    ctrlC.tol = ctrl.tol;
    ctrlC.alwaysRecomputeNorms = ctrl.alwaysRecomputeNorms;
    return ctrlC;
}

inline QRCtrl<float> CReflect( const ElQRCtrl_s& ctrlC )
{ 
    QRCtrl<float> ctrl;
    ctrl.colPiv = ctrlC.colPiv;
    ctrl.boundRank = ctrlC.boundRank;
    ctrl.maxRank = ctrlC.maxRank;
    ctrl.adaptive = ctrlC.adaptive;
    ctrl.tol = ctrlC.tol;
    ctrl.alwaysRecomputeNorms = ctrlC.alwaysRecomputeNorms;
    return ctrl;
}
inline QRCtrl<double> CReflect( const ElQRCtrl_d& ctrlC )
{ 
    QRCtrl<double> ctrl;
    ctrl.colPiv = ctrlC.colPiv;
    ctrl.boundRank = ctrlC.boundRank;
    ctrl.maxRank = ctrlC.maxRank;
    ctrl.adaptive = ctrlC.adaptive;
    ctrl.tol = ctrlC.tol;
    ctrl.alwaysRecomputeNorms = ctrlC.alwaysRecomputeNorms;
    return ctrl;
}

// Properties
// ^^^^^^^^^^
inline ElNormType CReflect( NormType type )
{ return static_cast<ElNormType>(type); }
inline NormType CReflect( ElNormType type )
{ return static_cast<NormType>(type); }

inline ElPseudospecNorm CReflect( PseudospecNorm psNorm )
{ return static_cast<ElPseudospecNorm>(psNorm); }
inline PseudospecNorm CReflect( ElPseudospecNorm psNorm )
{ return static_cast<PseudospecNorm>(psNorm); }

inline ElSnapshotCtrl CReflect( const SnapshotCtrl& ctrl )
{
    ElSnapshotCtrl ctrlC;
    ctrlC.realSize = ctrl.realSize; 
    ctrlC.imagSize = ctrl.imagSize;
    ctrlC.imgSaveFreq = ctrl.imgSaveFreq;
    ctrlC.numSaveFreq = ctrl.numSaveFreq;
    ctrlC.imgDispFreq = ctrl.imgDispFreq;
    ctrlC.imgSaveCount = ctrl.imgSaveCount;
    ctrlC.numSaveCount = ctrl.numSaveCount;
    ctrlC.imgDispCount = ctrl.imgDispCount;
    ctrlC.imgBase = CReflect(ctrl.imgBase);
    ctrlC.numBase = CReflect(ctrl.numBase);
    ctrlC.imgFormat = CReflect(ctrl.imgFormat);
    ctrlC.numFormat = CReflect(ctrl.numFormat);
    ctrlC.itCounts = ctrl.itCounts;
    return ctrlC;
}
inline SnapshotCtrl CReflect( const ElSnapshotCtrl& ctrlC )
{
    SnapshotCtrl ctrl;
    ctrl.realSize = ctrlC.realSize; 
    ctrl.imagSize = ctrlC.imagSize;
    ctrl.imgSaveFreq = ctrlC.imgSaveFreq;
    ctrl.numSaveFreq = ctrlC.numSaveFreq;
    ctrl.imgDispFreq = ctrlC.imgDispFreq;
    ctrl.imgSaveCount = ctrlC.imgSaveCount;
    ctrl.numSaveCount = ctrlC.numSaveCount;
    ctrl.imgDispCount = ctrlC.imgDispCount;
    ctrl.imgBase = CReflect(ctrlC.imgBase);
    ctrl.numBase = CReflect(ctrlC.numBase);
    ctrl.imgFormat = CReflect(ctrlC.imgFormat);
    ctrl.numFormat = CReflect(ctrlC.numFormat);
    ctrl.itCounts = ctrlC.itCounts;
    return ctrl;
}

inline ElPseudospecCtrl_s CReflect( const PseudospecCtrl<float>& ctrl )
{
    ElPseudospecCtrl_s ctrlC;
    ctrlC.norm = CReflect(ctrl.norm);
    ctrlC.blockWidth = ctrl.norm;
    ctrlC.schur = ctrl.schur;
    ctrlC.forceComplexSchur = ctrl.forceComplexSchur;
    ctrlC.forceComplexPs = ctrl.forceComplexPs;
    ctrlC.schurCtrl = CReflect(ctrl.schurCtrl);
    ctrlC.maxIts = ctrl.maxIts;
    ctrlC.tol = ctrl.tol;
    ctrlC.deflate = ctrl.deflate;
    ctrlC.arnoldi = ctrl.arnoldi;
    ctrlC.basisSize = ctrl.basisSize;
    ctrlC.reorthog = ctrl.reorthog;
    ctrlC.progress = ctrl.progress;
    ctrlC.snapCtrl = CReflect(ctrl.snapCtrl);
    return ctrlC;
}
inline ElPseudospecCtrl_d CReflect( const PseudospecCtrl<double>& ctrl )
{
    ElPseudospecCtrl_d ctrlC;
    ctrlC.norm = CReflect(ctrl.norm);
    ctrlC.blockWidth = ctrl.norm;
    ctrlC.schur = ctrl.schur;
    ctrlC.forceComplexSchur = ctrl.forceComplexSchur;
    ctrlC.forceComplexPs = ctrl.forceComplexPs;
    ctrlC.schurCtrl = CReflect(ctrl.schurCtrl);
    ctrlC.maxIts = ctrl.maxIts;
    ctrlC.tol = ctrl.tol;
    ctrlC.deflate = ctrl.deflate;
    ctrlC.arnoldi = ctrl.arnoldi;
    ctrlC.basisSize = ctrl.basisSize;
    ctrlC.reorthog = ctrl.reorthog;
    ctrlC.progress = ctrl.progress;
    ctrlC.snapCtrl = CReflect(ctrl.snapCtrl);
    return ctrlC;
}

inline PseudospecCtrl<float> CReflect( const ElPseudospecCtrl_s& ctrlC )
{
    PseudospecCtrl<float> ctrl;
    ctrl.norm = CReflect(ctrlC.norm);
    ctrl.blockWidth = ctrlC.norm;
    ctrl.schur = ctrlC.schur;
    ctrl.forceComplexSchur = ctrlC.forceComplexSchur;
    ctrl.forceComplexPs = ctrlC.forceComplexPs;
    ctrl.schurCtrl = CReflect(ctrlC.schurCtrl);
    ctrl.maxIts = ctrlC.maxIts;
    ctrl.tol = ctrlC.tol;
    ctrl.deflate = ctrlC.deflate;
    ctrl.arnoldi = ctrlC.arnoldi;
    ctrl.basisSize = ctrlC.basisSize;
    ctrl.reorthog = ctrlC.reorthog;
    ctrl.progress = ctrlC.progress;
    ctrl.snapCtrl = CReflect(ctrlC.snapCtrl);
    return ctrl;
}
inline PseudospecCtrl<double> CReflect( const ElPseudospecCtrl_d& ctrlC )
{
    PseudospecCtrl<double> ctrl;
    ctrl.norm = CReflect(ctrlC.norm);
    ctrl.blockWidth = ctrlC.norm;
    ctrl.schur = ctrlC.schur;
    ctrl.forceComplexSchur = ctrlC.forceComplexSchur;
    ctrl.forceComplexPs = ctrlC.forceComplexPs;
    ctrl.schurCtrl = CReflect(ctrlC.schurCtrl);
    ctrl.maxIts = ctrlC.maxIts;
    ctrl.tol = ctrlC.tol;
    ctrl.deflate = ctrlC.deflate;
    ctrl.arnoldi = ctrlC.arnoldi;
    ctrl.basisSize = ctrlC.basisSize;
    ctrl.reorthog = ctrlC.reorthog;
    ctrl.progress = ctrlC.progress;
    ctrl.snapCtrl = CReflect(ctrlC.snapCtrl);
    return ctrl;
}

// Solvers
// ^^^^^^^
inline ElTikhonovAlg CReflect( TikhonovAlg alg )
{ return static_cast<ElTikhonovAlg>(alg); }
inline TikhonovAlg CReflect( ElTikhonovAlg alg )
{ return static_cast<TikhonovAlg>(alg); }

inline ElRidgeAlg CReflect( RidgeAlg alg )
{ return static_cast<ElRidgeAlg>(alg); }
inline RidgeAlg CReflect( ElRidgeAlg alg )
{ return static_cast<RidgeAlg>(alg); }

// Optimization
// ------------
inline ElRegularization CReflect( Regularization penalty )
{ return static_cast<ElRegularization>(penalty); }
inline Regularization CReflect( ElRegularization penalty )
{ return static_cast<Regularization>(penalty); }

} // namespace El

#endif // ifndef EL_CREFLECT_C_HPP
