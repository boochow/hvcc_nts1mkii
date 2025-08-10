/*
 *  File: header.c
 *
 *  NTS-1 mkII oscillator unit header definition
 *
 */

#include "unit_osc.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_osc,
    .api = UNIT_API_VERSION,
    .dev_id = 0x0U,
    .unit_id = 0x0U,
    .version = 0x00010000U,
    .name = "{{patch_name}}",
    .num_params = {{2 + num_param}},
    .params = {
        // Format:
        // min, max, center, default, type, frac. bits, frac. mode, <reserved>, name
        {0, 1023, 0, 0, k_unit_param_type_none, 0, 0, 0, {"SHPE"}},
        {0, 1023, 0, 0, k_unit_param_type_none, 0, 0, 0, {"ALT"}},
        {% for i in range(1, 9) %}
        {% set id = "param_id" ~ i %}
        {% if param[id] is defined %}
        {{'{' ~ param[id]['min']}}, {{param[id]['max']}}, {{param[id]['default']}}, {{param[id]['default']}}, k_unit_param_type_none, 0, 0, 0, {{ '{"' ~ param[id]['display'] ~ '"}}' }}{% if not loop.last %},{{"\n"}}{% endif %}
        {% else %}{% raw %}
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}}{% endraw %}{% if not loop.last %},{% endif %}
        {% endif %}
        {% if loop.last %}}{{"\n"}}{% endif %}
        {% endfor %}
};
