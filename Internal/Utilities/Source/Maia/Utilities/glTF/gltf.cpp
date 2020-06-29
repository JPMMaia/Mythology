module maia.utilities.gltf;

import <array>;
import <cassert>;
import <cmath>;
import <cstddef>;
import <memory_resource>;
import <optional>;
import <string>;
import <unordered_map>;
import <variant>;
import <vector>;

import <nlohmann/json.hpp>;

namespace nlohmann
{
	template <>
	struct adl_serializer<Maia::Utilities::glTF::Vector3f>
	{
		static void to_json(json& j, Maia::Utilities::glTF::Vector3f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Utilities::glTF::Vector3f& value)
		{
			value.x = j.at(0).get<float>();
			value.y = j.at(1).get<float>();
			value.z = j.at(2).get<float>();
		}
	};

	template <>
	struct adl_serializer<Maia::Utilities::glTF::Vector4f>
	{
		static void to_json(json& j, Maia::Utilities::glTF::Vector4f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Utilities::glTF::Vector4f& value)
		{
			value.x = j.at(0).get<float>();
			value.y = j.at(1).get<float>();
			value.z = j.at(2).get<float>();
			value.w = j.at(3).get<float>();
		}
	};

	template <>
	struct adl_serializer<Maia::Utilities::glTF::Quaternionf>
	{
		static void to_json(json& j, Maia::Utilities::glTF::Quaternionf const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Utilities::glTF::Quaternionf& value)
		{
			value.x = j.at(0).get<float>();
			value.y = j.at(1).get<float>();
			value.z = j.at(2).get<float>();
			value.w = j.at(3).get<float>();
		}
	};

	template <>
	struct adl_serializer<Maia::Utilities::glTF::Matrix4f>
	{
		static void to_json(json& j, Maia::Utilities::glTF::Matrix4f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Utilities::glTF::Matrix4f& value)
		{
			for (std::size_t i = 0; i < 16; ++i)
			{
				value.values[i] = j.at(i).get<float>();
			}
		}
	};

	template <>
	struct adl_serializer<Maia::Utilities::glTF::Accessor::Type>
	{
		static void to_json(json& j, Maia::Utilities::glTF::Accessor::Type const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Utilities::glTF::Accessor::Type& value)
		{
			using namespace Maia::Utilities::glTF;

			std::string string_value = j.get<std::string>();

			if (string_value == "SCALAR")
				value = Accessor::Type::Scalar;
			else if (string_value == "VEC2")
				value = Accessor::Type::Vector2;
			else if (string_value == "VEC3")
				value = Accessor::Type::Vector3;
			else if (string_value == "VEC4")
				value = Accessor::Type::Vector4;
			else if (string_value == "MAT2")
				value = Accessor::Type::Matrix2x2;
			else if (string_value == "MAT3")
				value = Accessor::Type::Matrix3x3;
			else if (string_value == "MAT4")
				value = Accessor::Type::Matrix4x4;
			else
				throw std::invalid_argument{ "String value does not match any of the possible values" };
		}
	};

	template <>
	struct adl_serializer<Maia::Utilities::glTF::Camera::Type>
	{
		static void to_json(json& j, Maia::Utilities::glTF::Camera::Type const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Utilities::glTF::Camera::Type& value)
		{
			using namespace Maia::Utilities::glTF;

			std::string string_value = j.get<std::string>();

			if (string_value == "perspective")
				value = Camera::Type::Perspective;
			else if (string_value == "orthographic")
				value = Camera::Type::Orthographic;
			else
				throw std::invalid_argument{ "String value does not match any of the possible values" };
		}
	};
}

namespace Maia::Utilities::glTF
{
	namespace
	{
		template <class Value_type>
		Value_type get_value(nlohmann::json const& json, char const* const key) noexcept
		{
			return json.at(key).get<Value_type>();
		}

		template <class Value_type>
		std::optional<Value_type> get_optional_value(nlohmann::json const& json, char const* const key) noexcept
		{
			nlohmann::json::const_iterator const location = json.find(key);

			if (location != json.end())
			{
				return location->get<Value_type>();
			}
			else
			{
				return {};
			}
		}

		template <class Value_type>
		Value_type get_optional_value_or(nlohmann::json const& json, char const* key, Value_type const value) noexcept
		{
			nlohmann::json::const_iterator const location = json.find(key);

			if (location != json.end())
			{
				return location->get<Value_type>();
			}
			else
			{
				return value;
			}
		}
	}


	std::uint8_t size_of(Component_type component_type) noexcept
	{
		switch (component_type)
		{
		case Component_type::Byte:
		case Component_type::Unsigned_byte:
			return 1;
		case Component_type::Short:
		case Component_type::Unsigned_short:
			return 2;
		case Component_type::Unsigned_int:
		case Component_type::Float:
			return 4;
		default: assert(false); return 1;
		}
	}


