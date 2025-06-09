#pragma once
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

#ifndef BLT_GFX_VBO_H
#define BLT_GFX_VBO_H

#include <vbo.h>
#include <vbo.h>
#include <vbo.h>
#include <blt/gfx/gl_includes.h>

namespace blt::gfx
{
	class unique_vbo_t;

	namespace detail
	{
		class vbo_context_t
		{
		public:
			explicit vbo_context_t(unique_vbo_t& vbo): vbo(vbo)
			{
				bind();
			}

			/**
			 * By default, the VBO is bound when this class is constructed (this class should only be constructed through the VBO bind() method)
			 *
			 * It is very unlikely that you need this method!
			 */
			vbo_context_t& bind();

			vbo_context_t& unbind();

			vbo_context_t& reserve(GLsizeiptr size, GLint mem_type);

			vbo_context_t& reserve(const size_t size, const GLint mem_type)
			{
				return reserve(static_cast<GLsizeiptr>(size), mem_type);
			}

			vbo_context_t& upload(size_t size, void* ptr, GLint mem_type);

			template <typename T>
			vbo_context_t& upload(const size_t size, T* ptr, const GLint mem_type)
			{
				return upload(size, static_cast<void*>(ptr), mem_type);
			}

			vbo_context_t& update(size_t offset, size_t size, const void* ptr);

			template <typename T>
			vbo_context_t& update(size_t offset, const size_t size, T* ptr)
			{
				return upload(offset, size, static_cast<void*>(ptr));
			}

		private:
			unique_vbo_t& vbo;
		};
	}

	class unique_vbo_t
	{
		friend class detail::vbo_context_t;

	public:
		explicit unique_vbo_t(const GLuint type): buffer_type(type)
		{
			glGenBuffers(1, &*vboID);
		}

		unique_vbo_t(const unique_vbo_t&) = delete;

		unique_vbo_t& operator=(const unique_vbo_t&) = delete;

		unique_vbo_t(unique_vbo_t&& other) noexcept: vboID(std::exchange(other.vboID, std::nullopt)), buffer_type(other.buffer_type),
													size(other.size), memory_type(other.memory_type)
		{}

		unique_vbo_t& operator=(unique_vbo_t&& other) noexcept
		{
			vboID = std::exchange(other.vboID, vboID);
			buffer_type = std::exchange(other.buffer_type, buffer_type);
			size = std::exchange(other.size, size);
			memory_type = std::exchange(other.memory_type, memory_type);
			return *this;
		}

		GLuint change_type(const GLuint type)
		{
			return std::exchange(this->buffer_type, type);
		}

		[[nodiscard]] auto bind();

		~unique_vbo_t()
		{
			if (vboID)
				glDeleteBuffers(1, &*vboID);
		}

	private:
		std::optional<GLuint> vboID;
		GLuint buffer_type;
		GLsizeiptr size = 0;
		GLint memory_type = 0;
	};

	class unique_ssbo_t : public unique_vbo_t
	{
	public:
		unique_ssbo_t(): unique_vbo_t{GL_SHADER_STORAGE_BUFFER}
		{}
	};

	class unique_ebo_t : public unique_vbo_t
	{
	public:
		unique_ebo_t(): unique_vbo_t{GL_ELEMENT_ARRAY_BUFFER}
		{}
	};

	class unique_ubo_t : public unique_vbo_t
	{
	public:
		unique_ubo_t(): unique_vbo_t{GL_UNIFORM_BUFFER}
		{}
	};
}

#endif //BLT_GFX_VBO_H
