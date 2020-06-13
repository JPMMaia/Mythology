bl_info = {
    "name": "Mythology Addon",
    "blender": (2, 82, 0),
    "category": "Object",
}

import bpy
import nodeitems_utils

from .common import *
from .draw import *
from .export import *
from .pipeline_state import *
from .render_node_tree import *
from .render_pass import *
from .resources import *

classes = (
    render_node_tree.RenderNodeTree,

    common.Extent2DNodeSocket,
    common.Offset2DNodeSocket,
    common.Rect2DNodeSocket,

    common.Extent2DNode,
    common.Offset2DNode,
    common.Rect2DNode,

    draw.ClearColorValueNodeSocket,
    draw.ExecutionNodeSocket,
    draw.ImageMemoryBarrierNodeSocket,

    draw.BeginFrameNode,
    draw.ClearColorValueNode,
    draw.ClearColorImageNode,
    draw.EndFrameNode,
    draw.ImageMemoryBarrierNode,
    draw.PipelineBarrierNode,

    pipeline_state.ColorBlendAttachmentStateNodeSocket,
    pipeline_state.ColorBlendStateNodeSocket,
    pipeline_state.DepthStencilStateNodeSocket,
    pipeline_state.DescriptorSetLayoutBindingNodeSocket,
    pipeline_state.DescriptorSetLayoutNodeSocket,
    pipeline_state.DynamicStateNodeSocket,
    pipeline_state.InputAssemblyStateNodeSocket,
    pipeline_state.PipelineDynamicStateNodeSocket,
    pipeline_state.PipelineLayoutNodeSocket,
    pipeline_state.PipelineNodeSocket,
    pipeline_state.PipelineShaderStageNodeSocket,
    pipeline_state.PushConstantRangeNodeSocket,
    pipeline_state.RasterizationStateNodeSocket,
    pipeline_state.SamplerNodeSocket,
    pipeline_state.ShaderModuleNodeSocket,
    pipeline_state.StencilOperationStateNodeSocket,
    pipeline_state.VertexInputAttributeNodeSocket,
    pipeline_state.VertexInputBindingNodeSocket,
    pipeline_state.VertexInputStateNodeSocket,
    pipeline_state.ViewportNodeSocket,
    pipeline_state.ViewportStateNodeSocket,
    
    pipeline_state.ColorBlendAttachmentStateNode,
    pipeline_state.ColorBlendStateNode,
    pipeline_state.DepthStencilStateNode,
    pipeline_state.DescriptorSetLayoutBindingNode,
    pipeline_state.DescriptorSetLayoutNode,
    pipeline_state.DynamicStateNode,
    pipeline_state.GraphicsPipelineStateNode,
    pipeline_state.InputAssemblyStateNode,
    pipeline_state.PipelineDynamicStateNode,
    pipeline_state.PipelineLayoutNode,
    pipeline_state.PipelineShaderStageNode,
    pipeline_state.PushConstantRangeNode,
    pipeline_state.RasterizationStateNode,
    pipeline_state.SamplerNode,
    pipeline_state.ShaderModuleNode,
    pipeline_state.StencilOperationStateNode,
    pipeline_state.VertexInputAttributeNode,
    pipeline_state.VertexInputBindingNode,
    pipeline_state.VertexInputStateNode,
    pipeline_state.ViewportNode,
    pipeline_state.ViewportStateNode,

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

    resources.ImageNodeSocket,
    resources.ImageSubresourceRangeNodeSocket,

    export.MythologyExportOperator,
)

def register():

    from bpy.utils import register_class
    for cls in classes:
        register_class(cls)

    nodeitems_utils.register_node_categories('COMMON', common_node_categories)
    nodeitems_utils.register_node_categories('COMMANDS', draw_node_categories)
    nodeitems_utils.register_node_categories('RENDER_PASS', render_pass_node_categories)
    nodeitems_utils.register_node_categories('PIPELINE_STATE', pipeline_state_node_categories)

    bpy.types.NODE_MT_node.append(export.mythology_export_menu)


def unregister():

    bpy.types.NODE_MT_node.remove(export.mythology_export_menu)

    nodeitems_utils.unregister_node_categories('COMMON')
    nodeitems_utils.unregister_node_categories('COMMANDS')
    nodeitems_utils.unregister_node_categories('PIPELINE_STATE')
    nodeitems_utils.unregister_node_categories('RENDER_PASS')

    from bpy.utils import unregister_class
    for cls in reversed(classes):
        unregister_class(cls)