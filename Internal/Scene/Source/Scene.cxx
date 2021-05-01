module;

#include <array>
#include <cstddef>
#include <memory_resource>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module maia.scene;

namespace Maia::Scene
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
		Component_type component_type{Component_type::Byte};
		std::size_t count{0};
		Type type{Type::Scalar};
		std::optional<Vector3f> max;
		std::optional<Vector3f> min;
	};

	std::uint8_t size_of(Accessor::Type accessor_type) noexcept;


	export struct Buffer
	{
		std::optional<std::pmr::string> uri;
		std::size_t byte_length{0};
	};

	export struct Buffer_view
	{
		Index buffer_index{0};
		std::size_t byte_offset{0};
		std::size_t byte_length{0};
	};

	export struct Pbr_metallic_roughness
	{
		Vector4f base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};
		float metallic_factor{1.0f};
		float roughness_factor{1.0f};
	};

	export struct Material
	{
		std::optional<std::pmr::string> name;
		Pbr_metallic_roughness pbr_metallic_roughness;
		Vector3f emissive_factor{0.0f, 0.0f, 0.0f};
		std::pmr::string alpha_mode{"OPAQUE"};
		float alpha_cutoff{0.5f};
		bool double_sided{false};
	};

	export struct Primitive
	{
		std::pmr::unordered_map<std::pmr::string, Index> attributes;
		std::optional<Index> indices_index;
		std::optional<Index> material_index;
	};

	export struct Mesh
	{
		std::pmr::vector<Primitive> primitives;
		std::optional<std::pmr::string> name;
	};

	export struct Camera
	{
		enum class Type
		{
			Orthographic,
			Perspective
		};

		struct Orthographic
		{
			float horizontal_magnification{0.0f};
			float vertical_magnification{0.0f};
			float near_z{0.0f};
			float far_z{0.0f};
		};

		struct Perspective
		{
			std::optional<float> aspect_ratio;
			float vertical_field_of_view{0.0f};
			float near_z{0.0f};
			std::optional<float> far_z;
		};

		Type type{Type::Orthographic};
		std::optional<std::pmr::string> name;
		std::variant<Orthographic, Perspective> projection{Orthographic{}};
	};

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

	export struct Scene
	{
		std::optional<std::pmr::string> name;
		std::optional<std::pmr::vector<Index>> nodes;
	};

	export struct World
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
}
