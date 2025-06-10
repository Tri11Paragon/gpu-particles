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
#include <blt/std/vector.h>

namespace blt::gfx
{
	class unique_vao_t;

	namespace detail
	{
		struct vao_vbo_storage_t
		{
			std::unique_ptr<unique_vbo_t> vbo;
			u32 attribute_number = 0;

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

		private:
			vao_vbo_context_t(unique_vao_t& vao, vao_vbo_storage_t& vbo): vbo(vbo), vao(vao)
			{}

			vao_vbo_storage_t& vbo;
			unique_vao_t& vao;
		};

		class vao_context_t
		{
			friend vao_vbo_context_t;
			friend class unique_vao_t;

		public:
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
