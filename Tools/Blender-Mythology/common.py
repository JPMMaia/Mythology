import bpy

from .render_node_tree import RenderTreeNode

def ignore_reroutes(node: bpy.types.Node) -> bpy.types.Node:

    current_node = node

    while current_node.bl_idname == "NodeReroute":
        previous_node = current_node.inputs["Input"].links[0].from_node
        current_node = previous_node

    assert current_node.bl_idname != "NodeReroute"
    return current_node


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

class Rect2DNodeSocket(bpy.types.NodeSocket):

    bl_label = "Rect 2D node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.25, 0.25, 1.0)

class Extent2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Extent 2D node'

    width_property: bpy.props.IntProperty(name="Width", default=800, min=0)
    height_property: bpy.props.IntProperty(name="Height", default=600, min=0)

    def init(self, context):
        
        self.outputs.new("Extent2DNodeSocket", "Extent 2D")

    def draw_buttons(self, context, layout):

        layout.prop(self, "width_property")
        layout.prop(self, "height_property")

class Offset2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Offset 2D node'

    x_property: bpy.props.IntProperty(name="X", default=0)
    y_property: bpy.props.IntProperty(name="Y", default=0)

    def init(self, context):
        
        self.outputs.new("Offset2DNodeSocket", "Offset 2D")

    def draw_buttons(self, context, layout):

        layout.prop(self, "x_property")
        layout.prop(self, "y_property")

class Rect2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Rect 2D node'

    def init(self, context):
        self.inputs.new("Offset2DNodeSocket", "Offset")
        self.inputs.new("Extent2DNodeSocket", "Extent")
        
        self.outputs.new("Rect2DNodeSocket", "Rect 2D")

import nodeitems_utils

class CommonNodeCategory(nodeitems_utils.NodeCategory):
    
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'RenderNodeTree'


common_node_categories = [
    CommonNodeCategory('COMMON', "Common", items=[
        nodeitems_utils.NodeItem("Extent2DNode"),
        nodeitems_utils.NodeItem("Offset2DNode"),
        nodeitems_utils.NodeItem("Rect2DNode"),
    ]),
]

import typing

JSONType = typing.Union[str, int, float, bool, None, typing.Dict[str, typing.Any], typing.List[typing.Any]]

def create_offset_2d_json(
    node: Offset2DNode
) -> JSONType:

    return {
        "x": node.get("x_property", 0),
        "y": node.get("y_property", 0),
    }

def create_extent_2d_json(
    node: Extent2DNode
) -> JSONType:

    return {
        "width": node.get("width_property", 800),
        "height": node.get("height_property", 600),
    }

def create_rect_2d_json(
    node: Rect2DNode
) -> JSONType:

    return {
        "offset": create_offset_2d_json(node.inputs["Offset"].links[0].from_node),
        "extent": create_extent_2d_json(node.inputs["Extent"].links[0].from_node),
    }