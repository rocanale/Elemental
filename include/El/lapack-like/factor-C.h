/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_LAPACK_FACTOR_C_H
#define EL_LAPACK_FACTOR_C_H

#include "El/core/DistMatrix-C.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  EL_BUNCH_KAUFMAN_A,
  EL_BUNCH_KAUFMAN_C,
  EL_BUNCH_KAUFMAN_D,
  EL_BUNCH_KAUFMAN_BOUNDED,
  EL_BUNCH_PARLETT
  /* TODO: Diagonal pivoting? */
} ElLDLPivotType;

typedef struct
{
  ElInt nb;
  ElInt from[2];
} ElLDLPivot;

typedef struct 
{
    bool boundRank;
    ElInt maxRank;
    
    bool adaptive;
    float tol;

    bool alwaysRecomputeNorms;    

    /* TODO: Add Chan ratio */
} ElQRCtrl_s;
ElError ElQRCtrlFillDefault_s( ElQRCtrl_s* ctrl );

typedef struct 
{
    bool boundRank;
    ElInt maxRank;
    
    bool adaptive;
    double tol;

    bool alwaysRecomputeNorms;    

    /* TODO: Add Chan ratio */
} ElQRCtrl_d;
ElError ElQRCtrlFillDefault_d( ElQRCtrl_d* ctrl );

/* Cholesky
   ======== */

/* Cholesky without pivoting, A = L L^H = U^H U
   ------------------------------------------------- */
ElError ElCholesky_s( ElUpperOrLower uplo, ElMatrix_s A );
ElError ElCholesky_d( ElUpperOrLower uplo, ElMatrix_d A );
ElError ElCholesky_c( ElUpperOrLower uplo, ElMatrix_c A );
ElError ElCholesky_z( ElUpperOrLower uplo, ElMatrix_z A );

/* NOTE: 'A' must be in a [MC,MR] distribution */
ElError ElCholeskyDist_s( ElUpperOrLower uplo, ElDistMatrix_s A );
ElError ElCholeskyDist_d( ElUpperOrLower uplo, ElDistMatrix_d A );
ElError ElCholeskyDist_c( ElUpperOrLower uplo, ElDistMatrix_c A );
ElError ElCholeskyDist_z( ElUpperOrLower uplo, ElDistMatrix_z A );

/* Reverse Cholesky without pivoting, A = L^H L or A = U U^H
   --------------------------------------------------------- */
ElError ElReverseCholesky_s( ElUpperOrLower uplo, ElMatrix_s A );
ElError ElReverseCholesky_d( ElUpperOrLower uplo, ElMatrix_d A );
ElError ElReverseCholesky_c( ElUpperOrLower uplo, ElMatrix_c A );
ElError ElReverseCholesky_z( ElUpperOrLower uplo, ElMatrix_z A );

/* NOTE: 'A' must be in a [MC,MR] distribution */
ElError ElReverseCholeskyDist_s( ElUpperOrLower uplo, ElDistMatrix_s A );
ElError ElReverseCholeskyDist_d( ElUpperOrLower uplo, ElDistMatrix_d A );
ElError ElReverseCholeskyDist_c( ElUpperOrLower uplo, ElDistMatrix_c A );
ElError ElReverseCholeskyDist_z( ElUpperOrLower uplo, ElDistMatrix_z A );

/* Cholesky with full (diagonal) pivoting, P A P^T = L L^H = U^H U
   --------------------------------------------------------------- */
ElError ElCholeskyPiv_s( ElUpperOrLower uplo, ElMatrix_s A, ElMatrix_i p );
ElError ElCholeskyPiv_d( ElUpperOrLower uplo, ElMatrix_d A, ElMatrix_i p );
ElError ElCholeskyPiv_c( ElUpperOrLower uplo, ElMatrix_c A, ElMatrix_i p );
ElError ElCholeskyPiv_z( ElUpperOrLower uplo, ElMatrix_z A, ElMatrix_i p );

/* NOTE: 'A' must be in a [MC,MR] distribution, and
         'p' must be in a [VC,STAR] distribution */
ElError ElCholeskyPivDist_s
( ElUpperOrLower uplo, ElDistMatrix_s A, ElDistMatrix_i p );
ElError ElCholeskyPivDist_d
( ElUpperOrLower uplo, ElDistMatrix_d A, ElDistMatrix_i p );
ElError ElCholeskyPivDist_c
( ElUpperOrLower uplo, ElDistMatrix_c A, ElDistMatrix_i p );
ElError ElCholeskyPivDist_z
( ElUpperOrLower uplo, ElDistMatrix_z A, ElDistMatrix_i p );

