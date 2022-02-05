import bpy
import nodeitems_utils
import typing

from .common import create_extent_2d_json, find_index
from .render_node_tree import RenderTreeNode
from .vulkan_enums import (
    buffer_create_flags_values,
    buffer_usage_flags_values,
    component_swizzle_values,
    format_values,
    image_aspect_flag_bits,
    image_create_flags,
    image_layout_values,
    image_type_values,
    image_view_type_values,
    image_usage_flag_bits,
    sample_count_flag_bits,
    tiling_values,
)


class BufferNodeSocket(bpy.types.NodeSocket):

    bl_label = "Buffer node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.35, 0.35, 0.5, 1.0)


class BufferViewNodeSocket(bpy.types.NodeSocket):

    bl_label = "Buffer View node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.80, 0.2, 0.9, 1.0)


class ComponentMappingNodeSocket(bpy.types.NodeSocket):

    bl_label = "Component Mapping node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.5, 0.25, 1.0)


class ImageNodeSocket(bpy.types.NodeSocket):

    bl_label = "Image node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.0, 1.0, 1.0)


class ImageSubresourceLayersNodeSocket(bpy.types.NodeSocket):

    bl_label = "Image Subresource Layers node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.1, 0.8, 0.2, 1.0)


class ImageSubresourceRangeNodeSocket(bpy.types.NodeSocket):

    bl_label = "Image Subresource Range node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.25, 0.25, 1.0, 1.0)


class ImageViewNodeSocket(bpy.types.NodeSocket):

    bl_label = "Image View node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.6, 0.4, 0.2, 1.0)


class BufferNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Buffer"

    buffer_create_flags_property: bpy.props.EnumProperty(
        name="Create Flags",
        items=buffer_create_flags_values,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )
    size_property: bpy.props.IntProperty(name="Size", min=1, default=1)
    buffer_usage_flags_property: bpy.props.EnumProperty(
        name="Usage Flags",
        items=buffer_usage_flags_values,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )

    def init(self, context):

        self.outputs.new("BufferNodeSocket", "Buffer")

    def draw_buttons(self, context, layout):

        layout.label(text="Flags")
        layout.prop(self, "buffer_create_flags_property")
        layout.prop(self, "size_property")
        layout.label(text="Usage")
        layout.prop(self, "buffer_usage_flags_property")


class BufferViewNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Buffer View"

    format_property: bpy.props.EnumProperty(name="Format", items=format_values)
    offset_property: bpy.props.IntProperty(name="Offset", min=0, default=0)
    range_property: bpy.props.IntProperty(name="Range", min=1, default=1)

    def init(self, context):

        self.inputs.new("BufferNodeSocket", "Buffer")

        self.outputs.new("BufferViewNodeSocket", "Buffer View")

    def draw_buttons(self, context, layout):

        layout.prop(self, "format_property")
        layout.prop(self, "offset_property")
        layout.prop(self, "range_property")


class ComponentMappingNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Component Mapping"

    r_property: bpy.props.EnumProperty(name="R", items=component_swizzle_values)
    g_property: bpy.props.EnumProperty(name="G", items=component_swizzle_values)
    b_property: bpy.props.EnumProperty(name="B", items=component_swizzle_values)
    a_property: bpy.props.EnumProperty(name="A", items=component_swizzle_values)

    def init(self, context):

        self.outputs.new("ComponentMappingNodeSocket", "Components")

    def draw_buttons(self, context, layout):

        layout.prop(self, "r_property")
        layout.prop(self, "g_property")
        layout.prop(self, "b_property")
        layout.prop(self, "a_property")


class ImageNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Image"

    create_flags_property: bpy.props.EnumProperty(
        name="Create Flags",
        items=image_create_flags,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )
    type_property: bpy.props.EnumProperty(
        name="Type", items=image_type_values, default="2D"
    )
    format_property: bpy.props.EnumProperty(name="Format", items=format_values)
    mip_levels_property: bpy.props.IntProperty(name="Mip Levels", min=1, default=1)
    array_layers_property: bpy.props.IntProperty(name="Array Layers", min=1, default=1)
    sample_count_flags_property: bpy.props.EnumProperty(
        name="Sample Count", items=sample_count_flag_bits
    )
    tiling_property: bpy.props.EnumProperty(name="Tiling", items=tiling_values)
    image_usage_flags_property: bpy.props.EnumProperty(
        name="Usage Flags",
        items=image_usage_flag_bits,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )
    initial_layout_property: bpy.props.EnumProperty(
        name="Initial Layout", items=image_layout_values
    )

    def init(self, context):

        self.inputs.new("Extent2DNodeSocket", "Extent")

        self.outputs.new("ImageNodeSocket", "Image")

    def draw_buttons(self, context, layout):

        layout.label(text="Flags")
        layout.prop(self, "create_flags_property")
        layout.prop(self, "type_property")
        layout.prop(self, "format_property")
        layout.prop(self, "mip_levels_property")
        layout.prop(self, "array_layers_property")
        layout.prop(self, "sample_count_flags_property")
        layout.prop(self, "tiling_property")
        layout.prop(self, "image_usage_flags_property")
        layout.prop(self, "initial_layout_property")


class ImageSubresourceLayersNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Image Subresource Layers"

    aspect_mask_property: bpy.props.EnumProperty(
        name="Aspect Mask",
        items=image_aspect_flag_bits,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )
    mip_level_property: bpy.props.IntProperty(name="Mip Level", min=0, default=0)
    base_array_layer_property: bpy.props.IntProperty(
        name="Base Array Layer", min=0, default=0
    )
    array_layer_count_property: bpy.props.IntProperty(
        name="Array Layer Count", min=1, default=1
    )

    def init(self, context):

        self.outputs.new("ImageSubresourceLayersNodeSocket", "Image Subresource Layers")

    def draw_buttons(self, context, layout):

        layout.label(text="Aspect Mask")
        layout.prop(self, "aspect_mask_property")
        layout.prop(self, "mip_level_property")
        layout.prop(self, "base_array_layer_property")
        layout.prop(self, "array_layer_count_property")


class ImageSubresourceRangeNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Image Subresource Range"

    aspect_mask_property: bpy.props.EnumProperty(
        name="Aspect Mask",
        items=image_aspect_flag_bits,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )
    base_mip_level_property: bpy.props.IntProperty(
        name="Base Mip Level", min=0, default=0
    )
    mip_level_count_property: bpy.props.IntProperty(
        name="Mip Level Count", min=1, default=1
    )
    base_array_layer_property: bpy.props.IntProperty(
        name="Base Array Layer", min=0, default=0
    )
    array_layer_count_property: bpy.props.IntProperty(
        name="Array Layer Count", min=1, default=1
    )

    def init(self, context):

        self.outputs.new("ImageSubresourceRangeNodeSocket", "Image Subresource Range")

    def draw_buttons(self, context, layout):

        layout.label(text="Aspect Mask")
        layout.prop(self, "aspect_mask_property")
        layout.prop(self, "base_mip_level_property")
        layout.prop(self, "mip_level_count_property")
        layout.prop(self, "base_array_layer_property")
        layout.prop(self, "array_layer_count_property")


class ImageViewNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Image View"

    view_type_property: bpy.props.EnumProperty(
        name="View Type", items=image_view_type_values, default="2D"
    )
    format_property: bpy.props.EnumProperty(name="Format", items=format_values)

    def init(self, context):

        self.inputs.new("ImageNodeSocket", "Image")
        self.inputs.new("ComponentMappingNodeSocket", "Components")
        self.inputs.new("ImageSubresourceRangeNodeSocket", "Subresource Range")

        self.outputs.new("ImageViewNodeSocket", "Image View")

    def draw_buttons(self, context, layout):

        layout.prop(self, "view_type_property")
        layout.prop(self, "format_property")


class ResourcesNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == "RenderNodeTree"


resources_node_categories = [
    ResourcesNodeCategory(
        "RESOURCES",
        "Resources",
        items=[
            nodeitems_utils.NodeItem("BufferNode"),
            nodeitems_utils.NodeItem("BufferViewNode"),
            nodeitems_utils.NodeItem("ComponentMappingNode"),
            nodeitems_utils.NodeItem("ImageNode"),
            nodeitems_utils.NodeItem("ImageSubresourceLayersNode"),
            nodeitems_utils.NodeItem("ImageSubresourceRangeNode"),
            nodeitems_utils.NodeItem("ImageViewNode"),
        ],
    ),
]


