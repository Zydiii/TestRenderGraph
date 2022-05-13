#pragma once

#include <variant>
#include <memory>
#include <string>

#include "RG_resource_base.h"
#include "RG_resource_realize.h"

namespace RG {
	class RG_renderpass_base;

	/// <summary>
	/// 资源类
	/// </summary>
	/// <typeparam name="description_type_">资源描述</typeparam>
	/// <typeparam name="actual_type_">实际类型</typeparam>
	template<typename description_type_, typename actual_type_>
	class RG_resource : public RG_resource_base {
	public:
		explicit RG_resource(const std::string& name, const RG_renderpass_base* creator, const description_type_& description)
			: RG_resource_base(name, creator), description_type(description), actual_type(std::unique_ptr<actual_type_>()) {

		}

		explicit RG_resource(const std::string& name, const description_type_& description, actual_type_* actual = nullptr)
			: RG_resource_base(name, nullptr), description_type(description), actual_type(actual) {
			if (!actual)
				actual_type = RG::realize<description_type_, actual_type_>(description_type);
		}

		~RG_resource() = default;

		const description_type_& description() const {
			return description_type;
		}

		actual_type_* actual() const {
			return std::holds_alternative<std::unique_ptr<actual_type_>>(actual_type) ?
				std::get<std::unique_ptr<actual_type_>>(actual_type).get() : std::get<actual_type_*>(actual_type);
		}
	protected:
		void realize() override {
			if(transient())
				std::get<std::unique_ptr<actual_type_>>(actual_type) = RG::realize<description_type_, actual_type_>(description_type);
		}

		void derealize() override {
			if(transient())
				std::get<std::unique_ptr<actual_type_>>(actual_type).reset();
		}

		description_type_ description_type; // 资源描述
		std::variant<std::unique_ptr<actual_type_>, actual_type_*> actual_type; // 实际类型
	};
}