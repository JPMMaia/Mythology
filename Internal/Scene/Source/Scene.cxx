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

	/**
	 * @brief The datatype of components.
	 * 
	 */
	export enum class Component_type
	{
		Byte = 5120,
		Unsigned_byte = 5121,
		Short = 5122,
		Unsigned_short = 5123,
		Unsigned_int = 5125,
		Float = 5126
	};
	
	/**
	 * @brief Return the size in bytes of the given component type.
	 * 
	 * @param component_type A component type.
	 * @return The size in bytes of \p component_type.
	 */
	export std::uint8_t size_of(Component_type component_type) noexcept;


	/**
	 * @brief Vector of 3 float components.
	 * 
	 */
	export struct Vector3f
	{
		float x{0.0f};
		float y{0.0f};
		float z{0.0f};

		friend auto operator<=>(Vector3f const&, Vector3f const&) noexcept = default;
	};

	/**
	 * @brief A vector of 4 float components.
	 * 
	 */
	export struct Vector4f
	{
		float x{0.0f};
		float y{0.0f};
		float z{0.0f};
		float w{0.0f};

		friend auto operator<=>(Vector4f const&, Vector4f const&) noexcept = default;
	};

	/**
	 * @brief A quaternion that can be used to represent 3D orientations and rotations.
	 * 
	 */
	export struct Quaternionf
	{
		float x{0.0f};
		float y{0.0f};
		float z{0.0f};
		float w{1.0f};

		friend auto operator<=>(Quaternionf const&, Quaternionf const&) noexcept = default;
	};

	/**
	 * @brief A 4x4 matrix of floats.
	 * 
	 */
	export struct Matrix4f
	{
		std::array<float, 16> values;

		friend auto operator<=>(Matrix4f const&, Matrix4f const&) noexcept = default;
	};

	/**
	 * @brief A typed view into a buffer view.
	 * 
	 */
	export struct Accessor
	{
		/**
		 * @brief The datatype of acessors.
		 * 
		 */
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

		/**
		 * @brief The index of the buffer view.
		 * 
		 */
		std::optional<Index> buffer_view_index;
		
		/**
		 * @brief The offset relative to the start of the buffer view in bytes.
		 * 
		 */
		std::size_t byte_offset{0};

		/**
		 * @brief Specifies whether integer data should be normalized into a certain range when converted into a floating-point type.
		 * 
		 * Unsigned types should be normalized to [0, 1].
		 * Signed types should be normalized to [-1, 1].
		 * 
		 */
		bool normalized{false};

		/**
		 * @brief The datatype of components in the attribute.
		 * 
		 */
		Component_type component_type{Component_type::Byte};

		/**
		 * @brief The number of attributes referenced by the accessor.
		 * 
		 */
		std::size_t count{0};

		/**
		 * @brief The datatype of the attribute.
		 * 
		 */
		Type type{Type::Scalar};

		/**
		 * @brief The maximum value of the components in this attributes.
		 * 
		 */
		std::optional<Vector3f> max;

		/**
		 * @brief The minimum value of the components in this attributes.
		 * 
		 */
		std::optional<Vector3f> min;

		/**
		 * @brief The name of the accessor.
		 * 
		 */
		std::optional<std::pmr::string> name;

		friend auto operator<=>(Accessor const&, Accessor const&) noexcept = default;
	};

	/**
	 * @brief Return the size in bytes of the given accessor type.
	 * 
	 * @param component_type An accessor type.
	 * @return The size in bytes of \p accessor_type.
	 */
	export std::uint8_t size_of(Accessor::Type accessor_type) noexcept;


	/**
	 * @brief A buffer that describes binary data.
	 * 
	 */
	export struct Buffer
	{
		/**
		 * @brief The uri of the buffer.
		 * 
		 */
		std::optional<std::pmr::string> uri;

		/**
		 * @brief The length of the buffer in bytes.
		 * 
		 */
		std::size_t byte_length{0};

		/**
		 * @brief The name of the buffer.
		 * 
		 */
		std::optional<std::pmr::string> name;

		friend auto operator<=>(Buffer const&, Buffer const&) noexcept = default;
	};

	/**
	 * @brief A view into a buffer.
	 * 
	 */
	export struct Buffer_view
	{
		/**
		 * @brief The index of the buffer.
		 * 
		 */
		Index buffer_index{0};

		/**
		 * @brief The offset into the buffer in bytes.
		 * 
		 */
		std::size_t byte_offset{0};

		/**
		 * @brief The length of the buffer in bytes.
		 * 
		 */
		std::size_t byte_length{0};

		/**
		 * @brief The name of the buffer view.
		 * 
		 */
		std::optional<std::pmr::string> name;

		friend auto operator<=>(Buffer_view const&, Buffer_view const&) noexcept = default;
	};

	/**
	 * @brief Parameters for metallic-roughness material model.
	 * 
	 */
	export struct Pbr_metallic_roughness
	{
		/**
		 * @brief The base color of the material.
		 * 
		 */
		Vector4f base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};

		/**
		 * @brief The metallness of the material.
		 * 
		 * 1.0 means the material is a metal. 0.0 means it's a dielectric.
		 * 
		 */
		float metallic_factor{1.0f};

		/**
		 * @brief The roughness of the material.
		 * 
		 * 1.0 means the material is completely rough. 0.0 means the material is completely smooth.
		 * 
		 */
		float roughness_factor{1.0f};

		friend auto operator<=>(Pbr_metallic_roughness const&, Pbr_metallic_roughness const&) noexcept = default;
	};

	/**
	 * @brief Specifies the interpretation of the alpha value of a color factor and texture.
	 * 
	 */
	export enum class Alpha_mode
	{
		/**
		 * @brief The alpha is ignored and the primitive is fully opaque.
		 * 
		 */
		Opaque = 0,

		/**
		 * @brief The primitive is either fully opaque of full transparent depending on the alpha value and the alpha
		 * cutoff of the material.
		 * 
		 */
		Mask,

		/**
		 * @brief The alpha value is used to interpolate between the source and destination pixels.
		 * 
		 */
		Blend
	};

	/**
	 * @brief Describes the appearance of the surface of a primitive.
	 * 
	 */
	export struct Material
	{
		/**
		 * @brief The parameters used to define the metallic-roughness material model.
		 * 
		 */
		Pbr_metallic_roughness pbr_metallic_roughness;

		/**
		 * @brief The emissive color of the material.
		 * 
		 */
		Vector3f emissive_factor{0.0f, 0.0f, 0.0f};

		/**
		 * @brief Specifies the interpretation of the alpha value of the color factors and textures.
		 * 
		 */
		Alpha_mode alpha_mode{Alpha_mode::Opaque};

		/**
		 * @brief The alpha cutoff value when \p alpha_mode is Mask.
		 * 
		 */
		float alpha_cutoff{0.5f};

		/**
		 * @brief Specifies whether the material is double sided.
		 * 
		 * False means that back-face culling is enabled.
		 * True means that back-face culling is disabled and both sides of a primitive are shaded.
		 * 
		 */
		bool double_sided{false};

		/**
		 * @brief The name of the material.
		 * 
		 */
		std::optional<std::pmr::string> name;

		friend auto operator<=>(Material const&, Material const&) noexcept = default;

	};

	/**
	 * @brief Vertex attribute used in the draw calls.
	 * 
	 */
	export struct Attribute
	{
		/**
		 * @brief The semantic of an attribute.
		 * 
		 */
		enum class Type
		{
			Position = 0,
			Normal,
			Tangent,
			Texture_coordinate,
			Color,
			Joints,
			Weights,
			Undefined,
		};

		/**
		 * @brief The semantic of the attribute.
		 * 
		 */
		Type type;

		/**
		 * @brief The index of the semantic.
		 * 
		 */
		Index index;

		friend auto operator<=>(Attribute const&, Attribute const&) noexcept = default;
	};

	/**
	 * @brief Computes the hash of an attribute.
	 * 
	 */
	export struct Attribute_hash
	{
		std::size_t operator()(Attribute attribute) const noexcept;
	};


	/**
	 * @brief Geometry and material to be rendered.
	 * 
	 */
	export struct Primitive
	{
		/**
		 * @brief The type of a primitive.
		 * 
		 */
		enum class Mode
		{
			Points = 0,
			Lines,
			Line_loop,
			Line_strip,
			Triangles,
			Triangle_strip,
			Triangle_fan
		};

		/**
		 * @brief Vertex attributes of the primitive.
		 * 
		 */
		std::pmr::unordered_map<Attribute, Index, Attribute_hash> attributes;

		/**
		 * @brief The index of the accessor that contains the indices.
		 * 
		 */
		std::optional<Index> indices_index;

		/**
		 * @brief The type of the primitive.
		 * 
		 */
		Mode mode{Mode::Triangles};

		/**
		 * @brief The index of the material.
		 * 
		 */
		std::optional<Index> material_index;

		friend auto operator<=>(Primitive const&, Primitive const&) noexcept = default;
	};

	/**
	 * @brief A set of primitives to render.
	 * 
	 */
	export struct Mesh
	{
		/**
		 * @brief Primitives to render, each defining geometry and material.
		 * 
		 */
		std::pmr::vector<Primitive> primitives;

		/**
		 * @brief The name of the mesh.
		 * 
		 */
		std::optional<std::pmr::string> name;

		friend auto operator<=>(Mesh const&, Mesh const&) noexcept = default;
	};

	/**
	 * @brief The camera's projection.
	 * 
	 */
	export struct Camera
	{
		/**
		 * @brief The type of camera projection.
		 * 
		 */
		enum class Type
		{
			Orthographic,
			Perspective
		};

		/**
		 * @brief Properties of an orthographic projection.
		 * 
		 */
		struct Orthographic
		{
			float horizontal_magnification{0.0f};
			float vertical_magnification{0.0f};
			float near_z{0.0f};
			float far_z{0.0f};

			friend auto operator<=>(Orthographic const&, Orthographic const&) noexcept = default;
		};

		/**
		 * @brief Properties of a perspective projection.
		 * 
		 */
		struct Perspective
		{
			std::optional<float> aspect_ratio;
			float vertical_field_of_view{0.0f};
			float near_z{0.0f};
			std::optional<float> far_z;

			friend auto operator<=>(Perspective const&, Perspective const&) noexcept = default;
		};

		/**
		 * @brief Type of the camera's projection.
		 * 
		 */
		Type type{Type::Orthographic};

		/**
		 * @brief The properties of the camera's projection.
		 * 
		 */
		std::variant<Orthographic, Perspective> projection{Orthographic{}};

		/**
		 * @brief The name of the camera.
		 * 
		 */
		std::optional<std::pmr::string> name;

		friend auto operator<=>(Camera const&, Camera const&) noexcept = default;
	};

	export bool operator==(
		std::variant<Camera::Orthographic, Camera::Perspective> const& lhs,
		std::variant<Camera::Orthographic, Camera::Perspective> const& rhs
	) noexcept;

	export bool operator!=(
		std::variant<Camera::Orthographic, Camera::Perspective> const& lhs,
		std::variant<Camera::Orthographic, Camera::Perspective> const& rhs
	) noexcept;

	/**
	 * @brief A node that is part of a scene hierarchy.
	 * 
	 */
	export struct Node
	{
		/**
		 * @brief The index of a mesh.
		 * 
		 */
		std::optional<Index> mesh_index;

		/**
		 * @brief The index of a camera.
		 * 
		 */
		std::optional<Index> camera_index;

		/**
		 * @brief The indices of the child nodes.
		 * 
		 */
		std::pmr::vector<Index> child_indices;

		/**
		 * @brief The orientation of the node.
		 * 
		 */
		Quaternionf rotation{0.0f, 0.0f, 0.0f, 1.0f};

		/**
		 * @brief The scale of the node.
		 * 
		 */
		Vector3f scale{1.0f, 1.0f, 1.0f};

		/**
		 * @brief The translation of the node.
		 * 
		 */
		Vector3f translation{0.0f, 0.0f, 0.0f};

		/**
		 * @brief The name of the node.
		 * 
		 */
		std::optional<std::pmr::string> name;

		friend auto operator<=>(Node const&, Node const&) noexcept = default;
	};

	/**
	 * @brief The root nodes of a scene.
	 * 
	 */
	export struct Scene
	{
		/**
		 * @brief The indices of each root node.
		 * 
		 */
		std::optional<std::pmr::vector<Index>> nodes;

		/**
		 * @brief The name of the scene.
		 * 
		 */
		std::optional<std::pmr::string> name;

		friend auto operator<=>(Scene const&, Scene const&) noexcept = default;
	};

	/**
	 * @brief A set of scenes and their associated data.
	 * 
	 */
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

		friend auto operator<=>(World const&, World const&) noexcept = default;
	};
}