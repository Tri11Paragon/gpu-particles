/*
 *  <Short Description>
 *  Copyright (C) 2025  Brett Terpstra
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
#include <vao.h>
#include <blt/gfx/window.h>
#include <blt/std/assert.h>
#include <blt/std/hashmap.h>

namespace blt::gfx
{
	#define BLT_DEBUG_LEVEL BLT_DEBUG_CONTRACTS

	#if blt_debug_has_flag(BLT_DEBUG_CONTRACTS)
		GLuint bound_vao_id;
	#endif

	detail::vao_context_t& detail::vao_context_t::bind()
	{
		BLT_CONTRACT(vao.vaoID, "Expected VAO to have an associated VAO ID!");
		glBindVertexArray(*vao.vaoID);
		bound_vao_id = *vao.vaoID;
		return *this;
	}

	detail::vao_context_t& detail::vao_context_t::unbind() // NOLINT
	{
		glBindVertexArray(0);
		bound_vao_id = 0;
		return *this;
	}

	detail::vao_vbo_context_t detail::vao_context_t::attach_vbo(unique_vbo_t&& vbo) const
	{
		BLT_CONTRACT(vao.vaoID, "Expected VAO to have an associated VAO ID!");
		BLT_CONTRACT(is_bound(), "Expected VAO to be bound before attaching VBO! (If you are using this API correctly, this has been done for you!)");

		auto& vbo_storage = vao.vbo_list.emplace_back(std::move(vbo));
		vbo_storage.vbo->bind();
		return vao_vbo_context_t{vao, vbo_storage};
	}

	bool detail::vao_context_t::is_bound() const
	{
		return *vao.vaoID == bound_vao_id;
	}

	detail::vao_context_t unique_vao_t::bind()
	{
		BLT_CONTRACT(glfwGetCurrentContext() != nullptr, "Expected active OpenGL context!");
		return detail::vao_context_t{*this};
	}
}
