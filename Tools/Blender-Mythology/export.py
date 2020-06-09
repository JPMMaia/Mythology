import bpy
import json

from .pipeline_state import pipeline_state_to_json, shader_module_to_json
from .render_pass import render_pass_to_json

class MythologyExportOperator(bpy.types.Operator):

    bl_idname = "mythology.export_json"
    bl_label = "Export JSON"

    def execute(self, context):

        nodes = [node
                 for node_group in bpy.data.node_groups
                 for node in node_group.nodes]

        render_passes_json = render_pass_to_json(nodes)
        print(json.dumps(render_passes_json))

        shader_modules = shader_module_to_json(nodes)
        print(json.dumps(shader_modules[1]))

        pipeline_states_json = pipeline_state_to_json(nodes, render_passes_json, shader_modules)
        print(json.dumps(pipeline_states_json))        
        
        return {'FINISHED'}

def mythology_export_menu(self, context):

    self.layout.operator("mythology.export_json")