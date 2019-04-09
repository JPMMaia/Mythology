#include "gltf.hpp"

namespace nlohmann
{
	template <>
	struct adl_serializer<Eigen::Vector3f>
	{
		static void to_json(json& j, Eigen::Vector3f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Eigen::Vector3f& value)
		{
			for (std::size_t i = 0; i < 3; ++i)
			{
				value(i) = j.at(i).get<float>();
			}
		}
	};

	template <>
	struct adl_serializer<Eigen::Vector4f>
	{
		static void to_json(json& j, Eigen::Vector4f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Eigen::Vector4f& value)
		{
			for (std::size_t i = 0; i < 4; ++i)
			{
				value(i) = j.at(i).get<float>();
			}
		}
	};

	template <>
	struct adl_serializer<Eigen::Quaternionf>
	{
		static void to_json(json& j, Eigen::Quaternionf const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Eigen::Quaternionf& value)
		{
			value.x() = j.at(0).get<float>();
			value.y() = j.at(1).get<float>();
			value.z() = j.at(2).get<float>();
			value.w() = j.at(3).get<float>();
		}
	};

	template <>
	struct adl_serializer<Eigen::Matrix4f>
	{
		static void to_json(json& j, Eigen::Matrix4f const& value)
		{
			assert(false);
		}

