#pragma once

#include <stack>
#include <fstream>
#include <type_traits>
#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>
#include <string>

#include "RG_resource.h"
#include "RG_renderpass.h"
#include "RG_renderpass_builder.h"

namespace RG {
	/// <summary>
	/// render graph
	/// </summary>
	class RenderGraph {
	public:
		RenderGraph() = default;
		virtual ~RenderGraph() = default;

		/// <summary>
		/// �� RG ����� render pass
		/// </summary>
		/// <typeparam name="data_type"></typeparam>
		/// <typeparam name="...argument_types"></typeparam>
		/// <param name="...arguments"></param>
		/// <returns></returns>
		template<typename data_type, typename... argument_types>
		RG_renderpass<data_type>* add_render_pass(argument_types&&... arguments) {
			render_passes_.emplace_back(std::make_unique<RG_renderpass<data_type>>(arguments...));
			auto render_pass = render_passes_.back().get();
			RG_renderpass_builder builder(this, render_pass);
			render_pass->setup(builder);
			return static_cast<RG::RG_renderpass<data_type>*>(render_pass);
		}

		/// <summary>
		/// ��Ӷ�֡�ɸ�����Դ
		/// </summary>
		/// <typeparam name="description_type"></typeparam>
		/// <typeparam name="actual_type"></typeparam>
		/// <param name="name"></param>
		/// <param name="description"></param>
		/// <param name="actual"></param>
		/// <returns></returns>
		template<typename description_type, typename actual_type>
		RG_resource<description_type, actual_type>* add_retained_resource(const std::string& name, const description_type& description, actual_type* actual = nullptr) {
			resources_.emplace_back(std::make_unique<RG_resource<description_type, actual_type>>(name, description, actual));
			return static_cast<RG_resource<description_type, actual_type>*>(resources_.back().get());
		}

		/// <summary>
		/// ����
		/// </summary>
		void compile() {
			// ������������
			for (auto& render_pass : render_passes_)
				render_pass->ref_count_ = render_pass->creates_.size() + render_pass->writes_.size();
			for (auto& resource : resources_)
				resource->ref_count_ = resource->readers_.size();

			// flood fill �޳�û�����õ���Դ
			std::stack<RG_resource_base*> unreferenced_resources;
			for (auto& resource : resources_) {
				if (resource->ref_count_ == 0 && resource->transient())
					unreferenced_resources.push(resource.get());
			}
			while (!unreferenced_resources.empty()) {
				auto unreferenced_resource = unreferenced_resources.top();
				unreferenced_resources.pop();

				// �޸Ĵ�������ص����ü���
				auto creator = const_cast<RG_renderpass_base*>(unreferenced_resource->creator_);
				if (creator->ref_count_ > 0)
					creator->ref_count_--;
				if (creator->ref_count_ == 0 && !creator->cull()) {
					for (auto read : creator->reads_) {
						auto read_resource = const_cast<RG_resource_base*>(read);
						if (read_resource->ref_count_ > 0)
							read_resource->ref_count_--;
						if (read_resource->ref_count_ == 0 && read_resource->transient())
							unreferenced_resources.push(read_resource);
					}
				}

				// �޸Ķ��ߵ�������ü���
				for (auto writer_ : unreferenced_resource->writers_) {
					auto writer = const_cast<RG_renderpass_base*>(writer_);
					if (writer->ref_count_ > 0)
						writer->ref_count_--;
					if (writer->ref_count_ == 0 && !writer->cull()) {
						for (auto read : writer->reads_) {
							auto read_resource = const_cast<RG_resource_base*>(read);
							if (read_resource->ref_count_ > 0)
								read_resource->ref_count_--;
							if (read_resource->ref_count_ == 0 && read_resource->transient())
								unreferenced_resources.push(read_resource);
						}
					}
				}
			}

			// ����ʱ����
			timeline_.clear();
			for (auto& render_pass : render_passes_) {
				if (render_pass->ref_count_ == 0 && !render_pass->cull())
					continue;

				std::vector<RG_resource_base*> realized_resources, derealized_resources;
				for (auto resource : render_pass->creates_) {
					realized_resources.push_back(const_cast<RG_resource_base*>(resource));
					if (resource->readers_.empty() && resource->writers_.empty())
						derealized_resources.push_back(const_cast<RG_resource_base*>(resource));
				}

				auto reads_writes = render_pass->reads_;
				reads_writes.insert(reads_writes.end(), render_pass->writes_.begin(), render_pass->writes_.end());
				for (auto resource : reads_writes) {
					if (!resource->transient())
						continue;

					// �ҵ�����Դ���Ķ�д��
					auto valid = false;
					std::size_t last_index;
					if (!resource->readers_.empty()) {
						auto last_reader = std::find_if(
							render_passes_.begin(),
							render_passes_.end(),
							[&resource](const std::unique_ptr<RG_renderpass_base>& reader) {
								return reader.get() == resource->readers_.back();
							});
						if (last_reader != render_passes_.end()) {
							valid = true;
							last_index = std::distance(render_passes_.begin(), last_reader);
						}
					}

					if (!resource->writers_.empty()) {
						auto last_writer = std::find_if(
							render_passes_.begin(),
							render_passes_.end(),
							[&resource](const std::unique_ptr<RG_renderpass_base>& writer) {
								return writer.get() == resource->writers_.back();
							});
						if (last_writer != render_passes_.end()) {
							valid = true;
							last_index = std::max(last_index, std::size_t(std::distance(render_passes_.begin(), last_writer)));
						}
					}

					if (valid && render_passes_[last_index] == render_pass)
						derealized_resources.push_back(const_cast<RG_resource_base*>(resource));
				}

				timeline_.push_back(step{ render_pass.get(), realized_resources, derealized_resources });
			}

		}

