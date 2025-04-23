#include "../onnx.h"
#include <string.h>
#include <math.h>

enum {
    DQLSTM_INPUT_X = 0,
    DQLSTM_INPUT_W = 1,
    DQLSTM_INPUT_R = 2,
    DQLSTM_INPUT_B = 3,
    DQLSTM_INPUT_SEQUENCE_LENS = 4,
    DQLSTM_INPUT_INITIAL_H = 5,
    DQLSTM_INPUT_INITIAL_C = 6,
    DQLSTM_INPUT_P = 7,
    DQLSTM_INPUT_W_SCALE = 8,
    DQLSTM_INPUT_W_ZERO_POINT = 9,
    DQLSTM_INPUT_R_SCALE = 10,
    DQLSTM_INPUT_R_ZERO_POINT = 11,
};

enum {
    DQLSTM_OUTPUT_Y = 0,
    DQLSTM_OUTPUT_Y_H = 1,
    DQLSTM_OUTPUT_Y_C = 2,
};

struct dqlstm_param_t {
    int has_bias;
    int has_sequence_lens;
    int has_initial_h;
    int has_initial_c;
    int has_peephole;
    int hidden_size;
    char * direction;
    float clip;
    int layout;
    int input_forget;
    int num_directions;
};

static inline float sigmoid(float x) 
{
    if (x >= 0)
        return 1.0f / (1.0f + expf(-x));
    else
        return expf(x) / (1.0f + expf(x));
}

static int DynamicQuantizeLSTM_init(struct onnx_node_t * n)
{
    struct dqlstm_param_t * p;
    char * dir;
    
    if (n->ninput >= 12 && n->noutput >= 1) 
    {
        p = malloc(sizeof(struct dqlstm_param_t));
        if (p) 
        {
            memset(p, 0, sizeof(struct dqlstm_param_t));
            
            // Set default values
            p->clip = 0.0;
            p->layout = 0;
            p->input_forget = 0;
            
            // Parse attributes using the API functions
            p->hidden_size = (int)onnx_attribute_read_int(n, "hidden_size", 0);
            p->clip = onnx_attribute_read_float(n, "clip", 0.0f);
            p->layout = (int)onnx_attribute_read_int(n, "layout", 0);
            p->input_forget = (int)onnx_attribute_read_int(n, "input_forget", 0);
            
            dir = onnx_attribute_read_string(n, "direction", "forward");
            if (dir) 
            {
                p->direction = dir;
            } 
            else 
            {
                p->direction = strdup("forward");
            }
            
            // Determine number of directions based on direction attribute
            if (strncmp(p->direction, "bidirectional", 13) == 0) 
            {
                p->num_directions = 2;
            } 
            else 
            {
                p->num_directions = 1;
            }
            
            // Check optional inputs
            p->has_bias = (n->ninput > DQLSTM_INPUT_B && n->inputs[DQLSTM_INPUT_B]) ? 1 : 0;
            p->has_sequence_lens = (n->ninput > DQLSTM_INPUT_SEQUENCE_LENS && n->inputs[DQLSTM_INPUT_SEQUENCE_LENS]) ? 1 : 0;
            p->has_initial_h = (n->ninput > DQLSTM_INPUT_INITIAL_H && n->inputs[DQLSTM_INPUT_INITIAL_H]) ? 1 : 0;
            p->has_initial_c = (n->ninput > DQLSTM_INPUT_INITIAL_C && n->inputs[DQLSTM_INPUT_INITIAL_C]) ? 1 : 0;
            p->has_peephole = (n->ninput > DQLSTM_INPUT_P && n->inputs[DQLSTM_INPUT_P]) ? 1 : 0;
            
            n->priv = p;
            return 1;
        }
    }
    return 0;
}

static int DynamicQuantizeLSTM_exit(struct onnx_node_t * n)
{
    struct dqlstm_param_t * p = (struct dqlstm_param_t *)n->priv;
    
    if (p) 
    {
        free(p);
    }
    return 1;
}

