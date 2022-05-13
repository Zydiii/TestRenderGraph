#pragma once

#include <string>

namespace RG {
	class RenderGraph;
	class RG_renderpass_base;

	/// <summary>
	/// �� render graph �й��� render pass
	/// </summary>
	class RG_renderpass_builder {
	public:
		explicit RG_renderpass_builder(RenderGraph* randergraph, RG_renderpass_base* renderpass)
			: rendergraph_(randergraph), renderpass_(renderpass) {

		}

		virtual ~RG_renderpass_builder() = default;

		template<typename resource_type, typename description_type>
		resource_type* create(const std::string& name, const description_type& description); // ������Դ
		template<typename resource_type>
		resource_type* read(resource_type* resource); // ��ȡ��Դ
		template<typename resource_type>
		resource_type* write(resource_type* resource); // д����Դ
	protected:
		RenderGraph* rendergraph_;
		RG_renderpass_base* renderpass_;
	};
}

