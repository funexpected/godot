#include "../onnx.h"
#include <math.h>

struct operator_pdata_t {
	int dummy;
};

static int DynamicQuantizeLinear_init(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat;

	if((n->ninput == 1) && (n->noutput == 3))
	{
		pdat = onnx_malloc(sizeof(struct operator_pdata_t));
		if(pdat)
		{
			pdat->dummy = 0;
			n->priv = pdat;
			return 1;
		}
	}
	return 0;
}

static int DynamicQuantizeLinear_exit(struct onnx_node_t * n)
{
	struct operator_pdata_t * pdat = (struct operator_pdata_t *)n->priv;

	if(pdat)
		onnx_free(pdat);
	return 1;
}

static int DynamicQuantizeLinear_reshape(struct onnx_node_t * n)
{
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * y_scale = n->outputs[1];
	struct onnx_tensor_t * y_zero_point = n->outputs[2];
	
	if(x->type != ONNX_TENSOR_TYPE_FLOAT32)
		return 0;

	if(!onnx_tensor_reshape(y, x->dims, x->ndim, ONNX_TENSOR_TYPE_UINT8))
		return 0;
	
	if(!onnx_tensor_reshape(y_scale, NULL, 0, ONNX_TENSOR_TYPE_FLOAT32))
		return 0;
	
	if(!onnx_tensor_reshape(y_zero_point, NULL, 0, ONNX_TENSOR_TYPE_UINT8))
		return 0;
	
	return 1;
}

static void DynamicQuantizeLinear_float32(struct onnx_node_t * n)
{
	struct onnx_tensor_t * x = n->inputs[0];
	struct onnx_tensor_t * y = n->outputs[0];
	struct onnx_tensor_t * y_scale = n->outputs[1];
	struct onnx_tensor_t * y_zero_point = n->outputs[2];
	
	float * px = (float *)x->datas;
	uint8_t * py = (uint8_t *)y->datas;
	float * py_scale = (float *)y_scale->datas;
	uint8_t * py_zero_point = (uint8_t *)y_zero_point->datas;
	
	// Find min and max values in input tensor
	float x_min = px[0];
	float x_max = px[0];
	
	for(size_t i = 1; i < x->ndata; i++)
	{
		if(px[i] < x_min)
			x_min = px[i];
		if(px[i] > x_max)
			x_max = px[i];
	}
	
	// Adjust data range to include 0
	x_min = fmin(0, x_min);
	x_max = fmax(0, x_max);
	
	// Calculate scale and zero point
	float scale = (x_max - x_min) / 255.0f;
	float zero_point_float = 0.0f;
	
	// Handle special case: if input range is zero (all inputs are the same value)
	if(scale == 0.0f)
	{
		scale = 1.0f;
		zero_point_float = 0.0f;
	}
	else
	{
		// Calculate zero point
		zero_point_float = (0.0f - x_min) / scale;
	}
	
	// Round to nearest even and saturate
	int zero_point_int = (int)roundf(zero_point_float);
	
	// Saturate to uint8 range [0, 255]
	if(zero_point_int < 0)
		zero_point_int = 0;
	else if(zero_point_int > 255)
		zero_point_int = 255;
	
	// Set output scale and zero point
	py_scale[0] = scale;
	py_zero_point[0] = (uint8_t)zero_point_int;
	
	// Quantize input data
	for(size_t i = 0; i < x->ndata; i++)
	{
		// Apply quantization formula: y = saturate(round(x / scale) + zero_point)
		float quantized_float = roundf(px[i] / scale) + zero_point_int;
		
		// Saturate to uint8 range [0, 255]
		if(quantized_float < 0.0f)
			py[i] = 0;
		else if(quantized_float > 255.0f)
			py[i] = 255;
		else
			py[i] = (uint8_t)quantized_float;
	}
}

void resolver_default_op_DynamicQuantizeLinear(struct onnx_node_t * n)
{
	if(n->opset >= 11)
	{
		if(n->inputs[0]->type == ONNX_TENSOR_TYPE_FLOAT32)
		{
			n->init = DynamicQuantizeLinear_init;
			n->exit = DynamicQuantizeLinear_exit;
			n->reshape = DynamicQuantizeLinear_reshape;
			n->operator_ = DynamicQuantizeLinear_float32;
		}
	}
}
