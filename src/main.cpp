/*
*  Copyright (C) 2024  Brett Terpstra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <blt/gfx/window.h>
#include "blt/gfx/renderer/resource_manager.h"
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include <blt/std/random.h>
#include <blt/gfx/vao.h>
#include <shaders/particle.frag.h>
#include <shaders/particle.vert.h>
#include <shaders/particle.comp.h>
#include <imgui.h>

constexpr blt::size_t PARTICLE_COUNT = 8192;

blt::gfx::matrix_state_manager   global_matrices;
blt::gfx::resource_manager       resources;
blt::gfx::batch_renderer_2d      renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera_2d camera;

constexpr float PARTICLE_SIZE = 25;

blt::vec2 quad[6] = {
	{-PARTICLE_SIZE, -PARTICLE_SIZE},
	{-PARTICLE_SIZE, PARTICLE_SIZE},
	{PARTICLE_SIZE, -PARTICLE_SIZE},

	{-PARTICLE_SIZE, PARTICLE_SIZE},
	{PARTICLE_SIZE, PARTICLE_SIZE},
	{PARTICLE_SIZE, -PARTICLE_SIZE},
};

blt::vec2 quad_uvs[6] = {
	{0, 0},
	{0, 1},
	{1, 0},

	{0, 1},
	{1, 1},
	{1, 0}
};


struct particle_data_t
{
	blt::vec2 position;
	blt::vec2 velocity;
	float     mass = 1, drag = 0.1;

	particle_data_t() = default;

	explicit particle_data_t(const blt::vec2 position): position{position}
	{
		// velocity[0] = 25;
	}
};


class gpu_particle_renderer
{
public:
	gpu_particle_renderer()
	{
		using namespace blt::gfx;

		particle_shader = std::unique_ptr<shader_t>(
			shader_t::make(shaders::particle::vert::particle_vert_str, shaders::particle::frag::particle_frag_str));
		compute_shader = std::make_unique<compute_shader_t>(shaders::particle::comp::particle_comp_str);

		blt::random::random_t        rand{std::random_device()()};
		std::vector<particle_data_t> data;
		data.resize(PARTICLE_COUNT);
		for (auto& v : data)
			v = particle_data_t{blt::vec2{rand.get_float(0, 1920), rand.get_float(0, 1080)}};

		particle_positions = std::make_unique<unique_ssbo_t>();
		// particle_positions->bind().resize(PARTICLE_COUNT * sizeof(particle_data_t), buffer::usage_t::dynamic_draw);
		particle_positions->bind().upload(data.size() * sizeof(particle_data_t),
										  data.data(),
										  buffer::usage_t::dynamic_draw).unbind();
		particle_positions->location(1);

		const auto cfg      = particle_vao.configure();
		auto       quad_vbo = unique_vbo_t{buffer::array};
		quad_vbo.bind().upload(sizeof(quad), quad, buffer::usage_t::static_draw);
		cfg.attach_vbo(std::move(quad_vbo)).attribute_ptr(0, 2, memory_t::f32, 0, 0);

		auto uv_vbo = unique_vbo_t{buffer::array};
		uv_vbo.bind().upload(sizeof(quad_uvs), quad_uvs, buffer::usage_t::static_draw);
		cfg.attach_vbo(std::move(uv_vbo)).attribute_ptr(1, 2, memory_t::f32, 0, 0);

		std::vector<blt::u32> alive_indexes;
		for (blt::size_t i = 0; i < PARTICLE_COUNT; i++)
			alive_indexes.push_back(static_cast<blt::u32>(i));

		auto alive_vbo = unique_vbo_t{buffer::array};
		// alive_vbo.bind().resize(PARTICLE_COUNT * sizeof(GLuint), buffer::usage_t::dynamic_draw);
		alive_vbo.bind().upload(alive_indexes.size() * sizeof(blt::u32),
								alive_indexes.data(),
								buffer::usage_t::dynamic_draw);
		cfg.attach_vbo(std::move(alive_vbo)).attribute_ptr(2, 1, memory_t::u32, 0, 0).per_instance();
	}

	void render()
	{
		compute_shader->bind();
		auto dt = static_cast<float>(blt::gfx::getFrameDeltaSeconds());
		if (dt > 0.25)
			dt = 0.25;
		compute_shader->setFloat("dt", dt);
		compute_shader->execute(PARTICLE_COUNT / 128, 1, 1);

		glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		compute_shader->unbind();

		particle_shader->bind();
		particle_shader->setInt("tex1", 0);
		particle_shader->setInt("tex2", 1);
		particle_vao.bind();
		// particle_vao.get_element()->get().bind();
		glActiveTexture(GL_TEXTURE0);
		resources.get("silly").value()->bind();
		glActiveTexture(GL_TEXTURE1);
		resources.get("happy").value()->bind();

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, alive_particles);
		// glDrawElements(GL_POINTS, static_cast<int>(alive_particles.size()), GL_UNSIGNED_INT, nullptr);
	}

private:
	blt::gfx::unique_vao_t                      particle_vao;
	std::unique_ptr<blt::gfx::shader_t>         particle_shader;
	std::unique_ptr<blt::gfx::compute_shader_t> compute_shader;
	std::unique_ptr<blt::gfx::unique_ssbo_t>    particle_positions;

	blt::i32 alive_particles = PARTICLE_COUNT;
};


std::optional<gpu_particle_renderer> particle_renderer;

void init(const blt::gfx::window_data&)
{
	using namespace blt::gfx;
	resources.setPrefixDirectory("../");

	resources.enqueue("res/silly.png", "silly");
	resources.enqueue("res/images_4.jpeg", "happy");

	particle_renderer = gpu_particle_renderer{};

	global_matrices.create_internals();
	resources.load_resources();
	renderer_2d.create();
}

void update(const blt::gfx::window_data& data)
{
	global_matrices.update_perspectives(data.width, data.height, 90, 0.1, 2000);

	camera.update();
	camera.update_view(global_matrices);
	global_matrices.update();

	renderer_2d.render(data.width, data.height);

	particle_renderer->render();
}

void destroy(const blt::gfx::window_data&)
{
	particle_renderer.reset();
	global_matrices.cleanup();
	resources.cleanup();
	renderer_2d.cleanup();
	blt::gfx::cleanup();
}

int main() { blt::gfx::init(blt::gfx::window_data{"My Sexy Window", init, update, destroy}.setSyncInterval(1)); }
