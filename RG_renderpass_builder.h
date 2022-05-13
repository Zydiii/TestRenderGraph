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

	template<typename resource_type, typename description_type>
	resource_type* RG_renderpass_builder::create(const std::string& name, const description_type& description) {
		static_assert(std::is_same<typename resource_type::description_type, description_type>::value, "Description does not match resources.");
		rendergraph_->resources_.emplace_back(std::make_unique<resource_type>(name, renderpass_, description));
		const auto resource = rendergraph_->resources_.back().get();
		renderpass_->creates_.push_back(resource);
		return static_cast<resource_type*>(resource);
	}

	template<typename resource_type>
	resource_type* RG_renderpass_builder::read(resource_type* resource) {
		resource->readers_.push_back(renderpass_);
		renderpass_->reads_.push_back(resource);
		return resource;
	}

	template<typename resource_type>
	resource_type* RG_renderpass_builder::write(resource_type* resource) {
		resource->writers_.push_back(renderpass_);
		renderpass_->writes_.push_back(resource);
		return resource;
	}
}

