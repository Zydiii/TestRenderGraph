#pragma once

#include <string>
#include <vector>

namespace RG {
	class RenderGraph;
	class RG_renderpass_builder;
	class RG_resource_base;

	/// <summary>
	/// render pass 基类
	/// </summary>
	class RG_renderpass_base {
	public:
		explicit RG_renderpass_base(const std::string& name) 
			: name_(name), cull_(false), ref_count_(0) {

		}

		virtual ~RG_renderpass_base() = default;

		const std::string& name() const {
			return name_;
		}

		void set_name(const std::string& name) {
			name_ = name;
		}

		bool cull() const {
			return cull;
		}

		void set_cull(const bool cull) {
			cull_ = cull;
		}
	protected:
		friend RenderGraph;
		friend RG_renderpass_builder;

		virtual void setup(RG_renderpass_builder& builder) = 0; // 构建
		virtual void execute() const = 0;  // 执行

		std::string name_; // 名称
		bool cull_; // 是否可以被剔除
		std::vector<const RG_resource_base*> creates_; // 创建的资源
		std::vector<const RG_resource_base*> reads_; // 读取的资源
		std::vector<const RG_resource_base*> writes_; // 写入的资源
		std::size_t ref_count_; // 引用计数
	};
}