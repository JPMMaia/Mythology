#ifndef MAIA_GAMEENGINE_TRANSFORMSYSTEM_H_INCLUDED
#define MAIA_GAMEENGINE_TRANSFORMSYSTEM_H_INCLUDED

#include <deque>
#include <functional>
#include <future>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <Maia/GameEngine/Entity.hpp>
#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/GameEngine/Components/Local_position.hpp>
#include <Maia/GameEngine/Components/Local_rotation.hpp>

namespace Maia::GameEngine::Systems
{
	// TODO move components to components folder

	struct Transform_tree_dirty
	{
		bool value{ true };
	};

	struct Transform_parent
	{
		Entity entity;
	};

	inline bool operator==(Transform_parent const& lhs, Transform_parent const& rhs)
	{
		return lhs.entity == rhs.entity;
	}
	inline bool operator!=(Transform_parent const& lhs, Transform_parent const& rhs)
	{
		return !(lhs == rhs);
	}


	struct Transform_root
	{
		Entity entity;
	};

	struct Transform_matrix
	{
		Eigen::Matrix4f value{ Eigen::Matrix4f::Identity() };
	};

	inline bool operator==(Transform_matrix const& lhs, Transform_matrix const& rhs)
	{
		return lhs.value.isApprox(rhs.value);
	}
	inline bool operator!=(Transform_matrix const& lhs, Transform_matrix const& rhs)
	{
		return !(lhs == rhs);
	}
	inline std::ostream& operator<<(std::ostream& outputStream, Transform_matrix const& value)
	{
		outputStream << value.value;
		return outputStream;
	}

	using Transforms_tree = std::unordered_multimap<Transform_parent, Entity>;
}

namespace std
{
	// TODO refactor

	template<> struct hash<Maia::GameEngine::Systems::Transform_parent>
	{
		using argument_type = Maia::GameEngine::Systems::Transform_parent;
		using result_type = std::size_t;

		result_type operator()(argument_type const& transform_parent) const noexcept
		{
			return std::hash<Maia::GameEngine::Entity::Integral_type>{}(transform_parent.entity.value);
		}
	};
}

namespace Maia::GameEngine::Systems
{
	using Local_position = Components::Local_position;
	using Local_rotation = Components::Local_rotation;

	Transform_matrix create_transform(Local_position const& position, Local_rotation const& rotation);

	Transforms_tree create_transforms_tree(
		Entity_manager const& entity_manager,
		Entity root_transform_entity
	);

	void update_child_transforms(
		Entity_manager& entity_manager,
		Transforms_tree const& transforms_tree,
		Entity root_transform_entity,
		Transform_matrix const& root_transform_matrix
	);


	class Transform_system
	{
	public:

		void execute(Entity_manager& entity_manager);

		// void execute(ThreadPool& thread_pool, Entity_manager& entity_manager);
		
		// std::future<void> execute_async(Entity_manager& entity_manager);
	};
}

#endif
