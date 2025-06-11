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
		/**
		 * So long as this class is called from vbo.bind(), it is always valid to chain any internal functions.
		 * This system is designed to be foolproof, so don't get too clever
		 */
		class vbo_context_t
		{
			friend unique_vbo_t;
		public:
			vbo_context_t(const vbo_context_t& copy) = delete;
			vbo_context_t(vbo_context_t&& move) = delete;
			vbo_context_t& operator=(const vbo_context_t& copy) = delete;
			vbo_context_t& operator=(vbo_context_t&& move) = delete;
			/**
			 * By default, the VBO is bound when this class is constructed (this class should only be constructed through the VBO bind() method)
			 *
			 * It is very unlikely that you need this method!
			 */
			vbo_context_t& bind();

			vbo_context_t& unbind();

			/**
			 * Reserves a chunk of GPU memory for future use
			 */
			vbo_context_t& resize(GLsizeiptr size, GLint mem_type);

			/**
			 * Reserves a chunk of GPU memory for future use
			 */
			vbo_context_t& resize(const size_t size, const GLint mem_type)
			{
				return resize(static_cast<GLsizeiptr>(size), mem_type);
			}

			/**
			 *	Uploads a chunk of memory to the GPU. If the existing VBO has enough space, the memory will be reused.
			 */
			vbo_context_t& upload(size_t size, const void* ptr, GLint mem_type);

			/**
			 *	Uploads a chunk of memory to the GPU. If the existing VBO has enough space, the memory will be reused.
			 */
			template <typename T, std::enable_if_t<!std::is_same_v<T, void>, bool> = true>
			vbo_context_t& upload(const size_t size, T* ptr, const GLint mem_type)
			{
				return upload(size, static_cast<void*>(ptr), mem_type);
			}

			/**
			 * Updates an internal segment of the VBO. This function will never reallocate.
			 */
			vbo_context_t& update(size_t offset, size_t size, const void* ptr);

			/**
			 * Updates an internal segment of the VBO. This function will never reallocate.
			 */
			template <typename T, std::enable_if_t<!std::is_same_v<T, void>, bool> = true>
			vbo_context_t& update(const size_t offset, const size_t size, T* ptr)
			{
				return update(offset, size, static_cast<void*>(ptr));
			}

		private:
			[[nodiscard]] bool is_bound() const;

			explicit vbo_context_t(unique_vbo_t& vbo): vbo(vbo)
			{
				bind();
			}

			unique_vbo_t& vbo;
		};
	}

	class unique_vbo_t
	{
		friend class detail::vbo_context_t;

	public:
		explicit unique_vbo_t(const GLuint type): vboID(0), buffer_type(type)
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

		/**
		 * Changes the internal buffer type of this VBO
		 */
		GLuint change_type(const GLuint type)
		{
			return std::exchange(this->buffer_type, type);
		}

		/**
		 * This function binds the VBO to the current buffer_type slot and returns an object which allows you to modify or use this VBO.
		 * This allows you to use the VBO without worrying about whether an operation is valid in this context.
		 * As so long as you use this object in line, or without binding other VBOs to the same buffer_type
		 * (violating the contracts this function attempts to create) then all functions on the associated object are valid to call.
		 *
		 * You can enable the flag BLT_DEBUG_CONTRACTS which will validate VBO bind state making most of ^ irrelevant
		 */
		detail::vbo_context_t bind();

		[[nodiscard]] auto native_handle() const
		{
			return vboID;
		}

		[[nodiscard]] GLsizeiptr get_size() const
		{
			return size;
		}

		[[nodiscard]] GLint get_memory_type() const
		{
			return memory_type;
		}

		[[nodiscard]] GLuint get_buffer_type() const
		{
			return buffer_type;
		}

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
		explicit unique_ubo_t(const i32 location): unique_vbo_t{GL_UNIFORM_BUFFER}
		{set_location(location);}

		void set_location(i32 new_location);

		[[nodiscard]] i32 get_location() const
		{
			return location;
		}
	private:
		i32 location = 0;
	};
}

#endif //BLT_GFX_VBO_H