/* Modify a Cholesky factorization, L L^H + alpha V V^H = LHat LHat^H
   ------------------------------------------------------------------ */
ElError ElCholeskyMod_s
( ElUpperOrLower uplo, ElMatrix_s T, float alpha, ElMatrix_s V );
ElError ElCholeskyMod_d
( ElUpperOrLower uplo, ElMatrix_d T, double alpha, ElMatrix_d V );
ElError ElCholeskyMod_c
( ElUpperOrLower uplo, ElMatrix_c T, float alpha, ElMatrix_c V );
ElError ElCholeskyMod_z
( ElUpperOrLower uplo, ElMatrix_z T, double alpha, ElMatrix_z V );

/* NOTE: 'T' and 'V' must be in a [MC,MR] distribution */
ElError ElCholeskyModDist_s
( ElUpperOrLower uplo, ElDistMatrix_s T, float alpha, ElDistMatrix_s V );
ElError ElCholeskyModDist_d
( ElUpperOrLower uplo, ElDistMatrix_d T, double alpha, ElDistMatrix_d V );
ElError ElCholeskyModDist_c
( ElUpperOrLower uplo, ElDistMatrix_c T, float alpha, ElDistMatrix_c V );
ElError ElCholeskyModDist_z
( ElUpperOrLower uplo, ElDistMatrix_z T, double alpha, ElDistMatrix_z V );

/* Compute the Cholesky factorization of a possibly singular matrix
   ---------------------------------------------------------------- */
ElError ElHPSDCholesky_s( ElUpperOrLower uplo, ElMatrix_s A );
ElError ElHPSDCholesky_d( ElUpperOrLower uplo, ElMatrix_d A );
ElError ElHPSDCholesky_c( ElUpperOrLower uplo, ElMatrix_c A );
ElError ElHPSDCholesky_z( ElUpperOrLower uplo, ElMatrix_z A );

/* NOTE: 'A' must be in a [MC,MR] distribution */
ElError ElHPSDCholeskyDist_s( ElUpperOrLower uplo, ElDistMatrix_s A );
ElError ElHPSDCholeskyDist_d( ElUpperOrLower uplo, ElDistMatrix_d A );
ElError ElHPSDCholeskyDist_c( ElUpperOrLower uplo, ElDistMatrix_c A );
ElError ElHPSDCholeskyDist_z( ElUpperOrLower uplo, ElDistMatrix_z A );

/* Solve linear systems after a Cholesky factorization
   --------------------------------------------------- */
ElError ElSolveAfterCholesky_s
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstMatrix_s A, ElMatrix_s B );
ElError ElSolveAfterCholesky_d
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstMatrix_d A, ElMatrix_d B );
ElError ElSolveAfterCholesky_c
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstMatrix_c A, ElMatrix_c B );
ElError ElSolveAfterCholesky_z
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstMatrix_z A, ElMatrix_z B );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions */
ElError ElSolveAfterCholeskyDist_s
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstDistMatrix_s A, ElDistMatrix_s B );
ElError ElSolveAfterCholeskyDist_d
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstDistMatrix_d A, ElDistMatrix_d B );
ElError ElSolveAfterCholeskyDist_c
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstDistMatrix_c A, ElDistMatrix_c B );
ElError ElSolveAfterCholeskyDist_z
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstDistMatrix_z A, ElDistMatrix_z B );

/* Solve linear systems after a Cholesky factorization with full pivoting
   ---------------------------------------------------------------------- */
ElError ElSolveAfterCholeskyFullPiv_s
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstMatrix_s A, ElConstMatrix_i p, ElMatrix_s B );
ElError ElSolveAfterCholeskyFullPiv_d
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstMatrix_d A, ElConstMatrix_i p, ElMatrix_d B );
ElError ElSolveAfterCholeskyFullPiv_c
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstMatrix_c A, ElConstMatrix_i p, ElMatrix_c B );
ElError ElSolveAfterCholeskyFullPiv_z
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstMatrix_z A, ElConstMatrix_i p, ElMatrix_z B );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions, while
         'p' must be in a [VC,STAR] distribution */
ElError ElSolveAfterCholeskyFullPivDist_s
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstDistMatrix_s A, ElConstDistMatrix_i p, ElDistMatrix_s B );
ElError ElSolveAfterCholeskyFullPivDist_d
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstDistMatrix_d A, ElConstDistMatrix_i p, ElDistMatrix_d B );
ElError ElSolveAfterCholeskyFullPivDist_c
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstDistMatrix_c A, ElConstDistMatrix_i p, ElDistMatrix_c B );
ElError ElSolveAfterCholeskyFullPivDist_z
( ElUpperOrLower uplo, ElOrientation orientation,
  ElConstDistMatrix_z A, ElConstDistMatrix_i p, ElDistMatrix_z B );

