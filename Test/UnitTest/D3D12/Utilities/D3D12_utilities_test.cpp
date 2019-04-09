#include <gtest/gtest.h>

#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

using namespace Maia::Renderer;

namespace Maia::Renderer::D3D12::Unit_test
{
	class D3D12_utilities_test : public ::testing::Test
	{
	};

	TEST_F(D3D12_utilities_test, Should_create_factory)
	{
		auto const factory = D3D12::create_factory({});
		
		EXPECT_NE(factory, nullptr);
	}

	TEST_F(D3D12_utilities_test, Should_select_software_adapter_when_select_WARP_adapter_is_true)
	{
		auto const factory = D3D12::create_factory({});
		auto constexpr select_WARP_adapter = true;
		auto const adapter = D3D12::select_adapter(*factory, select_WARP_adapter);
		DXGI_ADAPTER_DESC1 description;
		winrt::check_hresult(adapter->GetDesc1(&description));

		ASSERT_NE(adapter, nullptr);
		EXPECT_TRUE(description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE);
	}
	TEST_F(D3D12_utilities_test, Should_not_select_software_adapter_when_select_WARP_adapter_is_false)
	{
		auto const factory = D3D12::create_factory({});
		auto constexpr select_WARP_adapter = false;
		auto const adapter = D3D12::select_adapter(*factory, select_WARP_adapter);
		DXGI_ADAPTER_DESC1 description;
		winrt::check_hresult(adapter->GetDesc1(&description));

		ASSERT_NE(adapter, nullptr);
		EXPECT_FALSE(description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE);
	}

	TEST_F(D3D12_utilities_test, Should_create_device)
	{
		auto const factory = D3D12::create_factory({});
		auto constexpr select_WARP_adapter = true;
		auto const adapter = D3D12::select_adapter(*factory, select_WARP_adapter);

		auto const device = D3D12::create_device(*adapter, D3D_FEATURE_LEVEL_11_0);

		EXPECT_NE(device, nullptr);
	}

	TEST_F(D3D12_utilities_test, Should_create_command_allocator)
	{
		auto const factory = D3D12::create_factory({});
		auto constexpr select_WARP_adapter = true;
		auto const adapter = D3D12::select_adapter(*factory, select_WARP_adapter);
		auto const device = D3D12::create_device(*adapter, D3D_FEATURE_LEVEL_11_0);

		auto const command_allocator = D3D12::create_command_allocator(*device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		EXPECT_NE(command_allocator, nullptr);
	}
	
	TEST_F(D3D12_utilities_test, Should_create_opened_command_list)
	{
		auto const factory = D3D12::create_factory({});
		auto constexpr select_WARP_adapter = true;
		auto const adapter = D3D12::select_adapter(*factory, select_WARP_adapter);
		auto const device = D3D12::create_device(*adapter, D3D_FEATURE_LEVEL_11_0);
		auto const command_allocator = D3D12::create_command_allocator(*device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		auto const command_list = D3D12::create_opened_graphics_command_list(*device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, *command_allocator, nullptr);
		
		EXPECT_NE(command_list, nullptr);
		EXPECT_EQ(S_OK, command_list->Close());
	}

	TEST_F(D3D12_utilities_test, Should_create_closed_command_list)
	{
		auto const factory = D3D12::create_factory({});
		auto constexpr select_WARP_adapter = true;
		auto const adapter = D3D12::select_adapter(*factory, select_WARP_adapter);
		auto const device = D3D12::create_device(*adapter, D3D_FEATURE_LEVEL_11_0);
		auto const command_allocator = D3D12::create_command_allocator(*device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		auto const command_list = D3D12::create_closed_graphics_command_list(*device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, *command_allocator, nullptr);

		EXPECT_NE(command_list, nullptr);
		EXPECT_EQ(E_FAIL, command_list->Close());
	}
}
