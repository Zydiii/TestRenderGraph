#pragma once

#include <memory>

namespace RG {
	/// <summary>
	/// δʵ��ʵ���� value Ϊ false
	/// </summary>
	/// <typeparam name="description_type">��Դ����</typeparam>
	/// <typeparam name="actual_type">ʵ������</typeparam>
	template<typename description_type, typename actual_type>
	struct missing_realize_implementation : std::false_type {};

	/// <summary>
	/// ����Ƿ�ʵ����ʵ����
	/// </summary>
	/// <typeparam name="description_type">��Դ����</typeparam>
	/// <typeparam name="actual_type">ʵ������</typeparam>
	/// <param name="description"></param>
	/// <returns></returns>
	template<typename description_type, typename actual_type>
	std::unique_ptr<actual_type> realize(const description_type& description) {
		static_assert(missing_realize_implementation<description_type, actual_type>::value, "Missing realize implementation!");
		return nullptr;
	}
}