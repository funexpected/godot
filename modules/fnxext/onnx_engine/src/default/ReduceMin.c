#include "../onnx.h"

struct operator_pdata_t {
	int * axes;
	int naxes;
	int keepdims;

	int * caxes;
};

static int ReduceMin_init(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat;
	int64_t * ints;
	int nint;
	int i;

	if((n->ninput == 1) && (n->noutput == 1))
	{
		pdat = onnx_malloc(sizeof(struct operator_pdata_t));
		if(pdat)
		{
			nint = onnx_attribute_read_ints(n, "axes", &ints);
			if(nint > 0)
				pdat->naxes = nint;
			else
				pdat->naxes = n->inputs[0]->ndim;
			pdat->axes = onnx_malloc(sizeof(int) * pdat->naxes);
			pdat->caxes = onnx_malloc(sizeof(int) * pdat->naxes);
			if(pdat->axes && pdat->caxes)
			{
				if(nint > 0)
				{
					for(i = 0; i < pdat->naxes; i++)
						pdat->axes[i] = ints[i];
				}
				else
				{
					for(i = 0; i < pdat->naxes; i++)
						pdat->axes[i] = i;
				}
				pdat->keepdims = onnx_attribute_read_int(n, "keepdims", 1);
				n->priv = pdat;
				return 1;
			}
			else
			{
				if(pdat->axes)
					onnx_free(pdat->axes);
				if(pdat->caxes)
					onnx_free(pdat->caxes);
				onnx_free(pdat);
			}
		}
	}
	return 0;
}

static int ReduceMin_exit(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;

	if(pdat)
	{
		if(pdat->axes)
			onnx_free(pdat->axes);
		if(pdat->caxes)
			onnx_free(pdat->caxes);
		onnx_free(pdat);
	}
	return 1;
}

static int ReduceMin_reshape(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	int ndim = x->ndim;
	int dims[ndim];
	int axis, found;
	int i, j;

	for(i = 0; i < pdat->naxes; i++)
	{
		axis = pdat->axes[i];
		if(axis < 0)
			axis += x->ndim;
		if(axis < 0 || axis >= x->ndim)
			return 0;
		pdat->caxes[i] = axis;
	}
	if(pdat->keepdims)
	{
		onnx_memcpy(dims, x->dims, sizeof(int) * ndim);
		for(i = 0; i < pdat->naxes; i++)
			dims[pdat->caxes[i]] = 1;
	}
	else
	{
		for(i = 0, ndim = 0; i < x->ndim; i++)
		{
			for(j = 0, found = 0; j < pdat->naxes; j++)
			{
				if(i == pdat->caxes[j])
				{
					found = 1;
					break;
				}
			}
			if(!found)
				dims[ndim++]= x->dims[i];
		}
	}
	return onnx_tensor_reshape(y, dims, ndim, x->type);
}

static inline int dim_next(int ndim, int * dims, int * dim_max)
{
	if(ndim == 0)
		return 0;
	while(1)
	{
		ndim = ndim - 1;
		dims[ndim] += 1;
		if(dims[ndim] < dim_max[ndim])
			return 1;
		else
		{
			if(ndim == 0)
				return 0;
			dims[ndim] = 0;
		}
	}
}

static inline int dim_offset(int ndim, int * dims, int * distance)
{
	int i, o;

	if(ndim == 0)
		return 0;
	for(i = ndim - 1, o = 0; i >= 0; i--)
		o += dims[i] * distance[i];
	return o;
}

static void ReduceMin_int8(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	int8_t * px = (int8_t *)x->datas;
	int8_t * py = (int8_t *)y->datas;
	int8_t minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = px[o];
		do
		{
			t = px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)];
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = minv;
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_int32(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	int32_t * px = (int32_t *)x->datas;
	int32_t * py = (int32_t *)y->datas;
	int32_t minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = px[o];
		do
		{
			t = px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)];
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = minv;
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_int64(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	int64_t * px = (int64_t *)x->datas;
	int64_t * py = (int64_t *)y->datas;
	int64_t minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = px[o];
		do
		{
			t = px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)];
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = minv;
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_uint8(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	uint8_t * px = (uint8_t *)x->datas;
	uint8_t * py = (uint8_t *)y->datas;
	uint8_t minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = px[o];
		do
		{
			t = px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)];
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = minv;
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_uint32(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	uint32_t * px = (uint32_t *)x->datas;
	uint32_t * py = (uint32_t *)y->datas;
	uint32_t minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = px[o];
		do
		{
			t = px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)];
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = minv;
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_uint64(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	uint64_t * px = (uint64_t *)x->datas;
	uint64_t * py = (uint64_t *)y->datas;
	uint64_t minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = px[o];
		do
		{
			t = px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)];
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = minv;
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_bfloat16(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	uint16_t * px = (uint16_t *)x->datas;
	uint16_t * py = (uint16_t *)y->datas;
	float minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = bfloat16_to_float32(px[o]);
		do
		{
			t = bfloat16_to_float32(px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)]);
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = float32_to_bfloat16(minv);
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_float16(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	uint16_t * px = (uint16_t *)x->datas;
	uint16_t * py = (uint16_t *)y->datas;
	float minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = float16_to_float32(px[o]);
		do
		{
			t = float16_to_float32(px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)]);
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = float32_to_float16(minv);
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_float32(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	float * px = (float *)x->datas;
	float * py = (float *)y->datas;
	float minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = px[o];
		do
		{
			t = px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)];
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = minv;
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