		static void from_json(const json& j, Eigen::Matrix4f& value)
		{
			for (std::size_t i = 0; i < 16; ++i)
			{
				value(i) = j.at(i).get<float>();
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
		void get_to_if_exists(nlohmann::json const& json, std::string_view key, std::optional<Value_type>& value)
		{
			nlohmann::json::const_iterator const location = json.find(key);

			if (location != json.end())
				value = location->get<Value_type>();
			else
				value = {};
		}

		template <class Value_type>
		void replace_default_if_exists(nlohmann::json const& json, std::string_view key, Value_type& value)
		{
			nlohmann::json::const_iterator const location = json.find(key);

			if (location != json.end())
				value = location->get<Value_type>();
		}
	}


	std::uint8_t size_of(Component_type component_type)
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


	void from_json(nlohmann::json const& json, Accessor& value)
	{
		get_to_if_exists(json, "bufferView", value.buffer_view_index);
		json.at("componentType").get_to(value.component_type);
		json.at("count").get_to(value.count);
		json.at("type").get_to(value.type);
		
		if (value.type == Accessor::Type::Vector3)
		{
			get_to_if_exists(json, "max", value.max);
			get_to_if_exists(json, "min", value.min);
		}
	}
	std::uint8_t size_of(Accessor::Type accessor_type)
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


	void from_json(nlohmann::json const& json, Buffer& value)
	{
		get_to_if_exists(json, "uri", value.uri);
		json.at("byteLength").get_to(value.byte_length);
	}


	void from_json(nlohmann::json const& json, Buffer_view& value)
	{
		json.at("buffer").get_to(value.buffer_index);
		replace_default_if_exists(json, "byteOffset", value.byte_offset);
		json.at("byteLength").get_to(value.byte_length);
	}


	void from_json(nlohmann::json const& json, PbrMetallicRoughness& value)
	{
		replace_default_if_exists(json, "baseColorFactor", value.base_color_factor);
		replace_default_if_exists(json, "metallicFactor", value.metallic_factor);
		replace_default_if_exists(json, "roughnessFactor", value.roughness_factor);
	}


	void from_json(nlohmann::json const& json, Material& value)
	{
		get_to_if_exists(json, "name", value.name);
		replace_default_if_exists(json, "pbrMetallicRoughness", value.pbr_metallic_roughness);
		replace_default_if_exists(json, "emissiveFactor", value.emissive_factor);
		replace_default_if_exists(json, "alphaMode", value.alpha_mode);
		replace_default_if_exists(json, "alphaCutoff", value.alpha_cutoff);
		replace_default_if_exists(json, "doubleSided", value.double_sided);
	}


	void from_json(nlohmann::json const& json, Primitive& value)
	{
		json.at("attributes").get_to(value.attributes);
		get_to_if_exists(json, "indices", value.indices_index);
		get_to_if_exists(json, "material", value.material_index);
	}


	void from_json(nlohmann::json const& json, Mesh& value)
	{
		get_to_if_exists<std::string>(json, "name", value.name);
		json.at("primitives").get_to(value.primitives);
	}


	namespace
	{
		struct Transform
		{
			Eigen::Vector3f translation;
			Eigen::Quaternionf rotation;
			Eigen::Vector3f scale;
		};

		Transform decompose(Eigen::Matrix4f const& matrix)
		{
			const Eigen::Affine3f transform{ matrix };

			Eigen::Matrix3f rotationMatrix;
			Eigen::Matrix3f scaleMatrix;
			transform.computeRotationScaling(&rotationMatrix, &scaleMatrix);

			const Eigen::Vector3f translation{ transform.translation() };
			const Eigen::Quaternionf rotation{ rotationMatrix };
			const Eigen::Vector3f scale{ scale(0, 0), scale(1, 1), scale(2, 2) };

			return { translation, rotation, scale };
		}
	}

	void from_json(nlohmann::json const& json, Node& value)
	{
		nlohmann::json::const_iterator const matrixLocation = json.find("matrix");

		if (matrixLocation != json.end())
		{
			assert(json.find("rotation") == json.end());
			assert(json.find("scale") == json.end());
			assert(json.find("translation") == json.end());

			const Eigen::Matrix4f matrix = matrixLocation->get<Eigen::Matrix4f>();

			const Transform transform = decompose(matrix);
			value.translation = transform.translation;
			value.rotation = transform.rotation;
			value.scale = transform.scale;
		}
		else
		{
			replace_default_if_exists(json, "rotation", value.rotation);
			replace_default_if_exists(json, "scale", value.scale);
			replace_default_if_exists(json, "translation", value.translation);
		}

		get_to_if_exists(json, "mesh", value.mesh_index);
		get_to_if_exists(json, "camera", value.camera_index);
		get_to_if_exists(json, "children", value.child_indices);
		get_to_if_exists(json, "name", value.name);
	}


	void from_json(nlohmann::json const& json, Camera::Orthographic& value)
	{
		json.at("xmag").get_to(value.horizontal_magnification);
		json.at("ymag").get_to(value.vertical_magnification);
		json.at("znear").get_to(value.near_z);
		json.at("zfar").get_to(value.far_z);
	}

	void from_json(nlohmann::json const& json, Camera::Perspective& value)
	{
		get_to_if_exists(json, "aspectRatio", value.aspect_ratio);
		json.at("yfov").get_to(value.vertical_field_of_view);
		json.at("znear").get_to(value.near_z);
		get_to_if_exists(json, "zfar", value.far_z);
	}

	void from_json(nlohmann::json const& json, Camera& value)
	{
		json.at("type").get_to(value.type);
		get_to_if_exists(json, "name", value.name);
		
		{
			nlohmann::json::const_iterator const orthographic_location = json.find("orthographic");

			if (orthographic_location != json.end())
			{
				value.projection = orthographic_location->get<Camera::Orthographic>();
			}
			else
			{
				nlohmann::json::const_iterator const perspective_location = json.find("perspective");

				if (perspective_location == json.end())
					throw std::invalid_argument{ "Either orthographic or perspective must be defined!" };

				value.projection = perspective_location->get<Camera::Perspective>();
			}
		}
	}


	void from_json(nlohmann::json const& json, Scene& value)
	{
		get_to_if_exists(json, "name", value.name);
		get_to_if_exists(json, "nodes", value.nodes);
	}


	void from_json(nlohmann::json const& json, Gltf& value)
	{
		get_to_if_exists(json, "accessors", value.accessors);
		get_to_if_exists(json, "buffers", value.buffers);
		get_to_if_exists(json, "bufferViews", value.buffer_views);
		get_to_if_exists(json, "cameras", value.cameras);
		get_to_if_exists(json, "materials", value.materials);
		get_to_if_exists(json, "meshes", value.meshes);
		get_to_if_exists(json, "nodes", value.nodes);
		get_to_if_exists(json, "scene", value.scene_index);
		get_to_if_exists(json, "scenes", value.scenes);
	}
}
