#include "../onnx.h"

enum auto_pad_t {
	AUTO_PAD_NOTSET		= 0,
	AUTO_PAD_SAME_UPPER	= 1,
	AUTO_PAD_SAME_LOWER	= 2,
	AUTO_PAD_VALID		= 3,
};

struct operator_pdata_t {
	enum auto_pad_t auto_pad;
	int group;
	int * kernels;
	int nkernel;
	int * dilations;
	int ndilation;
	int * pads;
	int npad;
	int * strides;
	int nstride;

	int cpads[32];
};

static int ConvInteger_init(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat;
	int64_t * ints;
	int i, l;

	if((n->ninput >= 2) && (n->noutput == 1))
	{
		pdat = onnx_malloc(sizeof(struct operator_pdata_t));
		if(pdat)
		{
			onnx_memset(pdat, 0, sizeof(struct operator_pdata_t));
			pdat->auto_pad = AUTO_PAD_NOTSET;
			// switch(shash(onnx_attribute_read_string(n, "auto_pad", "NOTSET")))
			// {
			// case 0xc3966fc2: /* "NOTSET" */
			// 	pdat->auto_pad = AUTO_PAD_NOTSET;
			// 	break;
			// case 0xcbbc7856: /* "SAME_UPPER" */
			// 	pdat->auto_pad = AUTO_PAD_SAME_UPPER;
			// 	break;
			// case 0xcb192d33: /* "SAME_LOWER" */
			// 	pdat->auto_pad = AUTO_PAD_SAME_LOWER;
			// 	break;
			// case 0x0e382d15: /* "VALID" */
			// 	pdat->auto_pad = AUTO_PAD_VALID;
			// 	break;
			// default:
			// 	pdat->auto_pad = AUTO_PAD_NOTSET;
			// 	break;
			// }
			pdat->group = onnx_attribute_read_int(n, "group", 1);
			pdat->nkernel = onnx_attribute_read_ints(n, "kernel_shape", &ints);
			if(pdat->nkernel > 0)
			{
				pdat->kernels = onnx_malloc(sizeof(int) * pdat->nkernel);
				for(i = 0; i < pdat->nkernel; i++)
					pdat->kernels[i] = ints[i];
			}
			pdat->ndilation = pdat->nkernel;
			pdat->dilations = onnx_malloc(sizeof(int) * pdat->ndilation);
			if(pdat->dilations)
			{
				l = onnx_attribute_read_ints(n, "dilations", &ints);
				for(i = 0; i < l; i++)
					pdat->dilations[i] = ints[i];
				for(; i < pdat->ndilation; i++)
					pdat->dilations[i] = 1;
			}
			pdat->npad = pdat->nkernel * 2;
			pdat->pads = onnx_malloc(sizeof(int) * pdat->npad);
			if(pdat->pads)
			{
				l = onnx_attribute_read_ints(n, "pads", &ints);
				for(i = 0; i < l; i++)
					pdat->pads[i] = ints[i];
				for(; i < pdat->npad; i++)
					pdat->pads[i] = 0;
			}
			pdat->nstride = pdat->nkernel;
			pdat->strides = onnx_malloc(sizeof(int) * pdat->nstride);
			if(pdat->strides)
			{
				l = onnx_attribute_read_ints(n, "strides", &ints);
				for(i = 0; i < l; i++)
					pdat->strides[i] = ints[i];
				for(; i < pdat->nstride; i++)
					pdat->strides[i] = 1;
			}
			n->priv = pdat;
			return 1;
		}
	}
	return 0;
}

static int ConvInteger_exit(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;

	if(pdat)
	{
		if(pdat->kernels)
			onnx_free(pdat->kernels);
		if(pdat->dilations)
			onnx_free(pdat->dilations);
		if(pdat->pads)
			onnx_free(pdat->pads);
		if(pdat->strides)
			onnx_free(pdat->strides);
		onnx_free(pdat);
	}
	return 1;
}

