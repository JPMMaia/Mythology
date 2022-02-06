import bpy
import typing

from .array_inputs import recreate_dynamic_inputs, update_dynamic_inputs
from .render_node_tree import RenderTreeNode


def ignore_reroutes(node: bpy.types.Node) -> bpy.types.Node:

    current_node = node

    while current_node.bl_idname == "NodeReroute":
        previous_node = current_node.inputs["Input"].links[0].from_node
        current_node = previous_node

    assert current_node.bl_idname != "NodeReroute"
    return current_node


def get_enum_property_int_value(
    enum_values: typing.Tuple[typing.Any, typing.Any, typing.Any, int],
    actual_value: typing.Any,
    default_value: int,
) -> int:

    return next(
        (enum[3] for enum in enum_values if enum[0] == actual_value), default_value
    )


def find_index(list, value) -> int:

    return next(
        index for index, current_value in enumerate(list) if current_value == value
    )


def get_input_node(node: bpy.types.Node, input_name: str, index: int) -> bpy.types.Node:

    return node.inputs[input_name].links[index].from_node


def create_index_json(global_list, frame_list, value) -> typing.Any:

    if value in global_list:
        return {"type": "pipeline_resource", "index": global_list.index(value)}
    elif value in frame_list:
        return {"type": "frame_resource", "index": frame_list.index(value)}
    else:
        raise ValueError("Value was not found in either list.")


class DataArrayNodeSocket(bpy.types.NodeSocket):

    bl_label = "Data Array node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.5, 0.5, 0.5, 1.0)


class Extent2DNodeSocket(bpy.types.NodeSocket):

    bl_label = "Extent 2D node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.75, 0.75, 0.75, 1.0)


class Offset2DNodeSocket(bpy.types.NodeSocket):

    bl_label = "Offset 2D node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.25, 0.25, 0.25, 1.0)


class Offset3DNodeSocket(bpy.types.NodeSocket):

    bl_label = "Offset 3D node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.75, 0.75, 0.75, 1.0)


class Rect2DNodeSocket(bpy.types.NodeSocket):

    bl_label = "Rect 2D node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.25, 0.25, 1.0)


class ValueNodeSocket(bpy.types.NodeSocket):

    bl_label = "Value node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 1.0, 1.0, 1.0)


class Extent2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Extent 2D node"

    width_property: bpy.props.IntProperty(name="Width", default=800, min=1)
    height_property: bpy.props.IntProperty(name="Height", default=600, min=1)

    def init(self, context):

        self.outputs.new("Extent2DNodeSocket", "Extent 2D")

    def draw_buttons(self, context, layout):

        layout.prop(self, "width_property")
        layout.prop(self, "height_property")


class Offset2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Offset 2D node"

    x_property: bpy.props.IntProperty(name="X", default=0, min=0)
    y_property: bpy.props.IntProperty(name="Y", default=0, min=0)

    def init(self, context):

        self.outputs.new("Offset2DNodeSocket", "Offset 2D")

    def draw_buttons(self, context, layout):

        layout.prop(self, "x_property")
        layout.prop(self, "y_property")


class Offset3DNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Offset 3D node"

    x_property: bpy.props.IntProperty(name="X", default=0, min=0)
    y_property: bpy.props.IntProperty(name="Y", default=0, min=0)
    z_property: bpy.props.IntProperty(name="Z", default=0, min=0)

    def init(self, context):

        self.outputs.new("Offset3DNodeSocket", "Offset 3D")

    def draw_buttons(self, context, layout):

        layout.prop(self, "x_property")
        layout.prop(self, "y_property")
        layout.prop(self, "z_property")


class Rect2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Rect 2D node"

    def init(self, context):
        self.inputs.new("Offset2DNodeSocket", "Offset")
        self.inputs.new("Extent2DNodeSocket", "Extent")

        self.outputs.new("Rect2DNodeSocket", "Rect 2D")


class DataArrayNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Data Array"
    recreating = False

    def init(self, context):
        self.recreating = True
        recreate_dynamic_inputs(self.id_data, self.inputs, "ValueNodeSocket")
        self.recreating = False

        self.outputs.new("DataArrayNodeSocket", "Data array")

    def update(self):
        if not self.recreating:
            self.recreating = True
            update_dynamic_inputs(self.id_data, self.inputs, "ValueNodeSocket")
            self.recreating = False


class IntValuesNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Int Values"

    data_type = "int32"
    data_size = 4
    count_property: bpy.props.IntProperty(name="Count", min=1, default=4)
    value_property: bpy.props.FloatVectorProperty(name="Value", size=4)

    def init(self, context):
        self.outputs.new("ValueNodeSocket", "Values")

    def draw_buttons(self, context, layout):

        layout.prop(self, "count_property")
        layout.prop(self, "value_property")


class FloatValuesNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Float Values"

    data_type = "float32"
    data_size = 4
    count_property: bpy.props.IntProperty(name="Count", min=1, default=4)
    value_property: bpy.props.FloatVectorProperty(name="Value", size=4)

    def init(self, context):
        self.outputs.new("ValueNodeSocket", "Values")

    def draw_buttons(self, context, layout):

        layout.prop(self, "count_property")
        layout.prop(self, "value_property")


import nodeitems_utils


class CommonNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == "RenderNodeTree"


common_node_categories = [
    CommonNodeCategory(
        "COMMON",
        "Common",
        items=[
            nodeitems_utils.NodeItem("Extent2DNode"),
            nodeitems_utils.NodeItem("Offset2DNode"),
            nodeitems_utils.NodeItem("Offset3DNode"),
            nodeitems_utils.NodeItem("Rect2DNode"),
            nodeitems_utils.NodeItem("DataArrayNode"),
            nodeitems_utils.NodeItem("IntValuesNode"),
            nodeitems_utils.NodeItem("FloatValuesNode"),
        ],
    ),
]

import typing

JSONType = typing.Union[
    str, int, float, bool, None, typing.Dict[str, typing.Any], typing.List[typing.Any]
]


def create_offset_2d_json(node: Offset2DNode) -> JSONType:

    return {
        "x": node.x_property,
        "y": node.y_property,
    }


def create_offset_3d_json(node: Offset3DNode) -> JSONType:

    return {
        "x": node.x_property,
        "y": node.y_property,
        "z": node.z_property,
    }


def create_extent_2d_json(node: Extent2DNode) -> JSONType:

    return {
        "width": node.get("width_property", 800),
        "height": node.get("height_property", 600),
    }


def create_rect_2d_json(node: Rect2DNode) -> JSONType:

    return {
        "offset": create_offset_2d_json(node.inputs["Offset"].links[0].from_node),
        "extent": create_extent_2d_json(node.inputs["Extent"].links[0].from_node),
    }


def calculate_data_array_size_in_bytes(
    values_nodes: typing.List[bpy.types.Node],
) -> int:

    sizes = [
        values_node.data_size * values_node.count_property
        for values_node in values_nodes
    ]

    return sum(sizes)


def create_data_array_json(node: DataArrayNode) -> JSONType:

    values_nodes = [
        input.links[0].from_node for input in node.inputs if len(input.links) == 1
    ]

    return {
        "size_in_bytes": calculate_data_array_size_in_bytes(values_nodes),
        "data": [
            {
                "type": values_node.data_type,
                "values": [
                    values_node.value_property[value_index]
                    for value_index in range(0, values_node.count_property)
                ],
            }
            for values_node in values_nodes
        ],
    }


def create_data_arrays_json(
    nodes: typing.List[bpy.types.Node],
) -> typing.Tuple[typing.List[DataArrayNode], JSONType]:

    data_array_nodes = [node for node in nodes if node.bl_idname == "DataArrayNode"]

    json = [create_data_array_json(node) for node in data_array_nodes]

    return [data_array_nodes, json]
