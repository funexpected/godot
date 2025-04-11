#include "../onnx.h"
#include "../matrix.h"

struct operator_pdata_t {
	int m;
	int n;
	int k;
	int8_t * a_zero_point;
	int8_t * b_zero_point;
	int has_a_zero_point;
	int has_b_zero_point;
};

static int MatMulInteger_init(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat;

	if((n->ninput >= 2) && (n->ninput <= 4) && (n->noutput == 1))
	{
		pdat = onnx_malloc(sizeof(struct operator_pdata_t));
		if(pdat)
		{
			pdat->m = 0;
			pdat->n = 0;
			pdat->k = 0;
			pdat->a_zero_point = NULL;
			pdat->b_zero_point = NULL;
			pdat->has_a_zero_point = (n->ninput >= 3) ? 1 : 0;
			pdat->has_b_zero_point = (n->ninput >= 4) ? 1 : 0;
			n->priv = pdat;
			return 1;
		}
	}
	return 0;
}

static int MatMulInteger_exit(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;

	if(pdat)
	{
		if(pdat->a_zero_point)
			onnx_free(pdat->a_zero_point);
		if(pdat->b_zero_point)
			onnx_free(pdat->b_zero_point);
		onnx_free(pdat);
	}
	return 1;
}

static int MatMulInteger_reshape(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	struct onnx_tensor_t * azp = pdat->has_a_zero_point ? n->inputs[2] : NULL;
	struct onnx_tensor_t * bzp = pdat->has_b_zero_point ? n->inputs[3] : NULL;
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

	// Handle zero points
	if(pdat->a_zero_point)
	{
		onnx_free(pdat->a_zero_point);
		pdat->a_zero_point = NULL;
	}
	if(pdat->b_zero_point)
	{
		onnx_free(pdat->b_zero_point);
		pdat->b_zero_point = NULL;
	}

	if(azp && azp->ndata > 0)
	{
		if(azp->type == ONNX_TENSOR_TYPE_INT8)
			pdat->a_zero_point = onnx_malloc(sizeof(int8_t) * azp->ndata);
		else // UINT8
			pdat->a_zero_point = onnx_malloc(sizeof(int8_t) * azp->ndata);
		
		if(pdat->a_zero_point)
		{
			if(azp->type == ONNX_TENSOR_TYPE_INT8)
				memcpy(pdat->a_zero_point, azp->datas, sizeof(int8_t) * azp->ndata);
			else // UINT8
			{
				// Convert uint8_t to int8_t
				uint8_t * p = (uint8_t *)azp->datas;
				for(size_t i = 0; i < azp->ndata; i++)
					pdat->a_zero_point[i] = (int8_t)(p[i]);
			}
		}
	}

	if(bzp && bzp->ndata > 0)
	{
		if(bzp->type == ONNX_TENSOR_TYPE_INT8)
			pdat->b_zero_point = onnx_malloc(sizeof(int8_t) * bzp->ndata);
		else // UINT8
			pdat->b_zero_point = onnx_malloc(sizeof(int8_t) * bzp->ndata);
		
		if(pdat->b_zero_point)
		{
			if(bzp->type == ONNX_TENSOR_TYPE_INT8)
				memcpy(pdat->b_zero_point, bzp->datas, sizeof(int8_t) * bzp->ndata);
			else // UINT8
			{
				// Convert uint8_t to int8_t
				uint8_t * p = (uint8_t *)bzp->datas;
				for(size_t i = 0; i < bzp->ndata; i++)
					pdat->b_zero_point[i] = (int8_t)(p[i]);
			}
		}
	}

	return onnx_tensor_reshape(y, dims, ndim, ONNX_TENSOR_TYPE_INT32);
}

static void MatMulInteger_int8(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	int32_t * py = (int32_t *)y->datas;
	int8_t * pa;
	int8_t * pb;
	int8_t a_zp = pdat->a_zero_point ? pdat->a_zero_point[0] : 0;
	int8_t b_zp = pdat->b_zero_point ? pdat->b_zero_point[0] : 0;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);

		// MatMul with zero point handling
		for(int u = 0; u < pdat->m; u++)
		{
			for(int v = 0; v < pdat->n; v++)
			{
				int32_t sum = 0;
				for(int w = 0; w < pdat->k; w++)
				{
					sum += ((int32_t)pa[u * pdat->k + w] - a_zp) * ((int32_t)pb[w * pdat->n + v] - b_zp);
				}
				py[i + u * pdat->n + v] = sum;
			}
		}
	}
}

