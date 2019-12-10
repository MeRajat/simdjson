#ifndef SIMDJSON_HASWELL_SIMD_INPUT_H
#define SIMDJSON_HASWELL_SIMD_INPUT_H

#include "simdjson/common_defs.h"
#include "simdjson/portability.h"
#include "simdjson/simdjson.h"
#include "haswell/intrinsics.h"

#ifdef IS_X86_64

TARGET_HASWELL
namespace simdjson::haswell {

struct simd_input {
  const __m256i chunks[2];

  really_inline simd_input() : chunks{__m256i(), __m256i()} {}

  really_inline simd_input(const __m256i chunk0, const __m256i chunk1)
      : chunks{chunk0, chunk1} {}

  really_inline simd_input(const uint8_t *ptr)
      : chunks{
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(ptr + 0*32)),
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(ptr + 1*32))
      } {}

  template <typename F>
  really_inline void each(F const& each_chunk) const
  {
    each_chunk(this->chunks[0]);
    each_chunk(this->chunks[1]);
  }

  template <typename F>
  really_inline simd_input map(F const& map_chunk) const {
    return simd_input(
      map_chunk(this->chunks[0]),
      map_chunk(this->chunks[1])
    );
  }

  template <typename F>
  really_inline simd_input map(const simd_input b, F const& map_chunk) const {
    return simd_input(
      map_chunk(this->chunks[0], b.chunks[0]),
      map_chunk(this->chunks[1], b.chunks[1])
    );
  }

  template <typename F>
  really_inline __m256i reduce(F const& reduce_pair) const {
    return reduce_pair(this->chunks[0], this->chunks[1]);
  }

  really_inline uint64_t to_bitmask() const {
    uint64_t r_lo = static_cast<uint32_t>(_mm256_movemask_epi8(this->chunks[0]));
    uint64_t r_hi =                       _mm256_movemask_epi8(this->chunks[1]);
    return r_lo | (r_hi << 32);
  }

  really_inline simd_input bit_or(const uint8_t m) const {
    const __m256i mask = _mm256_set1_epi8(m);
    return this->map( [&](auto a) {
      return _mm256_or_si256(a, mask);
    });
  }

  really_inline uint64_t eq(const uint8_t m) const {
    const __m256i mask = _mm256_set1_epi8(m);
    return this->map( [&](auto a) {
      return _mm256_cmpeq_epi8(a, mask);
    }).to_bitmask();
  }

  really_inline uint64_t lteq(const uint8_t m) const {
    const __m256i maxval = _mm256_set1_epi8(m);
    return this->map( [&](auto a) {
      return _mm256_cmpeq_epi8(_mm256_max_epu8(maxval, a), maxval);
    }).to_bitmask();
  }

}; // struct simd_input

} // namespace simdjson::haswell
UNTARGET_REGION

#endif // IS_X86_64
#endif // SIMDJSON_HASWELL_SIMD_INPUT_H
