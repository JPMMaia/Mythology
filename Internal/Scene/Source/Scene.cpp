module;

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <memory_resource>
#include <span>
#include <string_view>
#include <variant>

module maia.scene;

namespace Maia::Scene
{
	std::uint8_t size_of(Component_type const component_type) noexcept
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
		default:
			assert(false);
			return 1;
		}
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

			std::pmr::vector<std::byte> output{ allocator };
			output.reserve(output_size);

			{
				std::uint32_t bits{ 0 };
				std::uint8_t bit_count{ 0 };

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
				char const* const base64_prefix{ "data:application/octet-stream;base64," };
				std::size_t const base64_prefix_size{ std::strlen(base64_prefix) };
				assert((uri.compare(0, base64_prefix_size, base64_prefix) == 0) && "Uri format not supported");

				std::string_view const data_view{ uri.data() + base64_prefix_size, uri.size() - base64_prefix_size };
				return decode_base64(data_view, byte_length, allocator);
			}
			else
			{
				std::filesystem::path const file_path{ prefix_path / uri };
				assert(std::filesystem::exists(file_path) && "Couldn't open file");
				assert(std::filesystem::file_size(file_path) == byte_length);

				std::pmr::vector<std::byte> file_content{ allocator };
				file_content.resize(byte_length);

				{
					std::basic_ifstream<std::byte> file_stream{ file_path, std::ios::binary };
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
		std::filesystem::path const& prefix_path,
		std::pmr::polymorphic_allocator<> const& allocator
	)
	{
		assert(buffer.uri.has_value());

		return generate_byte_data(*buffer.uri, prefix_path, buffer.byte_length, allocator);
	}

	std::pmr::vector<std::byte> read_buffer_view_data(
		Buffer_view const& buffer_view,
		std::span<Buffer const> const buffers,
		std::filesystem::path const& prefix_path,
		std::pmr::polymorphic_allocator<> const& allocator
	)
	{
		return {};
	}

	std::uint8_t size_of(Accessor::Type const accessor_type) noexcept
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
		default:
			assert(false);
			return 1;
		}
	}

	std::pmr::vector<std::byte> read_accessor_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<Buffer const> const buffers,
		std::filesystem::path const& prefix_path,
		std::pmr::polymorphic_allocator<> const& allocator
	)
	{
		return {};
	}

	std::size_t Attribute_hash::operator() (Attribute const attribute) const noexcept
	{
		constexpr std::size_t maximum_index = 100;
		assert(attribute.index < maximum_index);

		std::size_t const type = static_cast<std::size_t>(attribute.type);
		Index const index = attribute.index;

		return type * maximum_index + index;
	}

	bool operator==(
		std::variant<Camera::Orthographic, Camera::Perspective> const& lhs,
		std::variant<Camera::Orthographic, Camera::Perspective> const& rhs
		) noexcept
	{
		if (lhs.index() != rhs.index())
		{
			return false;
		}

		if (lhs.index() == 0)
		{
			Camera::Orthographic const& lhs_orthographic = std::get<Camera::Orthographic>(lhs);
			Camera::Orthographic const& rhs_orthographic = std::get<Camera::Orthographic>(rhs);

			return lhs_orthographic == rhs_orthographic;
		}
		else
		{
			Camera::Perspective const& lhs_perspective = std::get<Camera::Perspective>(lhs);
			Camera::Perspective const& rhs_perspective = std::get<Camera::Perspective>(rhs);

			return lhs_perspective == rhs_perspective;
		}
	}

	bool operator!=(
		std::variant<Camera::Orthographic, Camera::Perspective> const& lhs,
		std::variant<Camera::Orthographic, Camera::Perspective> const& rhs
		) noexcept
	{
		return !(lhs == rhs);
	}
}
