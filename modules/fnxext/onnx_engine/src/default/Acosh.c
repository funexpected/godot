#include "../onnx.h"

static int Acosh_init(struct onnx_node_t * n)
{
	if((n->ninput == 1) && (n->noutput == 1))
		return 1;
	return 0;
}

static int Acosh_exit(struct onnx_node_t * n)
{
	return 1;
}

static int Acosh_reshape(struct onnx_node_t * n)
{
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];

	return onnx_tensor_reshape_identity(y, x, x->type);
}

static void Acosh_bfloat16(struct onnx_node_t * n)
{
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	uint16_t * px = (uint16_t *)x->datas;
	uint16_t * py = (uint16_t *)y->datas;
	float v;
	size_t i, l;

	for(i = 0, l = y->ndata; i < l; i++)
	{
		v = bfloat16_to_float32(px[i]);
		py[i] = float32_to_bfloat16(acoshf(v));
	}
}

static void Acosh_float16(struct onnx_node_t * n)
{
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	uint16_t * px = (uint16_t *)x->datas;
	uint16_t * py = (uint16_t *)y->datas;
	float v;
	size_t i, l;

	for(i = 0, l = y->ndata; i < l; i++)
	{
		v = float16_to_float32(px[i]);
		py[i] = float32_to_float16(acoshf(v));
	}
}

static void Acosh_float32(struct onnx_node_t * n)
{
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	float * px = (float *)x->datas;
	float * py = (float *)y->datas;
	size_t i, l;

	for(i = 0, l = y->ndata; i < l; i++)
		py[i] = acoshf(px[i]);
}

static void Acosh_float64(struct onnx_node_t * n)
{
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	double * px = (double *)x->datas;
	double * py = (double *)y->datas;
	size_t i, l;

	for(i = 0, l = y->ndata; i < l; i++)
		py[i] = acosh(px[i]);
}

void resolver_default_op_Acosh(struct onnx_node_t * n)
{
	if(n->opset >= 22)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_BFLOAT16:
			n->init = Acosh_init;
			n->exit = Acosh_exit;
			n->reshape = Acosh_reshape;
			n->operator_ = Acosh_bfloat16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = Acosh_init;
			n->exit = Acosh_exit;
			n->reshape = Acosh_reshape;
			n->operator_ = Acosh_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = Acosh_init;
			n->exit = Acosh_exit;
			n->reshape = Acosh_reshape;
			n->operator_ = Acosh_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = Acosh_init;
			n->exit = Acosh_exit;
			n->reshape = Acosh_reshape;
			n->operator_ = Acosh_float64;
			break;
		default:
			break;
		}
	}
	else if(n->opset >= 9)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_FLOAT16:
			n->init = Acosh_init;
			n->exit = Acosh_exit;
			n->reshape = Acosh_reshape;
			n->operator_ = Acosh_float16;
			break;
		case ONNX_TENSOR_TYPE_FLOAT32:
			n->init = Acosh_init;
			n->exit = Acosh_exit;
			n->reshape = Acosh_reshape;
			n->operator_ = Acosh_float32;
			break;
		case ONNX_TENSOR_TYPE_FLOAT64:
			n->init = Acosh_init;
			n->exit = Acosh_exit;
			n->reshape = Acosh_reshape;
			n->operator_ = Acosh_float64;
			break;
		default:
			break;
		}
	}
}
