#include <array>

#include "RenderGraph.h"

namespace resource_type {
	using buffer = std::size_t;
	using texture_1d = std::size_t;
	using texture_2d = std::size_t;
	using texture_3d = std::size_t;

	struct buffer_description
	{
		std::size_t size;
	};

	struct texture_description
	{
		std::size_t levels;
		std::size_t format;
		std::array<std::size_t, 3> size;
	};

	using buffer_resource = RG::RG_resource<buffer_description, buffer>;
	using texture_1d_resource = RG::RG_resource<texture_description, texture_1d>;
	using texture_2d_resource = RG::RG_resource<texture_description, texture_2d>;
	using texture_3d_resource = RG::RG_resource<texture_description, texture_3d>;
}

namespace RG {
	template<>
	std::unique_ptr<resource_type::buffer> realize(const resource_type::buffer_description& description) {
		return std::make_unique<resource_type::buffer>(description.size);
	}

	template<>
	std::unique_ptr<resource_type::texture_2d> realize(const resource_type::texture_description& description) {
		return std::make_unique<resource_type::buffer>(description.levels);
	}
}

int main()
{
    RG::RenderGraph rendergraph;

    auto retained_resource = rendergraph.add_retained_resource("Retained Resource 1", resource_type::texture_description(), static_cast<resource_type::texture_2d*>(nullptr));

    // First render task declaration.
    struct render_task_1_data
    {
        resource_type::texture_2d_resource* output1;
        resource_type::texture_2d_resource* output2;
        resource_type::texture_2d_resource* output3;
        resource_type::texture_2d_resource* output4;
    };
    auto render_task_1 = rendergraph.add_render_pass<render_task_1_data>(
        "Render Task 1",
        [&](render_task_1_data& data, RG::RG_renderpass_builder& builder)
        {
            data.output1 = builder.create<resource_type::texture_2d_resource>("Resource 1", resource_type::texture_description());
            data.output2 = builder.create<resource_type::texture_2d_resource>("Resource 2", resource_type::texture_description());
            data.output3 = builder.create<resource_type::texture_2d_resource>("Resource 3", resource_type::texture_description());
            data.output4 = builder.write <resource_type::texture_2d_resource>(retained_resource);
        },
        [=](const render_task_1_data& data)
        {
            // Perform actual rendering. You may load resources from CPU by capturing them.
            auto actual1 = data.output1->actual();
            auto actual2 = data.output2->actual();
            auto actual3 = data.output3->actual();
            auto actual4 = data.output4->actual();
        });

    auto& data_1 = render_task_1->data();

    // Second render pass declaration.
    struct render_task_2_data
    {
        resource_type::texture_2d_resource* input1;
        resource_type::texture_2d_resource* input2;
        resource_type::texture_2d_resource* output1;
        resource_type::texture_2d_resource* output2;
    };
    auto render_task_2 = rendergraph.add_render_pass<render_task_2_data>(
        "Render Task 2",
        [&](render_task_2_data& data, RG::RG_renderpass_builder& builder)
        {
            data.input1 = builder.read(data_1.output1);
            data.input2 = builder.read(data_1.output2);
            data.output1 = builder.write(data_1.output3);
            data.output2 = builder.create<resource_type::texture_2d_resource>("Resource 4", resource_type::texture_description());
        },
        [=](const render_task_2_data& data)
        {
            // Perform actual rendering. You may load resources from CPU by capturing them.
            auto actual1 = data.input1->actual();
            auto actual2 = data.input2->actual();
            auto actual3 = data.output1->actual();
            auto actual4 = data.output2->actual();
        });

    auto& data_2 = render_task_2->data();

    struct render_task_3_data
    {
        resource_type::texture_2d_resource* input1;
        resource_type::texture_2d_resource* input2;
        resource_type::texture_2d_resource* output;
    };
    auto render_task_3 = rendergraph.add_render_pass<render_task_3_data>(
        "Render Task 3",
        [&](render_task_3_data& data, RG::RG_renderpass_builder& builder)
        {
            data.input1 = builder.read(data_2.output1);
            data.input2 = builder.read(data_2.output2);
            data.output = builder.write(retained_resource);
        },
        [=](const render_task_3_data& data)
        {
            // Perform actual rendering. You may load resources from CPU by capturing them.
            auto actual1 = data.input1->actual();
            auto actual2 = data.input2->actual();
            auto actual3 = data.output->actual();
        });

    rendergraph.compile();
    for (auto i = 0; i < 100; i++)
        rendergraph.execute();
    rendergraph.export_graphviz("rendergraph.gv");
    rendergraph.clear();

    return 0;
}
