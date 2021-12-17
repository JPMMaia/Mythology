import bpy
import nodeitems_utils
import typing

from .array_inputs import recreate_dynamic_inputs, update_dynamic_inputs
from .render_node_tree import RenderTreeNode
from .vulkan_enums import (
    descriptor_type_values,
    get_descriptor_type_value,
    shader_stage_flag_values,
)


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
            nodeitems_utils.NodeItem("DescriptorSetLayoutBindingNode"),
            nodeitems_utils.NodeItem("DescriptorSetLayoutNode"),
            nodeitems_utils.NodeItem("DescriptorSetLayoutArrayNode"),
        ],
    ),
]
