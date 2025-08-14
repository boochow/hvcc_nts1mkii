#include <errno.h>
#include <sys/types.h>
#include <cstdlib>

#include "unit_osc.h"

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

static bool stop_osc_param;
static HeavyContextInterface* hvContext;

typedef enum {
    k_user_osc_param_shape = 0,
    k_user_osc_param_shiftshape,
    k_user_osc_param_id1,
    k_user_osc_param_id2,
    k_user_osc_param_id3,
    k_user_osc_param_id4,
    k_user_osc_param_id5,
    k_user_osc_param_id6,
    k_user_osc_param_id7,
    k_user_osc_param_id8,
    k_num_user_osc_param_id
} user_osc_param_id_t;

static unit_runtime_desc_t s_desc;
static int32_t params[k_num_user_osc_param_id];

{% if shape is defined %}
    {% if shape['range'] is defined %}
static int32_t shape;
static bool shape_dirty;
    {% elif shape['range_f'] is defined %}
static float shape;
static bool shape_dirty;
    {% endif %}
{% endif %}
{% if alt is defined %}
    {% if alt['range'] is defined %}
static int32_t alt;
static bool alt_dirty;
    {% elif alt['range_f'] is defined %}
static float alt;
static bool alt_dirty;
    {% endif %}
{% endif %}

{% if noteon_trig is defined %}
static bool noteon_trig_dirty;
static uint8_t noteon_velocity;
{% endif %}
{% if noteoff_trig is defined %}
static bool noteoff_trig_dirty;
{% endif %}

{% for i in range(1, 9) %}
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
    stop_osc_param = true;
#ifdef RENDER_HALF
    hvContext = hv_{{patch_name}}_new_with_options(24000, HV_MSGPOOLSIZE, HV_INPUTQSIZE, HV_OUTPUTQSIZE);
#else
    hvContext = hv_{{patch_name}}_new_with_options(48000, HV_MSGPOOLSIZE, HV_INPUTQSIZE, HV_OUTPUTQSIZE);
#endif
    {% if shape is defined %}
    shape = {{shape['default']}};
    shape_dirty = true;
    {% endif %}
    {% if alt is defined %}
    alt = {{alt['default']}};
    alt_dirty = true;
    {% endif %}
    {% for i in range(1, 9) %}
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

    if (desc->input_channels != 2 || desc->output_channels != 1) 
      return k_unit_err_geometry;
               
    s_desc = *desc;
    return k_unit_err_none;
}

__unit_callback void unit_render(const float * in, float * out, uint32_t frames)
{
    const unit_runtime_osc_context_t *ctxt = static_cast<const unit_runtime_osc_context_t *>(s_desc.hooks.runtime_context);
#ifdef RENDER_HALF
    float buffer[frames >> 1];
    static float last_buf = 0.f;
#else
    float buffer[frames];
#endif
    float * __restrict p = buffer;
    float * __restrict y = out;
    const float * y_e = y + frames;
    {% if slfo is defined %}
    float slfo;
    {% endif %}
    {% if pitch is defined %} 
    const float pitch = osc_w0f_for_note((ctxt->pitch)>>8, ctxt->pitch & 0xFF) * k_samplerate;
    {% endif %}
    {% if pitch_note is defined %}
    const float note_f = (ctxt->pitch >> 8) + (ctxt->pitch & 0xFF) * 0.00390625;
    {% endif %}

    stop_osc_param = false;
    {% if slfo is defined %}
    slfo = q31_to_f32(ctxt->shape_lfo);
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SLFO, slfo);
    {% endif %}
    {% if pitch is defined %} 
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH, pitch);
    {% endif %}
    {% if pitch_note is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH_NOTE, note_f);
    {% endif %}
    {% if shape is defined %}
        {% if shape['range'] is defined %}
    if (shape_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE, shape)) {
            shape_dirty = false;
        }
    }
        {% elif shape['range_f'] is defined %}
    if (shape_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE_F, shape)) {
            shape_dirty = false;
        }
    }
        {% endif %}
    {% endif %}
    {% if alt is defined %}
        {% if alt['range'] is defined %}
    if (alt_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT, alt)) {
            alt_dirty = false;
        }
    }
        {% elif alt['range_f'] is defined %}
    if (alt_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT_F, alt)) {
            alt_dirty = false;
        }
    }
        {% endif %}
    {% endif %}
    {% for i in range(1, 9) %}
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
    {% if noteon_trig is defined %}
    if (noteon_trig_dirty) {
        if (hv_sendBangToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_NOTEON_TRIG)) {
            noteon_trig_dirty = false;
        }
    }
    {% endif %}
    {% if noteoff_trig is defined %}
    if (noteoff_trig_dirty) {
        if (hv_sendBangToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_NOTEOFF_TRIG)) {
            noteoff_trig_dirty = false;
        }
    }
    {% endif %}

#ifdef RENDER_HALF
    hv_processInlineInterleaved(hvContext, (float *) in, buffer, frames >> 1);
    for(int i = 0; y!= y_e; i++) {
        if (i & 1) {
            last_buf = *p++;
            *(y++) = last_buf;
        } else {
            *(y++) = (*p + last_buf) * 0.5;
        }
    }
#else
    hv_processInlineInterleaved(hvContext, (float *) in, buffer, frames);
    for(; y!= y_e; ) {
        *(y++) = *p++;
    }
#endif
}

__unit_callback void unit_note_on(uint8_t note, uint8_t velo)
{
    {% if noteon_trig is defined %} 
    noteon_trig_dirty = true;
    noteon_velocity = velo;
    {% else %}
    (void) note, velo;
    {% endif %}
}

__unit_callback void unit_note_off(uint8_t note)
{
    {% if noteoff_trig is defined %} 
    noteoff_trig_dirty = true;
    {% else %}
    (void) note;
    {% endif %}
}

__unit_callback void unit_set_param_value(uint8_t id, int32_t value)
{
    float knob_f = param_val_to_f32(value);
    float f;

    if (stop_osc_param) {
        return; // avoid all parameters to be zero'ed after unit_init()
    }
    params[id] = value;
    switch(id){
    case k_user_osc_param_shape:
        {% if shape is defined %}
            {% if shape['range'] is defined %}
        shape = value;
        shape_dirty = true;
            {% elif shape['range_f'] is defined %}
        shape = {{shape['range_f']}} * knob_f + {{shape['min']}};
        shape_dirty = true;
            {% endif %}
        {% endif %}
        break;        
    case k_user_osc_param_shiftshape:
        {% if alt is defined %}
            {% if alt['range'] is defined %}
        alt = value;
        alt_dirty = true;
            {% elif alt['range_f'] is defined %}
        alt = {{alt['range_f']}} * knob_f + {{alt['min']}};
        alt_dirty = true;
            {% endif %}
        {% endif %}
        break;
    {% for i in range(1, 9) %}
    {% set id = "param_id" ~ i %}
    {% if param[id] is defined %}
    case k_user_osc_{{id}}:
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

__unit_callback void unit_all_note_off() {
}

__unit_callback void unit_pitch_bend(uint16_t bend) {
}

__unit_callback void unit_channel_pressure(uint8_t press) {
}

__unit_callback void unit_aftertouch(uint8_t note, uint8_t press) {
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
