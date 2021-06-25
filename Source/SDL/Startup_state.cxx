module;

#include <filesystem>
#include <memory>
#include <memory_resource>
#include <string>
#include <unordered_map>

export module mythology.sdl.startup_state;

import mythology.sdl.state;

namespace Mythology::SDL
{
    export class Startup_state final : public State
    {
    public:

        explicit Startup_state(
            std::pmr::unordered_map<std::pmr::string, std::filesystem::path> render_pipelines
        ) noexcept;
        ~Startup_state() noexcept final;

        std::unique_ptr<State> run() final;


    private:

        std::pmr::unordered_map<std::pmr::string, std::filesystem::path> m_render_pipelines;
    };
}