static int ConvInteger_reshape(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * w = n->inputs[1];
	int ndim = x->ndim;
	int dims[ndim];
	int pad;
	int i;

	switch(pdat->auto_pad)
	{
	case AUTO_PAD_NOTSET:
		onnx_memcpy(pdat->cpads, pdat->pads, sizeof(int) * pdat->npad);
		break;
	case AUTO_PAD_SAME_UPPER:
		for(i = 0; i < pdat->npad / 2; i++)
		{
			pad = (ceilf(x->dims[i + 2] / (float)pdat->strides[i]) - 1) * pdat->strides[i] + ((pdat->kernels[i] - 1) * pdat->dilations[i] + 1) - x->dims[i + 2];
			pdat->cpads[i] = pad / 2;
			pdat->cpads[i + pdat->nkernel] = pad - pdat->cpads[i];
		}
		break;
	case AUTO_PAD_SAME_LOWER:
		for(i = 0; i < pdat->npad / 2; i++)
		{
			pad = (ceilf(x->dims[i + 2] / (float)pdat->strides[i]) - 1) * pdat->strides[i] + ((pdat->kernels[i] - 1) * pdat->dilations[i] + 1) - x->dims[i + 2];
			pdat->cpads[i + pdat->nkernel] = pad / 2;
			pdat->cpads[i] = pad - pdat->cpads[i + pdat->nkernel];
		}
		break;
	case AUTO_PAD_VALID:
		onnx_memset(pdat->cpads, 0, sizeof(int) * pdat->npad);
		break;
	default:
		break;
	}
	dims[0] = x->dims[0];
	dims[1] = w->dims[0];
	for(i = 0; i < ndim - 2; i++)
	{
		switch(pdat->auto_pad)
		{
		case AUTO_PAD_NOTSET:
			dims[i + 2] = floorf((x->dims[i + 2] + pdat->cpads[i] + pdat->cpads[i + pdat->nkernel] - ((pdat->kernels[i] - 1) * pdat->dilations[i] + 1)) / (float)pdat->strides[i] + 1);
			break;
		case AUTO_PAD_SAME_UPPER:
		case AUTO_PAD_SAME_LOWER:
			dims[i + 2] = ceilf(x->dims[i + 2] / (float)pdat->strides[i]);
			break;
		case AUTO_PAD_VALID:
			dims[i + 2] = ceilf((x->dims[i + 2] - ((pdat->kernels[i] - 1) * pdat->dilations[i] + 1) + 1) / (float)pdat->strides[i]);
			break;
		default:
			break;
		}
	}
	return onnx_tensor_reshape(y, dims, ndim, ONNX_TENSOR_TYPE_INT32);
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

static inline int dim_offset(int ndim, int * dims, int * dim_max)
{
	int o, s;
	int i;

	for(i = ndim - 1, o = 0, s = 1; i >= 0; i--)
	{
		o += dims[i] * s;
		s *= dim_max[i];
	}
	return o;
}

static void ConvInteger_int8_uint8(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * w = n->inputs[1];
	struct onnx_tensor_t * x_zero_point = (n->ninput > 2) ? n->inputs[2] : NULL;
	struct onnx_tensor_t * w_zero_point = (n->ninput > 3) ? n->inputs[3] : NULL;
	int32_t * py = (int32_t *)y->datas;
	int8_t * px = (int8_t *)x->datas;
	int8_t * pw = (int8_t *)w->datas;
	int8_t x_zp = x_zero_point ? *((int8_t *)x_zero_point->datas) : 0;
	int8_t w_zp = w_zero_point ? *((int8_t *)w_zero_point->datas) : 0;

	int32_t sum;
	int v, weight;
	int ndim = x->ndim;
	int M = w->dims[0];
	int C = w->dims[1];
	int H = w->dims[2];
	int W = w->dims[3];
	int ch, i;

	if(ndim == 4)
	{
		int iC = x->dims[1];
		int iH = x->dims[2];
		int iW = x->dims[3];

		int oN = y->dims[0];
		int oC = w->dims[0];
		int oH = y->dims[2];
		int oW = y->dims[3];

		typedef int8_t (*pxtype)[iC][iH][iW];
		typedef int8_t (*pwtype)[C][H][W];
		typedef int32_t (*pytype)[M][oH][oW];

		for(int h = 0; h < oH; ++h)
		{
			for(int w = 0; w < oW; ++w)
			{
				int base_h = h * pdat->strides[0] - pdat->cpads[0];
				int base_w = w * pdat->strides[1] - pdat->cpads[1];

				for(int n = 0; n < oN; ++n)
				{
					for(int c = 0; c < oC; ++c)
					{
						int base_c = (c * pdat->group / M) * C;
						sum = 0;
						for(int i = (base_h < 0 ? (-base_h) / pdat->dilations[0] : 0); i < H; ++i)
						{
							int input_h = base_h + i * pdat->dilations[0];
							if(input_h >= iH)
								break;
							for(int j = (base_w < 0 ? (-base_w) / pdat->dilations[1] : 0); j < W; ++j)
							{
								int input_w = base_w + j * pdat->dilations[1];
								if(input_w >= iW)
									break;
								for(int w_channel = 0; w_channel < C; ++w_channel)
								{
									ch = base_c + w_channel;
									v = ((pxtype)px)[n][ch][input_h][input_w] - x_zp;
									weight = ((pwtype)pw)[c][w_channel][i][j] - w_zp;
									sum += v * weight;
								}
							}
						}
						((pytype)py)[n][c][h][w] = sum;
					}
				}
			}
		}
	}
	else
	{
		int i_dim[ndim];
		int o_dim[ndim];
		int w_dim[ndim];
		int b_dim[ndim];

		onnx_memset(o_dim, 0, sizeof(o_dim));
		do {
			b_dim[0] = o_dim[0];
			for(i = 2; i < ndim; i++)
				b_dim[i] = o_dim[i] * pdat->strides[i - 2] - pdat->cpads[i - 2];
			sum = 0;
			onnx_memset(w_dim, 0, sizeof(w_dim));
			w_dim[0] = o_dim[1];
			do {
				if(w_dim[1] == 1)
					break;
				i_dim[0] = b_dim[0];
				for(i = 2; i < ndim; i++)
					i_dim[i] = b_dim[i] + w_dim[i] * pdat->dilations[i - 2];
				for(ch = 0; ch < C; ch++)
				{
					i_dim[1] = (o_dim[1] * pdat->group / M) * C + ch;
					w_dim[1] = ch;
					for(i = 0; i < ndim; i++)
					{
						if((i_dim[i] < 0) || (i_dim[i] >= x->dims[i]))
						{
							v = -x_zp;
							break;
						}
					}
					if(i >= ndim)
						v = px[dim_offset(ndim, i_dim, x->dims)] - x_zp;
					for(i = 0; i < ndim; i++)
					{
						if((w_dim[i] < 0) || (w_dim[i] >= w->dims[i]))
						{
							weight = -w_zp;
							break;
						}
					}
					if(i >= ndim)
						weight = pw[dim_offset(ndim, w_dim, w->dims)] - w_zp;
					sum += v * weight;
				}
				w_dim[1] = 0;
			} while(dim_next(ndim, w_dim, w->dims));
			py[dim_offset(ndim, o_dim, y->dims)] = sum;
		} while(dim_next(ndim, o_dim, y->dims));
	}
}

static void ConvInteger_uint8_uint8(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * w = n->inputs[1];
	struct onnx_tensor_t * x_zero_point = (n->ninput > 2) ? n->inputs[2] : NULL;
	struct onnx_tensor_t * w_zero_point = (n->ninput > 3) ? n->inputs[3] : NULL;
	int32_t * py = (int32_t *)y->datas;
	uint8_t * px = (uint8_t *)x->datas;
	uint8_t * pw = (uint8_t *)w->datas;
	uint8_t x_zp = x_zero_point ? *((uint8_t *)x_zero_point->datas) : 0;
	uint8_t w_zp = w_zero_point ? *((uint8_t *)w_zero_point->datas) : 0;

	int32_t sum;
	int v, weight;
	int ndim = x->ndim;
	int M = w->dims[0];
	int C = w->dims[1];
	int H = w->dims[2];
	int W = w->dims[3];
	int ch, i;

	if(ndim == 4)
	{
		int iC = x->dims[1];
		int iH = x->dims[2];
		int iW = x->dims[3];

		int oN = y->dims[0];
		int oC = w->dims[0];
		int oH = y->dims[2];
		int oW = y->dims[3];

		typedef uint8_t (*pxtype)[iC][iH][iW];
		typedef uint8_t (*pwtype)[C][H][W];
		typedef int32_t (*pytype)[M][oH][oW];

		for(int h = 0; h < oH; ++h)
		{
			for(int w = 0; w < oW; ++w)
			{
				int base_h = h * pdat->strides[0] - pdat->cpads[0];
				int base_w = w * pdat->strides[1] - pdat->cpads[1];

				for(int n = 0; n < oN; ++n)
				{
					for(int c = 0; c < oC; ++c)
					{
						int base_c = (c * pdat->group / M) * C;
						sum = 0;
						for(int i = (base_h < 0 ? (-base_h) / pdat->dilations[0] : 0); i < H; ++i)
						{
							int input_h = base_h + i * pdat->dilations[0];
							if(input_h >= iH)
								break;
							for(int j = (base_w < 0 ? (-base_w) / pdat->dilations[1] : 0); j < W; ++j)
							{
								int input_w = base_w + j * pdat->dilations[1];
								if(input_w >= iW)
									break;
								for(int w_channel = 0; w_channel < C; ++w_channel)
								{
									ch = base_c + w_channel;
									v = ((pxtype)px)[n][ch][input_h][input_w] - x_zp;
									weight = ((pwtype)pw)[c][w_channel][i][j] - w_zp;
									sum += v * weight;
								}
							}
						}
						((pytype)py)[n][c][h][w] = sum;
					}
				}
			}
		}
	}
	else
	{
		int i_dim[ndim];
		int o_dim[ndim];
		int w_dim[ndim];
		int b_dim[ndim];

		onnx_memset(o_dim, 0, sizeof(o_dim));
		do {
			b_dim[0] = o_dim[0];
			for(i = 2; i < ndim; i++)
				b_dim[i] = o_dim[i] * pdat->strides[i - 2] - pdat->cpads[i - 2];
			sum = 0;
			onnx_memset(w_dim, 0, sizeof(w_dim));
			w_dim[0] = o_dim[1];
			do {
				if(w_dim[1] == 1)
					break;
				i_dim[0] = b_dim[0];
				for(i = 2; i < ndim; i++)
					i_dim[i] = b_dim[i] + w_dim[i] * pdat->dilations[i - 2];
				for(ch = 0; ch < C; ch++)
				{
					i_dim[1] = (o_dim[1] * pdat->group / M) * C + ch;
					w_dim[1] = ch;
					for(i = 0; i < ndim; i++)
					{
						if((i_dim[i] < 0) || (i_dim[i] >= x->dims[i]))
						{
							v = -x_zp;
							break;
						}
					}
					if(i >= ndim)
						v = px[dim_offset(ndim, i_dim, x->dims)] - x_zp;
					for(i = 0; i < ndim; i++)
					{
						if((w_dim[i] < 0) || (w_dim[i] >= w->dims[i]))
						{
							weight = -w_zp;
							break;
						}
					}
					if(i >= ndim)
						weight = pw[dim_offset(ndim, w_dim, w->dims)] - w_zp;
					sum += v * weight;
				}
				w_dim[1] = 0;
			} while(dim_next(ndim, w_dim, w->dims));
			py[dim_offset(ndim, o_dim, y->dims)] = sum;
		} while(dim_next(ndim, o_dim, y->dims));
	}
}

void resolver_default_op_ConvInteger(struct onnx_node_t * n)
{
	if(n->opset >= 10)
	{
		if((n->inputs[0]->type == ONNX_TENSOR_TYPE_INT8) && (n->inputs[1]->type == ONNX_TENSOR_TYPE_INT8))
		{
			n->init = ConvInteger_init;
			n->exit = ConvInteger_exit;
			n->reshape = ConvInteger_reshape;
			n->operator_ = ConvInteger_int8_uint8;
		}
		else if((n->inputs[0]->type == ONNX_TENSOR_TYPE_UINT8) && (n->inputs[1]->type == ONNX_TENSOR_TYPE_UINT8))
		{
			n->init = ConvInteger_init;
			n->exit = ConvInteger_exit;
			n->reshape = ConvInteger_reshape;
			n->operator_ = ConvInteger_uint8_uint8;
		}
	}
}