/* Generalized QR
   ============== */

/* Return the packed Generalized QR factorization 
   ---------------------------------------------- */
ElError ElGQR_s
( ElMatrix_s A, ElMatrix_s tA, ElMatrix_s dA,
  ElMatrix_s B, ElMatrix_s tB, ElMatrix_s dB );
ElError ElGQR_d
( ElMatrix_d A, ElMatrix_d tA, ElMatrix_d dA,
  ElMatrix_d B, ElMatrix_d tB, ElMatrix_d dB );
ElError ElGQR_c
( ElMatrix_c A, ElMatrix_c tA, ElMatrix_s dA,
  ElMatrix_c B, ElMatrix_c tB, ElMatrix_s dB );
ElError ElGQR_z
( ElMatrix_z A, ElMatrix_z tA, ElMatrix_d dA,
  ElMatrix_z B, ElMatrix_z tB, ElMatrix_d dB );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions, while 
         'tA', 'tB', 'dA', and 'dB' must be in [MD,STAR] distributions */
ElError ElGQRDist_s
( ElDistMatrix_s A, ElDistMatrix_s tA, ElDistMatrix_s dA,
  ElDistMatrix_s B, ElDistMatrix_s tB, ElDistMatrix_s dB );
ElError ElGQRDist_d
( ElDistMatrix_d A, ElDistMatrix_d tA, ElDistMatrix_d dA,
  ElDistMatrix_d B, ElDistMatrix_d tB, ElDistMatrix_d dB );
ElError ElGQRDist_c
( ElDistMatrix_c A, ElDistMatrix_c tA, ElDistMatrix_s dA,
  ElDistMatrix_c B, ElDistMatrix_c tB, ElDistMatrix_s dB );
ElError ElGQRDist_z
( ElDistMatrix_z A, ElDistMatrix_z tA, ElDistMatrix_d dA,
  ElDistMatrix_z B, ElDistMatrix_z tB, ElDistMatrix_d dB );

/* Return just the triangular factors from the GRQ factorization
   ------------------------------------------------------------- */
ElError ElGQRTriang_s( ElMatrix_s A, ElMatrix_s B );
ElError ElGQRTriang_d( ElMatrix_d A, ElMatrix_d B );
ElError ElGQRTriang_c( ElMatrix_c A, ElMatrix_c B );
ElError ElGQRTriang_z( ElMatrix_z A, ElMatrix_z B );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions, while 
         'tA', 'tB', 'dA', and 'dB' must be in [MD,STAR] distributions */
ElError ElGQRTriangDist_s( ElDistMatrix_s A, ElDistMatrix_s B );
ElError ElGQRTriangDist_d( ElDistMatrix_d A, ElDistMatrix_d B );
ElError ElGQRTriangDist_c( ElDistMatrix_c A, ElDistMatrix_c B );
ElError ElGQRTriangDist_z( ElDistMatrix_z A, ElDistMatrix_z B );

/* Generalized RQ
   ============== */

/* Return the packed Generalized RQ factorization 
   ---------------------------------------------- */
ElError ElGRQ_s
( ElMatrix_s A, ElMatrix_s tA, ElMatrix_s dA,
  ElMatrix_s B, ElMatrix_s tB, ElMatrix_s dB );
ElError ElGRQ_d
( ElMatrix_d A, ElMatrix_d tA, ElMatrix_d dA,
  ElMatrix_d B, ElMatrix_d tB, ElMatrix_d dB );
ElError ElGRQ_c
( ElMatrix_c A, ElMatrix_c tA, ElMatrix_s dA,
  ElMatrix_c B, ElMatrix_c tB, ElMatrix_s dB );
ElError ElGRQ_z
( ElMatrix_z A, ElMatrix_z tA, ElMatrix_d dA,
  ElMatrix_z B, ElMatrix_z tB, ElMatrix_d dB );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions, while 
         'tA', 'tB', 'dA', and 'dB' must be in [MD,STAR] distributions */
ElError ElGRQDist_s
( ElDistMatrix_s A, ElDistMatrix_s tA, ElDistMatrix_s dA,
  ElDistMatrix_s B, ElDistMatrix_s tB, ElDistMatrix_s dB );
ElError ElGRQDist_d
( ElDistMatrix_d A, ElDistMatrix_d tA, ElDistMatrix_d dA,
  ElDistMatrix_d B, ElDistMatrix_d tB, ElDistMatrix_d dB );
