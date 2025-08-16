import os
import shutil
import time
import jinja2
import re
import json

from typing import Dict, Optional

from hvcc.types.compiler import CompilerResp, ExternInfo, Generator, CompilerNotif, CompilerMsg
from hvcc.interpreters.pd2hv.NotificationEnum import NotificationEnum
from hvcc.types.meta import Meta

fixed_params = ['time', 'depth', 'mix']
fixed_params_f = ['time_f', 'depth_f', 'mix_f']
max_param_num = 10 - len(fixed_params)
fx_type = "del"

def set_min_value(dic, key, value):
    if key not in dic:
        dic[key] = value
    else:
        dic[key] = min(value, dic[key])
    return dic[key] == value

def render_from_template(template_file, rendered_file, context):
    common_templates_dir = os.path.join(os.path.dirname(__file__), "common", "templates")
    fx_templates_dir = os.path.join(os.path.dirname(__file__), fx_type, "templates")
    templates_dir = [common_templates_dir, fx_templates_dir]
    loader = jinja2.FileSystemLoader(templates_dir)
    env = jinja2.Environment(loader=loader, trim_blocks=True, lstrip_blocks=True)
    rendered = env.get_template(template_file).render(**context)
    with open(rendered_file, 'w') as f:
        f.write(rendered)

