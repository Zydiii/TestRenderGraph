#pragma once

#include <memory>

namespace RG {
	/// <summary>
	/// 未实现实例化 value 为 false
	/// </summary>
	/// <typeparam name="description_type">资源描述</typeparam>
	/// <typeparam name="actual_type">实际类型</typeparam>
	template<typename description_type, typename actual_type>
	struct missing_realize_implementation : std::false_type {};

	/// <summary>
	/// 检查是否实现了实例化
	/// </summary>
	/// <typeparam name="description_type">资源描述</typeparam>
	/// <typeparam name="actual_type">实际类型</typeparam>
	/// <param name="description"></param>
	/// <returns></returns>
	template<typename description_type, typename actual_type>
	std::unique_ptr<actual_type> realize(const description_type& description) {
		static_assert(missing_realize_implementation<description_type, actual_type>::value, "Missing realize implementation!");
		return nullptr;
	}
}