def component_mapping_to_json(node: bpy.types.Node) -> typing.Any:

    return {
        "r": node.get("r_property", 0),
        "g": node.get("g_property", 0),
        "b": node.get("b_property", 0),
        "a": node.get("a_property", 0),
    }


def image_subresource_layers_to_json(node: bpy.types.Node) -> typing.Any:

    return {
        "aspect_mask": node.get("aspect_mask_property", 0),
        "mip_level": node.get("mip_level_property", 0),
        "base_array_layer": node.get("base_array_layer_property", 0),
        "array_layer_count": node.get("array_layer_count_property", 1),
    }


def image_subresource_range_to_json(node: bpy.types.Node) -> typing.Any:

    return {
        "aspect_mask": node.get("aspect_mask_property", 0),
        "base_mip_level": node.get("base_mip_level_property", 0),
        "mip_level_count": node.get("mip_level_count_property", 1),
        "base_array_layer": node.get("base_array_layer_property", 0),
        "array_layer_count": node.get("array_layer_count_property", 1),
    }


def create_buffers_json(
    nodes: typing.List[bpy.types.Node],
) -> typing.Tuple[typing.List[BufferNode], typing.Any]:

    buffer_nodes = [node for node in nodes if node.bl_idname == "BufferNode"]

    json = [
        {
            "flags": node.get("buffer_create_flags_property", 0),
            "size": node.get("size_property", 1),
            "usage": node.get("buffer_usage_flags_property", 0),
        }
        for node in buffer_nodes
    ]

    return (buffer_nodes, json)


def create_buffer_views_json(
    nodes: typing.List[bpy.types.Node], buffer_nodes: typing.List[BufferNode]
) -> typing.Tuple[typing.List[BufferViewNode], typing.Any]:

    buffer_view_nodes = [
        node
        for node in nodes
        if node.bl_idname == "BufferViewNode"
        and node.inputs["Buffer"].links[0].from_node.bl_idname == "BufferNode"
    ]

    json = [
        {
            "buffer": find_index(
                buffer_nodes,
                node.inputs["Buffer"].links[0].from_node,
            ),
            "format": node.get("format_property", 0),
            "offset": node.get("offset_property", 0),
            "range": node.get("range_property", 1),
        }
        for node in buffer_view_nodes
    ]

    return (buffer_view_nodes, json)


def create_images_json(
    nodes: typing.List[bpy.types.Node],
) -> typing.Tuple[typing.List[ImageNode], typing.Any]:

    image_nodes = [node for node in nodes if node.bl_idname == "ImageNode"]

    json = [
        {
            "flags": node.get("create_flags_property", 0),
            "type": node.get("type_property", 0),
            "format": node.get("format_property", 0),
            "mip_levels": node.get("mip_levels_property", 1),
            "array_layers": node.get("array_layers_property", 1),
            "sample_count": node.get("sample_count_flags_property", 0),
            "tiling": node.get("tiling_property", 0),
            "usage": node.get("image_usage_flags_property", 0),
            "initial_layout": node.get("initial_layout_property", 0),
            "extent": create_extent_2d_json(node.inputs["Extent"].links[0].from_node),
        }
        for node in image_nodes
    ]

    return (image_nodes, json)


def create_image_views_json(
    nodes: typing.List[bpy.types.Node],
    image_nodes: typing.List[ImageNode],
) -> typing.Tuple[typing.List[ImageViewNode], typing.Any]:

    image_view_nodes = [
        node
        for node in nodes
        if node.bl_idname == "ImageViewNode"
        and node.inputs["Image"].links[0].from_node.bl_idname == "ImageNode"
    ]

    json = [
        {
            "flags": 0,
            "image": find_index(image_nodes, node.inputs["Image"].links[0].from_node),
            "view_type": node.get("view_type_property", 0),
            "format": node.get("format_property", 0),
            "components": component_mapping_to_json(
                node.inputs["Components"].links[0].from_node
            ),
            "subresource_range": image_subresource_range_to_json(
                node.inputs["Subresource Range"].links[0].from_node
            ),
        }
        for node in image_view_nodes
    ]

    return (image_view_nodes, json)
