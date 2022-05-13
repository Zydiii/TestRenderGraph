#pragma once

#include "RG_renderpass_builder.h"

namespace RG {

	class RenderGraph {

	protected:
		friend RG_renderpass_builder;

		template<typename resource_type, typename description_type>
		resource_type* RG_renderpass_builder::create(const std::string& name, const description_type& description) {

		}
		template<typename resource_type>
		resource_type* RG_renderpass_builder::read(resource_type* resource) {

		}
		template<typename resource_type>
		resource_type* RG_renderpass_builder::write(resource_type* resource) {

		}
	};

}