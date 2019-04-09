#ifndef MAIA_GAMEENGINE_COMPONENTSCHUNK_H_INCLUDED
#define MAIA_GAMEENGINE_COMPONENTSCHUNK_H_INCLUDED

#include <cstddef>
#include <type_traits>
#include <vector>

#include <gsl/span>

namespace Maia::GameEngine
{
	class Components_chunk
	{
	public:

		template <typename Component>
		using Reference = std::remove_const_t<std::remove_reference_t<Component>>&;

		template <typename Component>
		using Const_reference = std::remove_const_t<std::remove_reference_t<Component>> const&;


		std::byte* data();

		std::byte const* data() const;


		std::size_t size() const;


		void resize(std::size_t count, std::byte value);


		template <typename Component>
		Const_reference<Component> get_component_data(std::size_t component_offset, std::size_t component_index) const
		{
			auto pointer = m_data.data() + component_offset + component_index * sizeof(Component);
			return reinterpret_cast<Const_reference<Component>>(*pointer);
		}

		template <typename Component>
		Reference<Component> get_component_data(std::size_t component_offset, std::size_t component_index)
		{
			auto pointer = m_data.data() + component_offset + component_index * sizeof(Component);
			return reinterpret_cast<Reference<Component>>(*pointer);
		}

		template <typename Component>
		void set_component_data(std::size_t component_offset, std::size_t component_index, Component&& component)
		{
			get_component_data<Component>(component_offset, component_index) = std::forward<Component>(component);
		}


		template <typename Component>
		gsl::span<Component> components(std::size_t offset, std::ptrdiff_t count)
		{
			return { reinterpret_cast<Component*>(m_data.data() + offset), count };
		}

		template <typename Component>
		gsl::span<Component const> components(std::size_t offset, std::ptrdiff_t count) const
		{
			return { reinterpret_cast<Component const*>(m_data.data() + offset), count };
		}


	private:

		std::vector<std::byte> m_data;

	};
}

#endif
