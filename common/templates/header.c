/*
 *  File: header.c
 *
 *  NTS-1 mkII {{unit_type}} unit header definition
 *
 */

#include "unit_{{unit_type}}.h"

#ifndef PROJECT_DEV_ID
 #define PROJECT_DEV_ID=(0x0U)
#endif

#ifndef PROJECT_UNIT_ID
 #define PROJECT_UNIT_ID=(0x0U)
#endif

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),
    .target = UNIT_TARGET_PLATFORM | k_unit_module_{{unit_type}},
    .api = UNIT_API_VERSION,
    .dev_id = PROJECT_DEV_ID,
    .unit_id = PROJECT_UNIT_ID,
    .version = 0x00010000U,
    .name = "{{patch_name}}",
    .num_params = {{num_fixed_param + num_param}},
    .params = {
        // Format:
        // min, max, center, default, type, frac. bits, frac. mode, <reserved>, name
{% if unit_type in ["osc"] %}
    {% if shape is defined %}
        {% if shape['range_f'] is defined %}
        {{'{' ~ shape['min'] | int}}, {{shape['max'] | int}}, {{((shape['max'] - shape['min']) / 2.0) | int}}, {{((shape['default'] - shape['min_f']) / shape['range_f'] * (shape['max'] - shape['min'])) | int}}, k_unit_param_type_none, 1, 0, 0, {"SHAPE"}},
        {% else %}
        {{'{' ~ shape['min'] | int}}, {{shape['max'] | int}}, {{shape['default'] | int}}, {{shape['default'] | int}}, k_unit_param_type_none, 0, 0, 0, {"SHPE"}},
        {% endif %}
    {% else %}
        {0, 1023, 0, 256, k_unit_param_type_none, 1, 0, 0, {"SHPE"}},
    {% endif %}
    {% if alt is defined %}
        {% if alt['range_f'] is defined %}
        {{'{' ~ alt['min'] | int}}, {{alt['max'] | int}}, {{((alt['max'] - alt['min']) / 2.0) | int}}, {{((alt['default'] - alt['min_f']) / alt['range_f'] * (alt['max'] - alt['min'])) | int}}, k_unit_param_type_none, 1, 0, 0, {"ALT"}},
        {% else %}
        {{'{' ~ alt['min'] | int}}, {{alt['max'] | int}}, {{alt['default'] | int}}, {{alt['default'] | int}}, k_unit_param_type_none, 0, 0, 0, {"ALT"}},
        {% endif %}
    {% else %}
        {0, 1023, 0, 256, k_unit_param_type_none, 1, 0, 0, {"ALT"}},
    {% endif %}
{% endif %}
{% if unit_type in ["modfx", "delfx", "revfx"] %}
    {% if time is defined %}
        {% if time['range_f'] is defined %}
        {{'{' ~ time['min'] | int}}, {{time['max'] | int}}, {{((time['max'] - time['min']) / 2.0) | int}}, {{((time['default'] - time['min_f']) / time['range_f'] * (time['max'] - time['min'])) | int}}, k_unit_param_type_none, 1, 0, 0, {"TIME"}},
        {% else %}
        {{'{' ~ time['min'] | int}}, {{time['max'] | int}}, {{time['default'] | int}}, {{time['default'] | int}}, k_unit_param_type_none, 0, 0, 0, {"TIME"}},
        {% endif %}
    {% else %}
        {0, 1023, 0, 256, k_unit_param_type_none, 1, 0, 0, {"TIME"}},
    {% endif %}
    {% if depth is defined %}
        {% if depth['range_f'] is defined %}
        {{'{' ~ depth['min'] | int}}, {{depth['max'] | int}}, {{((depth['max'] - depth['min']) / 2.0) | int}}, {{((depth['default'] - depth['min_f']) / depth['range_f'] * (depth['max'] - depth['min'])) | int}}, k_unit_param_type_none, 1, 0, 0, {"DPTH"}},
        {% else %}
        {{'{' ~ depth['min'] | int}}, {{depth['max'] | int}}, {{depth['default'] | int}}, {{depth['default'] | int}}, k_unit_param_type_none, 1, 0, 0, {"DPTH"}},
        {% endif %}
    {% else %}
        {0, 1023, 0, 256, k_unit_param_type_none, 1, 0, 0, {"DPTH"}},
    {% endif %}
{% endif %}
{% if unit_type in ["delfx", "revfx"] %}
    {% if mix is defined %}
        {{'{' ~ mix['min'] | int}}, {{mix['max'] | int}}, {{mix['default'] | int}}, {{mix['default'] | int}}, k_unit_param_type_drywet, 1, 1, 0, {"MIX"}},
    {% else %}
        {-1000, 1000, 0, 0, k_unit_param_type_drywet, 1, 1, 0, {"MIX"}},
    {% endif %}
{% endif %}
        {% for i in range(1, 11 - num_fixed_param) %}
        {% set id = "param_id" ~ i %}
        {% if param[id] is defined %}
        {% if param[id]['range_f'] is defined %}
        {{'{' ~ (param[id]['min'] * 10) | int}}, {{(param[id]['max'] * 10) | int}}, {{(param[id]['default'] * 10) | int}}, {{(param[id]['default'] * 10) | int}}, k_unit_param_type_none, 1, 1, 0, {{ '{"' ~ param[id]['display'] ~ '"}}' }}{% if not loop.last %},{{"\n"}}{% endif %}
        {% else %}
        {{'{' ~ param[id]['min'] | int}}, {{param[id]['max'] | int}}, {{param[id]['default'] | int}}, {{param[id]['default'] | int}}, k_unit_param_type_none, 0, 0, 0, {{ '{"' ~ param[id]['display'] ~ '"}}' }}{% if not loop.last %},{{"\n"}}{% endif %}
        {% endif %}
        {% else %}{% raw %}
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}}{% endraw %}{% if not loop.last %},{% endif %}
        {% endif %}
        {% if loop.last %}}{{"\n"}}{% endif %}
        {% endfor %}
};
