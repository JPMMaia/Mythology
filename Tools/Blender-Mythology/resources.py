import bpy


class ImageNodeSocket(bpy.types.NodeSocket):

    bl_label = "Image node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.0, 1.0, 1.0)


class ImageSubresourceRangeNodeSocket(bpy.types.NodeSocket):

    bl_label = "Image Subresource Range node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.25, 0.25, 1.0, 1.0)
