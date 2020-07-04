module maia.utilities.gltf;

import <array>;
import <cassert>;
import <cmath>;
import <cstddef>;
import <cstdint>;
import <filesystem>;
import <fstream>;
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
		Value_type get_value(nlohmann::json const& json) noexcept
		{
			return json.get<Value_type>();
		}

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

		std::pmr::string get_string_value(
			nlohmann::json const& json,
			std::pmr::polymorphic_allocator<> const& allocator) noexcept
		{
			std::string const& value = json.get<std::string>();

			return {value.begin(), value.end(), allocator};
		}
		
		template <class Value_type, class Function_type>
		std::pmr::vector<Value_type> get_vector_from_json(
			nlohmann::json const& json,
			Function_type&& element_from_json,
			std::pmr::polymorphic_allocator<> const& allocator
		) noexcept
		{
			std::pmr::vector<Value_type> elements{allocator};
			elements.reserve(json.size());

			for (nlohmann::json const& element_json : json)
			{
				elements.push_back(
					element_from_json(element_json)
				);
			}

			return elements;
		}

		template <class Value_type, class Function_type>
		std::pmr::vector<Value_type> get_vector_from_json(
			nlohmann::json const& json,
			char const* const key,
			Function_type&& element_from_json,
			std::pmr::polymorphic_allocator<> const& allocator
		) noexcept
		{
			auto const iterator = json.find(key);

			if (iterator != json.end())
			{
				return get_vector_from_json<Value_type>(*iterator, std::forward<Function_type>(element_from_json), allocator);
			}
			else
			{
				return {};
			}
		}

		template <class Value_type, class Function_type>
		std::optional<Value_type> get_optional_value(
			nlohmann::json const& json,
			char const* const key,
			Function_type&& to_value
		) noexcept
		{
			auto const iterator = json.find(key);

			if (iterator != json.end())
			{
				return to_value(*iterator);
			}
			else
			{
				return {};
			}
		}

		template <class Value_type, class Function_type>
		Value_type get_optional_value_or(
			nlohmann::json const& json,
			char const* const key,
			Value_type&& default_value,
			Function_type&& to_value
		) noexcept
		{
			auto const iterator = json.find(key);

			if (iterator != json.end())
			{
				return to_value(*iterator);
			}
			else
			{
				return std::forward<Value_type>(default_value);
			}
		}

		template <class Key_type, class Value_type, class To_key_function_type, class To_value_function_type>
		std::pmr::unordered_map<Key_type, Value_type> get_unordered_map_from_json(
			nlohmann::json const& json,
			To_key_function_type&& to_key,
			To_value_function_type&& to_value,
			std::pmr::polymorphic_allocator<> const& allocator
		) noexcept
		{
			std::pmr::unordered_map<Key_type, Value_type> map{allocator};

			for (auto const& element : json.items())
			{
				map.emplace(element.key(), element.value());
			}

			return map;
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
		auto const to_string = [&allocator](nlohmann::json const& json) -> std::pmr::string
		{
			return get_string_value(json, allocator);
		};

		return
		{
			.uri = get_optional_value<std::pmr::string>(json, "uri", to_string),
			.byte_length = get_value<std::size_t>(json, "byteLength"),
		};
	}

	namespace
	{
		std::pmr::vector<std::byte> decode_base64(
			std::string_view const input,
			std::size_t const output_size,
			std::pmr::polymorphic_allocator<> const& allocator
		) noexcept
		{
			std::array<std::uint8_t, 128> constexpr reverse_table
			{
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
				64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
				52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
				64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
				15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
				64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
				41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
			};

			std::pmr::vector<std::byte> output{allocator};
			output.reserve(output_size);

			{
				std::uint32_t bits{0};
				std::uint8_t bit_count{0};

				for (char const c : input)
				{
					if (std::isspace(c) || c == '=')
					{
						continue;
					}

					assert(c > 0);
					assert(reverse_table[c] < 64);

					bits = (bits << 6) | reverse_table[c];
					bit_count += 6;

					if (bit_count >= 8)
					{
						bit_count -= 8;
						output.push_back(static_cast<std::byte>((bits >> bit_count) & 0xFF));
					}
				}
			}

			assert(output.size() == output_size);

			return output;
		}

		std::pmr::vector<std::byte> generate_byte_data(
			std::string_view const uri,
			std::size_t const byte_length,
			std::pmr::polymorphic_allocator<> const& allocator
		) noexcept
		{
			if (uri.compare(0, 5, "data:") == 0)
			{
				char const* const base64_prefix{"data:application/octet-stream;base64,"};
				std::size_t const base64_prefix_size{std::strlen(base64_prefix)};
				assert((uri.compare(0, base64_prefix_size, base64_prefix) == 0) && "Uri format not supported");

				std::string_view const data_view{uri.data() + base64_prefix_size, uri.size() - base64_prefix_size};
				return decode_base64(data_view, byte_length, allocator);
			}
			else
			{
				std::filesystem::path const file_path{uri};
				assert(std::filesystem::exists(file_path) && "Couldn't open file");
				assert(std::filesystem::file_size(file_path) == byte_length);

				std::pmr::vector<std::byte> file_content{allocator};
				file_content.resize(byte_length);

				{
					std::basic_ifstream<std::byte> file_stream{file_path, std::ios::binary};
					assert(file_stream.good());

					file_stream.read(file_content.data(), byte_length);
					assert(file_stream.good());
				}

				return file_content;
			}
		}
	}

	std::pmr::vector<std::byte> read_buffer_data(
		Buffer const& buffer,
		std::pmr::polymorphic_allocator<> const& allocator
	) noexcept
	{
		assert(buffer.uri.has_value());

		return generate_byte_data(*buffer.uri, buffer.byte_length, allocator);
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


	Pbr_metallic_roughness pbr_metallic_roughness_from_json(nlohmann::json const& json) noexcept
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
		auto const to_string = [&allocator](nlohmann::json const& json) -> std::pmr::string
		{
			return get_string_value(json, allocator);
		};

		return
		{
			.name = get_optional_value<std::pmr::string>(json, "name", to_string),
			.pbr_metallic_roughness = pbr_metallic_roughness_from_json(json.at("pbrMetallicRoughness")),
			.emissive_factor = get_optional_value_or<Vector3f>(json, "emissiveFactor", {0.0f, 0.0f, 0.0f}),
			.alpha_mode = get_optional_value_or<std::pmr::string>(json, "alphaMode", {"OPAQUE", allocator}, to_string),
			.alpha_cutoff = get_optional_value_or<float>(json, "alphaCutoff", 0.5f),
			.double_sided = get_optional_value_or<bool>(json, "doubleSided", false),
		};
	}


	Primitive primitive_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		auto const to_index = [](nlohmann::json const& json) -> Index
		{
			return json.get<Index>();
		};

		auto const to_string = [&allocator](nlohmann::json const& json) -> std::pmr::string
		{
			return get_string_value(json, allocator);
		};

		return
		{
			.attributes = get_unordered_map_from_json<std::pmr::string, Index>(json.at("attributes"), to_string, to_index, allocator),
			.indices_index = get_optional_value<Index>(json, "indices"),
			.material_index = get_optional_value<Index>(json, "material"),
		};
	}


	Mesh mesh_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		auto const to_primitive = [&allocator](nlohmann::json const& json) -> Primitive
		{
			return primitive_from_json(json, allocator);
		};

		auto const to_string = [&allocator](nlohmann::json const& json) -> std::pmr::string
		{
			return get_string_value(json, allocator);
		};

		return
		{
			.primitives = get_vector_from_json<Primitive>(json.at("primitives"), to_primitive, allocator),
			.name = get_optional_value<std::pmr::string>(json, "name", to_string),
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
		auto const to_index = [](nlohmann::json const& json) -> Index
		{
			return json.get<Index>();
		};

		auto const to_string = [&allocator](nlohmann::json const& json) -> std::pmr::string
		{
			return get_string_value(json, allocator);
		};

		std::optional<Index> const mesh_index = get_optional_value<Index>(json, "mesh");
		std::optional<Index> const camera_index = get_optional_value<Index>(json, "camera");
		std::pmr::vector<Index> child_indices = get_vector_from_json<Index>(json, "children", to_index, allocator);
		std::optional<std::pmr::string> name = get_optional_value<std::pmr::string>(json, "name", to_string);

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

		auto const to_string = [&allocator](nlohmann::json const& json) -> std::pmr::string
		{
			return get_string_value(json, allocator);
		};

		Camera::Type const type = get_value<Camera::Type>(json, "type");
		std::optional<std::pmr::string> const name = get_optional_value<std::pmr::string>(json, "name", to_string);
		
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
		auto const to_index = [](nlohmann::json const& json) -> Index
		{
			return json.get<Index>();
		};

		auto const to_string = [&allocator](nlohmann::json const& json) -> std::pmr::string
		{
			return get_string_value(json, allocator);
		};

		return
		{
			.name = get_optional_value<std::pmr::string>(json, "names", to_string),
			.nodes = get_vector_from_json<Index>(json, "nodes", to_index, allocator),
		};
	}

	namespace
	{
		template <class Value_type>
		std::pmr::vector<Value_type> get_allocated_elements_from_json(
			nlohmann::json const& json,
			char const* const key,
			Value_type (*element_from_json) (nlohmann::json const&, std::pmr::polymorphic_allocator<> const&),
			std::pmr::polymorphic_allocator<> const& allocator
		) noexcept
		{
			auto const to_value = [&element_from_json, &allocator](nlohmann::json const& json) -> Value_type
			{
				return element_from_json(json, allocator);
			};

			return get_vector_from_json<Value_type>(json, key, to_value, allocator);
		}
	}

	Gltf gltf_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator) noexcept
	{
		return
		{
			.accessors = get_vector_from_json<Accessor>(json, "accessors", accessor_from_json, allocator),
			.buffers = get_allocated_elements_from_json(json, "buffers", buffer_from_json, allocator),
			.buffer_views = get_vector_from_json<Buffer_view>(json, "bufferViews", buffer_view_from_json, allocator),
			.cameras = get_allocated_elements_from_json(json, "cameras", camera_from_json, allocator),
			.materials = get_allocated_elements_from_json(json, "materials", material_from_json, allocator),
			.meshes = get_allocated_elements_from_json(json, "meshes", mesh_from_json, allocator),
			.nodes = get_allocated_elements_from_json(json, "nodes", node_from_json, allocator),
			.scene_index = get_optional_value<Index>(json, "scene"),
			.scenes = get_allocated_elements_from_json(json, "scenes", scene_from_json, allocator),
		};
	}
}
