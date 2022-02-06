bl_info = {
    "name": "Mythology Addon",
    "blender": (2, 82, 0),
    "category": "Object",
}

import bpy
import nodeitems_utils

from .common import *
from .descriptors import *
from .draw import *
from .export import *
from .pipeline_state import *
from .ray_tracing import *
from .render_node_tree import *
from .render_pass import *
from .resources import *

classes = (
    render_node_tree.RenderNodeTree,
    common.DataArrayNodeSocket,
    common.Extent2DNodeSocket,
    common.Offset2DNodeSocket,
    common.Offset3DNodeSocket,
    common.Rect2DNodeSocket,
    common.ValueNodeSocket,
    common.Extent2DNode,
    common.Offset2DNode,
    common.Offset3DNode,
    common.Rect2DNode,
    common.DataArrayNode,
    common.IntValuesNode,
    common.FloatValuesNode,
    descriptors.DescriptorBufferInfoNode,
    descriptors.DescriptorBufferInfoNodeSocket,
    descriptors.DescriptorBufferInfoArrayNode,
    descriptors.DescriptorBufferInfoArrayNodeSocket,
    descriptors.DescriptorImageInfoNode,
    descriptors.DescriptorImageInfoNodeSocket,
    descriptors.DescriptorImageInfoArrayNode,
    descriptors.DescriptorImageInfoArrayNodeSocket,
    descriptors.DescriptorSetLayoutBindingNode,
    descriptors.DescriptorSetLayoutBindingNodeSocket,
    descriptors.DescriptorSetLayoutNode,
    descriptors.DescriptorSetLayoutNodeSocket,
    descriptors.DescriptorSetLayoutArrayNode,
    descriptors.DescriptorSetLayoutArrayNodeSocket,
    descriptors.DescriptorSetBindingNode,
    descriptors.DescriptorSetBindingNodeSocket,
    descriptors.DescriptorSetNode,
    descriptors.DescriptorSetNodeSocket,
    descriptors.DescriptorSetArrayNode,
    descriptors.DescriptorSetArrayNodeSocket,
    draw.BufferMemoryBarrierNodeSocket,
    draw.ClearColorValueNodeSocket,
    draw.ClearDepthStencilValueNodeSocket,
    draw.ClearSubpassNodeSocket,
    draw.ClearValueNodeSocket,
    draw.DependencyInfoNodeSocket,
    draw.DynamicOffsetNodeSocket,
    draw.DynamicOffsetArrayNodeSocket,
    draw.ExecutionNodeSocket,
    draw.FramebufferNodeSocket,
    draw.ImageBlitNodeSocket,
    draw.ImageMemoryBarrierNodeSocket,
    draw.MemoryBarrierNodeSocket,
    draw.BeginFrameNode,
    draw.BeginRenderPassNode,
    draw.BindDescriptorSetNode,
    draw.BindPipelineNode,
    draw.BlitImageNode,
    draw.BufferMemoryBarrierNode,
    draw.ClearColorValueNode,
    draw.ClearColorImageNode,
    draw.ClearDepthStencilValueNode,
    draw.ClearSubpassNode,
    draw.ClearValueNode,
    draw.DependencyInfoNode,
    draw.DynamicOffsetNode,
    draw.DynamicOffsetArrayNode,
    draw.DrawNode,
    draw.EndFrameNode,
    draw.EndRenderPassNode,
    draw.ImageBlitNode,
    draw.ImageMemoryBarrierNode,
    draw.MemoryBarrierNode,
    draw.PipelineBarrierNode,
    draw.PushConstantsNode,
    draw.SetScreenViewportAndScissorsNode,
    draw.TraceRaysNode,
    pipeline_state.ColorBlendAttachmentStateNodeSocket,
    pipeline_state.ColorBlendStateNodeSocket,
    pipeline_state.DepthStencilStateNodeSocket,
    pipeline_state.DynamicStateNodeSocket,
    pipeline_state.InputAssemblyStateNodeSocket,
    pipeline_state.PipelineDynamicStateNodeSocket,
    pipeline_state.PipelineLayoutNodeSocket,
    pipeline_state.PipelineLibraryNodeSocket,
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
    pipeline_state.DynamicStateNode,
    pipeline_state.GraphicsPipelineStateNode,
    pipeline_state.InputAssemblyStateNode,
    pipeline_state.PipelineDynamicStateNode,
    pipeline_state.PipelineLayoutNode,
    pipeline_state.PipelineLibraryNode,
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
    ray_tracing.AccelerationStructureNodeSocket,
    ray_tracing.AccelerationStructureArrayNode,
    ray_tracing.AccelerationStructureArrayNodeSocket,
    ray_tracing.RayTracingPipelineInterfaceNodeSocket,
    ray_tracing.RayTracingPipelineInterfaceNode,
    ray_tracing.RayTracingPipelineStateNode,
    ray_tracing.RayTracingShaderGroupNode,
    ray_tracing.RayTracingShaderGroupNodeSocket,
    ray_tracing.ShaderBindingTableNode,
    ray_tracing.ShaderBindingTableNodeSocket,
    ray_tracing.ShaderGroupsArrayNode,
    ray_tracing.ShaderGroupsArrayNodeSocket,
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
    resources.BufferNode,
    resources.BufferNodeSocket,
    resources.BufferViewNode,
    resources.BufferViewNodeSocket,
    resources.ComponentMappingNode,
    resources.ComponentMappingNodeSocket,
    resources.ImageNode,
    resources.ImageNodeSocket,
    resources.ImageSubresourceLayersNode,
    resources.ImageSubresourceLayersNodeSocket,
    resources.ImageSubresourceRangeNode,
    resources.ImageSubresourceRangeNodeSocket,
    resources.ImageViewNode,
    resources.ImageViewNodeSocket,
    export.MythologyExportProperties,
    export.MythologyAddonPreferences,
    export.MythologyExportOperator,
    export.MythologyExportPanel,
)


def register():

    from bpy.utils import register_class

    for cls in classes:
        register_class(cls)

    nodeitems_utils.register_node_categories("COMMON", common_node_categories)
    nodeitems_utils.register_node_categories("COMMANDS", draw_node_categories)
    nodeitems_utils.register_node_categories("DESCRIPTORS", descriptor_node_categories)
    nodeitems_utils.register_node_categories("RAY_TRACING", ray_tracing_node_categories)
    nodeitems_utils.register_node_categories("RENDER_PASS", render_pass_node_categories)
    nodeitems_utils.register_node_categories("RESOURCES", resources_node_categories)
    nodeitems_utils.register_node_categories(
        "PIPELINE_STATE", pipeline_state_node_categories
    )

    bpy.types.NODE_MT_node.append(export.mythology_export_menu)

    bpy.types.Scene.mythology_export_settings = bpy.props.PointerProperty(
        type=export.MythologyExportProperties
    )


def unregister():

    bpy.types.NODE_MT_node.remove(export.mythology_export_menu)

    nodeitems_utils.unregister_node_categories("COMMON")
    nodeitems_utils.unregister_node_categories("COMMANDS")
    nodeitems_utils.unregister_node_categories("DESCRIPTORS")
    nodeitems_utils.unregister_node_categories("PIPELINE_STATE")
    nodeitems_utils.unregister_node_categories("RAY_TRACING")
    nodeitems_utils.unregister_node_categories("RENDER_PASS")
    nodeitems_utils.unregister_node_categories("RESOURCES")

    from bpy.utils import unregister_class

    for cls in reversed(classes):
        unregister_class(cls)
