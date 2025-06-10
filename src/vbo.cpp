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
#include <vbo.h>
#include <blt/gfx/window.h>
#include <blt/std/assert.h>

namespace blt::gfx
{
	namespace detail
	{
		vbo_context_t& vbo_context_t::bind()
		{
			BLT_CONTRACT(vbo.vboID, "Expected VBO to have an assoicated VBO ID!");
			glBindBuffer(vbo.buffer_type, *vbo.vboID);
			return *this;
		}

		vbo_context_t& vbo_context_t::unbind()
		{
			glBindBuffer(vbo.buffer_type, 0);
			return *this;
		}

		vbo_context_t& vbo_context_t::resize(const GLsizeiptr size, const GLint mem_type)
		{
			vbo.size = size;
			vbo.memory_type = mem_type;
			glBufferData(vbo.buffer_type, size, nullptr, mem_type);
			return *this;
		}

		vbo_context_t& vbo_context_t::upload(const size_t size, const void* ptr, const GLint mem_type)
		{
			if (mem_type != vbo.memory_type || static_cast<size_t>(vbo.size) < size)
			{
				vbo.size = static_cast<GLsizeiptr>(size);
				vbo.memory_type = mem_type;
				glBufferData(vbo.buffer_type, vbo.size, ptr, mem_type);
			} else
			{
				update(0, size, ptr);
			}
			return *this;
		}

		vbo_context_t& vbo_context_t::update(const size_t offset, const size_t size, const void* ptr)
		{
			glBufferSubData(vbo.buffer_type, static_cast<GLsizeiptr>(offset), static_cast<GLsizeiptr>(size), ptr);
			return *this;
		}
	}

	detail::vbo_context_t unique_vbo_t::bind()
	{
		BLT_CONTRACT(glfwGetCurrentContext() != nullptr, "Expected active OpenGL context!");
		return detail::vbo_context_t{*this};
	}

	void unique_ubo_t::set_location(const i32 new_location)
	{
		BLT_CONTRACT(native_handle().has_value(), "Expected UBO to have an associated buffer! (You are probably calling this on a moved-from value)");
		location = new_location;
		glBindBufferBase(GL_UNIFORM_BUFFER, location, *native_handle());
	}
}
