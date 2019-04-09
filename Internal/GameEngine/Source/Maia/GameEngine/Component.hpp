#ifndef MAIA_GAMEENGINE_COMPONENT_H_INCLUDED
#define MAIA_GAMEENGINE_COMPONENT_H_INCLUDED

#include <cstdint>

namespace Maia::GameEngine
{
	struct Component_ID
	{
		std::uint16_t value;

	private:

		static Component_ID create_component_id();

		template <class Component>
		static Component_ID get_impl()
		{
			static Component_ID id = create_component_id();

			return id;
		}

	public:

		template <class Component>
		static Component_ID get()
		{
			using Raw_component = typename std::remove_cv_t<typename std::remove_reference_t<Component>>;

			return get_impl<Raw_component>();
		}
	};

	inline bool operator==(Component_ID lhs, Component_ID rhs)
	{
		return lhs.value == rhs.value;
	}

	inline bool operator!=(Component_ID lhs, Component_ID rhs)
	{
		return !(lhs == rhs);
	}


	struct Component_size
	{
		std::uint16_t value;
	};


	struct Component_info
	{
		Component_ID id;
		Component_size size;
	};

	template <class Component>
	Component_info create_component_info()
	{
		return 
		{
			Component_ID::get<Component>(),
			{ sizeof(Component) }
		};
	}
}

#endif
