/*
 * Compile with:
 * gcc -O2 -Wall -mtune=skylake-avx512 -march=skylake-avx512 test_avx512.c --save-temps -o test_avx512
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <x86intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define VEC_SIZE 16

void fprint_vec(FILE *outf, float* tab) {
  char *sep = "";
  fprintf(outf, "[");
  for (int i = 0; i < VEC_SIZE; i++) {
      fprintf(outf, "%s%.2f, ", sep, tab[i]);
    sep = ", ";
  }
  fprintf(outf, "]\n");
}

int64_t eval_fmadd_parallel_16(void *args[], int64_t runs) {
  __m512 a = _mm512_set1_ps(2);
  __m512 b = _mm512_set1_ps(3);
  __m512 acc = _mm512_set1_ps(1);
  __m512 acc2 = _mm512_set1_ps(2);
  __m512 acc3 = _mm512_set1_ps(3);
  __m512 acc4 = _mm512_set1_ps(4);
  __m512 acc5 = _mm512_set1_ps(5);
  __m512 acc6 = _mm512_set1_ps(6);
  __m512 acc7 = _mm512_set1_ps(7);
  __m512 acc8 = _mm512_set1_ps(8);
  __m512 acc9 = _mm512_set1_ps(9);
  __m512 acc10 = _mm512_set1_ps(10);
  __m512 acc11 = _mm512_set1_ps(11);
  __m512 acc12 = _mm512_set1_ps(12);
  __m512 acc13 = _mm512_set1_ps(13);
  __m512 acc14 = _mm512_set1_ps(14);
  __m512 acc15 = _mm512_set1_ps(15);
  __m512 acc16 = _mm512_set1_ps(16);
  for (int64_t i = 0; i < runs; i++) {
#define UNROLLED 16
    acc = _mm512_fmadd_ps(a, b, acc);
    acc2 = _mm512_fmadd_ps(a, b, acc2);
    acc3 = _mm512_fmadd_ps(a, b, acc3);
    acc4 = _mm512_fmadd_ps(a, b, acc4);
    acc5 = _mm512_fmadd_ps(a, b, acc5);
    acc6 = _mm512_fmadd_ps(a, b, acc6);
    acc7 = _mm512_fmadd_ps(a, b, acc7);
    acc8 = _mm512_fmadd_ps(a, b, acc8);
    acc9 = _mm512_fmadd_ps(a, b, acc9);
    acc10 = _mm512_fmadd_ps(a, b, acc10);
    acc11 = _mm512_fmadd_ps(a, b, acc11);
    acc12 = _mm512_fmadd_ps(a, b, acc12);
    acc13 = _mm512_fmadd_ps(a, b, acc13);
    acc14 = _mm512_fmadd_ps(a, b, acc14);
    acc15 = _mm512_fmadd_ps(a, b, acc15);
    acc16 = _mm512_fmadd_ps(a, b, acc16);
  }
  float *out = (float *)args[0];
  _mm512_store_ps(&out[0*VEC_SIZE], acc);
  _mm512_store_ps(&out[1*VEC_SIZE], acc2);
  _mm512_store_ps(&out[2*VEC_SIZE], acc3);
  _mm512_store_ps(&out[3*VEC_SIZE], acc4);
  _mm512_store_ps(&out[4*VEC_SIZE], acc5);
  _mm512_store_ps(&out[5*VEC_SIZE], acc6);
  _mm512_store_ps(&out[6*VEC_SIZE], acc7);
  _mm512_store_ps(&out[7*VEC_SIZE], acc8);
  _mm512_store_ps(&out[8*VEC_SIZE], acc9);
  _mm512_store_ps(&out[9*VEC_SIZE], acc10);
  _mm512_store_ps(&out[10*VEC_SIZE], acc11);
  _mm512_store_ps(&out[11*VEC_SIZE], acc12);
  _mm512_store_ps(&out[12*VEC_SIZE], acc13);
  _mm512_store_ps(&out[13*VEC_SIZE], acc14);
  _mm512_store_ps(&out[14*VEC_SIZE], acc15);
  _mm512_store_ps(&out[15*VEC_SIZE], acc16);
  return runs * UNROLLED;
#undef UNROLLED
}

int main() {
  int num_fmadds;
  void *args[1];

  args[0] = aligned_alloc(32, 16 * VEC_SIZE * sizeof(float));

  num_fmadds = eval_fmadd_parallel_16(args, 10);

  printf("Num fmadd: %d\n", num_fmadds);
  for (int i = 0; i < 16; i++) {
      fprint_vec(stdout, &((float *)args[0])[i*VEC_SIZE]);
  }

  free(args[0]);
  return 0;
}
