export module mythology.core.utilities;

import <cassert>;
import <cstddef>;
import <cstring>;
import <filesystem>;
import <fstream>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Mythology::Core
{
    export std::pmr::vector<std::byte> read_bytes(std::filesystem::path const& file_path) noexcept
    {
        std::ifstream input_stream{file_path, std::ios::in | std::ios::binary};
        assert(input_stream.good());

        input_stream.seekg(0, std::ios::end);
        auto const size_in_bytes = input_stream.tellg();

        std::pmr::vector<std::byte> buffer;
        buffer.resize(size_in_bytes);

        input_stream.seekg(0, std::ios::beg);
        input_stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

        return buffer;
    }

    export template<typename Value_type>
    std::pmr::vector<Value_type> convert_bytes(std::span<std::byte const> const bytes) noexcept
    {
        assert(bytes.size_bytes() % sizeof(Value_type) == 0);

        std::pmr::vector<Value_type> values;
        values.resize(bytes.size_bytes() / sizeof(Value_type));

        std::memcpy(values.data(), bytes.data(), bytes.size_bytes());

        return values;
    }
}