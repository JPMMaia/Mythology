import bpy

class MythologyExportOperator(bpy.types.Operator):

    bl_idname = "mythology.export_json"
    bl_label = "Export JSON"

    def execute(self, context):

        # TODO call function that outputs a json corresponding to the selected nodes

        print("Hello World")
        return {'FINISHED'}

def mythology_export_menu(self, context):

    self.layout.operator("mythology.export_json")