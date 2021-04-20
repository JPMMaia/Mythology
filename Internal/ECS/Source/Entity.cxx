module;

#include <cstddef>
#include <functional>
#include <iosfwd>

export module maia.ecs.entity;

namespace Maia::ECS
{
	/**
	 * @brief Represents a general purpose object.
	 * 
	 * It consists of only one unique ID. A set of components is usually associated with an entity.
	 */
	export struct Entity
	{
		/**
		 * @brief Type of the entity's ID.
		 * 
		 */
		using Integral_type = std::size_t;

		/**
		 * @brief ID of the entity.
		 * 
		 */
		Integral_type index{0};
	};

	/**
	 * @brief Return true if entities are equal.
	 * 
	 * @param lhs Left-hand side entity.
	 * @param rhs Right-hand side entity.
	 * @return True if entities are equal, false otherwise.
	 */
	export bool operator==(Entity lhs, Entity rhs) noexcept;

	/**
	 * @brief Return true if entities are distinct.
	 * 
	 * @param lhs Left-hand side entity.
	 * @param rhs Right-hand side entity.
	 * @return True if entities are distinct, false otherwise. 
	 */
	export bool operator!=(Entity lhs, Entity rhs) noexcept;

	/**
	 * @brief Print user-friendly representation of an entity.
	 * 
	 * @param output_stream Stream to output the entity's representation.
	 * @param value Entity to print.
	 * @return The \p output_stream argument.
	 */
	export std::ostream& operator<<(std::ostream& output_stream, Entity value) noexcept;

	/**
	 * @brief Compute an hash of an entity, so that they can be used in hash containers.
	 * 
	 */
	export struct Entity_hash
	{
		using argument_type = Maia::ECS::Entity;
		using result_type = std::size_t;

		result_type operator()(argument_type const& entity) const noexcept
		{
			return std::hash<Maia::ECS::Entity::Integral_type>{}(entity.index);
		}
	};
}
