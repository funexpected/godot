#include "../onnx.h"
#include "../matrix.h"
#include <string.h>
#include <math.h>

enum {
    LSTM_INPUT_X = 0,
    LSTM_INPUT_W = 1,
    LSTM_INPUT_R = 2,
    LSTM_INPUT_B = 3,
    LSTM_INPUT_SEQUENCE_LENS = 4,
    LSTM_INPUT_INITIAL_H = 5,
    LSTM_INPUT_INITIAL_C = 6,
    LSTM_INPUT_P = 7,
};

enum {
    LSTM_OUTPUT_Y = 0,
    LSTM_OUTPUT_Y_H = 1,
    LSTM_OUTPUT_Y_C = 2,
};

struct lstm_param_t {
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

static int LSTM_init(struct onnx_node_t * n)
{
    struct lstm_param_t * p;
    char * dir;
    
    if (n->ninput >= 3 && n->noutput >= 1) 
    {
        p = malloc(sizeof(struct lstm_param_t));
        if (p) 
        {
            memset(p, 0, sizeof(struct lstm_param_t));
            
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
            p->has_bias = (n->ninput > LSTM_INPUT_B && n->inputs[LSTM_INPUT_B]) ? 1 : 0;
            p->has_sequence_lens = (n->ninput > LSTM_INPUT_SEQUENCE_LENS && n->inputs[LSTM_INPUT_SEQUENCE_LENS]) ? 1 : 0;
            p->has_initial_h = (n->ninput > LSTM_INPUT_INITIAL_H && n->inputs[LSTM_INPUT_INITIAL_H]) ? 1 : 0;
            p->has_initial_c = (n->ninput > LSTM_INPUT_INITIAL_C && n->inputs[LSTM_INPUT_INITIAL_C]) ? 1 : 0;
            p->has_peephole = (n->ninput > LSTM_INPUT_P && n->inputs[LSTM_INPUT_P]) ? 1 : 0;
            
            n->priv = p;
            return 1;
        }
    }
    return 0;
}

static int LSTM_exit(struct onnx_node_t * n)
{
    struct lstm_param_t * p = (struct lstm_param_t *)n->priv;
    
    if (p) 
    {
        free(p);
    }
    return 1;
}

static int LSTM_reshape(struct onnx_node_t * n)
{
    struct onnx_tensor_t * x = n->inputs[LSTM_INPUT_X];
    struct lstm_param_t * p = (struct lstm_param_t *)n->priv;
    int seq_length, batch_size;
    
    if (!p)
        return 0;
    
    // Parse dimensions based on layout
    if (p->layout == 0) 
    {
        // Layout 0: [seq_length, batch_size, input_size]
        seq_length = x->dims[0];
        batch_size = x->dims[1];
        // input_size = x->dims[2] - не используется в этой функции
    } 
    else 
    {
        // Layout 1: [batch_size, seq_length, input_size]
        batch_size = x->dims[0];
        seq_length = x->dims[1];
        // input_size = x->dims[2] - не используется в этой функции
    }
    
    // Reshape Y if requested
    if (n->noutput > LSTM_OUTPUT_Y && n->outputs[LSTM_OUTPUT_Y]) 
    {
        struct onnx_tensor_t * y = n->outputs[LSTM_OUTPUT_Y];
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
        
        if (!onnx_tensor_reshape(y, dims, 4, x->type))
            return 0;
    }
    
    // Reshape Y_h if requested
    if (n->noutput > LSTM_OUTPUT_Y_H && n->outputs[LSTM_OUTPUT_Y_H]) 
    {
        struct onnx_tensor_t * y_h = n->outputs[LSTM_OUTPUT_Y_H];
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
        
        if (!onnx_tensor_reshape(y_h, dims, 3, x->type))
            return 0;
    }
    
    // Reshape Y_c if requested
    if (n->noutput > LSTM_OUTPUT_Y_C && n->outputs[LSTM_OUTPUT_Y_C]) 
    {
        struct onnx_tensor_t * y_c = n->outputs[LSTM_OUTPUT_Y_C];
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
        
        if (!onnx_tensor_reshape(y_c, dims, 3, x->type))
            return 0;
    }
    
    return 1;
}

static void LSTM_float32(struct onnx_node_t * n)
{
    struct lstm_param_t * p = (struct lstm_param_t *)n->priv;
    struct onnx_tensor_t * x = n->inputs[LSTM_INPUT_X];
    struct onnx_tensor_t * w = n->inputs[LSTM_INPUT_W];
    struct onnx_tensor_t * r = n->inputs[LSTM_INPUT_R];
    struct onnx_tensor_t * b = p->has_bias ? n->inputs[LSTM_INPUT_B] : NULL;
    struct onnx_tensor_t * initial_h = p->has_initial_h ? n->inputs[LSTM_INPUT_INITIAL_H] : NULL;
    struct onnx_tensor_t * initial_c = p->has_initial_c ? n->inputs[LSTM_INPUT_INITIAL_C] : NULL;
    struct onnx_tensor_t * p_tensor = p->has_peephole ? n->inputs[LSTM_INPUT_P] : NULL;
    struct onnx_tensor_t * y = (n->noutput > LSTM_OUTPUT_Y) ? n->outputs[LSTM_OUTPUT_Y] : NULL;
    struct onnx_tensor_t * y_h = (n->noutput > LSTM_OUTPUT_Y_H) ? n->outputs[LSTM_OUTPUT_Y_H] : NULL;
    struct onnx_tensor_t * y_c = (n->noutput > LSTM_OUTPUT_Y_C) ? n->outputs[LSTM_OUTPUT_Y_C] : NULL;
    float * px, * pw, * pr, * ppi, * ppo, * ppf, * py, * py_h, * py_c;
    float * ph_t, * pc_t, * pbx, * pbh;
    float * gates, * it, * ot, * ft, * ct;
    int seq_length, batch_size, input_size, hidden_size;
    int num_directions = p->num_directions;
    int layout = p->layout;
    int i, j, d, b_idx, s_idx;
    
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
    
    // Allocate memory for intermediate calculations and hidden/cell states
    gates = (float *)malloc(batch_size * 4 * hidden_size * sizeof(float));
    ph_t = (float *)malloc(batch_size * hidden_size * sizeof(float));
    pc_t = (float *)malloc(batch_size * hidden_size * sizeof(float));
    
    if (!gates || !ph_t || !pc_t) 
    {
        if (gates) free(gates);
        if (ph_t) free(ph_t);
        if (pc_t) free(pc_t);
        return;
    }
    
    // Set up gate pointers
    it = gates;
    ot = gates + batch_size * hidden_size;
    ft = gates + 2 * batch_size * hidden_size;
    ct = gates + 3 * batch_size * hidden_size;
    
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
        
        // Get weight and bias pointers for this direction
        pw = (float *)w->datas + d * 4 * hidden_size * input_size;
        pr = (float *)r->datas + d * 4 * hidden_size * hidden_size;
        
        if (b) 
        {
            pbx = (float *)b->datas + d * 8 * hidden_size;            // W bias
            pbh = (float *)b->datas + d * 8 * hidden_size + 4 * hidden_size;  // R bias
        }
        
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
            if (strcmp(p->direction, "reverse") == 0) 
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
            
            // Calculate gates: Xt*(Wi^T) + Ht-1*(Ri^T) + Wbi + Rbi
            for (b_idx = 0; b_idx < batch_size; b_idx++) 
            {
                float * x_b = px + b_idx * input_size;
                float * h_b = ph_t + b_idx * hidden_size;
                float * c_b = pc_t + b_idx * hidden_size;
                float * gates_b = gates + b_idx * hidden_size;
                
                // X * W^T for all gates (i, o, f, c)
                for (i = 0; i < 4; i++) 
                {
                    float * g = gates_b + i * batch_size * hidden_size;
                    float * wi = pw + i * hidden_size * input_size;
                    
                    // Matrix multiplication X * W^T for this gate
                    for (j = 0; j < hidden_size; j++) 
                    {
                        float sum = 0.0f;
                        for (int k = 0; k < input_size; k++) 
                        {
                            sum += x_b[k] * wi[j * input_size + k];
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
                
                // H * R^T for all gates (i, o, f, c)
                for (i = 0; i < 4; i++) 
                {
                    float * g = gates_b + i * batch_size * hidden_size;
                    float * ri = pr + i * hidden_size * hidden_size;
                    
                    // Matrix multiplication H * R^T for this gate
                    for (j = 0; j < hidden_size; j++) 
                    {
                        float sum = 0.0f;
                        for (int k = 0; k < hidden_size; k++) 
                        {
                            sum += h_b[k] * ri[j * hidden_size + k];
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
                
                // Add peephole connections if available
                if (p->has_peephole) 
                {
                    // Input gate: + Pi (.) Ct-1
                    for (j = 0; j < hidden_size; j++) 
                    {
                        it[j] += ppi[j] * c_b[j];
                    }
                    
                    // Forget gate: + Pf (.) Ct-1
                    for (j = 0; j < hidden_size; j++) 
                    {
                        ft[j] += ppf[j] * c_b[j];
                    }
                }
                
                // Apply activation functions and calculate cell & hidden states
                // it = f(Xt*(Wi^T) + Ht-1*(Ri^T) + Pi (.) Ct-1 + Wbi + Rbi)
                // ft = f(Xt*(Wf^T) + Ht-1*(Rf^T) + Pf (.) Ct-1 + Wbf + Rbf)
                // ct = g(Xt*(Wc^T) + Ht-1*(Rc^T) + Wbc + Rbc)
                // Ct = ft (.) Ct-1 + it (.) ct
                // ot = f(Xt*(Wo^T) + Ht-1*(Ro^T) + Po (.) Ct + Wbo + Rbo)
                // Ht = ot (.) h(Ct)
                
                // Apply activations and update cell state
                for (j = 0; j < hidden_size; j++) 
                {
                    float i_val = sigmoid(it[j]);
                    float f_val = sigmoid(ft[j]);
                    float c_val = tanhf(ct[j]);
                    
                    // Update cell state: Ct = ft (.) Ct-1 + it (.) ct
                    c_b[j] = f_val * c_b[j] + i_val * c_val;
                    
                    // Apply clip if specified
                    if (p->clip > 0) 
                    {
                        if (c_b[j] > p->clip)
                            c_b[j] = p->clip;
                        else if (c_b[j] < -p->clip)
                            c_b[j] = -p->clip;
                    }
                }
                
                // Add peephole to output gate if available: Po (.) Ct
                if (p->has_peephole) 
                {
                    for (j = 0; j < hidden_size; j++) 
                    {
                        ot[j] += ppo[j] * c_b[j];
                    }
                }
                
                // Update hidden state: Ht = ot (.) h(Ct)
                for (j = 0; j < hidden_size; j++) 
                {
                    float o_val = sigmoid(ot[j]);
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
}

void resolver_default_op_LSTM(struct onnx_node_t * n)
{
    if(n->opset >= 14)
    {
    }
    else if(n->opset >= 7)
    {
        switch(n->inputs[0]->type)
        {
        case ONNX_TENSOR_TYPE_FLOAT32:
            n->init = LSTM_init;
            n->exit = LSTM_exit;
            n->reshape = LSTM_reshape;
            n->operator_ = LSTM_float32;
            break;
        default:
            break;
        }
    }
    else if(n->opset >= 1)
    {
        // Handle older opsets if needed
    }
}
