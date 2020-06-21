import bpy
import pathlib
import subprocess

def compile_shaders(
    compiler: pathlib.PurePath,
    relative_path: pathlib.PurePath,
    nodes: bpy.types.Node
) -> bool:

    shader_module_nodes = [node
                           for node in nodes
                           if node.bl_idname == "ShaderModuleNode"]

    assert all(node.language_property == "HLSL" for node in shader_module_nodes)

    targets = ["%s_%s" % (node.shader_type_property.lower(), node.shader_model_property)
               for node in shader_module_nodes]

    compile_commands = [
        [
            str(compiler),
            "-spirv",
            "-T", target,
            "-E", node.entry_point_property,
            node.additional_compile_flags_property,
            bpy.path.abspath(node.input_shader_file_property),
            "-Fo", str(relative_path / node.output_shader_filename_property)
        ]
        for node, target in zip(shader_module_nodes, targets)
    ]

    for command, node in zip(compile_commands, shader_module_nodes):
        print("Compiling shader %s to %s: %s" % (node.input_shader_file_property, node.output_shader_filename_property, command))
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, error = process.communicate()

        if process.returncode != 0:
            print("Shader compilation failed with exit code %d: %s %s %s" % (process.returncode, command, output, error))
            return False

    return True
