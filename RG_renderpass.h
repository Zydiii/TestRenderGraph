#pragma once

#include <functional>
#include <string>

#include "RG_renderpass_base.h"

namespace RG {
	class RG_renderpass_builder;

	/// <summary>
	/// render pass
	/// </summary>
	/// <typeparam name="resource_type_">资源类型</typeparam>
	template<typename resource_type_>
	class RG_renderpass : public RG_renderpass_base {
	public:
		explicit RG_renderpass(const std::string& name, const std::function<void(resource_type_&, RG_renderpass_builder&)>& setup, const std::function<void(const resource_type_&)>& execute)
			: RG_renderpass_base(name), setup_(setup), execute_(execute) {

		}

		virtual ~RG_renderpass() = default;

		const resource_type_& data() const {
			return resource_;
		}

	protected:
		void setup(RG_renderpass_builder& builder) override {
			setup_(resource_, builder);
		}

		void execute() const override {
			execute_(resource_);
		}

		resource_type_ resource_; // 资源
		const std::function<void(resource_type_&, RG_renderpass_builder&)> setup_; // 构建函数
		const std::function<void(const resource_type_&)> execute_; // 执行函数
	};
}
