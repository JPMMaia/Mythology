module;

#include "Addon_interface.hpp"

#if _WIN32
#include <windows.h>
#include <libloaderapi.h>
#else
#include <dlfcn.h>
#endif

#include <cassert>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>

module mythology.load_addon;

import mythology.addon_interface;

namespace Mythology
{
    namespace
    {
        Native_library::Handle null_handle() noexcept
        {
#ifdef _WIN32
            return NULL;
#else
            return nullptr;
#endif
        }

        Native_library::Handle load_library(std::filesystem::path const& addon_path) noexcept
        {
#ifdef _WIN32
            return ::LoadLibraryExW(addon_path.c_str(), NULL, 0);
#else
            return ::dlopen(addon_path.c_str(), RTLD_LAZY);
#endif
        }

        void free_library(Native_library::Handle const handle) noexcept
        {
            if (handle != null_handle())
            {
#ifdef _WIN32
                ::FreeLibrary(handle);
#else
                ::destroy(handle);
#endif
            }
        }

        void* get_process_address(Native_library::Handle const handle, char const* const function_name) noexcept
        {
#ifdef _WIN32
            return ::GetProcAddress(handle, function_name);
#else
            return ::dlsym(handle, function_name);
#endif
        }
    }

    Native_library::Native_library(std::filesystem::path const& addon_path) noexcept :
        m_handle{ load_library(addon_path) }
    {
    }

    Native_library::Native_library(Native_library&& other) noexcept :
        m_handle{ std::exchange(other.m_handle, null_handle()) }
    {
    }

    Native_library::~Native_library() noexcept
    {
        free_library(m_handle);
    }

    Native_library& Native_library::operator=(Native_library&& other) noexcept
    {
        m_handle = std::exchange(other.m_handle, null_handle());
        return *this;
    }

    bool Native_library::is_valid() const noexcept
    {
        return m_handle != null_handle();
    }

    Native_library::Handle Native_library::handle() const noexcept
    {
        return m_handle;
    }

    std::optional<Addon> load_addon(std::filesystem::path const& addon_path)
    {
        Native_library library{ addon_path };

        if (library.is_valid())
        {
            Create_addon_interface_function_pointer const create_addon_interface =
                reinterpret_cast<Create_addon_interface_function_pointer>(
                    get_process_address(library.handle(), "create_addon_interface")
                    );

            if (create_addon_interface == nullptr)
            {
                std::cerr << "Failed to load create_addon_interface from " << addon_path << '\n';
                return {};
            }

            Destroy_addon_interface_function_pointer const destroy_addon_interface =
                reinterpret_cast<Destroy_addon_interface_function_pointer>(
                    get_process_address(library.handle(), "destroy_addon_interface")
                    );

            if (destroy_addon_interface == nullptr)
            {
                std::cerr << "Failed to load destroy_addon_interface from " << addon_path << '\n';
                return {};
            }

            return Addon
            {
                .library = std::move(library),
                .create_addon_interface = create_addon_interface,
                .destroy_addon_interface = destroy_addon_interface,
            };
        }
        else
        {
            std::cerr << "Could not load " << addon_path << '\n';
            return {};
        }
    }

    Addon_interface_pointer create_addon_interface(
        Create_addon_interface_function_pointer const create_addon_interface,
        Destroy_addon_interface_function_pointer const destroy_addon_interface
    )
    {
        assert(create_addon_interface != nullptr && destroy_addon_interface != nullptr);

        return
        {
            create_addon_interface(),
            destroy_addon_interface
        };
    }
}
