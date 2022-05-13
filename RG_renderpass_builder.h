#pragma once

#include <string>

namespace RG {
	class RenderGraph;
	class RG_renderpass_base;

	/// <summary>
	/// 在 render graph 中构建 render pass
	/// </summary>
	class RG_renderpass_builder {
	public:
		explicit RG_renderpass_builder(RenderGraph* randergraph, RG_renderpass_base* renderpass)
			: rendergraph_(randergraph), renderpass_(renderpass) {

		}

		virtual ~RG_renderpass_builder() = default;

		template<typename resource_type, typename description_type>
		resource_type* create(const std::string& name, const description_type& description); // 创建资源
		template<typename resource_type>
		resource_type* read(resource_type* resource); // 读取资源
		template<typename resource_type>
		resource_type* write(resource_type* resource); // 写入资源
	protected:
		RenderGraph* rendergraph_;
		RG_renderpass_base* renderpass_;
	};
}

