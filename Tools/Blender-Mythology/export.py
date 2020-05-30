import bpy
import json

from .render_pass import render_pass_to_json

class MythologyExportOperator(bpy.types.Operator):

    bl_idname = "mythology.export_json"
    bl_label = "Export JSON"

    def execute(self, context):

        nodes = [node
                 for node in bpy.data.node_groups['NodeTree'].nodes]

        render_passes_json = render_pass_to_json(nodes)

        print(json.dumps(render_passes_json))
        
        return {'FINISHED'}

def mythology_export_menu(self, context):

    self.layout.operator("mythology.export_json")