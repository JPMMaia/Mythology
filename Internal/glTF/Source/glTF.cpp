module;

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory_resource>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

module maia.glTF;

namespace nlohmann
{
	template <>
	struct adl_serializer<Maia::Scene::Vector3f>
	{
		static void to_json(json& j, Maia::Scene::Vector3f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Vector3f& value)
		{
			value.x = j.at(0).get<float>();
			value.y = j.at(1).get<float>();
			value.z = j.at(2).get<float>();
		}
	};

	template <>
	struct adl_serializer<Maia::Scene::Vector4f>
	{
		static void to_json(json& j, Maia::Scene::Vector4f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Vector4f& value)
		{
			value.x = j.at(0).get<float>();
			value.y = j.at(1).get<float>();
			value.z = j.at(2).get<float>();
			value.w = j.at(3).get<float>();
		}
	};

	template <>
	struct adl_serializer<Maia::Scene::Quaternionf>
	{
		static void to_json(json& j, Maia::Scene::Quaternionf const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Quaternionf& value)
		{
			value.x = j.at(0).get<float>();
			value.y = j.at(1).get<float>();
			value.z = j.at(2).get<float>();
			value.w = j.at(3).get<float>();
		}
	};

	template <>
	struct adl_serializer<Maia::Scene::Matrix4f>
	{
		static void to_json(json& j, Maia::Scene::Matrix4f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Matrix4f& value)
		{
			for (std::size_t i = 0; i < 16; ++i)
			{
				value.values[i] = j.at(i).get<float>();
			}
		}
	};

