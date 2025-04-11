#ifndef ONNX_ENGINE_HPP
#define ONNX_ENGINE_HPP


#include "onnx_engine/src/onnx.h"
#include "core/reference.h"


class OnnxEngine: public Reference {
    GDCLASS(OnnxEngine, Reference);

    struct onnx_context_t* ctx;
	struct onnx_tensor_t* input;
	struct onnx_tensor_t* output;

protected:
    static void _bind_methods();
public:
    void _init();

    Variant load_from_file(const String &file_path, const Dictionary &params);

    Variant set_input_layer(const String &layer_name);
    Variant set_output_layer(const String &layer_name);

    const char *_get_input_layer_name();
    const char *_get_output_layer_name();
    Array run(const Array& data);
    void print_layers();
    OnnxEngine();
    ~OnnxEngine();
};

#endif
