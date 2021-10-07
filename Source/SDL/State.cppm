module;

#include <memory>

export module mythology.sdl.state;

namespace Mythology::SDL
{
    export class State
    {
    public:

        virtual ~State() noexcept {};

        virtual std::unique_ptr<State> run() = 0;

    };
}
