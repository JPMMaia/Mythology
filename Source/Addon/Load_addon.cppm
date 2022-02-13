module;

#include "Addon_interface.hpp"

#if _WIN32
#include <windows.h>
#include <libloaderapi.h>
#endif

#include <filesystem>
#include <functional>
#include <memory>

export module mythology.load_addon;

import mythology.addon_interface;

namespace Mythology
{
    class Native_library
    {
    public:

#if _WIN32
        using Handle = HMODULE;
#else
        using Handle = void*;
#endif

        Native_library(std::filesystem::path const& addon_path) noexcept;
        Native_library(Native_library const&) = delete;
        Native_library(Native_library&& other) noexcept;
        ~Native_library() noexcept;

        Native_library& operator=(Native_library const&) = delete;
        Native_library& operator=(Native_library&& other) noexcept;

        bool is_valid() const noexcept;

        Handle handle() const noexcept;

    private:

        Handle m_handle;
    };

    export struct Addon
    {
        Native_library library;
        Create_addon_interface_function_pointer create_addon_interface;
        Destroy_addon_interface_function_pointer destroy_addon_interface;
    };

    export std::optional<Addon> load_addon(std::filesystem::path const& addon_path);

    using Addon_interface_pointer = std::unique_ptr<Addon_interface, Destroy_addon_interface_function_pointer>;

    export Addon_interface_pointer create_addon_interface(
        Create_addon_interface_function_pointer create_addon_interface,
        Destroy_addon_interface_function_pointer destroy_addon_interface
    );
}

