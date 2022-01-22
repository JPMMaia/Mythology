import bpy
import json
import pathlib

from .resources import (
    create_buffers_json,
    create_buffer_views_json,
    create_image_views_json,
    create_images_json,
)


class MythologyAddonPreferences(bpy.types.AddonPreferences):

    bl_idname = __package__

    directx_shader_compiler: bpy.props.StringProperty(
        name="DirectX Shader Compiler",
        subtype="FILE_PATH",
        default="dxc",
    )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "directx_shader_compiler")


from .compile_shader import compile_shaders
from .descriptors import (
    create_descriptor_set_layouts_json,
    create_global_descriptor_sets,
    create_frame_descriptor_sets,
)
from .draw import frame_commands_to_json
from .pipeline_state import (
    create_samplers_json,
    create_pipeline_layouts_json,
    create_pipeline_states_json,
    graphics_pipeline_state_to_json,
    shader_module_to_json,
)
from .ray_tracing import (
    create_ray_tracing_pipelines_json,
    create_shader_binding_tables_json,
)
from .render_pass import render_pass_to_json


class MythologyExportProperties(bpy.types.PropertyGroup):

    output_file: bpy.props.StringProperty(
        name="Output JSON File",
        subtype="FILE_PATH",
        default="//pipeline.json",
    )


class MythologyExportPanel(bpy.types.Panel):

    bl_label = "Export to Mythology"
    bl_idname = "OUTPUT_PT_mythology_export_panel"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "output"

    def draw(self, context):

        export_settings = context.scene.mythology_export_settings

        layout = self.layout
        layout.prop(export_settings, "output_file")

        layout.operator("mythology.export_json")


def write_json_to_file(output_json, output_file: pathlib.PurePath) -> None:

    with open(str(output_file), "w") as output_stream:
        json.dump(output_json, output_stream, indent=4)


class MythologyExportOperator(bpy.types.Operator):

    bl_idname = "mythology.export_json"
    bl_label = "Export JSON"

    def execute(self, context):

        preferences = context.preferences
        addon_preferences = preferences.addons[__package__].preferences
        export_settings = context.scene.mythology_export_settings

        shader_compiler = pathlib.PurePath(addon_preferences.directx_shader_compiler)
        output_json_file = pathlib.PurePath(
            bpy.path.abspath(export_settings.output_file)
        )
        output_json_directory = output_json_file.parents[0]

        nodes = [
            node for node_group in bpy.data.node_groups for node in node_group.nodes
        ]

        if not compile_shaders(shader_compiler, output_json_directory, nodes):
            return {"FINISHED"}

        render_passes = render_pass_to_json(nodes)
        shader_modules = shader_module_to_json(nodes)
        buffers = create_buffers_json(nodes)
        buffer_views = create_buffer_views_json(nodes, buffers[0])
        images = create_images_json(nodes)
        image_views = create_image_views_json(nodes, images[0])
        samplers = create_samplers_json(nodes)
        descriptor_set_layouts = create_descriptor_set_layouts_json(nodes, samplers[0])
        descriptor_sets = create_global_descriptor_sets(
            nodes, descriptor_set_layouts[0], buffers[0], image_views[0], samplers[0]
        )
        frame_descriptor_sets = create_frame_descriptor_sets(
            nodes, descriptor_set_layouts[0], buffers[0], image_views[0], samplers[0]
        )
        pipeline_layouts = create_pipeline_layouts_json(
            nodes, descriptor_set_layouts[0]
        )
        compute_pipeline_states = ([], [])
        graphics_pipeline_states = graphics_pipeline_state_to_json(
            nodes, render_passes, shader_modules[0], pipeline_layouts
        )
        ray_tracing_pipeline_states = create_ray_tracing_pipelines_json(
            nodes, shader_modules[0], pipeline_layouts[0]
        )
        pipeline_states = create_pipeline_states_json(
            compute_pipeline_states[0],
            graphics_pipeline_states[0],
            ray_tracing_pipeline_states[0],
        )
        shader_binding_tables = create_shader_binding_tables_json(
            nodes, pipeline_states[0]
        )
        frame_commands = frame_commands_to_json(
            nodes,
            descriptor_sets[0],
            frame_descriptor_sets[0],
            pipeline_layouts[0],
            pipeline_states[0],
            render_passes,
            shader_binding_tables[0],
        )

        output_json = {
            "render_passes": render_passes[2],
            "shader_modules": shader_modules[1],
            "buffers": buffers[1],
            "buffer_views": buffer_views[1],
            "images": images[1],
            "image_views": image_views[1],
            "samplers": samplers[1],
            "descriptor_set_layouts": descriptor_set_layouts[1],
            "descriptor_sets": descriptor_sets[1],
            "pipeline_layouts": pipeline_layouts[1],
            "pipeline_states": {
                "compute_pipeline_states": compute_pipeline_states[1],
                "graphics_pipeline_states": graphics_pipeline_states[1],
                "ray_tracing_pipeline_states": ray_tracing_pipeline_states[1],
                "pipeline_states": pipeline_states[1],
            },
            "shader_binding_tables": shader_binding_tables[1],
            "frame_resources": {"descriptor_sets": frame_descriptor_sets[1]},
            "frame_commands": frame_commands,
        }

        write_json_to_file(output_json, output_json_file)

        return {"FINISHED"}


def mythology_export_menu(self, context):

    self.layout.operator("mythology.export_json")
