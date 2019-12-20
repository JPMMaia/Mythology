export module maia.ecs.systems.transform_system;

import maia.ecs.entity;
import maia.ecs.entity_manager;
import maia.ecs.components.local_position;
import maia.ecs.components.local_rotation;

import <Eigen/Core>;
import <Eigen/Geometry>;

import <functional>;
import <future>;
import <memory_resource>;
import <unordered_map>;

namespace Maia::ECS::Systems
{
	export struct Transform_tree_dirty
	{
		bool value{ true };
	};

	export struct Transform_parent
	{
		Entity entity;
	};

	export inline bool operator==(Transform_parent const& lhs, Transform_parent const& rhs) noexcept
	{
		return lhs.entity == rhs.entity;
	}
	export inline bool operator!=(Transform_parent const& lhs, Transform_parent const& rhs) noexcept
	{
		return !(lhs == rhs);
	}


	export struct Transform_root
	{
		Entity entity;
	};

	export struct Transform_matrix
	{
		Eigen::Matrix4f value{ Eigen::Matrix4f::Identity() };
	};

	export inline bool operator==(Transform_matrix const& lhs, Transform_matrix const& rhs) noexcept
	{
		return lhs.value.isApprox(rhs.value);
	}
	export inline bool operator!=(Transform_matrix const& lhs, Transform_matrix const& rhs) noexcept
	{
		return !(lhs == rhs);
	}
	export inline std::ostream& operator<<(std::ostream& outputStream, Transform_matrix const& value) noexcept
	{
		outputStream << value.value;
		return outputStream;
	}

	export using Transforms_tree = std::pmr::unordered_multimap<Transform_parent, Entity>;
}

namespace std
{
	export template<> 
	struct hash<Maia::ECS::Systems::Transform_parent>
	{
		using argument_type = Maia::ECS::Systems::Transform_parent;
		using result_type = std::size_t;

		result_type operator()(argument_type const& transform_parent) const noexcept
		{
			return std::hash<Maia::ECS::Entity::Integral_type>{}(transform_parent.entity.value);
		}
	};
}

namespace Maia::ECS::Systems
{
	export using Local_position = Components::Local_position;
	export using Local_rotation = Components::Local_rotation;

	export Transform_matrix create_transform(
		Local_position const& position,
		Local_rotation const& rotation
	) noexcept;

	export Transforms_tree create_transforms_tree(
		Entity_manager const& entity_manager,
		Entity root_transform_entity
	) noexcept;

	export void update_child_transforms(
		Entity_manager& entity_manager,
		Transforms_tree const& transforms_tree,
		Entity root_transform_entity,
		Transform_matrix const& root_transform_matrix
	) noexcept;


	export class Transform_system
	{
	public:

		void execute(Entity_manager& entity_manager) noexcept;

		// void execute(ThreadPool& thread_pool, Entity_manager& entity_manager);
		
		// std::future<void> execute_async(Entity_manager& entity_manager);
	};
}