ElError ElGRQDist_c
( ElDistMatrix_c A, ElDistMatrix_c tA, ElDistMatrix_s dA,
  ElDistMatrix_c B, ElDistMatrix_c tB, ElDistMatrix_s dB );
ElError ElGRQDist_z
( ElDistMatrix_z A, ElDistMatrix_z tA, ElDistMatrix_d dA,
  ElDistMatrix_z B, ElDistMatrix_z tB, ElDistMatrix_d dB );

/* Return just the triangular factors from the GRQ factorization
   ------------------------------------------------------------- */
ElError ElGRQTriang_s( ElMatrix_s A, ElMatrix_s B );
ElError ElGRQTriang_d( ElMatrix_d A, ElMatrix_d B );
ElError ElGRQTriang_c( ElMatrix_c A, ElMatrix_c B );
ElError ElGRQTriang_z( ElMatrix_z A, ElMatrix_z B );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions, while 
         'tA', 'tB', 'dA', and 'dB' must be in [MD,STAR] distributions */
ElError ElGRQTriangDist_s( ElDistMatrix_s A, ElDistMatrix_s B );
ElError ElGRQTriangDist_d( ElDistMatrix_d A, ElDistMatrix_d B );
ElError ElGRQTriangDist_c( ElDistMatrix_c A, ElDistMatrix_c B );
ElError ElGRQTriangDist_z( ElDistMatrix_z A, ElDistMatrix_z B );

/* Interpolative Decomposition
   =========================== */
ElError ElID_s
( ElMatrix_s A, ElMatrix_i p, ElMatrix_s Z, ElQRCtrl_s ctrl, 
  bool canOverwrite );
ElError ElID_d
( ElMatrix_d A, ElMatrix_i p, ElMatrix_d Z, ElQRCtrl_d ctrl,
  bool canOverwrite );
ElError ElID_c
( ElMatrix_c A, ElMatrix_i p, ElMatrix_c Z, ElQRCtrl_s ctrl,
  bool canOverWrite );
ElError ElID_z
( ElMatrix_z A, ElMatrix_i p, ElMatrix_z Z, ElQRCtrl_d ctrl,
  bool canOverwrite );

/* NOTE: 'A' must be in a [MC,MR] distribution, while
         'Z' must be in a [STAR,VR] distribution, and
         'p' must be in a [VR,STAR] distribution */
ElError ElIDDist_s
( ElDistMatrix_s A, ElDistMatrix_i p, ElDistMatrix_s Z, ElQRCtrl_s ctrl, 
  bool canOverwrite );
ElError ElIDDist_d
( ElDistMatrix_d A, ElDistMatrix_i p, ElDistMatrix_d Z, ElQRCtrl_d ctrl, 
  bool canOverwrite );
ElError ElIDDist_c
( ElDistMatrix_c A, ElDistMatrix_i p, ElDistMatrix_c Z, ElQRCtrl_s ctrl, 
  bool canOverwrite );
ElError ElIDDist_z
( ElDistMatrix_z A, ElDistMatrix_i p, ElDistMatrix_z Z, ElQRCtrl_d ctrl, 
  bool canOverwrite );

/* LDL factorization
   ================= */

/* Packed LDL factorization (without pivoting) 
   ------------------------------------------- */
ElError ElLDL_s( ElMatrix_s A );
ElError ElLDL_d( ElMatrix_d A );
ElError ElLDL_c( ElMatrix_c A, bool conjugate );
ElError ElLDL_z( ElMatrix_z A, bool conjugate );

/* NOTE: 'A' must be in a [MC,MR] distribution */
ElError ElLDLDist_s( ElDistMatrix_s A );
ElError ElLDLDist_d( ElDistMatrix_d A );
ElError ElLDLDist_c( ElDistMatrix_c A, bool conjugate );
ElError ElLDLDist_z( ElDistMatrix_z A, bool conjugate );

/* Packed LDL factorization (with pivoting)
   ---------------------------------------- */
ElError ElLDLPiv_s
( ElMatrix_s A, ElMatrix_s dSub, ElMatrix_i p, ElLDLPivotType pivotType );
ElError ElLDLPiv_d
( ElMatrix_d A, ElMatrix_d dSub, ElMatrix_i p, ElLDLPivotType pivotType );
ElError ElLDLPiv_c
( ElMatrix_c A, ElMatrix_c dSub, ElMatrix_i p, bool conjugate, 
  ElLDLPivotType pivotType );
ElError ElLDLPiv_z
( ElMatrix_z A, ElMatrix_z dSub, ElMatrix_i p, bool conjugate, 
  ElLDLPivotType pivotType );