class Nts1mkii_del(Generator):
    @classmethod
    def compile(
            cls,
            c_src_dir: str,
            out_dir: str,
            externs: ExternInfo,
            patch_name: Optional[str] = None,
            patch_meta: Meta = Meta(),
            num_input_channels: int = 0,
            num_output_channels: int = 0,
            copyright: Optional[str] = None,
            verbose: Optional[bool] = False
    ) -> CompilerResp:
        begin_time = time.time()
        print(f"--> Invoking nts1mkii_{fx_type}")

        out_dir = os.path.join(out_dir, "logue_unit")

        try:
            # check num of channels
            if num_input_channels > 2:
                print(f"Warning: {num_input_channels} input channels(ignored)")
            if num_output_channels != 2:
                raise Exception("FX units support only stereo output.")

            # ensure that the output directory does not exist
            out_dir = os.path.abspath(out_dir)
            if os.path.exists(out_dir):
                shutil.rmtree(out_dir)

            # copy over static files
            common_static_dir = os.path.join(os.path.dirname(__file__), "common", "static")
            fx_static_dir = os.path.join(os.path.dirname(__file__), fx_type, "static")
            os.makedirs(out_dir, exist_ok=False)
            for static_dir in [common_static_dir, fx_static_dir]:
                for filename in os.listdir(static_dir):
                    src_path = os.path.join(static_dir, filename)
                    dst_path = os.path.join(out_dir, filename)
                    shutil.copy2(src_path, dst_path)

            # copy C files
            for file in os.listdir(c_src_dir):
                src_file = os.path.join(c_src_dir, file)
                if os.path.isfile(src_file):
                    dest_file = os.path.join(out_dir, file)
                    shutil.copy2(src_file, dest_file)

            # values for rendering templates
            context = {
                'patch_name': patch_name,
                'msg_pool_size_kb': 1,     # minimum
                'input_queue_size_kb': 1,  # minimum
                'output_queue_size_kb': 0, # minimum
                'num_output_channels' : num_output_channels
            }

            # list of source files
            heavy_files_c = [
                f for f in os.listdir(c_src_dir)
                if os.path.isfile(os.path.join(out_dir, f)) and f.endswith('.c')
            ]
            context['heavy_files_c'] =  " ".join(heavy_files_c)

            heavy_files_cpp = [
                f for f in os.listdir(c_src_dir)
                if os.path.isfile(os.path.join(out_dir, f)) and f.endswith('.cpp')
            ]
            context['heavy_files_cpp'] = ' '.join(heavy_files_cpp)

            # external parameters excluding unit parameters
            numbered_params = []
            other_params = []
            for param in externs.parameters.inParam:
                p_name, p_rcv = param
                p_attr = p_rcv.attributes
                p_range = p_attr['max'] - p_attr['min']
                if p_name in fixed_params:
                    context[p_name] = {'name' : p_name}
                    context[p_name]['range'] = p_range
                    context[p_name]['min'] = p_attr['min']
                    context[p_name]['default'] = p_attr['default']
                    context['p_'+p_name+'hash'] = p_rcv.hash
                elif p_name in fixed_params_f:
                    context[p_name[:-2]] = {'name' : p_name}
                    context[p_name[:-2]]['range_f'] = p_range
                    context[p_name[:-2]]['min'] = p_attr['min']
                    context[p_name[:-2]]['default'] = p_attr['default']
                    context[p_name[:-2]]['hash'] = p_rcv.hash
                elif re.match(r'^_\d+_', p_name):
                    numbered_params.append(param)
                else:
                    other_params.append(param)
            
            # unit parameters
            unit_params = [None] * max_param_num

            # process parameter names
            for param in numbered_params:
                p_name, p_rcv = param
                match = re.match(r"^_(\d+)_(.+)$", p_name)
                param_num = int(match.group(1)) - 1
                if not (0 <= param_num < max_param_num):
                    raise IndexError(f"Index {param_num} is out of range.")
                p_display = match.group(2)
                if unit_params[param_num] is not None:
                    print(f'Warning: parameter {param_num} is duplicated ({unit_params[param_num]}, {p_name})')
                else:
                    unit_params[param_num] = param

            for param in other_params:
                p_name, p_rcv = param
                for i, value in enumerate(unit_params):
                    if value is None:
                        unit_params[i] = param
                        break
                else:
                    print(f"Warning: too many parameters")

            # store parameter attributes into a dcitionary (context)
            context['param'] = {}
            for i in range(max_param_num):
                if unit_params[i] is None:
                    continue
                # prefix (parameter number)
                p_key = f'param_id{i+1}'
                p_name, p_rcv = unit_params[i]
                p_attr = p_rcv.attributes
                context['param'][p_key] = {'name' : p_name}
                match = re.match(r'^_\d+_(.+)$', p_rcv.display)
                if match:
                    p_display = match.group(1)
                else:
                    p_display = p_rcv.display
                # postfix (floating-point)
                if p_name.endswith("_f"):
                    p_display = p_display[:-2]
                    p_max = p_attr['max']
                    p_min = p_attr['min']
                    p_range = p_max - p_min
                    if p_min < 0:
                        p_param_max = 1000
                        p_param_min = -1000
                    else:
                        p_param_max = 1023
                        p_param_min = 0

                    if set_min_value(context['param'][p_key], 'range_f', p_range):
                        context['param'][p_key]['max_f'] = p_max
                        context['param'][p_key]['min_f'] = p_min
                        context['param'][p_key]['max'] = p_param_max
                        context['param'][p_key]['min'] = p_param_min
                else:
                    p_max = p_attr['max']
                    p_min = p_attr['min']
                    p_range = p_max - p_min
                    if set_min_value(context['param'][p_key], 'range', p_range):
                        context['param'][p_key]['max'] = max(-32768, min(32767, int(p_max)))
                        context['param'][p_key]['min'] = max(-32768, int(p_min))
                # store other key-values
                context['param'][p_key]['hash'] = p_rcv.hash
                context['param'][p_key]['name'] = p_name
                context['param'][p_key]['default'] = p_attr['default']
                context['param'][p_key]['display'] = p_display

            # find the total number of parameters
            for i in range(max_param_num - 1, -1, -1):
                if unit_params[i] is not None:
                    num_param = i + 1
                    break
            else:
                num_param = 0
            context['num_param'] = num_param

            # store tables into a dcitionary (context)
            context['table'] = {}
            for table in externs.tables:
                t_name, t_tbl = table
                context['table'][t_name] = {'name' : t_name}
                context['table'][t_name]['hash'] = t_tbl.hash
                if t_name.endswith('_r'):
                    context['table'][t_name]['type'] = 'random'
                else:
                    context['table'][t_name]['type'] = 'none'

            # verbose
            if verbose:
                print(f"input channels:{num_input_channels}")
                print(f"output channels:{num_output_channels}")
                print(f"parameters: {externs.parameters}")
                print(f"events: {externs.events}")
                print(f"midi: {externs.midi}")
                print(f"tables: {externs.tables}")
                print(f"context: {json.dumps(context, indent=2, ensure_ascii=False)}")
            
            # estimate required heap memory
            render_from_template('Makefile.testmem',
                                 os.path.join(out_dir, "Makefile.testmem"),
                                 context)
            render_from_template('testmem.cpp',
                                 os.path.join(out_dir, "testmem.cpp"),
                                 context)
            
            # render files
            render_from_template('config.mk',
                                 os.path.join(out_dir, "config.mk"),
                                 context)
            render_from_template('logue_heavy.cpp',
                                 os.path.join(out_dir, "logue_heavy.cpp"),
                                 context)
            render_from_template('header.c',
                                 os.path.join(out_dir, "header.c"),
                                 context)

            # add definitions to HvUtils.h
            hvutils_src_path = os.path.join(c_src_dir, "HvUtils.h")
            hvutils_dst_path = os.path.join(out_dir, "HvUtils.h")
            with open(hvutils_src_path, 'r', encoding='utf-8') as f:
                src_lines = f.readlines()
    
            dst_lines = []
            for line in src_lines:
                if "// Assert" in line:
                    dst_lines.append('#include "logue_mem_hv.h"')
                    dst_lines.append("\n\n")
                elif "// Atomics" in line:
                    dst_lines.append('#include "logue_math_hv.h"')
                    dst_lines.append("\n\n")
                dst_lines.append(line)

            with open(hvutils_dst_path, 'w', encoding='utf-8') as f:
                f.writelines(dst_lines)

            # done
            end_time = time.time()

            return CompilerResp(
                stage='example_hvcc_generator',  # module name
                compile_time=end_time - begin_time,
                in_dir=c_src_dir,
                out_dir=out_dir
            )

        except Exception as e:
            return CompilerResp(
                stage=f"nts1mkii_{fx_type}",
                notifs=CompilerNotif(
                    has_error=True,
                    exception=e,
                    warnings=[],
                    errors=[CompilerMsg(
                        enum=NotificationEnum.ERROR_EXCEPTION,
                        message=str(e)
                    )]
                ),
                in_dir=c_src_dir,
                out_dir=out_dir,
                compile_time=time.time() - begin_time
            )
