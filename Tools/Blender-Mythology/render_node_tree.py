import bpy


class RenderNodeTree(bpy.types.NodeTree):

    """A custom node tree type that will show up in the editor type list"""

    bl_label = "Render Node Tree"
    bl_icon = "NODETREE"


class RenderTreeNode:
    @classmethod
    def poll(cls, ntree):
        return ntree.bl_idname == "RenderNodeTree"
