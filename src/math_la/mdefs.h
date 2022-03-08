#ifndef DEFCTS_H
#define DEFCTS_H

#define INTEL
#define MKL

#include <vector>
#ifdef INTEL
	#include "tbb/cache_aligned_allocator.h"
	#include "tbb/scalable_allocator.h"
	#include "tbb/tbbmalloc_proxy.h"
#endif

#ifdef MKL
	#include "mkl.h"
#endif

#define SIMD
#define GPU_AMP

/**
* Define SINGLE_PREC when single precision floating point is required
*/
#define DOUBLE_PREC  // SINGLE_PREC will recompile the entire math modules


#define DCHUNK_SIZE 8192
#define BCHUNK_SIZE 256
#define CHUNK_SIZE 64
#define SCHUNK_SIZE 16

using std::vector;

#ifdef GPU_AMP
	#include <amp.h>
	#include <amp_graphics.h>
	#define GPU  restrict(amp,cpu)
	#define GPUP restrict(amp)
	#define CPU  restrict(cpu)
#endif

#define vec(T) vector<T, tbb::cache_aligned_allocator<T>>


typedef unsigned char uchar;
typedef unsigned int uint;

#ifndef VECTOR_ENTRIES
	#define eX 0x00000000
	#define eY 0x00000001
	#define eZ 0x00000002
	#define eW 0x00000003
#endif

#ifndef FILESTRUCTS
#define FILESTRUCTS

	#define SIZEFLOAT 4
	#define SIZEINT 4
	#define SIZEUINT 4
	#define SIZEDOUBLE 8
	#define SIZELONG 8
	#define SIZEULONG 8
	#define SIZEBOOL 1

	union FloatChar
	{
		float fValue;
		char chValue[SIZEFLOAT];
	};

	union IntChar
	{
		int iValue;
		char chValue[SIZEINT];
	};

	union UIntChar
	{
		unsigned int iValue;
		char chValue[SIZEUINT];
	};

	union DoubleChar
	{
		double dValue;
		char chValue[SIZEDOUBLE];
	};

	union BoolChar
	{
		bool bValue;
		char chValue[SIZEBOOL];
	};

	union LongIntChar
	{
		long long lvaue;
		char chValue[SIZELONG];
	};

	union LongUIntChar
	{
		unsigned long long lvalue;
		char chValue[SIZEULONG];
	};

#endif

#ifdef SINGLE_PREC
	typedef float scalar;
	#define EPSILON 1.2e-7
	
	#include <tbb/scalable_allocator.h>
	#define allocScalar(S) (scalar*) mkl_malloc(S*sizeof(scalar), 64)
	#define freeScalar(P)  mkl_free(P)

	#ifdef FILESTRUCTS
		#define scalarchar FloatChar;
		#define sizescalar SIZEFLOAT;
	#endif
#endif

#define MAX_ITERATIONS 3500

#ifdef DOUBLE_PREC
	typedef double scalar;
    	#define EPSILON  1.2e-18
		#ifdef INTEL
			#include <tbb/scalable_allocator.h>
			#define allocScalar(S) (scalar*) mkl_malloc(S*sizeof(scalar), 64)
			#define freeScalar(Surface_Relaxivity)  mkl_free(Surface_Relaxivity)
			#define allocInt(S) (int*) mkl_malloc(S*sizeof(int),64)		
			#define allocUInt(S) (uint*) mkl_malloc(S*sizeof(uint),64)
			#define freeInt(Surface_Relaxivity) mkl_free(Surface_Relaxivity)
			#define freeUInt(Surface_Relaxivity) mkl_free(Surface_Relaxivity)
		#endif
	#ifdef FILESTRUCTS
		#define scalarchar DoubleChar;		
		#define sizescalar SIZEDOUBLE;
	#endif
#endif

inline void crossR3(const scalar x1, const scalar y1, const scalar z1,
	const scalar x2, const scalar y2, const scalar z2,
	scalar& xr, scalar& yr, scalar& zr)
{
	xr = y1*z2 - y2*z1;
	yr = -(x1*z2 - x2*z1);
	zr = x1*y2 - x2*y1;
}

struct int2
{
	int x;
	int y;
	bool operator<(const int2& i) const
	{
		if (this->x == i.x)
		{
			return(this->y < i.y);
		}
		else
		{
			return(this->x < i.x);
		}
	}
};

struct scalar2
{
	scalar x;
	scalar y;
	bool operator<(const scalar2& i) const
	{
		if (this->x == i.x)
		{
			return(this->y < i.y);
		}
		else
		{
			return(this->x < i.x);
		}
	}
};

struct float3
{
	float x;
	float y;
	float z;
	float3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	float3()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}
};

struct float2
{
	float x;
	float y;
	float2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
	float2()
	{
		this->x = 0;
		this->y = 0;
	}
};


struct s_int2
{
	int sx;
	int s;
	s_int2()
	{
		this->sx = 0;
		this->s = 0;
	}

	bool operator<(const s_int2& i) const
	{
		return(this->sx < i.sx);
	}

	bool contains(int v) const
	{
		return ((v >= this->sx) && (v <= this->s));
	}
};


#ifdef INTEL
	#define concurrent_int_int_map tbb::concurrent_hash_map<uint,int,Hash_Uint_Compare>
	#define concurrent_int_pixicf_map tbb::concurrent_hash_map<uint,pix_icf,Hash_Uint_Compare>
	#define concurrent_int_intf_map tbb::concurrent_hash_map<uint,intf,Hash_Uint_Compare>
#endif

#endif