static int DynamicQuantizeLSTM_reshape(struct onnx_node_t * n)
{
    struct onnx_tensor_t * x = n->inputs[DQLSTM_INPUT_X];
    struct dqlstm_param_t * p = (struct dqlstm_param_t *)n->priv;
    int seq_length, batch_size;
    
    if (!p)
        return 0;
    
    // Parse dimensions based on layout
    if (p->layout == 0) 
    {
        // Layout 0: [seq_length, batch_size, input_size]
        seq_length = x->dims[0];
        batch_size = x->dims[1];
    } 
    else 
    {
        // Layout 1: [batch_size, seq_length, input_size]
        batch_size = x->dims[0];
        seq_length = x->dims[1];
    }
    
    // Reshape Y if requested
    if (n->noutput > DQLSTM_OUTPUT_Y && n->outputs[DQLSTM_OUTPUT_Y]) 
    {
        struct onnx_tensor_t * y = n->outputs[DQLSTM_OUTPUT_Y];
        int dims[4];
        
        if (p->layout == 0) 
        {
            // Y shape is [seq_length, num_directions, batch_size, hidden_size]
            dims[0] = seq_length;
            dims[1] = p->num_directions;
            dims[2] = batch_size;
            dims[3] = p->hidden_size;
        } 
        else 
        {
            // Y shape is [batch_size, seq_length, num_directions, hidden_size]
            dims[0] = batch_size;
            dims[1] = seq_length;
            dims[2] = p->num_directions;
            dims[3] = p->hidden_size;
        }
        
        if (!onnx_tensor_reshape(y, dims, 4, ONNX_TENSOR_TYPE_FLOAT32))
            return 0;
    }
    
    // Reshape Y_h if requested
    if (n->noutput > DQLSTM_OUTPUT_Y_H && n->outputs[DQLSTM_OUTPUT_Y_H]) 
    {
        struct onnx_tensor_t * y_h = n->outputs[DQLSTM_OUTPUT_Y_H];
        int dims[3];
        
        if (p->layout == 0) 
        {
            // Y_h shape is [num_directions, batch_size, hidden_size]
            dims[0] = p->num_directions;
            dims[1] = batch_size;
            dims[2] = p->hidden_size;
        } 
        else 
        {
            // Y_h shape is [batch_size, num_directions, hidden_size]
            dims[0] = batch_size;
            dims[1] = p->num_directions;
            dims[2] = p->hidden_size;
        }
        
        if (!onnx_tensor_reshape(y_h, dims, 3, ONNX_TENSOR_TYPE_FLOAT32))
            return 0;
    }
    
    // Reshape Y_c if requested
    if (n->noutput > DQLSTM_OUTPUT_Y_C && n->outputs[DQLSTM_OUTPUT_Y_C]) 
    {
        struct onnx_tensor_t * y_c = n->outputs[DQLSTM_OUTPUT_Y_C];
        int dims[3];
        
        if (p->layout == 0) 
        {
            // Y_c shape is [num_directions, batch_size, hidden_size]
            dims[0] = p->num_directions;
            dims[1] = batch_size;
            dims[2] = p->hidden_size;
        } 
        else 
        {
            // Y_c shape is [batch_size, num_directions, hidden_size]
            dims[0] = batch_size;
            dims[1] = p->num_directions;
            dims[2] = p->hidden_size;
        }
        
        if (!onnx_tensor_reshape(y_c, dims, 3, ONNX_TENSOR_TYPE_FLOAT32))
            return 0;
    }
    
    return 1;
}

