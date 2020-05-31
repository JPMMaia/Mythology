bl_info = {
    "name": "Mythology Addon",
    "blender": (2, 82, 0),
    "category": "Object",
}

import bpy
import nodeitems_utils
from .export import *
from .render_node_tree import *
from .render_pass import *

classes = (
    render_node_tree.RenderNodeTree,
    
    render_pass.AccessFlagsNodeSocket,
    render_pass.AttachmentNodeSocket,
    render_pass.AttachmentReferenceNodeSocket,
    render_pass.DependencyFlagsNodeSocket,
    render_pass.FormatNodeSocket,
    render_pass.ImageLayoutNodeSocket,
    render_pass.LoadOperationNodeSocket,
    render_pass.PipelineBindPointNodeSocket,
    render_pass.PipelineStageFlagsNodeSocket,
    render_pass.RenderPassNodeSocket,
    render_pass.SampleCountNodeSocket,
    render_pass.StoreOperationNodeSocket,
    render_pass.SubpassDependencyNodeSocket,
    render_pass.SubpassNodeSocket,

    render_pass.AccessFlagsNode,
    render_pass.AttachmentNode,
    render_pass.AttachmentReferenceNode,
    render_pass.RenderPassNode,
    render_pass.SubpassDependencyNode,
    render_pass.SubpassNode,

    export.MythologyExportOperator,
)

def register():

    from bpy.utils import register_class
    for cls in classes:
        register_class(cls)

    nodeitems_utils.register_node_categories('RENDER_PASS', render_pass_node_categories)

    bpy.types.NODE_MT_node.append(export.mythology_export_menu)


def unregister():

    bpy.types.NODE_MT_node.remove(export.mythology_export_menu)

    nodeitems_utils.unregister_node_categories('RENDER_PASS')

    from bpy.utils import unregister_class
    for cls in reversed(classes):
        unregister_class(cls)