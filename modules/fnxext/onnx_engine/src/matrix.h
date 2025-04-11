#ifndef __MATRIX_H__
#define __MATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "onnx.h"

void matrix_mul(int m, int n, int k, void * A, void * B, void * X, enum onnx_tensor_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* __MATRIX_H__ */