// Исправленная функция для дэквантизации весов
static void dequantize_weights(const void* quantized_data, float* dequantized_data, 
                              const float* scale, const uint8_t* zero_point, 
                              int size, int scale_size, int is_signed, int num_directions, 
                              int input_or_hidden_size, int hidden_size) 
{
    // Размер 4*hidden_size для каждого направления
    int gate_size = 4 * hidden_size;
    
    if (scale_size == 1) {
        // Поэлементное квантование (per-tensor)
        float s = scale[0];
        int zp = (int)zero_point[0];
        
        if (is_signed) {
            const int8_t* signed_data = (const int8_t*)quantized_data;
            for (int i = 0; i < size; i++) {
                dequantized_data[i] = s * (float)(signed_data[i]);  // Для signed весов zp обычно 0
            }
        } else {
            const uint8_t* unsigned_data = (const uint8_t*)quantized_data;
            for (int i = 0; i < size; i++) {
                dequantized_data[i] = s * (float)(unsigned_data[i] - zp);
            }
        }
    } else if (scale_size == num_directions) {
        // Поканальное квантование по направлениям
        if (is_signed) {
            const int8_t* signed_data = (const int8_t*)quantized_data;
            for (int d = 0; d < num_directions; d++) {
                float s = scale[d];
                // Для signed весов zp обычно 0
                for (int i = 0; i < size / num_directions; i++) {
                    int idx = d * (size / num_directions) + i;
                    dequantized_data[idx] = s * (float)(signed_data[idx]);
                }
            }
        } else {
            const uint8_t* unsigned_data = (const uint8_t*)quantized_data;
            for (int d = 0; d < num_directions; d++) {
                float s = scale[d];
                int zp = (int)zero_point[d];
                for (int i = 0; i < size / num_directions; i++) {
                    int idx = d * (size / num_directions) + i;
                    dequantized_data[idx] = s * (float)(unsigned_data[idx] - zp);
                }
            }
        }
    } else if (scale_size == num_directions * gate_size / hidden_size) {
        // Поканальное квантование по гейтам (4 гейта для каждого направления)
        // scale_size должно быть равно num_directions * 4
        
        // Количество элементов на один канал (на один гейт)
        int elements_per_gate = input_or_hidden_size * hidden_size;
        
        if (is_signed) {
            const int8_t* signed_data = (const int8_t*)quantized_data;
            for (int d = 0; d < num_directions; d++) {
                for (int g = 0; g < 4; g++) {  // 4 гейта: i, o, f, c
                    float s = scale[d * 4 + g];
                    // Для signed весов zp обычно 0
                    
                    for (int i = 0; i < elements_per_gate; i++) {
                        int idx = d * 4 * elements_per_gate + g * elements_per_gate + i;
                        dequantized_data[idx] = s * (float)(signed_data[idx]);
                    }
                }
            }
        } else {
            const uint8_t* unsigned_data = (const uint8_t*)quantized_data;
            for (int d = 0; d < num_directions; d++) {
                for (int g = 0; g < 4; g++) {  // 4 гейта: i, o, f, c
                    float s = scale[d * 4 + g];
                    int zp = (int)zero_point[d * 4 + g];
                    
                    for (int i = 0; i < elements_per_gate; i++) {
                        int idx = d * 4 * elements_per_gate + g * elements_per_gate + i;
                        dequantized_data[idx] = s * (float)(unsigned_data[idx] - zp);
                    }
                }
            }
        }
    } else {
        // Поканальное квантование по другому принципу, применяем упрощенную схему
        // где каждый scale и zero_point применяются к блоку элементов
        int items_per_channel = size / scale_size;
        
        if (is_signed) {
            const int8_t* signed_data = (const int8_t*)quantized_data;
            for (int c = 0; c < scale_size; c++) {
                float s = scale[c];
                // Для signed весов zp обычно 0
                
                for (int i = 0; i < items_per_channel; i++) {
                    int idx = c * items_per_channel + i;
                    dequantized_data[idx] = s * (float)(signed_data[idx]);
                }
            }
        } else {
            const uint8_t* unsigned_data = (const uint8_t*)quantized_data;
            for (int c = 0; c < scale_size; c++) {
                float s = scale[c];
                int zp = (int)zero_point[c];
                
                for (int i = 0; i < items_per_channel; i++) {
                    int idx = c * items_per_channel + i;
                    dequantized_data[idx] = s * (float)(unsigned_data[idx] - zp);
                }
            }
        }
    }
}

