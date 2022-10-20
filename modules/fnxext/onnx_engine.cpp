
#include "onnx_engine/src/onnx.h"
#include "onnx_engine.h"
#include "core/print_string.h"
#include "core/project_settings.h"


// #	var a = OnnxEngine.new()
// #	a.load_from_file("res://mnist-12.onnx")

void OnnxEngine::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_input_layer", "layer_name"), &OnnxEngine::set_input_layer);
    ClassDB::bind_method(D_METHOD("set_output_layer", "layer_name"), &OnnxEngine::set_output_layer);
    ClassDB::bind_method(D_METHOD("run", "input_data"), &OnnxEngine::run);
    ClassDB::bind_method(D_METHOD("print_layers"), &OnnxEngine::print_layers);
    ClassDB::bind_method(D_METHOD("load_from_file", "p_path"), &OnnxEngine::load_from_file);
}

String fix_res_path(String r_path) {
    String resource_path = ProjectSettings::get_singleton()->get_resource_path();
    if (resource_path != "") {
        return r_path.replace("res:/", resource_path);
    }
    return r_path.replace("res://", "");

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


Variant OnnxEngine::load_from_file(const String &file_path) {
    ctx = onnx_context_alloc_from_file(fix_res_path(file_path).utf8().get_data(), NULL, 0);
    
    if (ctx) {
        return Variant(true);
    } else {
        return Variant(false);
    }
}
void OnnxEngine::print_layers() {
    if (ctx) {
        onnx_context_dump(ctx, 0);
    } else {
        print_line(String("Onnx file don't load"));

    }
}

OnnxEngine::~OnnxEngine(){
    if (ctx) {
        onnx_context_free(ctx);
    }
}
