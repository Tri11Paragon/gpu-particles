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

#include <vbo.h>

namespace blt::gfx
{

	class unique_vao_t
	{
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

		~unique_vao_t()
		{
			if (vaoID)
				glDeleteVertexArrays(1, &*vaoID);
		}

	private:
		std::optional<GLuint> vaoID;
	};

}

#endif //BLT_GFX_VAO_H
