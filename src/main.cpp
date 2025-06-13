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
#include <imgui.h>
#include <blt/gfx/vao.h>
#include <blt/gfx/vbo.h>
#include <blt/gfx/window.h>
#include <blt/std/random.h>
#include <shaders/particle.frag.h>
#include <shaders/particle.vert.h>
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include "blt/gfx/renderer/resource_manager.h"

constexpr blt::size_t PARTICLE_COUNT = 8192;

blt::gfx::matrix_state_manager global_matrices;
blt::gfx::resource_manager resources;
blt::gfx::batch_renderer_2d renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera_2d camera;

// use types for state that way you are not confused about what is happening?

struct particle_t
{
	blt::vec2 position;
	blt::vec2 velocity;
	blt::vec2 acceleration;
	float mass = 10;
	float unused;

	static particle_t make_particle()
	{
		static blt::random::random_t random{std::random_device()()};
		return {blt::vec2{random.get(20.0f, 1880.0f), random.get(20.0f, 1000.0f)}, blt::vec2{}, blt::vec2{}};
	}
};

class gpu_particle_renderer
{
public:
	gpu_particle_renderer()
	{
		using namespace blt::gfx;

		for (blt::size_t i = 0; i < PARTICLE_COUNT; i++)
		{
			particles.push_back(particle_t::make_particle());
			alive_particles.push_back(i);
		}

		particle_shader = std::unique_ptr<shader_t>(shader_t::make(shaders::particle_vert_str, shaders::particle_frag_str));

		unique_vbo_t particle_vbo(GL_ARRAY_BUFFER);
		particle_vbo.bind().upload(sizeof(particle_t) * particles.size(), particles.data(), GL_DYNAMIC_DRAW);

		unique_ebo_t alive_particles_ebo;
		alive_particles_ebo.bind().upload(sizeof(blt::u32) * alive_particles.size(), alive_particles.data(), GL_DYNAMIC_DRAW);

		const auto vao_ctx = particle_vao.configure();
		vao_ctx.attach_vbo(std::move(particle_vbo)).attribute_ptr(0, 2, GL_FLOAT, sizeof(particle_t), 0);
		vao_ctx.attach_vbo(std::move(alive_particles_ebo));
	}

	void render()
	{
		glPointSize(25);

		particle_shader->bind();
		particle_shader->setInt("tex1", 0);
		particle_shader->setInt("tex2", 1);
		particle_vao.bind();
		glActiveTexture(GL_TEXTURE0);
		resources.get("silly").value()->bind();
		glActiveTexture(GL_TEXTURE1);
		resources.get("happy").value()->bind();

		glDrawElements(GL_POINTS, static_cast<int>(alive_particles.size()), GL_UNSIGNED_INT, nullptr);
	}

private:
	blt::gfx::unique_vao_t particle_vao;
	std::unique_ptr<blt::gfx::shader_t> particle_shader;

	std::vector<blt::u32> alive_particles;
	std::vector<blt::u32> dead_particles;
	std::vector<particle_t> particles;
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

int main()
{
	blt::gfx::init(blt::gfx::window_data{"My Sexy Window", init, update, destroy}.setSyncInterval(1));
}