/* NOTE: 'A' must be in a [MC,MR] distribution, 
         'dSub' must be in a [MD,STAR] distribution, and 
         'p' must be in a [VC,STAR] distribution */
ElError ElLDLPivDist_s
( ElDistMatrix_s A, ElDistMatrix_s dSub, ElDistMatrix_i p, 
  ElLDLPivotType pivotType );
ElError ElLDLPivDist_d
( ElDistMatrix_d A, ElDistMatrix_d dSub, ElDistMatrix_i p, 
  ElLDLPivotType pivotType );
ElError ElLDLPivDist_c
( ElDistMatrix_c A, ElDistMatrix_c dSub, ElDistMatrix_i p, bool conjugate, 
  ElLDLPivotType pivotType );
ElError ElLDLPivDist_z
( ElDistMatrix_z A, ElDistMatrix_z dSub, ElDistMatrix_i p, bool conjugate, 
  ElLDLPivotType pivotType );

/* Return the inertia given the quasi-diagonal factor from an LDL^H 
   ---------------------------------------------------------------- */
ElError ElInertiaAfterLDL_s
( ElConstMatrix_s d, ElConstMatrix_s dSub, ElInertiaType* inertia );
ElError ElInertiaAfterLDL_d
( ElConstMatrix_d d, ElConstMatrix_d dSub, ElInertiaType* inertia );
ElError ElInertiaAfterLDL_c
( ElConstMatrix_s d, ElConstMatrix_c dSub, ElInertiaType* inertia );
ElError ElInertiaAfterLDL_z
( ElConstMatrix_d d, ElConstMatrix_z dSub, ElInertiaType* inertia );

/* NOTE: 'd' and 'dSub' must be in [MD,STAR] distributions */
ElError ElInertiaAfterLDLDist_s
( ElConstDistMatrix_s d, ElConstDistMatrix_s dSub, ElInertiaType* inertia );
ElError ElInertiaAfterLDLDist_d
( ElConstDistMatrix_d d, ElConstDistMatrix_d dSub, ElInertiaType* inertia );
ElError ElInertiaAfterLDLDist_c
( ElConstDistMatrix_s d, ElConstDistMatrix_c dSub, ElInertiaType* inertia );
ElError ElInertiaAfterLDLDist_z
( ElConstDistMatrix_d d, ElConstDistMatrix_z dSub, ElInertiaType* inertia );

/* Multiply vectors after an unpivoted LDL factorization
   ----------------------------------------------------- */
ElError ElMultiplyAfterLDL_s( ElConstMatrix_s A, ElMatrix_s B );
ElError ElMultiplyAfterLDL_d( ElConstMatrix_d A, ElMatrix_d B );
ElError ElMultiplyAfterLDL_c( ElConstMatrix_c A, ElMatrix_c B, bool conjugate );
ElError ElMultiplyAfterLDL_z( ElConstMatrix_z A, ElMatrix_z B, bool conjugate );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions */
ElError ElMultiplyAfterLDLDist_s
( ElConstDistMatrix_s A, ElDistMatrix_s B );
ElError ElMultiplyAfterLDLDist_d
( ElConstDistMatrix_d A, ElDistMatrix_d B );
ElError ElMultiplyAfterLDLDist_c
( ElConstDistMatrix_c A, ElDistMatrix_c B, bool conjugate );
ElError ElMultiplyAfterLDLDist_z
( ElConstDistMatrix_z A, ElDistMatrix_z B, bool conjugate );

/* Multiply vectors after a pivoted LDL factorization
   -------------------------------------------------- */
ElError ElMultiplyAfterLDLPiv_s
( ElConstMatrix_s A, ElConstMatrix_s dSub, ElConstMatrix_i p, ElMatrix_s B );
ElError ElMultiplyAfterLDLPiv_d
( ElConstMatrix_d A, ElConstMatrix_d dSub, ElConstMatrix_i p, ElMatrix_d B );
ElError ElMultiplyAfterLDLPiv_c
( ElConstMatrix_c A, ElConstMatrix_c dSub, ElConstMatrix_i p, ElMatrix_c B, 
  bool conjugate );
ElError ElMultiplyAfterLDLPiv_z
( ElConstMatrix_z A, ElConstMatrix_z dSub, ElConstMatrix_i p, ElMatrix_z B, 
  bool conjugate );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions, while
         'dSub' must be in a [MD,STAR] distribution and
         'p' must be in a [VR,STAR] distribution */
