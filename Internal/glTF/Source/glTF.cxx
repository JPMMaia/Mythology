module;

#include <cstddef>
#include <filesystem>
#include <memory_resource>
#include <vector>

#include <nlohmann/json.hpp>

export module maia.glTF;

import maia.scene;

namespace Maia::glTF
{
	/**
	 * @brief Create an accessor from json.
	 * 
	 * @param json A JSON object that describes an accessor.
	 * @param allocator Allocator for the dynamic memory required for creating the accessor.
	 * @return The converted accessor.
	 */
	export Maia::Scene::Accessor accessor_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Create a buffer from json.
	 * 
	 * @param json A JSON object that describes a buffer.
	 * @param allocator Allocator for the dynamic memory required for creating the buffer.
	 * @return The converted buffer.
	 */
	export Maia::Scene::Buffer buffer_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Create a buffer view from json.
	 * 
	 * @param json A JSON object that describes a buffer view.
	 * @param allocator Allocator for the dynamic memory required for creating the buffer view.
	 * @return The converted buffer view.
	 */
	export Maia::Scene::Buffer_view buffer_view_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Read the PBR material parameters from json.
	 * 
	 * @param json A JSON object that describes the PBR material parameters.
	 * @return The converted PBR metallic roughness parameters.
	 */
	export Maia::Scene::Pbr_metallic_roughness pbr_metallic_roughness_from_json(
		nlohmann::json const& json
	);

	/**
	 * @brief Create a material from json.
	 * 
	 * @param json A JSON object that describes a material.
	 * @param allocator Allocator for the dynamic memory required for creating the material.
	 * @return The converted material.
	 */
	export Maia::Scene::Material material_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Create a primitive from json.
	 * 
	 * @param json A JSON object that describes a primitive.
	 * @param allocator Allocator for the dynamic memory required for creating the primitive.
	 * @return The converted primitive.
	 */
	export Maia::Scene::Primitive primitive_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Create a mesh from json.
	 * 
	 * @param json A JSON object that describes a mesh.
	 * @param allocator Allocator for the dynamic memory required for creating the mesh.
	 * @return The converted mesh.
	 */
	export Maia::Scene::Mesh mesh_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Create a camera from json.
	 * 
	 * @param json A JSON object that describes a camera.
	 * @param allocator Allocator for the dynamic memory required for creating the camera.
	 * @return The converted camera.
	 */
	export Maia::Scene::Camera camera_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Create a node from json.
	 * 
	 * @param json A JSON object that describes a node.
	 * @param allocator Allocator for the dynamic memory required for creating the node.
	 * @return The converted node.
	 */
	export Maia::Scene::Node node_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Create a scene from json.
	 * 
	 * @param json A JSON object that describes a scene.
	 * @param allocator Allocator for the dynamic memory required for creating the scene.
	 * @return The converted scene.
	 */
	export Maia::Scene::Scene scene_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);

	/**
	 * @brief Create a world object from json.
	 * 
	 * @param json A JSON object that describes the GLTF scene.
	 * @param allocator Allocator for the dynamic memory required for creating the world object.
	 * @return The converted world object.
	 */
	export Maia::Scene::World gltf_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	);
}