	Accessor accessor_from_json(nlohmann::json const& json) noexcept
	{
		Accessor::Type const type = get_value<Accessor::Type>(json, "type");

		return
		{
			.buffer_view_index = get_optional_value<Index>(json, "bufferView"),
			.component_type = get_value<Component_type>(json, "componentType"),
			.count = get_value<std::size_t>(json, "count"),
			.type = type,
			.max = type == Accessor::Type::Vector3 ? get_optional_value<Vector3f>(json, "max") : std::optional<Vector3f>{},
			.min = type == Accessor::Type::Vector3 ? get_optional_value<Vector3f>(json, "min") : std::optional<Vector3f>{}
		};
	}
	std::uint8_t size_of(Accessor::Type accessor_type) noexcept
	{
		switch (accessor_type)
		{
		case Accessor::Type::Scalar: return 1;
		case Accessor::Type::Vector2: return 2;
		case Accessor::Type::Vector3: return 3;
		case Accessor::Type::Vector4: return 4;
		case Accessor::Type::Matrix2x2: return 4;
		case Accessor::Type::Matrix3x3: return 9;
		case Accessor::Type::Matrix4x4: return 16;
		default: assert(false); return 1;
		}
	}


	Buffer buffer_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		return
		{
			.uri = get_optional_value<std::pmr::string>(json, "uri"),
			.byte_length = get_value<std::size_t>(json, "byteLength"),
		};
	}


	Buffer_view buffer_view_from_json(nlohmann::json const& json) noexcept
	{
		return
		{
			.buffer_index = get_value<Index>(json, "buffer"),
			.byte_offset = get_optional_value_or<Index>(json, "byteOffset", 0),
			.byte_length = get_value<std::size_t>(json, "byteLength"),
		};
	}


	PbrMetallicRoughness pbr_metallic_roughness_from_json(nlohmann::json const& json) noexcept
	{
		return
		{
			.base_color_factor = get_optional_value_or<Vector4f>(json, "baseColorFactor", {1.0f, 1.0f, 1.0f, 1.0f}),
			.metallic_factor = get_optional_value_or<float>(json, "metallicFactor", 1.0f),
			.roughness_factor = get_optional_value_or<float>(json, "roughnessFactor", 1.0f),
		};
	}


	Material material_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		return
		{
			.name = get_optional_value<std::pmr::string>(json, "name"),
			.pbr_metallic_roughness = pbr_metallic_roughness_from_json(json.at("pbrMetallicRoughness")),
			.emissive_factor = get_optional_value_or<Vector3f>(json, "emissiveFactor", {0.0f, 0.0f, 0.0f}),
			.alpha_mode = get_optional_value_or<std::pmr::string>(json, "alphaMode", "OPAQUE"),
			.alpha_cutoff = get_optional_value_or<float>(json, "alphaCutoff", 0.5f),
			.double_sided = get_optional_value_or<bool>(json, "doubleSided", false),
		};
	}


	Primitive primitive_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		return
		{
			.attributes = get_value<std::pmr::unordered_map<std::pmr::string, Index>>(json, "attributes"),
			.indices_index = get_optional_value<Index>(json, "indices"),
			.material_index = get_optional_value<Index>(json, "material"),
		};
	}


	Mesh from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		return
		{
			.primitives = get_value<std::pmr::vector<Primitive>>(json, "primitives"),
			.name = get_optional_value<std::pmr::string>(json, "name"),
		};
	}


	namespace
	{
		struct Transform
		{
			Vector3f translation;
			Quaternionf rotation;
			Vector3f scale;
		};

		Transform decompose(Matrix4f const matrix) noexcept
		{
			std::array<float, 16> matrix_values = matrix.values;

			Vector3f const translation = {matrix_values[12], matrix_values[13], matrix_values[14]};
			matrix_values[12] = matrix_values[13] = matrix_values[14] = 0.0f;

			Vector3f const column_0 = {matrix_values[0], matrix_values[1], matrix_values[2]};
			Vector3f const column_1 = {matrix_values[4], matrix_values[5], matrix_values[6]};
			Vector3f const column_2 = {matrix_values[8], matrix_values[9], matrix_values[10]};

			auto const length_of = [](Vector3f const vector) -> float
			{
				return std::sqrt(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
			};

			Vector3f const scale = {length_of(column_0), length_of(column_1), length_of(column_2)};

			matrix_values[0] /= scale.x;
			matrix_values[1] /= scale.x;
			matrix_values[2] /= scale.x;
			matrix_values[4] /= scale.y;
			matrix_values[5] /= scale.y;
			matrix_values[6] /= scale.y;
			matrix_values[8] /= scale.z;
			matrix_values[9] /= scale.z;
			matrix_values[10] /= scale.z;

			float const rotation_w = std::sqrt(1.0f + matrix_values[0] + matrix_values[5] + matrix_values[10]) / 2.0f;
			Quaternionf const rotation
			{
				.x = (matrix_values[6] - matrix_values[9]) / (4.0f * rotation_w),
				.y = (matrix_values[8] - matrix_values[2]) / (4.0f * rotation_w),
				.z = (matrix_values[5] - matrix_values[4]) /(4.0f * rotation_w),
				.w = rotation_w,
			};

			return
			{
				.translation = translation,
				.rotation = rotation,
				.scale = scale,
			};
		}
	}

	Node node_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		std::optional<Index> const mesh_index = get_optional_value<Index>(json, "mesh");
		std::optional<Index> const camera_index = get_optional_value<Index>(json, "camera");
		std::optional<std::pmr::vector<Index>> child_indices = get_optional_value<std::pmr::vector<Index>>(json, "children");
		std::optional<std::pmr::string> name = get_optional_value<std::pmr::string>(json, "name");

		nlohmann::json::const_iterator const matrix_location = json.find("matrix");

		if (matrix_location != json.end())
		{
			assert(json.find("rotation") == json.end());
			assert(json.find("scale") == json.end());
			assert(json.find("translation") == json.end());

			Matrix4f const matrix = matrix_location->get<Matrix4f>();
			Transform const transform = decompose(matrix);

			return
			{
				.name = std::move(name),
				.mesh_index = mesh_index,
				.camera_index = camera_index,
				.child_indices = std::move(child_indices),
				.rotation = transform.rotation,
				.scale = transform.scale,
				.translation = transform.translation,
			};
		}
		else
		{
			return
			{
				.name = std::move(name),
				.mesh_index = mesh_index,
				.camera_index = camera_index,
				.child_indices = std::move(child_indices),
				.rotation = get_optional_value_or<Quaternionf>(json, "rotation", {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f}),
				.scale = get_optional_value_or<Vector3f>(json, "scale", {.x = 1.0f, .y = 1.0f, .z = 1.0f}),
				.translation = get_optional_value_or<Vector3f>(json, "translation", {.x = 0.0f, .y = 0.0f, .z = 0.0f}),
			};
		}
	}


	Camera::Orthographic orthographic_camera_from_json(nlohmann::json const& json) noexcept
	{
		return
		{
			.horizontal_magnification = get_value<float>(json, "xmag"),
			.vertical_magnification = get_value<float>(json, "ymag"),
			.near_z = get_value<float>(json, "znear"),
			.far_z = get_value<float>(json, "zfar"),
		};
	}

	Camera::Perspective perspective_camera_from_json(nlohmann::json const& json) noexcept
	{
		return
		{
			.aspect_ratio = get_optional_value<float>(json, "aspectRatio"),
			.vertical_field_of_view = get_value<float>(json, "yfov"),
			.near_z = get_value<float>(json, "znear"),
			.far_z = get_optional_value<float>(json, "zfar"),
		};
	}

	Camera camera_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		assert(json.contains("orthographic") || json.contains("perspective"));

		Camera::Type const type = get_value<Camera::Type>(json, "type");
		std::optional<std::pmr::string> const name = get_optional_value<std::pmr::string>(json, "name");
		
		{
			nlohmann::json::const_iterator const orthographic_location = json.find("orthographic");

			if (orthographic_location != json.end())
			{
				return
				{
					.type = type,
					.name = name,
					.projection = orthographic_camera_from_json(*orthographic_location),
				};
			}
			else
			{
				nlohmann::json::const_iterator const perspective_location = json.find("perspective");
				
				return
				{
					.type = type,
					.name = name,
					.projection = perspective_camera_from_json(*perspective_location),
				};
			}
		}
	}


	Scene scene_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		return
		{
			.name = get_optional_value<std::pmr::string>(json, "names"),
			.nodes = get_optional_value<std::pmr::vector<Index>>(json, "nodes"),
		};
	}


	Gltf gltf_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		return
		{
			.accessors = get_optional_value<std::pmr::vector<Accessor>>(json, "accessors"),
			.buffers = get_optional_value<std::pmr::vector<Buffer>>(json, "buffers"),
			.buffer_views = get_optional_value<std::pmr::vector<Buffer_view>>(json, "bufferViews"),
			.cameras = get_optional_value<std::pmr::vector<Camera>>(json, "cameras"),
			.materials = get_optional_value<std::pmr::vector<Material>>(json, "materials"),
			.meshes = get_optional_value<std::pmr::vector<Mesh>>(json, "meshes"),
			.nodes = get_optional_value<std::pmr::vector<Node>>(json, "nodes"),
			.scene_index = get_optional_value<Index>(json, "scene"),
			.scenes = get_optional_value<std::pmr::vector<Scene>>(json, "scenes"),
		};
	}
}