ElError ElMultiplyAfterLDLPivDist_s
( ElConstDistMatrix_s A, ElConstDistMatrix_s dSub, ElConstDistMatrix_i p, 
  ElDistMatrix_s B );
ElError ElMultiplyAfterLDLPivDist_d
( ElConstDistMatrix_d A, ElConstDistMatrix_d dSub, ElConstDistMatrix_i p, 
  ElDistMatrix_d B );
ElError ElMultiplyAfterLDLPivDist_c
( ElConstDistMatrix_c A, ElConstDistMatrix_c dSub, ElConstDistMatrix_i p, 
  ElDistMatrix_c B, bool conjugate );
ElError ElMultiplyAfterLDLPivDist_z
( ElConstDistMatrix_z A, ElConstDistMatrix_z dSub, ElConstDistMatrix_i p, 
  ElDistMatrix_z B, bool conjugate );

/* LQ factorization
   ================ */

/* Return the packed LQ factorization 
   ---------------------------------- */
ElError ElLQ_s( ElMatrix_s A, ElMatrix_s t, ElMatrix_s d );
ElError ElLQ_d( ElMatrix_d A, ElMatrix_d t, ElMatrix_d d );
ElError ElLQ_c( ElMatrix_c A, ElMatrix_c t, ElMatrix_s d );
ElError ElLQ_z( ElMatrix_z A, ElMatrix_z t, ElMatrix_d d );

/* NOTE: 'A' must be in a [MC,MR] distribution while
         't' and 'd' must be in [MD,STAR] distributions */
ElError ElLQDist_s( ElDistMatrix_s A, ElDistMatrix_s t, ElDistMatrix_s d );
ElError ElLQDist_d( ElDistMatrix_d A, ElDistMatrix_d t, ElDistMatrix_d d );
ElError ElLQDist_c( ElDistMatrix_c A, ElDistMatrix_c t, ElDistMatrix_s d );
ElError ElLQDist_z( ElDistMatrix_z A, ElDistMatrix_z t, ElDistMatrix_d d );

/* Return the explicit LQ factorization 
   ------------------------------------ */
ElError ElLQExplicit_s( ElMatrix_s L, ElMatrix_s A );
ElError ElLQExplicit_d( ElMatrix_d L, ElMatrix_d A );
ElError ElLQExplicit_c( ElMatrix_c L, ElMatrix_c A );
ElError ElLQExplicit_z( ElMatrix_z L, ElMatrix_z A );

/* NOTE: 'L' and 'A' must be in [MC,MR] distributions */
ElError ElLQExplicitDist_s( ElDistMatrix_s L, ElDistMatrix_s A );
ElError ElLQExplicitDist_d( ElDistMatrix_d L, ElDistMatrix_d A );
ElError ElLQExplicitDist_c( ElDistMatrix_c L, ElDistMatrix_c A );
ElError ElLQExplicitDist_z( ElDistMatrix_z L, ElDistMatrix_z A );

/* Only return the triangular factor, L
   ------------------------------------ */
ElError ElLQTriang_s( ElMatrix_s A );
ElError ElLQTriang_d( ElMatrix_d A );
ElError ElLQTriang_c( ElMatrix_c A );
ElError ElLQTriang_z( ElMatrix_z A );

/* NOTE: 'A' must be in a [MC,MR] distribution */
ElError ElLQTriangDist_s( ElDistMatrix_s A );
ElError ElLQTriangDist_d( ElDistMatrix_d A );
ElError ElLQTriangDist_c( ElDistMatrix_c A );
ElError ElLQTriangDist_z( ElDistMatrix_z A );

/* Only return the unitary factor, Q
   --------------------------------- */
ElError ElLQUnitary_s( ElMatrix_s A );
ElError ElLQUnitary_d( ElMatrix_d A );
ElError ElLQUnitary_c( ElMatrix_c A );
ElError ElLQUnitary_z( ElMatrix_z A );

/* NOTE: 'A' must be in a [MC,MR] distribution */
ElError ElLQUnitaryDist_s( ElDistMatrix_s A );
ElError ElLQUnitaryDist_d( ElDistMatrix_d A );
ElError ElLQUnitaryDist_c( ElDistMatrix_c A );
ElError ElLQUnitaryDist_z( ElDistMatrix_z A );

/* Apply Q from an LQ factorization to vectors
   ------------------------------------------- */
ElError ElApplyQAfterLQ_s
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstMatrix_s A, ElConstMatrix_s t, 
  ElConstMatrix_s d, ElMatrix_s B );
ElError ElApplyQAfterLQ_d
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstMatrix_d A, ElConstMatrix_d t, 
  ElConstMatrix_d d, ElMatrix_d B );
