#include <cstdio>
#include "Heavy_{{patch_name}}.h"

extern size_t heap_offset;
extern size_t sdram_offset;

int main(int argc, char* argv[]) {
    HeavyContextInterface* hvContext;
    float in_buffer[64 * 2];
    float out_buffer[64 * 2];

    hvContext = hv_{{patch_name}}_new_with_options(48000, {{msg_pool_size_kb}}, {{input_queue_size_kb}}, {{output_queue_size_kb}});
    for(int i = 0; i < 100; i++) {
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
        {% if mix is defined %}
        {% if mix['range'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_MIX, {{mix['default']}});
        {% elif mix['range_f'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_MIX_F, {{mix['default']}});
        {% endif %}
        {% endif %}
        {% for i in range(1, 9) %}
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
