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
#include <vao.h>
#include <vbo.h>
#include <shaders/particle.frag>
#include <shaders/particle.vert>
#include <imgui.h>

constexpr blt::size_t PARTICLE_COUNT = 8192;

blt::gfx::matrix_state_manager global_matrices;
blt::gfx::resource_manager resources;
blt::gfx::batch_renderer_2d renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera camera;

// use types for state that way you are not confused about what is happening?

struct particle_t
{
	blt::vec2 position;
	blt::vec2 velocity;
	blt::vec2 acceleration;

	static particle_t make_particle()
	{
		blt::random::random_t random{std::random_device()()};
		return {blt::vec2{random.get(0.0f, 500.0f), random.get(0.0f, 500.0f)}, blt::vec2{}, blt::vec2{}};
	}
};

std::optional<blt::gfx::unique_vao_t> particle_vao;
std::unique_ptr<blt::gfx::shader_t> particle_shader;

std::vector<blt::u32> alive_particles;
std::vector<blt::u32> dead_particles;
std::vector<particle_t> particles;

void init(const blt::gfx::window_data&)
{
	using namespace blt::gfx;

	particle_shader = std::unique_ptr<shader_t>(shader_t::make(shader_particle_2d_vert, shader_particle_2d_frag));

	for (blt::size_t i = 0; i < PARTICLE_COUNT; i++)
	{
		particles.push_back(particle_t::make_particle());
		alive_particles.push_back(i);
	}

	unique_vbo_t particle_vbo(GL_ARRAY_BUFFER);
	particle_vbo.bind().upload(sizeof(particle_t) * particles.size(), particles.data(), GL_DYNAMIC_DRAW);

	unique_ebo_t alive_particles_ebo;
	alive_particles_ebo.bind().upload(sizeof(blt::u32) * alive_particles.size(), alive_particles.data(), GL_DYNAMIC_DRAW);

	particle_vao = unique_vao_t();
	const auto vao_ctx = particle_vao->bind();
	vao_ctx.attach_vbo(std::move(particle_vbo)).attribute_ptr(0, 2, GL_FLOAT, sizeof(particle_t), 0);
	vao_ctx.attach_vbo(std::move(alive_particles_ebo));

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
}

void destroy(const blt::gfx::window_data&)
{
	particle_vao.reset();
	particle_shader.reset();
	global_matrices.cleanup();
	resources.cleanup();
	renderer_2d.cleanup();
	blt::gfx::cleanup();
}

int main()
{
	blt::gfx::init(blt::gfx::window_data{"My Sexy Window", init, update, destroy}.setSyncInterval(1));
}