static void MatMulInteger_uint8(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	int32_t * py = (int32_t *)y->datas;
	uint8_t * pa;
	uint8_t * pb;
	uint8_t a_zp = pdat->a_zero_point ? (uint8_t)pdat->a_zero_point[0] : 0;
	uint8_t b_zp = pdat->b_zero_point ? (uint8_t)pdat->b_zero_point[0] : 0;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);

		// MatMul with zero point handling
		for(int u = 0; u < pdat->m; u++)
		{
			for(int v = 0; v < pdat->n; v++)
			{
				int32_t sum = 0;
				for(int w = 0; w < pdat->k; w++)
				{
					sum += ((int32_t)pa[u * pdat->k + w] - a_zp) * ((int32_t)pb[w * pdat->n + v] - b_zp);
				}
				py[i + u * pdat->n + v] = sum;
			}
		}
	}
}

static void MatMulInteger_int8_uint8(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	int32_t * py = (int32_t *)y->datas;
	int8_t * pa;
	uint8_t * pb;
	int8_t a_zp = pdat->a_zero_point ? pdat->a_zero_point[0] : 0;
	uint8_t b_zp = pdat->b_zero_point ? (uint8_t)pdat->b_zero_point[0] : 0;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);

		// MatMul with zero point handling
		for(int u = 0; u < pdat->m; u++)
		{
			for(int v = 0; v < pdat->n; v++)
			{
				int32_t sum = 0;
				for(int w = 0; w < pdat->k; w++)
				{
					sum += ((int32_t)pa[u * pdat->k + w] - a_zp) * ((int32_t)pb[w * pdat->n + v] - b_zp);
				}
				py[i + u * pdat->n + v] = sum;
			}
		}
	}
}

static void MatMulInteger_uint8_int8(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * a = n->inputs[0];
	struct onnx_tensor_t * b = n->inputs[1];
	int32_t * py = (int32_t *)y->datas;
	uint8_t * pa;
	int8_t * pb;
	uint8_t a_zp = pdat->a_zero_point ? (uint8_t)pdat->a_zero_point[0] : 0;
	int8_t b_zp = pdat->b_zero_point ? pdat->b_zero_point[0] : 0;

	for(size_t i = 0, l = y->ndata; i < l; i += pdat->m * pdat->n)
	{
		pa = onnx_tensor_broadcast_map_address(a, y, i);
		pb = onnx_tensor_broadcast_map_address(b, y, i);

		// MatMul with zero point handling
		for(int u = 0; u < pdat->m; u++)
		{
			for(int v = 0; v < pdat->n; v++)
			{
				int32_t sum = 0;
				for(int w = 0; w < pdat->k; w++)
				{
					sum += ((int32_t)pa[u * pdat->k + w] - a_zp) * ((int32_t)pb[w * pdat->n + v] - b_zp);
				}
				py[i + u * pdat->n + v] = sum;
			}
		}
	}
}

void resolver_default_op_MatMulInteger(struct onnx_node_t * n)
{
	if(n->opset >= 10)
	{
		if((n->inputs[0]->type == ONNX_TENSOR_TYPE_INT8) && (n->inputs[1]->type == ONNX_TENSOR_TYPE_INT8))
		{
			n->init = MatMulInteger_init;
			n->exit = MatMulInteger_exit;
			n->reshape = MatMulInteger_reshape;
			n->operator_ = MatMulInteger_int8;
		}
		else if((n->inputs[0]->type == ONNX_TENSOR_TYPE_UINT8) && (n->inputs[1]->type == ONNX_TENSOR_TYPE_UINT8))
		{
			n->init = MatMulInteger_init;
			n->exit = MatMulInteger_exit;
			n->reshape = MatMulInteger_reshape;
			n->operator_ = MatMulInteger_uint8;
		}
		else if((n->inputs[0]->type == ONNX_TENSOR_TYPE_INT8) && (n->inputs[1]->type == ONNX_TENSOR_TYPE_UINT8))
		{
			n->init = MatMulInteger_init;
			n->exit = MatMulInteger_exit;
			n->reshape = MatMulInteger_reshape;
			n->operator_ = MatMulInteger_int8_uint8;
		}
		else if((n->inputs[0]->type == ONNX_TENSOR_TYPE_UINT8) && (n->inputs[1]->type == ONNX_TENSOR_TYPE_INT8))
		{
			n->init = MatMulInteger_init;
			n->exit = MatMulInteger_exit;
			n->reshape = MatMulInteger_reshape;
			n->operator_ = MatMulInteger_uint8_int8;
		}
	}
}