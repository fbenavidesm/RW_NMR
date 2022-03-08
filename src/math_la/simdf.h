#ifndef SIMD_H
#define SIMD_H

#include "mdefs.h"

#ifdef SIMD
#include <emmintrin.h>

inline void pair_add(double* r, const double* a, const double* b)
{
	__m128d va = _mm_load_pd(a);
	__m128d vb = _mm_load_pd(b);
	__m128d vr = _mm_add_pd(va,vb);
	_mm_store_pd(r,vr);
}

inline void pair_sub(double* r, const double* a, const double* b)
{
	__m128d va = _mm_load_pd(a);
	__m128d vb = _mm_load_pd(b);
	__m128d vr = _mm_sub_pd(va,vb);
	_mm_store_pd(r,vr);
}

inline void pair_mul(double* r, const double* a, const double* b)
{
	__m128d va = _mm_load_pd(a);
	__m128d vb = _mm_load_pd(b);
	__m128d vr = _mm_mul_pd(va,vb);
	_mm_store_pd(r,vr);
}

inline void pair_div(double* r, const double* a, const double* b)
{
	__m128d va = _mm_load_pd(a);
	__m128d vb = _mm_load_pd(b);
	__m128d vr = _mm_div_pd(va, vb);
	_mm_store_pd(r, vr);
}

inline void quad_add(float* r, const float* a, const float* b)
{
	__m128 va = _mm_load_ps(a);
	__m128 vb = _mm_load_ps(b);
	__m128 vr = _mm_add_ps(va, vb);
	_mm_store_ps(r, vr);
}

inline void quad_sub(float* r, const float* a, const float* b)
{
	__m128 va = _mm_load_ps(a);
	__m128 vb = _mm_load_ps(b);
	__m128 vr = _mm_sub_ps(va, vb);
	_mm_store_ps(r, vr);
}

inline void quad_mul(float* r, const float* a, const float* b)
{
	__m128 va = _mm_load_ps(a);
	__m128 vb = _mm_load_ps(b);
	__m128 vr = _mm_mul_ps(va, vb);
	_mm_store_ps(r, vr);
}

inline void quad_div(float* r, const float* a, const float* b)
{
	__m128 va = _mm_load_ps(a);
	__m128 vb = _mm_load_ps(b);
	__m128 vr = _mm_div_ps(va, vb);
	_mm_store_ps(r, vr);
}

inline void quad_addu(float* r, const float* a, const float* b)
{
	__m128 va = _mm_loadu_ps(a);
	__m128 vb = _mm_loadu_ps(b);
	__m128 vr = _mm_add_ps(va, vb);
	_mm_storeu_ps(r, vr);
}

inline void quad_subu(float* r, const float* a, const float* b)
{
	__m128 va = _mm_loadu_ps(a);
	__m128 vb = _mm_loadu_ps(b);
	__m128 vr = _mm_sub_ps(va, vb);
	_mm_storeu_ps(r, vr);
}

inline void quad_mulu(float* r, const float* a, const float* b)
{
	__m128 va = _mm_loadu_ps(a);
	__m128 vb = _mm_loadu_ps(b);
	__m128 vr = _mm_mul_ps(va, vb);
	_mm_storeu_ps(r, vr);
}

inline void quad_divu(float* r, const float* a, const float* b)
{
	__m128 va = _mm_loadu_ps(a);
	__m128 vb = _mm_loadu_ps(b);
	__m128 vr = _mm_div_ps(va, vb);
	_mm_storeu_ps(r, vr);
}

inline void pair_addu(double* r, const double* a, const double* b)
{
	__m128d va = _mm_loadu_pd(a);
	__m128d vb = _mm_loadu_pd(b);
	__m128d vr = _mm_add_pd(va,vb);
	_mm_storeu_pd(r,vr);
}

inline void pair_subu(double* r, const double* a, const double* b)
{
	__m128d va = _mm_loadu_pd(a);
	__m128d vb = _mm_loadu_pd(b);
	__m128d vr = _mm_sub_pd(va,vb);
	_mm_storeu_pd(r,vr);
}

inline void pair_mulu(double* r, const double* a, const double* b)
{
	__m128d va = _mm_loadu_pd(a);
	__m128d vb = _mm_loadu_pd(b);
	__m128d vr = _mm_mul_pd(va,vb);
	_mm_storeu_pd(r,vr);
}

inline void pair_divu(double* r, const double* a, const double* b)
{
	__m128d va = _mm_loadu_pd(a);
	__m128d vb = _mm_loadu_pd(b);
	__m128d vr = _mm_div_pd(va, vb);
	_mm_storeu_pd(r, vr);
}



#endif

#endif