		/// <summary>
		/// ִ��
		/// </summary>
		void execute() const {
			for (auto& step : timeline_) {
				for (auto resource : step.realized_resources)
					resource->realize();
				step.render_pass->execute();
				for (auto resource : step.derealized_resources)
					resource->derealize();
			}
		}

		/// <summary>
		/// ���
		/// </summary>
		void clear() {
			render_passes_.clear();
			resources_.clear();
		}

		/// <summary>
		/// ���� graphviz ��ʽ
		/// </summary>
		/// <param name="filepath"></param>
		void export_graphviz(const std::string& filepath)
		{
			std::ofstream stream(filepath);
			stream << "digraph framegraph \n{\n";

			stream << "rankdir = LR\n";
			stream << "bgcolor = white\n\n";
			stream << "node [shape=rectangle, fontname=\"Times-Roman\", fontsize=12]\n\n";

			// render pass �ڵ� ��ɫ
			for (auto& render_pass : render_passes_)
				stream << "\"" << render_pass->name() << "\" [label=\"" << render_pass->name() << "\\nRefs: " << render_pass->ref_count_ << "\", style=filled, fillcolor=orange]\n";
			stream << "\n";

			// ��Դ�ڵ� ��̬��Դǳ��ɫ��������Դ����ɫ
			for (auto& resource : resources_)
				stream << "\"" << resource->name() << "\" [label=\"" << resource->name() << "\\nRefs: " << resource->ref_count_ << "\\nID: " << resource->id() << "\", style=filled, fillcolor= " << (resource->transient() ? "skyblue" : "skyblue4") << "]\n";
			stream << "\n";

			for (auto& render_pass : render_passes_)
			{
				// ������ ��ɫ��ͷ
				stream << "\"" << render_pass->name() << "\" -> { ";
				for (auto& resource : render_pass->creates_)
					stream << "\"" << resource->name() << "\" ";
				stream << "} [color=firebrick]\n";

				// д�� �ۺ��ͷ
				stream << "\"" << render_pass->name() << "\" -> { ";
				for (auto& resource : render_pass->writes_)
					stream << "\"" << resource->name() << "\" ";
				stream << "} [color=deeppink]\n";
			}
			stream << "\n";

			// ���� ��ɫ��ͷ
			for (auto& resource : resources_)
			{
				stream << "\"" << resource->name() << "\" -> { ";
				for (auto& render_task : resource->readers_)
					stream << "\"" << render_task->name() << "\" ";
				stream << "} [color=forestgreen]\n";
			}
			stream << "}";
		}

	protected:
		friend RG_renderpass_builder;

		struct step // ÿһ��ʱ�䲽ִ�е���Ⱦ������漰����Դ
		{
			RG_renderpass_base* render_pass;
			std::vector<RG_resource_base*> realized_resources;
			std::vector<RG_resource_base*> derealized_resources;
		};
		
		std::vector<std::unique_ptr<RG_renderpass_base>> render_passes_; // ���е���Ⱦ����
		std::vector<std::unique_ptr<RG_resource_base>> resources_; // ���е���Դ
		std::vector<step> timeline_; // ʱ����
	};

	template<typename resource_type, typename description_type>
	resource_type* RG_renderpass_builder::create(const std::string& name, const description_type& description) {
		//static_assert(std::is_same<typename resource_type::description_type, description_type>::value, "Description does not match resources.");
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
