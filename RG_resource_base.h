#pragma once

#include <string>
#include <vector>

namespace RG {
	class RenderGraph;
	class RG_renderpass_base;
	class RG_renderpass_builder;

	/// <summary>
	/// 资源基类
	/// </summary>
	class RG_resource_base {
	public:
		explicit RG_resource_base(const std::string& name, const RG_renderpass_base* creator)
			: name_(name), creator_(creator), ref_count_(0)
		{
			static std::size_t id = 0;
			id_ = id++;
		}
		virtual ~RG_resource_base() = default;

		std::size_t id() const {
			return id_;
		}

		const std::string& name() const {
			return name_;
		}

		void set_name(const std::string& name) {
			name_ = name;
		}

	protected:
		friend RenderGraph;
		friend RG_renderpass_builder;

		virtual void realize() = 0; // 实例化
		virtual void derealize() = 0; // 释放资源

		std::size_t id_; // 编号
		std::string name_; // 名称
		std::size_t ref_count_; // 引用计数
		const RG_renderpass_base* creator_; // 资源创建者
		std::vector<const RG_renderpass_base*> readers_; // 资源读取者
		std::vector<const RG_renderpass_base*> writers_; // 资源写入者
	};
}