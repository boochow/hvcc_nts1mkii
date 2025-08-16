#include <errno.h>
#include <sys/types.h>
#include <cstdlib>

#include "unit_delfx.h"
#include "osc_api.h"

#include "Heavy_{{patch_name}}.h"

#ifndef HV_MSGPOOLSIZE
 #define HV_MSGPOOLSIZE {{msg_pool_size_kb}}
#endif
#ifndef HV_INPUTQSIZE
 #define HV_INPUTQSIZE {{input_queue_size_kb}}
#endif
#ifndef HV_OUTPUTQSIZE
 #define HV_OUTPUTQSIZE {{output_queue_size_kb}}
#endif

static bool stop_unit_param;
static HeavyContextInterface* hvContext;

typedef enum {
    k_user_unit_param_time = 0,
    k_user_unit_param_depth,
    k_user_unit_param_mix,
    k_user_unit_param_id1,
    k_user_unit_param_id2,
    k_user_unit_param_id3,
    k_user_unit_param_id4,
    k_user_unit_param_id5,
    k_user_unit_param_id6,
    k_user_unit_param_id7,
    k_num_user_unit_param_id
} user_unit_param_id_t;

static unit_runtime_desc_t s_desc;
static int32_t params[k_num_user_unit_param_id];

{% if time is defined %}
    {% if time['range'] is defined %}
static int32_t time;
static bool time_dirty;
    {% elif time['range_f'] is defined %}
static float time;
static bool time_dirty;
    {% endif %}
{% endif %}
{% if depth is defined %}
    {% if depth['range'] is defined %}
static int32_t depth;
static bool depth_dirty;
    {% elif depth['range_f'] is defined %}
static float depth;
static bool depth_dirty;
    {% endif %}
{% endif %}
{% if mix is defined %}
    {% if mix['range'] is defined %}
static int32_t mix;
static bool mix_dirty;
    {% elif depth['range_f'] is defined %}
static float mix;
static bool mix_dirty;
    {% endif %}
{% endif %}

{% for i in range(1, 8) %}
    {% set id = "param_id" ~ i %}
    {% if param[id] is defined %}
        {% if param[id]['range'] is defined %}
static int32_t {{param[id]['name']}};
        {% elif param[id]['range_f'] is defined %}
static float {{param[id]['name']}};
        {% endif %}
    {% endif %}
{% endfor %}
{% if num_param > 0 %}
static bool param_dirty[{{num_param}}];
{% endif %}
{% for key, entry in table.items() %}
static float * table_{{ key }};
static unsigned int table_{{ key }}_len;
{% endfor %}

__unit_callback int8_t unit_init(const unit_runtime_desc_t * desc)
{
    stop_unit_param = true;
    {% if time is defined %}
    time = {{time['default']}};
    time_dirty = true;
    {% endif %}
    {% if depth is defined %}
    depth = {{depth['default']}};
    depth_dirty = true;
    {% endif %}
    {% if mix is defined %}
    mix = {{mix['default']}};
    mix_dirty = true;
    {% endif %}
    {% for i in range(1, 8) %}
      {% set id = "param_id" ~ i %}
      {% if param[id] is defined %}
    {{param[id]['name']}} = {{param[id]['default']}};
    param_dirty[{{i - 1}}] = true;
      {% endif %}
    {% endfor %}
    {% for key, entry in table.items() %}
    table_{{ key }} = hv_table_getBuffer(hvContext, HV_{{patch_name|upper}}_TABLE_{{key|upper}});
    table_{{ key }}_len = hv_table_getLength(hvContext, HV_{{patch_name|upper}}_TABLE_{{key|upper}});
    {% endfor %}

    if (!desc)
      return k_unit_err_undef;

    if (desc->target != unit_header.target)
      return k_unit_err_target;

    if (!UNIT_API_IS_COMPAT(desc->api))
      return k_unit_err_api_version;

    if (desc->samplerate != 48000)
      return k_unit_err_samplerate;

    if (desc->input_channels != 2 || desc->output_channels != 2) 
      return k_unit_err_geometry;

#if defined(UNIT_SDRAM_SIZE) && (UNIT_SDRAM_SIZE) > 0
    if (!desc->hooks.sdram_alloc)
      return k_unit_err_memory;
    init_sdram(desc->hooks.sdram_alloc);
#endif

    s_desc = *desc;
#ifdef RENDER_HALF
    hvContext = hv_{{patch_name}}_new_with_options(24000, HV_MSGPOOLSIZE, HV_INPUTQSIZE, HV_OUTPUTQSIZE);
#else
    hvContext = hv_{{patch_name}}_new_with_options(48000, HV_MSGPOOLSIZE, HV_INPUTQSIZE, HV_OUTPUTQSIZE);
#endif
    return k_unit_err_none;
}

