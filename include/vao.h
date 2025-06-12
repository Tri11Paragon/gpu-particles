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

#ifndef BLT_GFX_VAO_H
#define BLT_GFX_VAO_H

#include <memory>
#include <vbo.h>
#include <blt/std/hashmap.h>
#include <blt/std/vector.h>

namespace blt::gfx
{
	class unique_vao_t;

	namespace detail
	{
		struct vao_vbo_storage_t
		{
			std::unique_ptr<unique_vbo_t> vbo;
			std::optional<hashset_t<u32>> attribute_numbers;

			[[nodiscard]] bool is_element() const
			{
				return vbo->get_buffer_type() == GL_ELEMENT_ARRAY_BUFFER;
			}

			explicit vao_vbo_storage_t(unique_vbo_t&& vbo): vbo(std::make_unique<unique_vbo_t>(std::move(vbo)))
			{}
		};

		class vao_vbo_context_t
		{
			friend class vao_context_t;
		public:
			vao_vbo_context_t(const vao_vbo_context_t& copy) = delete;
			vao_vbo_context_t(vao_vbo_context_t&& move) = delete;
			vao_vbo_context_t& operator=(const vao_vbo_context_t& copy) = delete;
			vao_vbo_context_t& operator=(vao_vbo_context_t&& move) = delete;
			/**
			 * This function takes ownership of the underlying VBO (GPU side). It will be freed when the basic vertex array is deleted
			 * @param attribute_number attribute number to bind to
			 * @param coordinate_size size of the data (number of elements, not the number of bytes)
			 * @param type GL_TYPE type of data
			 * @param stride how many bytes this data takes (for the entire per-vertex data structure) 0 will assume packed data
			 *               This is in effect how many bytes until the next block of data
			 * @param offset offset into the data structure to where the data is stored
			 */
			vao_vbo_context_t& attribute_ptr(int attribute_number, int coordinate_size, GLenum type, int stride, long offset);

			vao_vbo_context_t& silence()
			{
				attributed = true;
				return *this;
			}

			/**
			 * Useless function, but if it makes you feel better, feel free to use it.
			 */
			vao_vbo_context_t& as_element()
			{
				return *this;
			}

			~vao_vbo_context_t();
		private:
			vao_vbo_context_t(unique_vao_t& vao, vao_vbo_storage_t& vbo): vbo(vbo), vao(vao)
			{}

			vao_vbo_storage_t& vbo;
			unique_vao_t& vao;
			bool attributed = false;
		};

		class vao_context_t
		{
			friend vao_vbo_context_t;
			friend unique_vao_t;
		public:
			vao_context_t(const vao_context_t& copy) = delete;
			vao_context_t(vao_context_t&& move) = delete;
			vao_context_t& operator=(const vao_context_t& copy) = delete;
			vao_context_t& operator=(vao_context_t&& move) = delete;

			vao_context_t& bind();

			vao_context_t& unbind();

			vao_vbo_context_t attach_vbo(unique_vbo_t&& vbo) const;
		private:
			[[nodiscard]] bool is_bound() const;

			explicit vao_context_t(unique_vao_t& vao): vao(vao)
			{
				bind();
			}

			unique_vao_t& vao;
		};
	}

	class unique_vao_t
	{
		friend detail::vao_vbo_context_t;
		friend detail::vao_context_t;

	public:
		unique_vao_t(): vaoID(0)
		{
			glGenVertexArrays(1, &*vaoID);
		}

		unique_vao_t(const unique_vao_t&) = delete;

		unique_vao_t& operator=(const unique_vao_t&) = delete;

		unique_vao_t(unique_vao_t&& other) noexcept: vaoID(std::exchange(other.vaoID, std::nullopt))
		{}

		unique_vao_t& operator=(unique_vao_t&& other) noexcept
		{
			vaoID = std::exchange(other.vaoID, vaoID);
			return *this;
		}

		detail::vao_context_t bind();

		[[nodiscard]] std::optional<ref<unique_vbo_t>> get_attribute(const u32 attribute) const
		{
			for (const auto& vbo_obj : vbo_list)
			{
				if (const auto attrs = vbo_obj.attribute_numbers)
				{
					if (attrs->contains(attribute))
						return *vbo_obj.vbo;
				}
			}
			return {};
		}

		[[nodiscard]] std::optional<ref<unique_vbo_t>> get_buffer_type(const GLuint buffer_type) const
		{
			for (const auto& vbo_obj : vbo_list)
			{
				if (vbo_obj.vbo->get_buffer_type() == buffer_type)
					return *vbo_obj.vbo;
			}
			return {};
		}

		[[nodiscard]] std::optional<ref<unique_vbo_t>> get_element() const
		{
			for (const auto& vbo_obj : vbo_list)
			{
				if (vbo_obj.is_element())
					return *vbo_obj.vbo;
			}
			return {};
		}

		~unique_vao_t()
		{
			if (vaoID)
				glDeleteVertexArrays(1, &*vaoID);
		}

	private:
		std::optional<GLuint> vaoID;
		std::vector<detail::vao_vbo_storage_t> vbo_list;
	};
}

#endif //BLT_GFX_VAO_H
