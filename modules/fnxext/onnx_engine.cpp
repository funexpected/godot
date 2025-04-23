
#include "onnx_engine/src/onnx.h"
#include "onnx_engine.h"
#include "core/print_string.h"
#include "core/project_settings.h"
#include "core/os/file_access.h"

// #	var a = OnnxEngine.new()
// #	a.load_from_file("res://mnist-12.onnx")

void OnnxEngine::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_input_layer", "layer_name"), &OnnxEngine::set_input_layer);
    ClassDB::bind_method(D_METHOD("set_output_layer", "layer_name"), &OnnxEngine::set_output_layer);
    ClassDB::bind_method(D_METHOD("run", "input_data"), &OnnxEngine::run);
    ClassDB::bind_method(D_METHOD("print_layers"), &OnnxEngine::print_layers);
    ClassDB::bind_method(D_METHOD("load_from_file", "p_path", "params", "input_layer_name", "output_layer_name"), &OnnxEngine::load_from_file, DEFVAL(Dictionary()), DEFVAL(""), DEFVAL(""));
}


Variant OnnxEngine::set_input_layer(const String &layer_name) {
   input = onnx_tensor_search(ctx, layer_name.utf8().get_data());
   if (input) {
        return Variant(true);
   }
   return Variant(false);
}

Variant OnnxEngine::set_output_layer(const String &layer_name) {
   output = onnx_tensor_search(ctx, layer_name.utf8().get_data());
   if (output) {
        return Variant(true);
    }
    return Variant(false);
}
// input = onnx_tensor_search(ctx, "Input3");
	// output = onnx_tensor_search(ctx, "Plus214_Output_0");



Array OnnxEngine::run(const Array& data) {
    if (!input || !output || data.size() != input->ndata) {
        return Variant(false);
    }
    if (input->type == ONNX_TENSOR_TYPE_FLOAT32) {

        float *p_input = (float*)input->datas;
        // unsigned char * px = x->pixels;
		for (int i = 0; i < input->ndata; i++) {
            if (data[i].get_type() != Variant::REAL) {
                return Variant(false);
            }
            p_input[i] = (float)data[i];
        }
    }
    onnx_run(ctx);
    Array result;
    float *p_result_layer = (float *)output->datas;
    for (int i = 0; i < output->ndata; ++i) {
        result.push_back(p_result_layer[i]);
    }

    return result;
}


Variant OnnxEngine::load_from_file(const String &file_path, const Dictionary &params, const String &input_layer_name, const String &output_layer_name) {
    Vector<uint8_t> onnx_data = FileAccess::get_file_as_array(file_path);
    
    List<Variant> shape_keys;
    params.get_key_list(&shape_keys);

    struct hmap_t *shape_params = hmap_alloc(0, NULL);
    if (!shape_params) {
        print_line(String("OnnxEngine - Cant alloc shape_params"));
        return 0;
    }

    for (List<Variant>::Element *E = shape_keys.front(); E; E = E->next()) {
        Variant key = E->get();
        String key_str = key.operator String();
        Variant value = params[key];
        
        if (value.get_type() == Variant::INT) {
            int64_t* param_value = (int64_t*)malloc(sizeof(int64_t));
            *param_value = (int64_t)value;
            hmap_add(shape_params, key_str.utf8().get_data(), param_value);
            allocated_params.push_back(param_value);
        } else if (value.get_type() == Variant::REAL) {
            float* param_value = (float*)malloc(sizeof(float));
            *param_value = (float)value;
            hmap_add(shape_params, key_str.utf8().get_data(), param_value);
            allocated_params.push_back(param_value);
        }
    }
    
    // Default parameters if none provided
    if (shape_keys.size() == 0) {
        int64_t* width = (int64_t*)malloc(sizeof(int64_t));
        int64_t* batch_size = (int64_t*)malloc(sizeof(int64_t));
        *width = 128;
        *batch_size = 1;
        hmap_add(shape_params, "width", width);
        hmap_add(shape_params, "batch_size", batch_size);
        allocated_params.push_back(width);
        allocated_params.push_back(batch_size);
    }


    ctx = onnx_context_alloc(onnx_data.ptr(), onnx_data.size(), NULL, 0, shape_params);

    if (ctx) {
        print_line(String("OnnxEngine- ctx created"));
        
        // Use provided input/output layer names if specified, otherwise use default ones
        
        CharString utf8_input_layer_name = input_layer_name.utf8();
        CharString utf8_output_layer_name = output_layer_name.utf8();

        const char *in_layer_name = input_layer_name.empty() ? _get_input_layer_name() : utf8_input_layer_name.get_data();
        const char *out_layer_name = output_layer_name.empty() ? _get_output_layer_name() : utf8_output_layer_name.get_data();
        
        input = onnx_tensor_search(ctx, in_layer_name);
        output = onnx_tensor_search(ctx, out_layer_name);
        
        if (input && output)
            return Variant(true);
        else
            return Variant(false);
    } else {
        return Variant(false);
    }
}

const char *OnnxEngine::_get_input_layer_name() {
    if (!ctx->g->nlen)
        return NULL;
    if (!ctx->g->nodes[0].inputs) 
        return NULL;
    return ctx->g->nodes[0].inputs[0]->name;
}

const char *OnnxEngine::_get_output_layer_name() {
    if (!ctx->g->nlen)
        return NULL;
    if (!ctx->g->nodes[ctx->g->nlen - 1].outputs) 
        return NULL;
    return ctx->g->nodes[ctx->g->nlen - 1].outputs[0]->name;
}

void OnnxEngine::print_layers() {
    if (ctx) {
        onnx_context_dump(ctx, 0);
    } else {
        print_line(String("Onnx file don't load"));

    }
}
OnnxEngine::OnnxEngine() {
    ctx = NULL;
    input = NULL;
    output = NULL;
}

OnnxEngine::~OnnxEngine(){
    for (int i = 0; i < allocated_params.size(); i++) {
        void* ptr = allocated_params[i];
        if (ptr) {
            free(ptr);
        }
    }
    allocated_params.clear();
    
    if (ctx != NULL) {
        onnx_context_free(ctx);
    }
}
