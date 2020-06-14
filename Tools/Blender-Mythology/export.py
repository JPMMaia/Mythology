import bpy
import json

from .draw import frame_commands_to_json
from .pipeline_state import create_samplers_json, create_descriptor_set_layouts_json, create_pipeline_layouts_json, pipeline_state_to_json, shader_module_to_json
from .render_pass import render_pass_to_json

class MythologyExportOperator(bpy.types.Operator):

    bl_idname = "mythology.export_json"
    bl_label = "Export JSON"

    def execute(self, context):

        nodes = [node
                 for node_group in bpy.data.node_groups
                 for node in node_group.nodes]

        render_passes = render_pass_to_json(nodes)
        shader_modules = shader_module_to_json(nodes)
        samplers = create_samplers_json(nodes)
        descriptor_set_layouts = create_descriptor_set_layouts_json(nodes, samplers)
        pipeline_layouts = create_pipeline_layouts_json(nodes, descriptor_set_layouts)
        pipeline_states = pipeline_state_to_json(nodes, render_passes, shader_modules, pipeline_layouts)
        frame_commands = frame_commands_to_json(nodes)

        output_json = {
            "render_passes": render_passes[2],
            "shader_modules": shader_modules[1],
            "samplers": samplers[1],
            "descriptor_set_layouts": descriptor_set_layouts[1],
            "pipeline_layouts": pipeline_layouts[1],
            "pipeline_states": pipeline_states[1],
            "frame_commands": frame_commands
        }

        print(json.dumps(output_json))
        
        return {'FINISHED'}

def mythology_export_menu(self, context):

    self.layout.operator("mythology.export_json")
