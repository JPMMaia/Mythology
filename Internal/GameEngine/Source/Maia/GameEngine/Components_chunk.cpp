#include "Components_chunk.hpp"

namespace Maia::GameEngine
{
	std::byte* Components_chunk::data()
	{
		return m_data.data();
	}

	std::byte const* Components_chunk::data() const
	{
		return m_data.data();
	}

	std::size_t Components_chunk::size() const
	{
		return m_data.size();
	}


	void Components_chunk::resize(std::size_t const count, std::byte const value)
	{
		m_data.resize(count, value);
	}
}