	template <>
	struct adl_serializer<Maia::Scene::Accessor::Type>
	{
		static void to_json(json& j, Maia::Scene::Accessor::Type const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Accessor::Type& value)
		{
			using namespace Maia::Scene;

			std::string const& string_value = j.get<std::string>();

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
	struct adl_serializer<Maia::Scene::Alpha_mode>
	{
		static void to_json(json& j, Maia::Scene::Alpha_mode const value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Alpha_mode& value)
		{
			using namespace Maia::Scene;

			std::string const& string_value = j.get<std::string>();

			if (string_value == "OPAQUE")
				value = Alpha_mode::Opaque;
			else if (string_value == "MASK")
				value = Alpha_mode::Mask;
			else if (string_value == "BLEND")
				value = Alpha_mode::Blend;
			else
				throw std::invalid_argument{ "String value does not match any of the possible values" };
		}
	};

	template <>
	struct adl_serializer<Maia::Scene::Attribute::Type>
	{
		static void to_json(json& j, Maia::Scene::Attribute::Type const value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Attribute::Type& value)
		{
			// TODO
			assert(false);
		}
	};

	template <>
	struct adl_serializer<Maia::Scene::Attribute>
	{
		static void to_json(json& j, Maia::Scene::Attribute const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Attribute& value)
		{
			using namespace Maia::Scene;

			std::string const& string_value = j.get<std::string>();

			auto const underscore_location = std::find(string_value.begin(), string_value.end(), '_');

			std::string_view semantic_name = [&]() -> std::string_view
			{
				if (underscore_location == string_value.end())
				{
					return {string_value.begin(), string_value.end()};
				}
				else
				{
					return {string_value.begin(), underscore_location};
				}
			}();

			Attribute::Type const semantic_type = [semantic_name]() -> Attribute::Type
			{
				if (semantic_name == "POSITION")
				{
					return Attribute::Type::Position;
				}
				else if (semantic_name == "NORMAL")
				{
					return Attribute::Type::Normal;
				}
				else if (semantic_name == "TANGENT")
				{
					return Attribute::Type::Tangent;
				}
				else if (semantic_name == "TEXCOORD")
				{
					return Attribute::Type::Texture_coordinate;
				}
				else if (semantic_name == "COLOR")
				{
					return Attribute::Type::Color;
				}
				else if (semantic_name == "JOINTS")
				{
					return Attribute::Type::Joints;
				}
				else if (semantic_name == "WEIGHTS")
				{
					return Attribute::Type::Weights;
				}
				else
				{
					return Attribute::Type::Undefined;
				}
			}();

			Index const semantic_index = [&]() -> Index
			{
				if (underscore_location == string_value.end())
				{
					return 0;
				}
				else
				{					
					auto const position = std::distance(string_value.begin(), underscore_location + 1);
					char* end = nullptr;
					return std::strtol(string_value.data() + position, &end, 10);
				}
			}();

			value = {semantic_type, semantic_index};
		}
	};

	template <>
	struct adl_serializer<Maia::Scene::Primitive::Mode>
	{
		static void to_json(json& j, Maia::Scene::Attribute::Type const value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Primitive::Mode& value)
		{
			using namespace Maia::Scene;

			unsigned int const mode = j.get<unsigned int>();

			switch (mode)
			{
			case 0:
				value = Primitive::Mode::Points;
				break;
			case 1:
				value = Primitive::Mode::Lines;
				break;
			case 2:
				value = Primitive::Mode::Line_loop;
				break;
			case 3:
				value = Primitive::Mode::Line_strip;
				break;
			case 4:
				value = Primitive::Mode::Triangles;
				break;
			case 5:
				value = Primitive::Mode::Triangle_strip;
				break;
			case 6:
				value = Primitive::Mode::Triangle_fan;
				break;
			default:
				value = Primitive::Mode::Triangles;
				break;
			}
		}
	};

	template <>
	struct adl_serializer<Maia::Scene::Camera::Type>
	{
		static void to_json(json& j, Maia::Scene::Camera::Type const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Maia::Scene::Camera::Type& value)
		{
			using namespace Maia::Scene;

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

namespace Maia::glTF
{
	using namespace Maia::Scene;

	namespace
	{
		template <class Value_type>
		Value_type get_value(nlohmann::json const& json)
		{
			return json.get<Value_type>();
		}

		template <class Value_type>
		Value_type get_value(nlohmann::json const& json, char const* const key)
		{
			return json.at(key).get<Value_type>();
		}

		template <class Value_type>
		std::optional<Value_type> get_optional_value(nlohmann::json const& json, char const* const key)
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
		Value_type get_optional_value_or(nlohmann::json const& json, char const* key, Value_type const value)
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
			std::pmr::polymorphic_allocator<> const& allocator
		)
		{
			std::string const& value = json.get<std::string>();

			return {value.begin(), value.end(), allocator};
		}

		std::optional<std::pmr::string> get_optional_string_value(
			nlohmann::json const& json,
			char const* const key,
			std::pmr::polymorphic_allocator<> const& allocator
		)
		{
			auto const iterator = json.find(key);

			if (iterator != json.end())
			{
				return get_string_value(*iterator, allocator);
			}
			else
			{
				return {};
			}
		}
		
		template <class Value_type, class Function_type>
		std::pmr::vector<Value_type> get_vector_from_json(
			nlohmann::json const& json,
			Function_type&& element_from_json,
			std::pmr::polymorphic_allocator<> const& allocator
		)
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
		)
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
		)
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
		)
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

		template <class Key_type, class Value_type, class Key_hash, class To_key_function_type, class To_value_function_type>
		std::pmr::unordered_map<Key_type, Value_type, Key_hash> get_unordered_map_from_json(
			nlohmann::json const& json,
			To_key_function_type&& to_key,
			To_value_function_type&& to_value,
			std::pmr::polymorphic_allocator<> const& allocator
		)
		{
			std::pmr::unordered_map<Key_type, Value_type, Key_hash> map{allocator};

			for (auto const& element : json.items())
			{
				map.emplace(to_key(element.key()), to_value(element.value()));
			}

			return map;
		}
	}


	Accessor accessor_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	)
	{
		Accessor::Type const type = get_value<Accessor::Type>(json, "type");

		return
		{
			.buffer_view_index = get_optional_value<Index>(json, "bufferView"),
			.byte_offset = get_optional_value_or<std::size_t>(json, "byteOffset", 0),
			.normalized = get_optional_value_or<bool>(json, "normalized", false),
			.component_type = get_value<Component_type>(json, "componentType"),
			.count = get_value<std::size_t>(json, "count"),
			.type = type,
			.max = type == Accessor::Type::Vector3 ? get_optional_value<Vector3f>(json, "max") : std::optional<Vector3f>{},
			.min = type == Accessor::Type::Vector3 ? get_optional_value<Vector3f>(json, "min") : std::optional<Vector3f>{},
			.name = get_optional_string_value(json, "name", allocator),
		};
	}