static void DynamicQuantizeLSTM_float32(struct onnx_node_t * n)
{
    struct dqlstm_param_t * p = (struct dqlstm_param_t *)n->priv;
    struct onnx_tensor_t * x = n->inputs[DQLSTM_INPUT_X];
    struct onnx_tensor_t * w = n->inputs[DQLSTM_INPUT_W];
    struct onnx_tensor_t * r = n->inputs[DQLSTM_INPUT_R];
    struct onnx_tensor_t * b = p->has_bias ? n->inputs[DQLSTM_INPUT_B] : NULL;
    struct onnx_tensor_t * sequence_lens = p->has_sequence_lens ? n->inputs[DQLSTM_INPUT_SEQUENCE_LENS] : NULL;
    struct onnx_tensor_t * initial_h = p->has_initial_h ? n->inputs[DQLSTM_INPUT_INITIAL_H] : NULL;
    struct onnx_tensor_t * initial_c = p->has_initial_c ? n->inputs[DQLSTM_INPUT_INITIAL_C] : NULL;
    struct onnx_tensor_t * p_tensor = p->has_peephole ? n->inputs[DQLSTM_INPUT_P] : NULL;
    struct onnx_tensor_t * w_scale = n->inputs[DQLSTM_INPUT_W_SCALE];
    struct onnx_tensor_t * w_zero_point = n->inputs[DQLSTM_INPUT_W_ZERO_POINT];
    struct onnx_tensor_t * r_scale = n->inputs[DQLSTM_INPUT_R_SCALE];
    struct onnx_tensor_t * r_zero_point = n->inputs[DQLSTM_INPUT_R_ZERO_POINT];
    struct onnx_tensor_t * y = (n->noutput > DQLSTM_OUTPUT_Y) ? n->outputs[DQLSTM_OUTPUT_Y] : NULL;
    struct onnx_tensor_t * y_h = (n->noutput > DQLSTM_OUTPUT_Y_H) ? n->outputs[DQLSTM_OUTPUT_Y_H] : NULL;
    struct onnx_tensor_t * y_c = (n->noutput > DQLSTM_OUTPUT_Y_C) ? n->outputs[DQLSTM_OUTPUT_Y_C] : NULL;
    float * px, * py, * py_h, * py_c;
    float * ph_t, * pc_t, * pbx, * pbh;
    float * gates, * it, * ot, * ft, * ct;
    float * ppi, * ppo, * ppf;
    int seq_length, batch_size, input_size, hidden_size;
    int num_directions = p->num_directions;
    int layout = p->layout;
    int i, j, d, b_idx, s_idx, k;
    
    // Указатель на sequence_lens для проверки длин последовательностей
    const int* sequence_lengths = sequence_lens ? (const int*)sequence_lens->datas : NULL;
    int max_sequence_length = 0;
    
    // Get dimensions
    if (layout == 0) 
    {
        // [seq_length, batch_size, input_size]
        seq_length = x->dims[0];
        batch_size = x->dims[1];
        input_size = x->dims[2];
    } 
    else 
    {
        // [batch_size, seq_length, input_size]
        batch_size = x->dims[0];
        seq_length = x->dims[1];
        input_size = x->dims[2];
    }
    hidden_size = p->hidden_size;
    
    // Если предоставлены sequence_lengths, определите максимальную длину
    if (sequence_lengths != NULL) {
        max_sequence_length = 0;
        for (int b = 0; b < batch_size; b++) {
            if (sequence_lengths[b] > max_sequence_length) {
                max_sequence_length = sequence_lengths[b];
            }
        }
        
        // Используем max_sequence_length вместо seq_length, если она меньше
        if (max_sequence_length > 0 && max_sequence_length < seq_length) {
            seq_length = max_sequence_length;
        }
    }
    
    // Get scale sizes
    int w_scale_size = w_scale->ndim > 0 ? (w_scale->ndim > 1 ? w_scale->dims[0] * w_scale->dims[1] : w_scale->dims[0]) : 1;
    int r_scale_size = r_scale->ndim > 0 ? (r_scale->ndim > 1 ? r_scale->dims[0] * r_scale->dims[1] : r_scale->dims[0]) : 1;
    
    // Check if weights are signed or unsigned
    int is_w_signed = (w->type == ONNX_TENSOR_TYPE_INT8);
    int is_r_signed = (r->type == ONNX_TENSOR_TYPE_INT8);
    
    // Allocate memory for intermediate calculations and hidden/cell states
    gates = (float *)malloc(batch_size * 4 * hidden_size * sizeof(float));
    ph_t = (float *)malloc(batch_size * hidden_size * sizeof(float));
    pc_t = (float *)malloc(batch_size * hidden_size * sizeof(float));
    
    // Allocate memory for dequantized weights
    float* w_dequantized = (float*)malloc(w->ndata * sizeof(float));
    float* r_dequantized = (float*)malloc(r->ndata * sizeof(float));
    
    if (!gates || !ph_t || !pc_t || !w_dequantized || !r_dequantized) 
    {
        if (gates) free(gates);
        if (ph_t) free(ph_t);
        if (pc_t) free(pc_t);
        if (w_dequantized) free(w_dequantized);
        if (r_dequantized) free(r_dequantized);
        return;
    }
    
    // Set up gate pointers (как в LSTM)
    it = gates;
    ot = gates + batch_size * hidden_size;
    ft = gates + 2 * batch_size * hidden_size;
    ct = gates + 3 * batch_size * hidden_size;
    
    // Get weight scale and zero point data
    float* w_scale_data = (float*)w_scale->datas;
    uint8_t* w_zp_data = (uint8_t*)w_zero_point->datas;
    float* r_scale_data = (float*)r_scale->datas;
    uint8_t* r_zp_data = (uint8_t*)r_zero_point->datas;
    
    // Дэквантизация весов (преобразование int8/uint8 в float)
    dequantize_weights(w->datas, w_dequantized, w_scale_data, w_zp_data, 
                      w->ndata, w_scale_size, is_w_signed, num_directions, input_size, hidden_size);
    dequantize_weights(r->datas, r_dequantized, r_scale_data, r_zp_data, 
                      r->ndata, r_scale_size, is_r_signed, num_directions, hidden_size, hidden_size);
    
    // Process each direction
    for (d = 0; d < num_directions; d++) 
    {
        // Initialize hidden and cell states
        if (initial_h) 
        {
            if (layout == 0)
                memcpy(ph_t, (float *)initial_h->datas + d * batch_size * hidden_size, batch_size * hidden_size * sizeof(float));
            else
                for (b_idx = 0; b_idx < batch_size; b_idx++)
                    memcpy(ph_t + b_idx * hidden_size, 
                           (float *)initial_h->datas + b_idx * num_directions * hidden_size + d * hidden_size, 
                           hidden_size * sizeof(float));
        } 
        else 
        {
            memset(ph_t, 0, batch_size * hidden_size * sizeof(float));
        }
        
        if (initial_c) 
        {
            if (layout == 0)
                memcpy(pc_t, (float *)initial_c->datas + d * batch_size * hidden_size, batch_size * hidden_size * sizeof(float));
            else
                for (b_idx = 0; b_idx < batch_size; b_idx++)
                    memcpy(pc_t + b_idx * hidden_size, 
                           (float *)initial_c->datas + b_idx * num_directions * hidden_size + d * hidden_size, 
                           hidden_size * sizeof(float));
        } 
        else 
        {
            memset(pc_t, 0, batch_size * hidden_size * sizeof(float));
        }
        
        // Получаем указатели на весовые матрицы для текущего направления
        float* pw_base = w_dequantized + d * input_size * 4 * hidden_size;
        float* pr_base = r_dequantized + d * hidden_size * 4 * hidden_size;
        
        // Указатели на bias
        if (b) 
        {
            pbx = (float *)b->datas + d * 8 * hidden_size;            // W bias
            pbh = (float *)b->datas + d * 8 * hidden_size + 4 * hidden_size;  // R bias
        }
        
        // Указатели на peephole коэффициенты
        if (p_tensor) 
        {
            ppi = (float *)p_tensor->datas + d * 3 * hidden_size;           // i peephole
            ppo = (float *)p_tensor->datas + d * 3 * hidden_size + hidden_size;   // o peephole
            ppf = (float *)p_tensor->datas + d * 3 * hidden_size + 2 * hidden_size; // f peephole
        }
        
        // Process sequence
        for (s_idx = 0; s_idx < seq_length; s_idx++) 
        {
            // Determine the actual sequence index based on direction
            int seq_idx = s_idx;
            if (strcmp(p->direction, "reverse") == 0 || 
                (strncmp(p->direction, "bidirectional", 13) == 0 && d == 1)) 
            {
                seq_idx = seq_length - 1 - s_idx;
            }
            
            // Get input data pointer for this sequence step
            if (layout == 0) 
            {
                // [seq_length, batch_size, input_size]
                px = (float *)x->datas + seq_idx * batch_size * input_size;
            } 
            else 
            {
                // [batch_size, seq_length, input_size]
                px = (float *)x->datas + batch_size * seq_idx * input_size;
            }
            
            // Вычисление гейтов для каждого batch элемента
            for (b_idx = 0; b_idx < batch_size; b_idx++) 
            {
                // Проверяем sequence_lengths для текущего batch элемента
                if (sequence_lengths != NULL) {
                    // Для обратного направления проверяем иначе
                    if (strcmp(p->direction, "reverse") == 0 || 
                        (strncmp(p->direction, "bidirectional", 13) == 0 && d == 1)) {
                        if (seq_length - 1 - seq_idx >= sequence_lengths[b_idx]) {
                            // Пропускаем этот шаг для текущего batch элемента
                            continue;
                        }
                    } else {
                        // Для прямого направления
                        if (seq_idx >= sequence_lengths[b_idx]) {
                            continue;
                        }
                    }
                }
                
                float * x_b = px + b_idx * input_size;
                float * h_b = ph_t + b_idx * hidden_size;
                float * c_b = pc_t + b_idx * hidden_size;
                
                // Получаем указатели на гейты для текущего элемента батча
                float * it_b = it + b_idx * hidden_size;
                float * ot_b = ot + b_idx * hidden_size;
                float * ft_b = ft + b_idx * hidden_size;
                float * ct_b = ct + b_idx * hidden_size;
                
                // Инициализация гейтов нулями перед суммированием
                memset(it_b, 0, hidden_size * sizeof(float));
                memset(ot_b, 0, hidden_size * sizeof(float));
                memset(ft_b, 0, hidden_size * sizeof(float));
                memset(ct_b, 0, hidden_size * sizeof(float));
                
                // Вычисляем все гейты (input, output, forget, cell)
                // X * W + bias для каждого гейта
                for (i = 0; i < 4; i++) 
                {
                    // Выбираем соответствующий гейт
                    float * g;
                    switch(i) {
                        case 0: g = it_b; break; // input gate
                        case 1: g = ot_b; break; // output gate
                        case 2: g = ft_b; break; // forget gate
                        case 3: g = ct_b; break; // cell gate
                        default: g = NULL; break;
                    }
                    
                    for (j = 0; j < hidden_size; j++) 
                    {
                        float sum = 0.0f;
                        for (k = 0; k < input_size; k++) 
                        {
                            
                            sum += x_b[k] * pw_base[k * 4 * hidden_size + i * hidden_size + j];
                        }
                        g[j] = sum;
                    }
                    
                    // Add bias if available
                    if (b) 
                    {
                        float * bi = pbx + i * hidden_size;
                        for (j = 0; j < hidden_size; j++) 
                        {
                            g[j] += bi[j];
                        }
                    }
                }
                
                // H * R + bias для каждого гейта
                for (i = 0; i < 4; i++) 
                {
                    // Выбираем соответствующий гейт
                    float * g;
                    switch(i) {
                        case 0: g = it_b; break; // input gate
                        case 1: g = ot_b; break; // output gate
                        case 2: g = ft_b; break; // forget gate
                        case 3: g = ct_b; break; // cell gate
                        default: g = NULL; break;
                    }
                    
                    for (j = 0; j < hidden_size; j++) 
                    {
                        float sum = 0.0f;
                        for (k = 0; k < hidden_size; k++) 
                        {
                            
                            sum += h_b[k] * pr_base[k * 4 * hidden_size + i * hidden_size + j];
                        }
                        g[j] += sum;
                    }
                    
                    // Add bias if available
                    if (b) 
                    {
                        float * bi = pbh + i * hidden_size;
                        for (j = 0; j < hidden_size; j++) 
                        {
                            g[j] += bi[j];
                        }
                    }
                }
                
                // Добавляем peephole для input и forget гейтов
                if (p->has_peephole) 
                {
                    // Input gate: + Pi (.) Ct-1
                    for (j = 0; j < hidden_size; j++) 
                    {
                        it_b[j] += ppi[j] * c_b[j];
                    }
                    
                    // Forget gate: + Pf (.) Ct-1
                    for (j = 0; j < hidden_size; j++) 
                    {
                        ft_b[j] += ppf[j] * c_b[j];
                    }
                }
                
                // Применяем активацию более консистентно с LSTM имплементацией
                for (j = 0; j < hidden_size; j++) 
                {
                    float i_val = sigmoid(it_b[j]);
                    float f_val;
                    
                    if (p->input_forget) {
                        // Если включен input_forget, то forget gate = 1 - input gate
                        f_val = 1.0f - i_val;
                    } else {
                        f_val = sigmoid(ft_b[j]);
                    }
                    
                    float c_val = tanhf(ct_b[j]);
                    
                    // Update cell state: Ct = ft (.) Ct-1 + it (.) ct
                    c_b[j] = f_val * c_b[j] + i_val * c_val;
                    
                    // Apply clip if specified
                    if (p->clip > 0) {
                        if (c_b[j] > p->clip)
                            c_b[j] = p->clip;
                        else if (c_b[j] < -p->clip)
                            c_b[j] = -p->clip;
                    }
                }
                
                // Добавляем peephole для output gate после обновления cell state
                if (p->has_peephole) {
                    // Output gate: + Po (.) Ct
                    for (j = 0; j < hidden_size; j++) {
                        ot_b[j] += ppo[j] * c_b[j];
                    }
                }
                
                // Применяем активацию к output gate и обновляем hidden state
                for (j = 0; j < hidden_size; j++) {
                    float o_val = sigmoid(ot_b[j]);
                    h_b[j] = o_val * tanhf(c_b[j]);
                }
                
                // Store the results to output tensors if needed
                if (y) 
                {
                    if (layout == 0) 
                    {
                        // Y shape is [seq_length, num_directions, batch_size, hidden_size]
                        py = (float *)y->datas + 
                             seq_idx * num_directions * batch_size * hidden_size +
                             d * batch_size * hidden_size +
                             b_idx * hidden_size;
                    } 
                    else 
                    {
                        // Y shape is [batch_size, seq_length, num_directions, hidden_size]
                        py = (float *)y->datas + 
                             b_idx * seq_length * num_directions * hidden_size +
                             seq_idx * num_directions * hidden_size +
                             d * hidden_size;
                    }
                    
                    memcpy(py, h_b, hidden_size * sizeof(float));
                }
            }
        }
        
        // Store final hidden and cell states
        if (y_h) 
        {
            if (layout == 0) 
            {
                // Y_h shape is [num_directions, batch_size, hidden_size]
                py_h = (float *)y_h->datas + d * batch_size * hidden_size;
                memcpy(py_h, ph_t, batch_size * hidden_size * sizeof(float));
            } 
            else 
            {
                // Y_h shape is [batch_size, num_directions, hidden_size]
                for (b_idx = 0; b_idx < batch_size; b_idx++) 
                {
                    py_h = (float *)y_h->datas + b_idx * num_directions * hidden_size + d * hidden_size;
                    memcpy(py_h, ph_t + b_idx * hidden_size, hidden_size * sizeof(float));
                }
            }
        }
        
        if (y_c) 
        {
            if (layout == 0) 
            {
                // Y_c shape is [num_directions, batch_size, hidden_size]
                py_c = (float *)y_c->datas + d * batch_size * hidden_size;
                memcpy(py_c, pc_t, batch_size * hidden_size * sizeof(float));
            } 
            else 
            {
                // Y_c shape is [batch_size, num_directions, hidden_size]
                for (b_idx = 0; b_idx < batch_size; b_idx++) 
                {
                    py_c = (float *)y_c->datas + b_idx * num_directions * hidden_size + d * hidden_size;
                    memcpy(py_c, pc_t + b_idx * hidden_size, hidden_size * sizeof(float));
                }
            }
        }
    }
    
    // Free allocated memory
    free(gates);
    free(ph_t);
    free(pc_t);
    free(w_dequantized);
    free(r_dequantized);
}

void resolver_default_op_DynamicQuantizeLSTM(struct onnx_node_t * n)
{
    if (n->opset >= 1)
    {
        if (n->inputs[0]->type == ONNX_TENSOR_TYPE_FLOAT32)
        {
            n->init = DynamicQuantizeLSTM_init;
            n->exit = DynamicQuantizeLSTM_exit;
            n->reshape = DynamicQuantizeLSTM_reshape;
            n->operator_ = DynamicQuantizeLSTM_float32;
        }
    }
}