static void ReduceMin_float64(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	double * px = (double *)x->datas;
	double * py = (double *)y->datas;
	double minv, t;
	int not_in_axes_num = x->ndim - pdat->naxes;
	int iter_not_in_axes_max[not_in_axes_num];
	int iter_not_in_axes[not_in_axes_num];
	int not_in_axes_axis_dis[x->ndim];
	int iter_in_axes_max[pdat->naxes];
	int in_axes_axis_dis[pdat->naxes];
	int iter_in_axes[pdat->naxes];
	uint32_t mask;
	int i, j, k, o;

	for(i = 0, mask = 0; i < pdat->naxes; i++)
		mask |= (1 << pdat->caxes[i]);
	for(i = 0, j = 0, k = 0; i < x->ndim; i++)
	{
		if(mask & (1 << i))
		{
			in_axes_axis_dis[j] = x->strides[i];
			iter_in_axes_max[j] = x->dims[i];
			j += 1;
			continue;
		}
		not_in_axes_axis_dis[k] = x->strides[i];
		iter_not_in_axes_max[k] = x->dims[i];
		k += 1;
	}
	i = 0;
	onnx_memset(iter_not_in_axes, 0, sizeof(int) * not_in_axes_num);
	do
	{
		onnx_memset(iter_in_axes, 0, sizeof(int) * pdat->naxes);
		o = dim_offset(not_in_axes_num, iter_not_in_axes, not_in_axes_axis_dis);
		minv = px[o];
		do
		{
			t = px[o + dim_offset(pdat->naxes, iter_in_axes, in_axes_axis_dis)];
			if(minv > t)
				minv = t;
		} while(dim_next(pdat->naxes, iter_in_axes, iter_in_axes_max));
		py[i++] = minv;
	} while(dim_next(not_in_axes_num, iter_not_in_axes, iter_not_in_axes_max));
}

void resolver_default_op_ReduceMin(struct onnx_node_t * n)
{
	if(n->opset >= 13)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_INT8:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int8;
			break;
		case ONNX_TENSOR_TYPE_INT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int32;
			break;
		case ONNX_TENSOR_TYPE_INT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int64;
			break;
		case ONNX_TENSOR_TYPE_UINT8:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint8;
			break;
		case ONNX_TENSOR_TYPE_UINT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint32;
			break;
		case ONNX_TENSOR_TYPE_UINT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint64;
			break;
		case ONNX_TENSOR_TYPE_BFLOAT16:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_bfloat16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float64;
			break;
		default:
			break;
		}
	}
	else if(n->opset >= 12)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_INT8:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int8;
			break;
		case ONNX_TENSOR_TYPE_INT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int32;
			break;
		case ONNX_TENSOR_TYPE_INT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int64;
			break;
		case ONNX_TENSOR_TYPE_UINT8:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint8;
			break;
		case ONNX_TENSOR_TYPE_UINT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint32;
			break;
		case ONNX_TENSOR_TYPE_UINT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint64;
			break;
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float64;
			break;
		default:
			break;
		}
	}
	else if(n->opset >= 11)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_INT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int32;
			break;
		case ONNX_TENSOR_TYPE_INT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int64;
			break;
		case ONNX_TENSOR_TYPE_UINT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint32;
			break;
		case ONNX_TENSOR_TYPE_UINT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint64;
			break;
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float64;
			break;
		default:
			break;
		}
	}
	else if(n->opset >= 1)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_INT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int32;
			break;
		case ONNX_TENSOR_TYPE_INT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_int64;
			break;
		case ONNX_TENSOR_TYPE_UINT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint32;
			break;
		case ONNX_TENSOR_TYPE_UINT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_uint64;
			break;
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = ReduceMin_init;
			n->exit = ReduceMin_exit;
			n->reshape = ReduceMin_reshape;
			n->operator_ = ReduceMin_float64;
			break;
		default:
			break;
		}
	}
}
