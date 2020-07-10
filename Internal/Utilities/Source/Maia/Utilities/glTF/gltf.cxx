export module maia.utilities.gltf;

import <array>;
import <cstddef>;
import <filesystem>;
import <memory_resource>;
import <optional>;
import <string>;
import <unordered_map>;
import <variant>;
import <vector>;

import <nlohmann/json.hpp>;

namespace Maia::Utilities::glTF
{
	export using Index = std::size_t;

	export enum class Component_type
	{
		Byte = 5120,
		Unsigned_byte = 5121,
		Short = 5122,
		Unsigned_short = 5123,
		Unsigned_int = 5125,
		Float = 5126
	};
	
	std::uint8_t size_of(Component_type component_type) noexcept;

	export struct Vector3f
	{
		float x{0.0f};
		float y{0.0f};
		float z{0.0f};
	};

	export struct Vector4f
	{
		float x{0.0f};
		float y{0.0f};
		float z{0.0f};
		float w{0.0f};
	};

	export struct Quaternionf
	{
		float x{0.0f};
		float y{0.0f};
		float z{0.0f};
		float w{1.0f};
	};

	export struct Matrix4f
	{
		std::array<float, 16> values;
	};

	export struct Accessor
	{
		enum class Type
		{
			Scalar,
			Vector2,
			Vector3,
			Vector4,
			Matrix2x2,
			Matrix3x3,
			Matrix4x4
		};

		std::optional<Index> buffer_view_index;
		Component_type component_type;
		std::size_t count;
		Type type;
		std::optional<Vector3f> max;
		std::optional<Vector3f> min;
	};

	Accessor accessor_from_json(nlohmann::json const& json) noexcept;
	void to_json(nlohmann::json& json, Accessor const& value) noexcept;
	std::uint8_t size_of(Accessor::Type accessor_type) noexcept;


	export struct Buffer
	{
		std::optional<std::pmr::string> uri;
		std::size_t byte_length;
	};

	Buffer buffer_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept;
	void to_json(nlohmann::json& json, Buffer const& value) noexcept;

	export std::pmr::vector<std::byte> read_buffer_data(
		Buffer const& buffer,
		std::filesystem::path const& prefix_path,
		std::pmr::polymorphic_allocator<> const& allocator
	) noexcept;


	export struct Buffer_view
	{
		Index buffer_index;
		std::size_t byte_offset{0};
		std::size_t byte_length;
	};

	Buffer_view buffer_view_from_json(nlohmann::json const& json) noexcept;
	void to_json(nlohmann::json& json, Buffer_view const& value) noexcept;


	export struct Pbr_metallic_roughness
	{
		Vector4f base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};
		float metallic_factor{1.0f};
		float roughness_factor{1.0f};
	};

	Pbr_metallic_roughness pbr_metallic_roughness_from_json(nlohmann::json const& json) noexcept;
	void to_json(nlohmann::json& json, Pbr_metallic_roughness const& value) noexcept;


	export struct Material
	{
		std::optional<std::pmr::string> name;
		Pbr_metallic_roughness pbr_metallic_roughness{};
		Vector3f emissive_factor{0.0f, 0.0f, 0.0f};
		std::pmr::string alpha_mode{"OPAQUE"};
		float alpha_cutoff{0.5f};
		bool double_sided{false};
	};

	Material material_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept;
	void to_json(nlohmann::json& json, Material const& value) noexcept;


	export struct Primitive
	{
		std::pmr::unordered_map<std::pmr::string, Index> attributes;
		std::optional<Index> indices_index;
		std::optional<Index> material_index;
	};

	Primitive primitive_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept;
	void to_json(nlohmann::json& json, Primitive const& value) noexcept;


	export struct Mesh
	{
		std::pmr::vector<Primitive> primitives;
		std::optional<std::pmr::string> name;
	};

	Mesh mesh_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept;
	void to_json(nlohmann::json& json, Mesh const& value) noexcept;


	export struct Camera
	{
		enum class Type
		{
			Orthographic,
			Perspective
		};

		struct Orthographic
		{
			float horizontal_magnification;
			float vertical_magnification;
			float near_z;
			float far_z;
		};

		struct Perspective
		{
			std::optional<float> aspect_ratio;
			float vertical_field_of_view;
			float near_z;
			std::optional<float> far_z;
		};

		Type type;
		std::optional<std::pmr::string> name;
		std::variant<Orthographic, Perspective> projection;
	};

	Camera camera_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept;
	void to_json(nlohmann::json& json, Camera const& value) noexcept;


	export struct Node
	{
		std::optional<std::pmr::string> name;

		std::optional<Index> mesh_index;
		std::optional<Index> camera_index;
		std::pmr::vector<Index> child_indices;

		Quaternionf rotation{0.0f, 0.0f, 0.0f, 1.0f};
		Vector3f scale{1.0f, 1.0f, 1.0f};
		Vector3f translation{0.0f, 0.0f, 0.0f};
	};

	Node node_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept;
	void to_json(nlohmann::json& json, Node const& value) noexcept;


	export struct Scene
	{
		std::optional<std::pmr::string> name;
		std::optional<std::pmr::vector<Index>> nodes;
	};

	Scene scene_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept;
	void to_json(nlohmann::json& json, Scene const& value) noexcept;


	export struct Gltf
	{
		std::pmr::vector<Accessor> accessors;
		std::pmr::vector<Buffer> buffers;
		std::pmr::vector<Buffer_view> buffer_views;
		std::pmr::vector<Camera> cameras;
		std::pmr::vector<Material> materials;
		std::pmr::vector<Mesh> meshes;
		std::pmr::vector<Node> nodes;
		std::optional<Index> scene_index;
		std::pmr::vector<Scene> scenes;
	};

	export Gltf gltf_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept;
	void to_json(nlohmann::json& json, Gltf const& value) noexcept;

}
