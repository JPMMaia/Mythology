module;

#include <array>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <memory_resource>
#include <span>
#include <string_view>
#include <type_traits>
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

	std::ostream& operator<<(std::ostream& output_stream, Vector2f const value)
	{
		output_stream << std::format("{{{}, {}}}", value.x, value.y);

		return output_stream;
	}

	std::ostream& operator<<(std::ostream& output_stream, Vector3f const value)
	{
		output_stream << std::format("{{{}, {}, {}}}", value.x, value.y, value.z);

		return output_stream;
	}

	std::ostream& operator<<(std::ostream& output_stream, Vector4f const value)
	{
		output_stream << std::format("{{{}, {}, {}, {}}}", value.x, value.y, value.z, value.w);

		return output_stream;
	}

	std::ostream& operator<<(std::ostream& output_stream, Quaternionf const value)
	{
		output_stream << std::format("{{{}, {}, {}, {}}}", value.x, value.y, value.z, value.w);

		return output_stream;
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

	template <typename Data_type>
	std::pmr::vector<Data_type> read_accessor_buffer_view_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<std::pmr::vector<std::byte> const> const buffers_data,
		std::pmr::polymorphic_allocator<> const& allocator
	)
	{
		assert(accessor.buffer_view_index.has_value());
		assert((size_of(accessor.type) * size_of(accessor.component_type)) == sizeof(Data_type));

		Buffer_view const& buffer_view = buffer_views[*accessor.buffer_view_index];
		std::span<std::byte const> const buffer_data = buffers_data[buffer_view.buffer_index];

		std::pmr::vector<Data_type> accessor_data{ allocator };
		accessor_data.resize(accessor.count);

		if (buffer_view.byte_stride.has_value())
		{
			std::byte const* const buffer_data_start = buffer_data.data() + buffer_view.byte_offset + accessor.byte_offset;

			for (std::size_t element_index = 0; element_index < accessor_data.size(); ++element_index)
			{
				std::byte const* const source = buffer_data_start + element_index * *buffer_view.byte_stride;

				std::memcpy(
					&accessor_data[element_index],
					source,
					sizeof(Data_type)
				);
			}
		}
		else
		{
			std::memcpy(
				accessor_data.data(),
				buffer_data.data() + buffer_view.byte_offset + accessor.byte_offset,
				accessor_data.size() * sizeof(Data_type)
			);
		}

		return accessor_data;
	}

	template <class Data_type>
	std::pmr::vector<Data_type> read_sparse_accessor_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<std::pmr::vector<std::byte> const> const buffers_data,
		std::pmr::polymorphic_allocator<> const& allocator,
		std::pmr::polymorphic_allocator<> const& temporaries_allocator
	)
	{
		assert(accessor.sparse.has_value());

		std::pmr::vector<Data_type> accessor_data = [&]() -> std::pmr::vector<Data_type>
		{
			if (accessor.buffer_view_index.has_value())
			{
				return read_accessor_buffer_view_data<Data_type>(
					accessor,
					buffer_views,
					buffers_data,
					temporaries_allocator
					);
			}
			else
			{
				std::pmr::vector<Data_type> accessor_data{ allocator };
				accessor_data.resize(accessor.count, Data_type{});
				return accessor_data;
			}
		}();

		Sparse::Indices const& sparse_indices = accessor.sparse->indices;

		std::pmr::vector<std::uint32_t> const sparse_indices_data = read_indices_32_accessor_data(
			Accessor
			{
				.buffer_view_index = sparse_indices.buffer_view_index,
				.byte_offset = sparse_indices.byte_offset,
				.component_type = sparse_indices.component_type,
				.count = accessor.sparse->count,
				.type = Accessor::Type::Scalar,
			},
			buffer_views,
			buffers_data,
			temporaries_allocator,
			temporaries_allocator
			);

		Sparse::Values const& sparse_values = accessor.sparse->values;

		std::pmr::vector<Data_type> const sparse_values_data = read_accessor_buffer_view_data<Data_type>(
			Accessor
			{
				.buffer_view_index = sparse_values.buffer_view_index,
				.byte_offset = sparse_values.byte_offset,
				.component_type = accessor.component_type,
				.count = accessor.sparse->count,
				.type = accessor.type,
			},
			buffer_views,
			buffers_data,
			temporaries_allocator
			);

		for (std::size_t index = 0; index < accessor.sparse->count; ++index)
		{
			std::uint32_t const sparse_index = sparse_indices_data[index];
			Data_type const& sparse_value = sparse_values_data[index];

			accessor_data[sparse_index] = sparse_value;
		}

		return accessor_data;
	}

	template <class Data_type>
	std::pmr::vector<Data_type> read_accessor_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<std::pmr::vector<std::byte> const> const buffers_data,
		std::pmr::polymorphic_allocator<> const& allocator,
		std::pmr::polymorphic_allocator<> const& temporaries_allocator
	)
	{
		assert(accessor.sparse.has_value() || accessor.buffer_view_index.has_value());

		if (!accessor.sparse.has_value() && !accessor.buffer_view_index.has_value())
		{
			throw std::runtime_error{ "Accessor without buffer view and sparse data is not supported!" };
		}

		if (accessor.buffer_view_index.has_value())
		{
			Buffer_view const& buffer_view = buffer_views[*accessor.buffer_view_index];
			std::size_t const single_element_byte_length = size_of(accessor.type) * size_of(accessor.component_type);

			assert((accessor.byte_offset + buffer_view.byte_stride.value_or(0) * (accessor.count - 1) + single_element_byte_length) <= buffer_view.byte_length);
		}

		if (accessor.sparse.has_value())
		{
			return read_sparse_accessor_data<Data_type>(
				accessor,
				buffer_views,
				buffers_data,
				allocator,
				temporaries_allocator
				);
		}
		else
		{
			assert(accessor.buffer_view_index.has_value());

			return read_accessor_buffer_view_data<Data_type>(
				accessor,
				buffer_views,
				buffers_data,
				allocator
				);
		}
	}

	std::pmr::vector<Vector3f> read_vector_3_float_accessor_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<std::pmr::vector<std::byte> const> const buffers_data,
		std::pmr::polymorphic_allocator<> const& allocator,
		std::pmr::polymorphic_allocator<> const& temporaries_allocator
	)
	{
		assert(accessor.sparse || accessor.buffer_view_index.has_value());
		assert(accessor.component_type == Component_type::Float);
		assert(accessor.type == Accessor::Type::Vector3);

		return read_accessor_data<Vector3f>(
			accessor,
			buffer_views,
			buffers_data,
			allocator,
			temporaries_allocator
			);
	}

	std::pmr::vector<Vector3f> read_position_accessor_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<std::pmr::vector<std::byte> const> const buffers_data,
		std::pmr::polymorphic_allocator<> const& allocator,
		std::pmr::polymorphic_allocator<> const& temporaries_allocator
	)
	{
		return read_vector_3_float_accessor_data(accessor, buffer_views, buffers_data, allocator, temporaries_allocator);
	}

	std::pmr::vector<std::uint8_t> read_indices_8_accessor_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<std::pmr::vector<std::byte> const> const buffers_data,
		std::pmr::polymorphic_allocator<> const& allocator,
		std::pmr::polymorphic_allocator<> const& temporaries_allocator
	)
	{
		assert(accessor.sparse || accessor.buffer_view_index.has_value());
		assert(accessor.component_type == Component_type::Unsigned_byte);
		assert(accessor.type == Accessor::Type::Scalar);

		return read_accessor_data<std::uint8_t>(
			accessor,
			buffer_views,
			buffers_data,
			allocator,
			temporaries_allocator
			);
	}

	std::pmr::vector<std::uint16_t> read_indices_16_accessor_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<std::pmr::vector<std::byte> const> const buffers_data,
		std::pmr::polymorphic_allocator<> const& allocator,
		std::pmr::polymorphic_allocator<> const& temporaries_allocator
	)
	{
		assert(accessor.sparse || accessor.buffer_view_index.has_value());
		assert(accessor.component_type == Component_type::Unsigned_byte || accessor.component_type == Component_type::Unsigned_short);
		assert(accessor.type == Accessor::Type::Scalar);

		if (accessor.component_type == Component_type::Unsigned_byte)
		{
			std::pmr::vector<std::uint8_t> const indices_8 = read_indices_8_accessor_data(
				accessor,
				buffer_views,
				buffers_data,
				temporaries_allocator,
				temporaries_allocator
			);

			std::pmr::vector<std::uint16_t> indices_16{ allocator };
			indices_16.resize(accessor.count);

			std::copy(
				indices_8.begin(),
				indices_8.end(),
				indices_16.begin()
			);

			return indices_16;
		}
		else
		{
			assert(accessor.component_type == Component_type::Unsigned_short);

			return read_accessor_data<std::uint16_t>(
				accessor,
				buffer_views,
				buffers_data,
				allocator,
				temporaries_allocator
				);
		}
	}

	std::pmr::vector<std::uint32_t> read_indices_32_accessor_data(
		Accessor const& accessor,
		std::span<Buffer_view const> const buffer_views,
		std::span<std::pmr::vector<std::byte> const> const buffers_data,
		std::pmr::polymorphic_allocator<> const& allocator,
		std::pmr::polymorphic_allocator<> const& temporaries_allocator
	)
	{
		assert(accessor.sparse || accessor.buffer_view_index.has_value());
		assert((accessor.component_type == Component_type::Unsigned_byte) || (accessor.component_type == Component_type::Unsigned_short) || (accessor.component_type == Component_type::Unsigned_int));
		assert(accessor.type == Accessor::Type::Scalar);

		if (accessor.component_type == Component_type::Unsigned_byte)
		{
			std::pmr::vector<std::uint8_t> const indices_8 = read_indices_8_accessor_data(
				accessor,
				buffer_views,
				buffers_data,
				temporaries_allocator,
				temporaries_allocator
			);

			std::pmr::vector<std::uint32_t> indices_16{ allocator };
			indices_16.resize(accessor.count);

			std::copy(
				indices_8.begin(),
				indices_8.end(),
				indices_16.begin()
			);

			return indices_16;
		}
		else if (accessor.component_type == Component_type::Unsigned_short)
		{
			std::pmr::vector<std::uint16_t> const indices_16 = read_indices_16_accessor_data(
				accessor,
				buffer_views,
				buffers_data,
				temporaries_allocator,
				temporaries_allocator
			);

			std::pmr::vector<std::uint32_t> indices_32{ allocator };
			indices_32.resize(accessor.count);

			std::copy(
				indices_16.begin(),
				indices_16.end(),
				indices_32.begin()
			);

			return indices_32;
		}
		else
		{
			assert(accessor.component_type == Component_type::Unsigned_int);

			return read_accessor_data<std::uint32_t>(
				accessor,
				buffer_views,
				buffers_data,
				allocator,
				temporaries_allocator
				);
		}
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