ElError ElApplyQAfterLQ_c
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstMatrix_c A, ElConstMatrix_c t, 
  ElConstMatrix_s d, ElMatrix_c B );
ElError ElApplyQAfterLQ_z
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstMatrix_z A, ElConstMatrix_z t, 
  ElConstMatrix_d d, ElMatrix_z B );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions, while
         't' and 'd' must be in [MD,STAR] distributions */
ElError ElApplyQAfterLQDist_s
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstDistMatrix_s A, ElConstDistMatrix_s t, 
  ElConstDistMatrix_s d, ElDistMatrix_s B );
ElError ElApplyQAfterLQDist_d
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstDistMatrix_d A, ElConstDistMatrix_d t, 
  ElConstDistMatrix_d d, ElDistMatrix_d B );
ElError ElApplyQAfterLQDist_c
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstDistMatrix_c A, ElConstDistMatrix_c t, 
  ElConstDistMatrix_s d, ElDistMatrix_c B );
ElError ElApplyQAfterLQDist_z
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstDistMatrix_z A, ElConstDistMatrix_z t, 
  ElConstDistMatrix_d d, ElDistMatrix_z B );

/* Solve against vectors after an LQ factorization
   ----------------------------------------------- */
ElError ElSolveAfterLQ_s
( ElOrientation orientation, ElConstMatrix_s A, ElConstMatrix_s t, 
  ElConstMatrix_s d, ElConstMatrix_s B, ElMatrix_s X );
ElError ElSolveAfterLQ_d
( ElOrientation orientation, ElConstMatrix_d A, ElConstMatrix_d t, 
  ElConstMatrix_d d, ElConstMatrix_d B, ElMatrix_d X );
ElError ElSolveAfterLQ_c
( ElOrientation orientation, ElConstMatrix_c A, ElConstMatrix_c t, 
  ElConstMatrix_s d, ElConstMatrix_c B, ElMatrix_c X );
ElError ElSolveAfterLQ_z
( ElOrientation orientation, ElConstMatrix_z A, ElConstMatrix_z t, 
  ElConstMatrix_d d, ElConstMatrix_z B, ElMatrix_z X );

/* NOTE: 'A', 'B', and 'X' must be in [MC,MR] distributions, while
         't' and 'd' must be in [MD,STAR] distributions */
ElError ElSolveAfterLQDist_s
( ElOrientation orientation, ElConstDistMatrix_s A, ElConstDistMatrix_s t, 
  ElConstDistMatrix_s d, ElConstDistMatrix_s B, ElDistMatrix_s X );
ElError ElSolveAfterLQDist_d
( ElOrientation orientation, ElConstDistMatrix_d A, ElConstDistMatrix_d t, 
  ElConstDistMatrix_d d, ElConstDistMatrix_d B, ElDistMatrix_d X );
ElError ElSolveAfterLQDist_c
( ElOrientation orientation, ElConstDistMatrix_c A, ElConstDistMatrix_c t, 
  ElConstDistMatrix_s d, ElConstDistMatrix_c B, ElDistMatrix_c X );
ElError ElSolveAfterLQDist_z
( ElOrientation orientation, ElConstDistMatrix_z A, ElConstDistMatrix_z t, 
  ElConstDistMatrix_d d, ElConstDistMatrix_z B, ElDistMatrix_z X );

/* RQ factorization
   ================ */

/* Return the packed RQ factorization 
   ---------------------------------- */
ElError ElRQ_s( ElMatrix_s A, ElMatrix_s t, ElMatrix_s d );
ElError ElRQ_d( ElMatrix_d A, ElMatrix_d t, ElMatrix_d d );
ElError ElRQ_c( ElMatrix_c A, ElMatrix_c t, ElMatrix_s d );
ElError ElRQ_z( ElMatrix_z A, ElMatrix_z t, ElMatrix_d d );

/* NOTE: 'A' must be in a [MC,MR] distribution while
         't' and 'd' must be in [MD,STAR] distributions */
ElError ElRQDist_s( ElDistMatrix_s A, ElDistMatrix_s t, ElDistMatrix_s d );
ElError ElRQDist_d( ElDistMatrix_d A, ElDistMatrix_d t, ElDistMatrix_d d );
ElError ElRQDist_c( ElDistMatrix_c A, ElDistMatrix_c t, ElDistMatrix_s d );
ElError ElRQDist_z( ElDistMatrix_z A, ElDistMatrix_z t, ElDistMatrix_d d );

/* TODO: Return the explicit RQ factorization 
   ------------------------------------------ */

/* Only return the triangular factor, L
   ------------------------------------ */
