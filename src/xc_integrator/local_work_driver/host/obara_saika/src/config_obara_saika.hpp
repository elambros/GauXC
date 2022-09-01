#pragma once

//#define X86_AVX
#define NPTS_LOCAL 64


// Scalar types
#define SCALAR_TYPE double
#define SCALAR_LENGTH 1

#define SCALAR_SET1(x) x

#define SCALAR_LOAD(x) *(x)
#define SCALAR_STORE(x, y) *(x) = y

#define SCALAR_ADD(x, y) x + y
#define SCALAR_SUB(x, y) x - y

#define SCALAR_MUL(x, y) x * y
#define SCALAR_FMA(x, y, z) z + x * y 
#define SCALAR_FNMA(x, y, z) z - x * y

#define SCALAR_RECIPROCAL(x) 1.0 / (1.0 * x)

#define SCALAR_DUPLICATE(x) *(x)



// AVX-512 SIMD Types
#if __AVX512F__ && __has_include(<zmmintrin.h>)

  #include <zmmintrin.h>
  
  #define SIMD_TYPE __m512d
  
  #define SIMD_LENGTH 8
  
  #define SIMD_ZERO() _mm512_setzero_pd()
  #define SIMD_SET1(x) _mm512_set1_pd(x)
  
  #define SIMD_ALIGNED_LOAD(x) _mm512_load_pd(x)
  #define SIMD_UNALIGNED_LOAD(x) _mm512_loadu_pd(x)
  
  #define SIMD_ALIGNED_STORE(x, y) _mm512_store_pd(x, y)
  #define SIMD_UNALIGNED_STORE(x, y) _mm512_storeu_pd(x, y)
  
  #define SIMD_ADD(x, y) _mm512_add_pd(x, y)
  #define SIMD_SUB(x, y) _mm512_sub_pd(x, y)
  
  #define SIMD_MUL(x, y) _mm512_mul_pd(x, y)
  #define SIMD_FMA(x, y, z) _mm512_fmadd_pd(x, y, z)
  #define SIMD_FNMA(x, y, z) _mm512_fnmadd_pd(x, y, z)
  
  #define SIMD_DUPLICATE(x) _mm512_broadcast_f64x4(_mm256_broadcast_sd(x))

// AVX-256 SIMD Types
#elif __AVX__ || __AVX2__

  #include <immintrin.h>
  
  #define SIMD_TYPE __m256d
  
  #define SIMD_LENGTH 4
  
  #define SIMD_ZERO() _mm256_setzero_pd()
  #define SIMD_SET1(x) _mm256_set1_pd(x)
  
  #define SIMD_ALIGNED_LOAD(x) _mm256_load_pd(x)
  #define SIMD_UNALIGNED_LOAD(x) _mm256_loadu_pd(x)
  
  #define SIMD_ALIGNED_STORE(x, y) _mm256_store_pd(x, y)
  #define SIMD_UNALIGNED_STORE(x, y) _mm256_storeu_pd(x, y)
  
  #define SIMD_ADD(x, y) _mm256_add_pd(x, y)
  #define SIMD_SUB(x, y) _mm256_sub_pd(x, y)
  
  #define SIMD_MUL(x, y) _mm256_mul_pd(x, y)
  #define SIMD_FMA(x, y, z) _mm256_fmadd_pd(x, y, z)
  #define SIMD_FNMA(x, y, z) _mm256_fnmadd_pd(x, y, z)
  
  #define SIMD_DUPLICATE(x) _mm256_broadcast_sd(x)

// Scalar SIMD Emulation
#else

  #warning "Warning: ISA Not Specified: Using Scalar Code"

  #define SIMD_TYPE double
  
  #define SIMD_LENGTH 1
  
  #define SIMD_ZERO() 0.0
  #define SIMD_SET1(x) x
  
  #define SIMD_ALIGNED_LOAD(x) *(x)
  #define SIMD_UNALIGNED_LOAD(x) *(x)
  
  #define SIMD_ALIGNED_STORE(x, y) *(x) = y
  #define SIMD_UNALIGNED_STORE(x, y) *(x) = y
  
  #define SIMD_ADD(x, y) x + y
  #define SIMD_SUB(x, y) x - y
  
  #define SIMD_MUL(x, y) x * y
  #define SIMD_FMA(x, y, z) z + x * y 
  #define SIMD_FNMA(x, y, z) z - x * y
  
  #define SIMD_DUPLICATE(x) *(x)

#endif

#if 0
#ifdef X86_SCALAR
#elif defined(X86_SSE)
#elif defined(X86_AVX)
#elif defined(X86_AVX512)
#else
  #error "That ISA is not recognized!!!\n"
#endif
#endif

