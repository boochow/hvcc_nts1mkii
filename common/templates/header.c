/*
 *  File: header.c
 *
 *  NTS-1 mkII delay effect unit header definition
 *
 */

#include "unit_delfx.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_delfx,
    .api = UNIT_API_VERSION,
    .dev_id = 0x0U,
    .unit_id = 0x0U,
    .version = 0x00010000U,
    .name = "{{patch_name}}",
    .num_params = {{3 + num_param}},
    .params = {
        // Format:
        // min, max, center, default, type, frac. bits, frac. mode, <reserved>, name
        {0, 1023, 0, 256, k_unit_param_type_none, 1, 0, 0, {"TIME"}},
        {0, 1023, 0, 256, k_unit_param_type_none, 1, 0, 0, {"DPTH"}},
        {-1000, 1000, 0, 0, k_unit_param_type_drywet, 1, 1, 0, {"MIX"}},
        {% for i in range(1, 8) %}
        {% set id = "param_id" ~ i %}
        {% if param[id] is defined %}
        {{'{' ~ param[id]['min'] | int}}, {{param[id]['max'] | int}}, {{param[id]['default'] | int}}, {{param[id]['default'] | int}}, k_unit_param_type_none, 0, 0, 0, {{ '{"' ~ param[id]['display'] ~ '"}}' }}{% if not loop.last %},{{"\n"}}{% endif %}
        {% else %}{% raw %}
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}}{% endraw %}{% if not loop.last %},{% endif %}
        {% endif %}
        {% if loop.last %}}{{"\n"}}{% endif %}
        {% endfor %}
};
