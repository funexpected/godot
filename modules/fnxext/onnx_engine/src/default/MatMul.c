#include "../onnx.h"
#include "../matrix.h"

struct operator_pdata_t {
	int m;
	int n;
	int k;
};

static int MatMul_init(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat;

	if((n->ninput == 2) && (n->noutput == 1))
	{
		pdat = onnx_malloc(sizeof(struct operator_pdata_t));
		if(pdat)
		{
			pdat->m = 0;
			pdat->n = 0;
			pdat->k = 0;
			n->priv = pdat;
			return 1;
		}
	}
	return 0;
}

static int MatMul_exit(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;

	if(pdat)
		onnx_free(pdat);
	return 1;
}

static int MatMul_reshape(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	int andim;
	int * adims;
	int bndim;
	int * bdims;

	if(a->ndim == 1)
	{
		adims = (int[]){ 1, a->dims[0] };
		andim = 2;
	}
	else
	{
		adims = a->dims;
		andim = a->ndim;
	}
	if(b->ndim == 1)
	{
		bdims = (int[]){ b->dims[0], 1 };
		bndim = 2;
	}
	else
	{
		bdims = b->dims;
		bndim = b->ndim;
	}
	int ndim = XMAX(andim, bndim);
	int dims[ndim];
	if(andim < 2 || bndim < 2)
		return 0;
	if(adims[andim - 1] != bdims[bndim - 2])
		return 0;
	dims[ndim - 2] = adims[andim - 2];
	dims[ndim - 1] = bdims[bndim - 1];
	for(int i = 3; i <= ndim; i++)
	{
		int alen = (andim - i) < 0 ? 1 : adims[andim - i];
		int blen = (bndim - i) < 0 ? 1 : bdims[bndim - i];
		if(alen != blen && alen > 1 && blen > 1)
			return 0;
		dims[ndim - i] = XMAX(alen, blen);
	}
	pdat->m = adims[andim - 2];
	pdat->n = bdims[bndim - 1];
	pdat->k = adims[andim - 1];
	return onnx_tensor_reshape(y, dims, ndim, a->type);
}

static void MatMul_int32(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	int32_t * py = (int32_t *)y->datas;
	int32_t * pa;
	int32_t * pb;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);
		matrix_mul(pdat->m, pdat->n, pdat->k, pa, pb, &py[i], ONNX_TENSOR_TYPE_INT32);
	}
}

static void MatMul_int64(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	int64_t * py = (int64_t *)y->datas;
	int64_t * pa;
	int64_t * pb;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);
		matrix_mul(pdat->m, pdat->n, pdat->k, pa, pb, &py[i], ONNX_TENSOR_TYPE_INT64);
	}
}

static void MatMul_uint32(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	uint32_t * py = (uint32_t *)y->datas;
	uint32_t * pa;
	uint32_t * pb;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);
		matrix_mul(pdat->m, pdat->n, pdat->k, pa, pb, &py[i], ONNX_TENSOR_TYPE_UINT32);
	}
}

static void MatMul_uint64(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	uint64_t * py = (uint64_t *)y->datas;
	uint64_t * pa;
	uint64_t * pb;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);
		matrix_mul(pdat->m, pdat->n, pdat->k, pa, pb, &py[i], ONNX_TENSOR_TYPE_UINT64);
	}
}

static void MatMul_bfloat16(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	uint16_t * py = (uint16_t *)y->datas;
	uint16_t * pa;
	uint16_t * pb;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);
		matrix_mul(pdat->m, pdat->n, pdat->k, pa, pb, &py[i], ONNX_TENSOR_TYPE_BFLOAT16);
	}
}

static void MatMul_float16(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	uint16_t * py = (uint16_t *)y->datas;
	uint16_t * pa;
	uint16_t * pb;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);
		matrix_mul(pdat->m, pdat->n, pdat->k, pa, pb, &py[i], ONNX_TENSOR_TYPE_FLOAT16);
	}
}

static void MatMul_float32(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	float * py = (float *)y->datas;
	float * pa;
	float * pb;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);
		matrix_mul(pdat->m, pdat->n, pdat->k, pa, pb, &py[i], ONNX_TENSOR_TYPE_FLOAT32);
	}
}

static void MatMul_float64(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	double * py = (double *)y->datas;
	double * pa;
	double * pb;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);
		matrix_mul(pdat->m, pdat->n, pdat->k, pa, pb, &py[i], ONNX_TENSOR_TYPE_FLOAT64);
	}
}

void resolver_default_op_MatMul(struct onnx_node_t * n)
{
	if(n->opset >= 13)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_INT32:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_int32;
			break;
		case ONNX_TENSOR_TYPE_INT64:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_int64;
			break;
		case ONNX_TENSOR_TYPE_UINT32:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_uint32;
			break;
		case ONNX_TENSOR_TYPE_UINT64:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_uint64;
			break;
		case ONNX_TENSOR_TYPE_BFLOAT16:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_bfloat16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float64;
			break;
		default:
			break;
		}
	}
	else if(n->opset >= 9)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_INT32:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_int32;
			break;
		case ONNX_TENSOR_TYPE_INT64:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_int64;
			break;
		case ONNX_TENSOR_TYPE_UINT32:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_uint32;
			break;
		case ONNX_TENSOR_TYPE_UINT64:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_uint64;
			break;
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float64;
			break;
		default:
			break;
		}
	}
	else if(n->opset >= 1)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = MatMul_init;
			n->exit = MatMul_exit;
			n->reshape = MatMul_reshape;
			n->operator_ = MatMul_float64;
			break;
		default:
			break;
		}
	}
}