ElError ElRQTriang_s( ElMatrix_s A );
ElError ElRQTriang_d( ElMatrix_d A );
ElError ElRQTriang_c( ElMatrix_c A );
ElError ElRQTriang_z( ElMatrix_z A );

/* NOTE: 'A' must be in a [MC,MR] distribution */
ElError ElRQTriangDist_s( ElDistMatrix_s A );
ElError ElRQTriangDist_d( ElDistMatrix_d A );
ElError ElRQTriangDist_c( ElDistMatrix_c A );
ElError ElRQTriangDist_z( ElDistMatrix_z A );

/* TODO: Only return the unitary factor, Q
   --------------------------------------- */

/* Apply Q from an RQ factorization to vectors
   ------------------------------------------- */
ElError ElApplyQAfterRQ_s
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstMatrix_s A, ElConstMatrix_s t, 
  ElConstMatrix_s d, ElMatrix_s B );
ElError ElApplyQAfterRQ_d
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstMatrix_d A, ElConstMatrix_d t, 
  ElConstMatrix_d d, ElMatrix_d B );
ElError ElApplyQAfterRQ_c
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstMatrix_c A, ElConstMatrix_c t, 
  ElConstMatrix_s d, ElMatrix_c B );
ElError ElApplyQAfterRQ_z
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstMatrix_z A, ElConstMatrix_z t, 
  ElConstMatrix_d d, ElMatrix_z B );

/* NOTE: 'A' and 'B' must be in [MC,MR] distributions, while
         't' and 'd' must be in [MD,STAR] distributions */
ElError ElApplyQAfterRQDist_s
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstDistMatrix_s A, ElConstDistMatrix_s t, 
  ElConstDistMatrix_s d, ElDistMatrix_s B );
ElError ElApplyQAfterRQDist_d
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstDistMatrix_d A, ElConstDistMatrix_d t, 
  ElConstDistMatrix_d d, ElDistMatrix_d B );
ElError ElApplyQAfterRQDist_c
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstDistMatrix_c A, ElConstDistMatrix_c t, 
  ElConstDistMatrix_s d, ElDistMatrix_c B );
ElError ElApplyQAfterRQDist_z
( ElLeftOrRight side, ElOrientation orientation, 
  ElConstDistMatrix_z A, ElConstDistMatrix_z t, 
  ElConstDistMatrix_d d, ElDistMatrix_z B );

/* Solve against vectors after an RQ factorization
   ----------------------------------------------- */
ElError ElSolveAfterRQ_s
( ElOrientation orientation, ElConstMatrix_s A, ElConstMatrix_s t, 
  ElConstMatrix_s d, ElConstMatrix_s B, ElMatrix_s X );
ElError ElSolveAfterRQ_d
( ElOrientation orientation, ElConstMatrix_d A, ElConstMatrix_d t, 
  ElConstMatrix_d d, ElConstMatrix_d B, ElMatrix_d X );
ElError ElSolveAfterRQ_c
( ElOrientation orientation, ElConstMatrix_c A, ElConstMatrix_c t, 
  ElConstMatrix_s d, ElConstMatrix_c B, ElMatrix_c X );
ElError ElSolveAfterRQ_z
( ElOrientation orientation, ElConstMatrix_z A, ElConstMatrix_z t, 
  ElConstMatrix_d d, ElConstMatrix_z B, ElMatrix_z X );

/* NOTE: 'A', 'B', and 'X' must be in [MC,MR] distributions, while
         't' and 'd' must be in [MD,STAR] distributions */
ElError ElSolveAfterRQDist_s
( ElOrientation orientation, ElConstDistMatrix_s A, ElConstDistMatrix_s t, 
  ElConstDistMatrix_s d, ElConstDistMatrix_s B, ElDistMatrix_s X );
ElError ElSolveAfterRQDist_d
( ElOrientation orientation, ElConstDistMatrix_d A, ElConstDistMatrix_d t, 
  ElConstDistMatrix_d d, ElConstDistMatrix_d B, ElDistMatrix_d X );
ElError ElSolveAfterRQDist_c
( ElOrientation orientation, ElConstDistMatrix_c A, ElConstDistMatrix_c t, 
  ElConstDistMatrix_s d, ElConstDistMatrix_c B, ElDistMatrix_c X );
ElError ElSolveAfterRQDist_z
( ElOrientation orientation, ElConstDistMatrix_z A, ElConstDistMatrix_z t, 
  ElConstDistMatrix_d d, ElConstDistMatrix_z B, ElDistMatrix_z X );

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ifndef EL_LAPACK_FACTOR_C_H */
