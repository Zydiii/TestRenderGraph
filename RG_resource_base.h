#pragma once

#include <string>
#include <vector>

namespace RG {
	class RenderGraph;
	class RG_renderpass_base;
	class RG_renderpass_builder;

	/// <summary>
	/// ��Դ����
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

		virtual void realize() = 0; // ʵ����
		virtual void derealize() = 0; // �ͷ���Դ

		std::size_t id_; // ���
		std::string name_; // ����
		std::size_t ref_count_; // ���ü���
		const RG_renderpass_base* creator_; // ��Դ������
		std::vector<const RG_renderpass_base*> readers_; // ��Դ��ȡ��
		std::vector<const RG_renderpass_base*> writers_; // ��Դд����
	};
}