	Buffer buffer_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	)
	{
		return
		{
			.uri = get_optional_string_value(json, "uri", allocator),
			.byte_length = get_value<std::size_t>(json, "byteLength"),
			.name = get_optional_string_value(json, "name", allocator),
		};
	}

	namespace
	{
		std::pmr::vector<std::byte> decode_base64(
			std::string_view const input,
			std::size_t const output_size,
			std::pmr::polymorphic_allocator<> const& allocator
		)
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
			std::filesystem::path const& prefix_path,
			std::size_t const byte_length,
			std::pmr::polymorphic_allocator<> const& allocator
		)
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
				std::filesystem::path const file_path{prefix_path / uri};
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


	Buffer_view buffer_view_from_json(
		nlohmann::json const& json,
		std::pmr::polymorphic_allocator<> const& allocator
	)
	{
		return
		{
			.buffer_index = get_value<Index>(json, "buffer"),
			.byte_offset = get_optional_value_or<Index>(json, "byteOffset", 0),
			.byte_length = get_value<std::size_t>(json, "byteLength"),
			.name = get_optional_string_value(json, "name", allocator),
		};
	}


	Pbr_metallic_roughness pbr_metallic_roughness_from_json(nlohmann::json const& json)
	{
		return
		{
			.base_color_factor = get_optional_value_or<Vector4f>(json, "baseColorFactor", {1.0f, 1.0f, 1.0f, 1.0f}),
			.metallic_factor = get_optional_value_or<float>(json, "metallicFactor", 1.0f),
			.roughness_factor = get_optional_value_or<float>(json, "roughnessFactor", 1.0f),
		};
	}


	Material material_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator)
	{
		return 
		{
			.pbr_metallic_roughness = pbr_metallic_roughness_from_json(json.at("pbrMetallicRoughness")),
			.emissive_factor = get_optional_value_or<Vector3f>(json, "emissiveFactor", {0.0f, 0.0f, 0.0f}),
			.alpha_mode = get_optional_value_or<Alpha_mode>(json, "alphaMode", Alpha_mode::Opaque),
			.alpha_cutoff = get_optional_value_or<float>(json, "alphaCutoff", 0.5f),
			.double_sided = get_optional_value_or<bool>(json, "doubleSided", false),
			.name = get_optional_string_value(json, "name", allocator),
		};
	}


	Primitive primitive_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator)
	{
		auto const to_attribute = [&allocator](nlohmann::json const& json) -> Attribute
		{
			return json.get<Attribute>();
		};

		auto const to_index = [](nlohmann::json const& json) -> Index
		{
			return json.get<Index>();
		};

		return
		{
			.attributes = get_unordered_map_from_json<Attribute, Index, Attribute_hash>(
				json.at("attributes"), to_attribute, to_index, allocator
			),
			.indices_index = get_optional_value<Index>(json, "indices"),
			.mode = get_optional_value_or<Primitive::Mode>(json, "mode", Primitive::Mode::Triangles),
			.material_index = get_optional_value<Index>(json, "material"),
		};
	}


	Mesh mesh_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator)
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

		Quaternionf to_rotation(std::array<float, 16> const& matrix) noexcept
		{
			using Trace = float;

			auto const m = [&matrix] (std::size_t const row, std::size_t const column) -> float
			{
				return matrix[4*row+column];
			};

			std::pair<Quaternionf, Trace> const quaternion_and_trace = [&]() -> std::pair<Quaternionf, Trace>
			{
				if (m(2, 2) < 0)
				{
					if (m(0, 0) > m(1, 1))
					{
						float const trace = 1.0f + m(0, 0) -m(1, 1) - m(2, 2);
						Quaternionf const quaternion
						{
							.x = trace,
							.y = m(0, 1) + m(1, 0),
							.z = m(2, 0) + m(0, 2),
							.w = m(1, 2) - m(2, 1)
						};

						return {quaternion, trace};
					}
					else
					{
						float const trace = 1.0f - m(0, 0) + m(1, 1) - m(2, 2);
						Quaternionf const quaternion
						{
							.x = m(0, 1) + m(1, 0),
							.y = trace,
							.z = m(1, 2) + m(2, 1),
							.w = m(2, 0)-m(0, 2)
						};

						return {quaternion, trace};
					}
				}
				else
				{
					if (m(0, 0) < -m(1, 1))
					{
						float const trace = 1.0f - m(0, 0) - m(1, 1) + m(2, 2);
						Quaternionf const quaternion
						{
							.x = m(2, 0) + m(0, 2),
							.y = m(1, 2) + m(2, 1),
							.z = trace,
							.w = m(0, 1) -m(1, 0)
						};

						return {quaternion, trace};
					}
					else
					{
						float const trace = 1.0f + m(0, 0) + m(1, 1) + m(2, 2);
						Quaternionf const quaternion
						{
							.x = m(1, 2) - m(2, 1),
							.y = m(2, 0) - m(0, 2),
							.z = m(0, 1) - m(1, 0),
							.w = trace
						};

						return {quaternion, trace};
					}
				}
			}();
			
			Quaternionf const quaternion = quaternion_and_trace.first;
			float const trace = quaternion_and_trace.second;
			float const scalar = 0.5f / std::sqrt(trace);

			return
			{
				.x = scalar * quaternion.x,
				.y = scalar * quaternion.y,
				.z = scalar * quaternion.z,
				.w = scalar * quaternion.w,
			};
		}

		Transform decompose(Matrix4f const& matrix) noexcept
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

			Quaternionf const rotation = to_rotation(matrix_values);

			return
			{
				.translation = translation,
				.rotation = rotation,
				.scale = scale,
			};
		}
	}

	Node node_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator)
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
				.mesh_index = mesh_index,
				.camera_index = camera_index,
				.child_indices = std::move(child_indices),
				.rotation = transform.rotation,
				.scale = transform.scale,
				.translation = transform.translation,
				.name = std::move(name),
			};
		}
		else
		{
			return
			{
				.mesh_index = mesh_index,
				.camera_index = camera_index,
				.child_indices = std::move(child_indices),
				.rotation = get_optional_value_or<Quaternionf>(json, "rotation", {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f}),
				.scale = get_optional_value_or<Vector3f>(json, "scale", {.x = 1.0f, .y = 1.0f, .z = 1.0f}),
				.translation = get_optional_value_or<Vector3f>(json, "translation", {.x = 0.0f, .y = 0.0f, .z = 0.0f}),
				.name = std::move(name),
			};
		}
	}


	Camera::Orthographic orthographic_camera_from_json(nlohmann::json const& json)
	{
		return
		{
			.horizontal_magnification = get_value<float>(json, "xmag"),
			.vertical_magnification = get_value<float>(json, "ymag"),
			.near_z = get_value<float>(json, "znear"),
			.far_z = get_value<float>(json, "zfar"),
		};
	}

	Camera::Perspective perspective_camera_from_json(nlohmann::json const& json)
	{
		return
		{
			.aspect_ratio = get_optional_value<float>(json, "aspectRatio"),
			.vertical_field_of_view = get_value<float>(json, "yfov"),
			.near_z = get_value<float>(json, "znear"),
			.far_z = get_optional_value<float>(json, "zfar"),
		};
	}

	Camera camera_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator)
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
					.projection = orthographic_camera_from_json(*orthographic_location),
					.name = name,
				};
			}
			else
			{
				nlohmann::json::const_iterator const perspective_location = json.find("perspective");
				
				return
				{
					.type = type,
					.projection = perspective_camera_from_json(*perspective_location),
					.name = name,
				};
			}
		}
	}


	Maia::Scene::Scene scene_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator)
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
			.nodes = get_vector_from_json<Index>(json, "nodes", to_index, allocator),
			.name = get_optional_value<std::pmr::string>(json, "name", to_string),
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
		)
		{
			auto const to_value = [&element_from_json, &allocator](nlohmann::json const& json) -> Value_type
			{
				return element_from_json(json, allocator);
			};

			return get_vector_from_json<Value_type>(json, key, to_value, allocator);
		}
	}

	World gltf_from_json(nlohmann::json const& json, std::pmr::polymorphic_allocator<> const& allocator)
	{
		return
		{
			.accessors = get_allocated_elements_from_json(json, "accessors", accessor_from_json, allocator),
			.buffers = get_allocated_elements_from_json(json, "buffers", buffer_from_json, allocator),
			.buffer_views = get_allocated_elements_from_json(json, "bufferViews", buffer_view_from_json, allocator),
			.cameras = get_allocated_elements_from_json(json, "cameras", camera_from_json, allocator),
			.materials = get_allocated_elements_from_json(json, "materials", material_from_json, allocator),
			.meshes = get_allocated_elements_from_json(json, "meshes", mesh_from_json, allocator),
			.nodes = get_allocated_elements_from_json(json, "nodes", node_from_json, allocator),
			.scene_index = get_optional_value<Index>(json, "scene"),
			.scenes = get_allocated_elements_from_json(json, "scenes", scene_from_json, allocator),
		};
	}
}
