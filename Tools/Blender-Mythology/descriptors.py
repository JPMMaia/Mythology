import bpy
import nodeitems_utils
import typing

from .array_inputs import recreate_dynamic_inputs, update_dynamic_inputs
from .render_node_tree import RenderTreeNode
from .vulkan_enums import (
    descriptor_type_values,
    get_descriptor_type_value,
    image_layout_values,
    shader_stage_flag_values,
)


class DescriptorBufferInfoNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Buffer Info node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.6, 0.6, 0.2, 1.0)


class DescriptorBufferInfoArrayNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Buffer Info Array node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.2, 0.2, 0.6, 1.0)


class DescriptorImageInfoNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Image Info node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.2, 0.6, 0.6, 1.0)


class DescriptorImageInfoArrayNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Image Info Array node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.6, 0.2, 0.6, 1.0)


class DescriptorSetLayoutBindingNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Set Layout Binding node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.2, 0.4, 1.0)


class DescriptorSetLayoutNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Set Layout node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.2, 0.4, 0.6, 1.0)


class DescriptorSetLayoutArrayNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Set Layout Array node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.6, 0.4, 0.2, 1.0)


class DescriptorSetBindingNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Set Binding node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.9, 0.4, 1.0)


class DescriptorSetNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Set node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.4, 1.0, 1.0)


class DescriptorSetArrayNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Set Array node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.4, 0.2, 1.0)


class DescriptorSetLayoutBindingNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Set Layout Binding"

    binding_property: bpy.props.IntProperty(name="Binding", min=0)
    descriptor_type_property: bpy.props.EnumProperty(
        name="Descriptor Type", items=descriptor_type_values
    )
    descriptor_count_property: bpy.props.IntProperty(name="Descriptor Count", min=1)
    stage_flags_property: bpy.props.EnumProperty(
        name="Stage Flags",
        items=shader_stage_flag_values,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )

    def init(self, context):

        self.inputs.new("SamplerNodeSocket", "Immutable Samplers")

        self.outputs.new(
            "DescriptorSetLayoutBindingNodeSocket", "Descriptor Set Layout Binding"
        )

    def draw_buttons(self, context, layout):

        layout.prop(self, "binding_property")
        layout.prop(self, "descriptor_type_property")
        layout.prop(self, "descriptor_count_property")
        layout.prop(self, "stage_flags_property")


class DescriptorSetLayoutNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Set Layout"

    def init(self, context):

        self.inputs.new("DescriptorSetLayoutBindingNodeSocket", "Bindings")
        self.inputs["Bindings"].link_limit = 0

        self.outputs.new("DescriptorSetLayoutNodeSocket", "Descriptor Set Layout")


class DescriptorSetLayoutArrayNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Set Layouts Array"
    recreating = False

    def init(self, context):
        self.recreating = True
        recreate_dynamic_inputs(
            self.id_data, self.inputs, "DescriptorSetLayoutNodeSocket"
        )
        self.recreating = False

        self.outputs.new(
            "DescriptorSetLayoutArrayNodeSocket", "Descriptor Set Layouts Array"
        )

    def update(self):
        if not self.recreating:
            self.recreating = True
            update_dynamic_inputs(
                self.id_data, self.inputs, "DescriptorSetLayoutNodeSocket"
            )
            self.recreating = False


class DescriptorBufferInfoNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Buffer Info"

    offset_property: bpy.props.IntProperty(name="Offset", min=0, default=0)
    range_property: bpy.props.IntProperty(name="Range", min=1, default=1)

    def init(self, context):

        self.inputs.new("BufferNodeSocket", "Buffer")

        self.outputs.new("DescriptorBufferInfoNodeSocket", "Descriptor Buffer Info")


class DescriptorBufferInfoArrayNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Buffer Info Array"
    recreating = False

    def init(self, context):
        self.recreating = True
        recreate_dynamic_inputs(
            self.id_data, self.inputs, "DescriptorBufferInfoNodeSocket"
        )
        self.recreating = False

        self.outputs.new(
            "DescriptorBufferInfoArrayNodeSocket", "Descriptor Buffer Info Array"
        )

    def update(self):
        if not self.recreating:
            self.recreating = True
            update_dynamic_inputs(
                self.id_data, self.inputs, "DescriptorBufferInfoNodeSocket"
            )
            self.recreating = False


class DescriptorImageInfoNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Image Info"

    image_layout_property: bpy.props.EnumProperty(
        name="Image Layout", items=image_layout_values
    )

    def init(self, context):

        self.inputs.new("SamplerNodeSocket", "Sampler")

        self.inputs.new("ImageViewNodeSocket", "Image View")

        self.outputs.new("DescriptorImageInfoNodeSocket", "Descriptor Image Info")


class DescriptorImageInfoArrayNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Image Info Array"
    recreating = False

    def init(self, context):
        self.recreating = True
        recreate_dynamic_inputs(
            self.id_data, self.inputs, "DescriptorImageInfoNodeSocket"
        )
        self.recreating = False

        self.outputs.new(
            "DescriptorImageInfoArrayNodeSocket", "Descriptor Image Info Array"
        )

    def update(self):
        if not self.recreating:
            self.recreating = True
            update_dynamic_inputs(
                self.id_data, self.inputs, "DescriptorImageInfoNodeSocket"
            )
            self.recreating = False


class DescriptorSetBindingNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Set  Binding"

    binding_property: bpy.props.IntProperty(name="Binding", min=0)
    descriptor_type_property: bpy.props.EnumProperty(
        name="Descriptor Type", items=descriptor_type_values
    )
    first_array_element_property: bpy.props.IntProperty(
        name="First Array Element", min=0
    )

    def init(self, context):

        self.inputs.new(
            "DescriptorBufferInfoArrayNodeSocket", "Descriptor Buffer Info Array"
        )
        self.inputs.new(
            "DescriptorImageInfoArrayNodeSocket", "Descriptor Image Info Array"
        )
        self.inputs.new(
            "AccelerationStructureArrayNodeSocket", "Acceleration Structure Array"
        )

        self.outputs.new("DescriptorSetBindingNodeSocket", "Descriptor Set Binding")

    def draw_buttons(self, context, layout):

        layout.prop(self, "binding_property")
        layout.prop(self, "descriptor_type_property")
        layout.prop(self, "first_array_element_property")


class DescriptorSetNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Set"

    def init(self, context):

        self.inputs.new("DescriptorSetLayoutNodeSocket", "Descriptor Set Layout")

        self.inputs.new("DescriptorSetBindingNodeSocket", "Descriptor Set Bindings")
        self.inputs["Descriptor Set Bindings"].link_limit = 0

        self.outputs.new("DescriptorSetNodeSocket", "Descriptor Set")


class DescriptorSetArrayNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Descriptor Set Array"
    recreating = False

    def init(self, context):
        self.recreating = True
        recreate_dynamic_inputs(self.id_data, self.inputs, "DescriptorSetNodeSocket")
        self.recreating = False

        self.outputs.new("DescriptorSetArrayNodeSocket", "Descriptor Set Array")

    def update(self):
        if not self.recreating:
            self.recreating = True
            update_dynamic_inputs(self.id_data, self.inputs, "DescriptorSetNodeSocket")
            self.recreating = False


def create_descriptor_set_layouts_json(
    nodes: typing.List[bpy.types.Node],
    samplers: typing.List[bpy.types.Node],
) -> typing.Tuple[typing.List[DescriptorSetLayoutNode], typing.Any]:

    descriptor_set_layout_nodes = [
        node for node in nodes if node.bl_idname == "DescriptorSetLayoutNode"
    ]

    json = [
        {
            "bindings": [
                {
                    "binding": link.from_node.get("binding_property", 0),
                    "descriptor_type": get_descriptor_type_value(
                        link.from_node.get("descriptor_type_property", 0)
                    ),
                    "descriptor_count": link.from_node.get(
                        "descriptor_count_property", 1
                    ),
                    "stage_flags_property": link.from_node.get(
                        "stage_flags_property_property", 0
                    ),
                    "immutable_samplers": [
                        samplers.index(sampler_link.from_node)
                        for sampler_link in link.from_node.inputs[
                            "Immutable Samplers"
                        ].links
                    ],
                }
                for link in node.inputs["Bindings"].links
            ],
        }
        for node in descriptor_set_layout_nodes
    ]

    return (descriptor_set_layout_nodes, json)


class DescriptorsNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == "RenderNodeTree"


descriptor_node_categories = [
    DescriptorsNodeCategory(
        "DESCRIPTORS",
        "Descriptors",
        items=[
            nodeitems_utils.NodeItem("DescriptorBufferInfoNode"),
            nodeitems_utils.NodeItem("DescriptorBufferInfoArrayNode"),
            nodeitems_utils.NodeItem("DescriptorImageInfoNode"),
            nodeitems_utils.NodeItem("DescriptorImageInfoArrayNode"),
            nodeitems_utils.NodeItem("DescriptorSetLayoutBindingNode"),
            nodeitems_utils.NodeItem("DescriptorSetLayoutNode"),
            nodeitems_utils.NodeItem("DescriptorSetLayoutArrayNode"),
            nodeitems_utils.NodeItem("DescriptorSetBindingNode"),
            nodeitems_utils.NodeItem("DescriptorSetNode"),
            nodeitems_utils.NodeItem("DescriptorSetArrayNode"),
        ],
    ),
]