__unit_callback void unit_render(const float * in, float * out, uint32_t frames)
{
#ifdef RENDER_HALF
    float buffer[frames];
    static float last_buf_l = 0.f;
    static float last_buf_r = 0.f;
#else
    float buffer[frames << 1];
#endif
    float * __restrict p = buffer;
    float * __restrict y = out;
    const float * y_e = y + 2 * frames;

    stop_unit_param = false;
    {% if time is defined %}
        {% if time['range'] is defined %}
    if (time_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_TIME, time)) {
            time_dirty = false;
        }
    }
        {% elif time['range_f'] is defined %}
    if (time_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_TIME_F, time)) {
            time_dirty = false;
        }
    }
        {% endif %}
    {% endif %}
    {% if depth is defined %}
        {% if depth['range'] is defined %}
    if (depth_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_DEPTH, depth)) {
            depth_dirty = false;
        }
    }
        {% elif depth['range_f'] is defined %}
    if (depth_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_DEPTH_F, depth)) {
            depth_dirty = false;
        }
    }
        {% endif %}
    {% endif %}
    {% if mix is defined %}
        {% if mix['range'] is defined %}
    if (mix_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_MIX, mix)) {
            mix_dirty = false;
        }
    }
        {% elif mix['range_f'] is defined %}
    if (mix_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_MIX_F, mix)) {
            mix_dirty = false;
        }
    }
        {% endif %}
    {% endif %}
    {% for i in range(1, 8) %}
    {% set id = "param_id" ~ i %}
    {% if param[id] is defined %}
    if (param_dirty[{{i - 1}}]) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{param[id]['name']|upper}}, {{param[id]['name']}})) {
            param_dirty[{{i - 1}}] = false;
        }
    }
    {% endif %}
    {% endfor %}
    {% for key, entry in table.items() %}
    {% if entry.type == "random" %}
    table_{{ key }}_len = hv_table_getLength(hvContext, HV_{{patch_name|upper}}_TABLE_{{key|upper}});
    for (int i = 0; i < table_{{ key }}_len ; i++) {
        table_{{ key }}[i] = osc_white();
    }
    {% endif %}
    {% endfor %}

#ifdef RENDER_HALF
    hv_processInlineInterleaved(hvContext, (float *) in, buffer, frames >> 1);
    for(int i = 0; y!= y_e; i++) {
        if (i & 1) {
            last_buf_l = *p++;
            last_buf_r = *p++;
            *(y++) = last_buf_l;
            *(y++) = last_buf_r;
        } else {
            *(y++) = (*p + last_buf_l) * 0.5;
            *(y++) = (*p + last_buf_r) * 0.5;
        }
    }
#else
    hv_processInlineInterleaved(hvContext, (float *) in, buffer, frames);
    for(; y!= y_e; ) {
        *(y++) = *p++;
    }
#endif
}

__unit_callback void unit_set_param_value(uint8_t id, int32_t value)
{
    float knob_f = param_val_to_f32(value);
    float f;

    if (stop_unit_param) {
        return; // avoid all parameters to be zero'ed after unit_init()
    }
    params[id] = value;
    switch(id){
    case k_user_unit_param_time:
        {% if time is defined %}
            {% if time['range'] is defined %}
        time = value;
        time_dirty = true;
            {% elif time['range_f'] is defined %}
        time = {{time['range_f']}} * knob_f + {{time['min']}};
        time_dirty = true;
            {% endif %}
        {% endif %}
        break;        
    case k_user_unit_param_depth:
        {% if depth is defined %}
            {% if depth['range'] is defined %}
        depth = value;
        depth_dirty = true;
            {% elif depth['range_f'] is defined %}
        depth = {{depth['range_f']}} * knob_f + {{depth['min']}};
        depth_dirty = true;
            {% endif %}
        {% endif %}
        break;
    case k_user_unit_param_mix:
        {% if mix is defined %}
            {% if mix['range'] is defined %}
        mix = value;
        mix_dirty = true;
            {% elif mix['range_f'] is defined %}
        mix = {{mix['range_f']}} * knob_f + {{mix['min']}};
        mix_dirty = true;
            {% endif %}
        {% endif %}
        break;
    {% for i in range(1, 8) %}
    {% set id = "param_id" ~ i %}
    {% if param[id] is defined %}
    case k_user_unit_{{id}}:
        {% if param[id]['range'] is defined %}
        {{param[id]['name']}} = value;
        param_dirty[{{i - 1}}] = true;
        {% elif param[id]['range_f'] is defined %}
        {{param[id]['name']}} = {{param[id]['range_f']}} * value / ({{param[id]['max'] - param[id]['min']}}) + ({{param[id]['min_f']}});;
        param_dirty[{{i - 1}}] = true;
        {% endif %}
        break;
    {% endif %}
    {% endfor %}
    default:
      break;
    }
}

__unit_callback int32_t unit_get_param_value(uint8_t id) {
    return params[id];
}

__unit_callback void unit_teardown() {
}

__unit_callback void unit_reset() {
}

__unit_callback void unit_resume() {
}

__unit_callback void unit_suspend() {
}

__unit_callback const char * unit_get_param_str_value(uint8_t id, int32_t value\
) {
    return nullptr;
}

__unit_callback void unit_set_tempo(uint32_t tempo) {
}

__unit_callback void unit_tempo_4ppqn_tick(uint32_t counter) {
}

// dummy implementation for some starndard functions

int snprintf(char str[], size_t size, const char *format, ...) {
  return 0;
}

extern "C" void __cxa_pure_virtual() {
    while (1);  // do nothing
}

extern "C" void operator delete(void* ptr) noexcept {
    free(ptr);
}

extern "C" void operator delete[](void* ptr) noexcept {
    free(ptr);
}

extern "C" void* _sbrk(ptrdiff_t incr) {
    errno = ENOMEM;
    return (void*)-1;
}
