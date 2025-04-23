

#include "matrix.h"

/**
 * Performs matrix multiplication for various data types
 * 
 * X = A * B
 * 
 * @param m Number of rows in A
 * @param n Number of columns in B
 * @param k Number of columns in A / rows in B
 * @param A Matrix A data
 * @param B Matrix B data
 * @param X Output matrix data
 * @param type Tensor data type
 */
void matrix_mul(int m, int n, int k, void * A, void * B, void * X, enum onnx_tensor_type_t type)
{
    switch(type)
    {
        case ONNX_TENSOR_TYPE_INT32:
        {
            int32_t * pa = (int32_t *)A;
            int32_t * pb = (int32_t *)B;
            int32_t * px = (int32_t *)X;
            int32_t sum;

            for(int u = 0; u < m; u++)
            {
                for(int v = 0; v < n; v++)
                {
                    sum = 0;
                    for(int w = 0; w < k; w++)
                        sum += pa[u * k + w] * pb[w * n + v];
                    px[u * n + v] = sum;
                }
            }
            break;
        }
        case ONNX_TENSOR_TYPE_INT64:
        {
            int64_t * pa = (int64_t *)A;
            int64_t * pb = (int64_t *)B;
            int64_t * px = (int64_t *)X;
            int64_t sum;

            for(int u = 0; u < m; u++)
            {
                for(int v = 0; v < n; v++)
                {
                    sum = 0;
                    for(int w = 0; w < k; w++)
                        sum += pa[u * k + w] * pb[w * n + v];
                    px[u * n + v] = sum;
                }
            }
            break;
        }
        case ONNX_TENSOR_TYPE_UINT32:
        {
            uint32_t * pa = (uint32_t *)A;
            uint32_t * pb = (uint32_t *)B;
            uint32_t * px = (uint32_t *)X;
            uint32_t sum;

            for(int u = 0; u < m; u++)
            {
                for(int v = 0; v < n; v++)
                {
                    sum = 0;
                    for(int w = 0; w < k; w++)
                        sum += pa[u * k + w] * pb[w * n + v];
                    px[u * n + v] = sum;
                }
            }
            break;
        }
        case ONNX_TENSOR_TYPE_UINT64:
        {
            uint64_t * pa = (uint64_t *)A;
            uint64_t * pb = (uint64_t *)B;
            uint64_t * px = (uint64_t *)X;
            uint64_t sum;

            for(int u = 0; u < m; u++)
            {
                for(int v = 0; v < n; v++)
                {
                    sum = 0;
                    for(int w = 0; w < k; w++)
                        sum += pa[u * k + w] * pb[w * n + v];
                    px[u * n + v] = sum;
                }
            }
            break;
        }
        case ONNX_TENSOR_TYPE_BFLOAT16:
        {
            uint16_t * pa = (uint16_t *)A;
            uint16_t * pb = (uint16_t *)B;
            uint16_t * px = (uint16_t *)X;
            float sum;

            for(int u = 0; u < m; u++)
            {
                for(int v = 0; v < n; v++)
                {
                    sum = 0;
                    for(int w = 0; w < k; w++)
                        sum += bfloat16_to_float32(pa[u * k + w]) * bfloat16_to_float32(pb[w * n + v]);
                    px[u * n + v] = float32_to_bfloat16(sum);
                }
            }
            break;
        }
        case ONNX_TENSOR_TYPE_FLOAT16:
        {
            uint16_t * pa = (uint16_t *)A;
            uint16_t * pb = (uint16_t *)B;
            uint16_t * px = (uint16_t *)X;
            float sum;

            for(int u = 0; u < m; u++)
            {
                for(int v = 0; v < n; v++)
                {
                    sum = 0;
                    for(int w = 0; w < k; w++)
                        sum += float16_to_float32(pa[u * k + w]) * float16_to_float32(pb[w * n + v]);
                    px[u * n + v] = float32_to_float16(sum);
                }
            }
            break;
        }
        case ONNX_TENSOR_TYPE_FLOAT32:
        {
            float * pa = (float *)A;
            float * pb = (float *)B;
            float * px = (float *)X;
            float sum;

            for(int u = 0; u < m; u++)
            {
                for(int v = 0; v < n; v++)
                {
                    sum = 0;
                    for(int w = 0; w < k; w++)
                        sum += pa[u * k + w] * pb[w * n + v];
                    px[u * n + v] = sum;
                }
            }
            break;
        }
        case ONNX_TENSOR_TYPE_FLOAT64:
        {
            double * pa = (double *)A;
            double * pb = (double *)B;
            double * px = (double *)X;
            double sum;

            for(int u = 0; u < m; u++)
            {
                for(int v = 0; v < n; v++)
                {
                    sum = 0;
                    for(int w = 0; w < k; w++)
                        sum += pa[u * k + w] * pb[w * n + v];
                    px[u * n + v] = sum;
                }
            }
            break;
        }
        default:
            break;
    }
}