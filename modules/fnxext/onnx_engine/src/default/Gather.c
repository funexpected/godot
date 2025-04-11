#include "../onnx.h"

static int Gather_init(struct onnx_node_t * n)
{
	if((n->ninput == 2) && (n->noutput == 1))
		return 1;
	return 0;
}

static int Gather_exit(struct onnx_node_t * n)
{
	return 1;
}

static int Gather_reshape(struct onnx_node_t * n)
{
	struct onnx_tensor_t * data = n->inputs[0];
	struct onnx_tensor_t * indices = n->inputs[1];
	struct onnx_tensor_t * y = n->outputs[0];
	int64_t axis = onnx_attribute_read_int(n, "axis", 0);
	int r = data->ndim;
	int q = indices->ndim;

	if(axis < 0)
		axis += r;
	if(axis < 0 || axis >= r)
		return 0;

	int ndim = q + (r - 1);
	int dims[ndim];
	int i, k;

	k = 0;
	for(i = 0; i < axis; i++)
		dims[k++] = data->dims[i];
	for(i = 0; i < q; i++) 
		dims[k++] = indices->dims[i];
	for(i = axis + 1; i < r; i++)
		dims[k++] = data->dims[i];

	return onnx_tensor_reshape(y, dims, ndim, data->type);
}

static void Gather_operator(struct onnx_node_t * n)
{
	struct onnx_tensor_t * data = n->inputs[0];
	struct onnx_tensor_t * indices = n->inputs[1];
	struct onnx_tensor_t * y = n->outputs[0];
	int32_t * pindices = (int32_t *)indices->datas;
	int64_t axis = onnx_attribute_read_int(n, "axis", 0);
	int r = data->ndim;
	int typesize = onnx_tensor_type_sizeof(data->type);
	
	if(axis < 0)
		axis += r;

	int outer_size = 1;
	int indices_size = 1;
	int inner_size = 1;
	int i;

	for(i = 0; i < axis; i++)
		outer_size *= data->dims[i];
	for(i = 0; i < indices->ndim; i++)
		indices_size *= indices->dims[i];
	for(i = axis + 1; i < r; i++)
		inner_size *= data->dims[i];

	if(data->type == ONNX_TENSOR_TYPE_STRING)
	{
		char ** pdata = (char **)data->datas;
		char ** py = (char **)y->datas;
		
		for(int out = 0; out < outer_size; out++) {
			for(int idx = 0; idx < indices_size; idx++) {
				int src_offset = out * data->dims[axis] * inner_size +
								pindices[idx] * inner_size;
				int dst_offset = out * indices_size * inner_size +
								idx * inner_size;

				for(int in = 0; in < inner_size; in++) {
					if(py[dst_offset + in])
						onnx_free(py[dst_offset + in]);
					py[dst_offset + in] = pdata[src_offset + in] ? onnx_strdup(pdata[src_offset + in]) : NULL;
				}
			}
		}
	}
	else
	{
		char * pdata = (char *)data->datas;
		char * py = (char *)y->datas;
		
		for(int out = 0; out < outer_size; out++) {
			for(int idx = 0; idx < indices_size; idx++) {
				int src_offset = out * data->dims[axis] * inner_size +
								pindices[idx] * inner_size;
				int dst_offset = out * indices_size * inner_size +
								idx * inner_size;

				onnx_memcpy(py + (dst_offset * typesize), 
						   pdata + (src_offset * typesize), 
						   inner_size * typesize);
			}
		}
	}
}

void resolver_default_op_Gather(struct onnx_node_t * n)
{
	if(n->opset >= 13)
	{
	}
	else if(n->opset >= 11)
	{
		switch(n->inputs[0]->type)
		{
		case ONNX_TENSOR_TYPE_BOOL:
		case ONNX_TENSOR_TYPE_INT8:
		case ONNX_TENSOR_TYPE_INT16:
		case ONNX_TENSOR_TYPE_INT32:
		case ONNX_TENSOR_TYPE_INT64:
		case ONNX_TENSOR_TYPE_UINT8:
		case ONNX_TENSOR_TYPE_UINT16:
		case ONNX_TENSOR_TYPE_UINT32:
		case ONNX_TENSOR_TYPE_UINT64:
		case ONNX_TENSOR_TYPE_BFLOAT16:
		case ONNX_TENSOR_TYPE_FLOAT16:
		case ONNX_TENSOR_TYPE_FLOAT32:
		case ONNX_TENSOR_TYPE_FLOAT64:
		case ONNX_TENSOR_TYPE_COMPLEX64:
		case ONNX_TENSOR_TYPE_COMPLEX128:
		case ONNX_TENSOR_TYPE_STRING:
			n->init = Gather_init;
			n->exit = Gather_exit;
			n->reshape = Gather_reshape;
			n->operator_ = Gather_operator;
			break;
		default:
			break;
		}
	}
	else if(n->opset >= 1)
	{
	}
}
