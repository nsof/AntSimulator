#pragma once
#include "async_va_renderer.hpp"
#include "grid.hpp"
#include "config.hpp"
#include "world_grid.hpp"


struct WorldRenderer : public AsyncRenderer
{
	const Grid<WorldCell>& grid;
	bool draw_markers;

	WorldRenderer(Grid<WorldCell>& grid_, DoubleObject<sf::VertexArray>& target)
		: AsyncRenderer(target)
		, grid(grid_)
		, draw_markers(true)
	{
		AsyncRenderer::start();
	}

	void initializeVertexArray(sf::VertexArray& va) override
	{
		va = sf::VertexArray(sf::Quads, grid.width * grid.height * 4);
		uint64_t i = 0;
		const float cell_size = to<float>(grid.cell_size);
		for (int32_t x(0); x < grid.width; x++) {
			for (int32_t y(0); y < grid.height; y++) {
				const sf::Vector2f position(x * cell_size, y * cell_size);
				va[4 * i + 0].position = position;
				va[4 * i + 1].position = position + sf::Vector2f(cell_size, 0.0f);
				va[4 * i + 2].position = position + sf::Vector2f(cell_size, cell_size);
				va[4 * i + 3].position = position + sf::Vector2f(0.0f, cell_size);
				++i;
			}
		}
	}

	void updateVertexArray() override
	{
		sf::VertexArray& va = vertex_array.getLast();

		uint64_t i = 0;
		const float cell_size = to<float>(grid.cell_size);
		for (int32_t x(0); x < grid.width; x++) {
			for (int32_t y(0); y < grid.height; y++) {
				const auto& cell = grid.getCst(sf::Vector2i(x, y));
				sf::Color color = sf::Color::Black;
				if (!cell.food && !cell.wall && draw_markers) {
					if ((cell.intensity[0] + cell.intensity[1]) > 0) {
						const sf::Vector3f to_home_color(Conf::TO_HOME_COLOR.r / 255.0f, Conf::TO_HOME_COLOR.g / 255.0f, Conf::TO_HOME_COLOR.b / 255.0f);
						const sf::Vector3f to_food_color(Conf::TO_FOOD_COLOR.r / 255.0f, Conf::TO_FOOD_COLOR.g / 255.0f, Conf::TO_FOOD_COLOR.b / 255.0f);
						const float max_intensity = 1000.0f;
						const float to_food_color_ratio = cell.intensity[1] / (cell.intensity[0] + cell.intensity[1]);
						const float color_decay_factor = 10;
						const float to_home_color_intensity = std::pow(cell.intensity[0] / max_intensity, color_decay_factor);
						const float to_food_color_intensity = std::pow(cell.intensity[1] / max_intensity, color_decay_factor/2);
						sf::Vector3f mixed_color = 255.0f * ((1.0f - to_food_color_ratio) * to_home_color_intensity * to_home_color + to_food_color_ratio * to_food_color_intensity * cell.intensity[1] * to_food_color);
						mixed_color.x = std::min(255.0f, mixed_color.x);
						mixed_color.y = std::min(255.0f, mixed_color.y);
						mixed_color.z = std::min(255.0f, mixed_color.z);
						color = sf::Color(to<uint8_t>(mixed_color.x), to<uint8_t>(mixed_color.y), to<uint8_t>(mixed_color.z));
					}
					const float offset = 32.0f;
					va[4 * i + 0].texCoords = sf::Vector2f(offset, offset);
					va[4 * i + 1].texCoords = sf::Vector2f(100.0f - offset, offset);
					va[4 * i + 2].texCoords = sf::Vector2f(100.0f - offset, 100.0f - offset);
					va[4 * i + 3].texCoords = sf::Vector2f(offset, 100.0f - offset);
				}
				else if (cell.food) {
					color = Conf::FOOD_COLOR;
					const float offset = 4.0f;
					va[4 * i + 0].texCoords = sf::Vector2f(100.0f + offset, offset);
					va[4 * i + 1].texCoords = sf::Vector2f(200.0f - offset, offset);
					va[4 * i + 2].texCoords = sf::Vector2f(200.0f - offset, 100.0f - offset);
					va[4 * i + 3].texCoords = sf::Vector2f(100.0f + offset, 100.0f - offset);
				}
				else if (cell.wall) {
					color = Conf::WALL_COLOR;
					const float offset = 4.0f;
					va[4 * i + 0].texCoords = sf::Vector2f(200.0f + offset, offset);
					va[4 * i + 1].texCoords = sf::Vector2f(300.0f - offset, offset);
					va[4 * i + 2].texCoords = sf::Vector2f(300.0f - offset, 100.0f - offset);
					va[4 * i + 3].texCoords = sf::Vector2f(200.0f + offset, 100.0f - offset);
				}
				va[4 * i + 0].color = color;
				va[4 * i + 1].color = color;
				va[4 * i + 2].color = color;
				va[4 * i + 3].color = color;
				++i;
			}
		}
	}
};
