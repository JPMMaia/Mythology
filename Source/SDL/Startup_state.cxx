module;

#include <memory>

export module mythology.sdl.startup_state;

import mythology.sdl.state;

namespace Mythology::SDL
{
    export class Startup_state final : public State
    {
    public:

        ~Startup_state() noexcept final;

        std::unique_ptr<State> run() final;

    };
}
