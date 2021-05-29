#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <fstream>
#include "colony.hpp"
#include "config.hpp"
#include "display_manager.hpp"
#include "json.hpp"
using json = nlohmann::json;

void loadUserConf()
{
	std::ifstream conf_file("conf.txt");
	if (conf_file) {
		conf_file >> Conf::WIN_WIDTH;
		conf_file >> Conf::WIN_HEIGHT;
		conf_file >> Conf::ANTS_COUNT;
	} else {
		std::ifstream json_conf_file("conf.json");
		if (json_conf_file) {
			json jconf;
			json_conf_file >> jconf;
			Conf::WIN_WIDTH = jconf["win_width"];
			Conf::WIN_HEIGHT = jconf["win_height"];
			Conf::WORLD_WIDTH = jconf["world_width"];
			Conf::WORLD_HEIGHT = jconf["world_height"];
			Conf::COLONY_SIZE = jconf["colony_size"];
			Conf::ANTS_COUNT = jconf["ants_count"];
			Conf::ANT_COLOR = sf::Color(jconf["ant_color"]["r"], jconf["ant_color"]["g"], jconf["ant_color"]["b"]);
			Conf::FOOD_COLOR = sf::Color(jconf["food_color"]["r"], jconf["food_color"]["g"], jconf["food_color"]["b"]);
			Conf::TO_HOME_COLOR = sf::Color(jconf["to_home_color"]["r"], jconf["to_home_color"]["g"], jconf["to_home_color"]["b"]);
			Conf::TO_FOOD_COLOR = sf::Color(jconf["to_food_color"]["r"], jconf["to_food_color"]["g"], jconf["to_food_color"]["b"]);
			Conf::COLONY_COLOR = sf::Color(jconf["colony_color"]["r"], jconf["colony_color"]["g"], jconf["colony_color"]["b"]);
			Conf::WALL_COLOR = sf::Color(jconf["wall_color"]["r"], jconf["wall_color"]["g"], jconf["wall_color"]["b"]);
	}
	else {
			std::cout << "Couldn't find 'conf.txt or conf.json', loading default" << std::endl;
		}
	}
}


int main()
{
	Conf::loadTextures();
	loadUserConf();

	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(Conf::WIN_WIDTH, Conf::WIN_HEIGHT), "AntSim", sf::Style::Fullscreen, settings);
	window.setFramerateLimit(60);

	World world(Conf::WORLD_WIDTH, Conf::WORLD_HEIGHT);
	Colony colony(Conf::COLONY_POSITION.x, Conf::COLONY_POSITION.y, Conf::ANTS_COUNT, Conf::COLONY_SIZE);
	for (uint32_t i(0); i < 64; ++i) {
		float angle = float(i) / 64.0f * (2.0f * PI);
		world.addMarker(colony.position + 16.0f * sf::Vector2f(cos(angle), sin(angle)), Mode::ToHome, 10.0f, true);
	}
	
	DisplayManager display_manager(window, window, world, colony);

	sf::Vector2f last_clic;

	sf::Image food_map;
	if (food_map.loadFromFile("map.png")) {
		for (uint32_t x(0); x < food_map.getSize().x; ++x) {
			for (uint32_t y(0); y < food_map.getSize().y; ++y) {
				const sf::Vector2f position = float(world.markers.cell_size) * sf::Vector2f(to<float>(x), to<float>(y));
				if (food_map.getPixel(x, y).g > 100) {
					world.addFoodAt(position.x, position.y, 5);
				} else if (food_map.getPixel(x, y).r > 100) {
					world.addWall(position);
				}
			}
		}
	}

	while (window.isOpen())
	{
		display_manager.processEvents();
		// Add food on clic
		if (display_manager.clic) {
			const sf::Vector2i mouse_position = sf::Mouse::getPosition(window);
			const sf::Vector2f world_position = display_manager.displayCoordToWorldCoord(sf::Vector2f(to<float>(mouse_position.x), to<float>(mouse_position.y)));
			const float clic_min_dist = 2.0f;
			if (getLength(world_position - last_clic) > clic_min_dist) {
				if (display_manager.wall_mode) {
					world.addWall(world_position);
				}
				else if (display_manager.remove_wall) {
					world.removeWall(world_position);
				}
				else {
					world.addFoodAt(world_position.x, world_position.y, 20);
				}
				last_clic = world_position;
			}
		}

		const float dt = 0.016f;

		if (!display_manager.pause) {
			colony.update(dt, world);
			world.update(dt);
		}

		window.clear(sf::Color(94, 87, 87));
		
		display_manager.draw();

		window.display();
	}

	// Free textures
	Conf::freeTextures();

	return 0;
}

#if defined(_WIN32)
#include <windows.h>
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline,
                     int cmdshow) {
  return main();
}
#endif