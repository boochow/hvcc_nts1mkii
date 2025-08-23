#include <cstdio>
#include <cstdlib>
#include "Heavy_{{patch_name}}.h"

extern size_t heap_offset;
extern size_t sdram_offset;

static unsigned char *sdram_alloc(unsigned int n) {
    return (unsigned char *) malloc(n);
}

int main(int argc, char* argv[]) {
    HeavyContextInterface* hvContext;
    float in_buffer[64 * 2];
    float out_buffer[64 * 2];

    init_sdram(sdram_alloc);

    hvContext = hv_{{patch_name}}_new_with_options(48000, {{msg_pool_size_kb}}, {{input_queue_size_kb}}, {{output_queue_size_kb}});

    for(int i = 0; i < 135000; i++) { // 135000 = 48000 * 60sec * 3min / 64
        {% if unit_type in ["osc"] %}
        {% if slfo is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SLFO, 0);
        {% endif %}
        {% if pitch is defined %} 
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH, 440.f);
        {% elif pitch_note is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH_NOTE, 60);
        {% endif %}
        {% if shape is defined %}
        {% if shape['range'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE, {{shape['default']}});
        {% elif shape['range_f'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE_F, {{shape['default']}});
        {% endif %}
        {% endif %}
        {% if alt is defined %}
        {% if alt['range'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT, {{alt['default']}});
        {% elif alt['range_f'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT_F, {{alt['default']}});
        {% endif %}
        {% endif %}
        {% endif %}
        {% if unit_type in ["modfx", "delfx", "revfx"] %}
        {% if time is defined %}
        {% if time['range'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_TIME, {{time['default']}});
        {% elif time['range_f'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_TIME_F, {{time['default']}});
        {% endif %}
        {% endif %}
        {% if depth is defined %}
        {% if depth['range'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_DEPTH, {{depth['default']}});
        {% elif depth['range_f'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_DEPTH_F, {{depth['default']}});
        {% endif %}
        {% endif %}
        {% endif %}
        {% if unit_type in ["delfx", "revfx"] %}
        {% if mix is defined %}
        {% if mix['range'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_MIX, {{mix['default']}});
        {% elif mix['range_f'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_MIX_F, {{mix['default']}});
        {% endif %}
        {% endif %}
        {% endif %}
        {% for i in range(1, 11 - num_fixed_param) %}
        {% set id = "param_id" ~ i %}
        {% if param[id] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{param[id]['name']|upper}}, {{param[id]['default']}});
        {% endif %}
        {% endfor %}
        hv_processInline(hvContext, in_buffer, out_buffer, 64);
    }
    printf("total: %ld\n", heap_offset);
    printf("sdram: %ld\n", sdram_offset);
}
