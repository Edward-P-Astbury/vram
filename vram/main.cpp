#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <string>

// Inspired from the Palinopsia Bug - https://hsmr.cc/palinopsia/
// Implementation derived from - https://github.com/sector-f/palinopsia-rs

void usage() {
	std::cerr << "Usage: VRAM [-v] HEIGHT WIDTH MEGABYTES\n";
	exit(1);
}

uint32_t arg_to_int(char* arg) {
	char* end;
	uint32_t value = std::strtoul(arg, &end, 10);
	if (*end != '\0') {
		usage();
	}
	return value;
}

int main(int argc, char* argv[]) {
	if (argc != 4 && argc != 5) {
		usage();
	}

	bool verbose = false;

	if (argc == 5) {
		if (std::string(argv[1]) == "-v" || std::string(argv[1]) == "--verbose") {
			verbose = true;
		}
		else {
			usage();
		}
	}

	int current = 0;

	int width, height;
	if (!verbose) {
		width = arg_to_int(argv[1]);
		height = arg_to_int(argv[2]);
	}
	else {
		width = arg_to_int(argv[2]);
		height = arg_to_int(argv[3]);
	}

	int num_buffers = (!verbose) ? arg_to_int(argv[3]) * 1024 * 1024 / (width * height * 4)
		: arg_to_int(argv[4]) * 1024 * 1024 / (width * height * 4);

	if (verbose) {
		std::cout << num_buffers << std::endl;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
		return 1;
	}

	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	std::vector<SDL_Texture*> txt;

	for (int i = 0; i < num_buffers; ++i) {
		if (verbose) {
			std::cout << "Now initializing buffer number " << i << " of " << num_buffers << std::endl;
		}

		txt.push_back(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height));
	}

	SDL_Event event;
	bool running = true;

	while (running) {
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, txt[current], nullptr, nullptr);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
				case SDLK_q:
					running = false;
					break;
				case SDLK_RIGHT:
					current += 1;
					break;
				case SDLK_LEFT:
					current -= 1;
					break;
				case SDLK_UP:
					current += 10;
					break;
				case SDLK_DOWN:
					current -= 10;
					break;
				case SDLK_SPACE: {
					std::string filename = "frame_" + std::to_string(current) + ".png";
					std::cout << "Saving frame " << current << std::endl;

					int w, h;
					SDL_GetWindowSize(window, &w, &h);

					SDL_Surface* window_surface = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
					SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA8888, window_surface->pixels, window_surface->pitch);

					SDL_SaveBMP(window_surface, filename.c_str());

					SDL_FreeSurface(window_surface);
					break;
				}
				default:
					break;
				}

				current = current % num_buffers;
				if (current < 0) {
					current = num_buffers + current;
				}

				if (verbose) {
					std::cout << "Now displaying buffer " << current << " of " << num_buffers << std::endl;
				}

				break;

			case SDL_QUIT:
				running = false;
				break;

			default:
				break;
			}
		}
	}

	for (int i = 0; i < num_buffers; ++i) {
		SDL_DestroyTexture(txt[i]);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	IMG_Quit();
	SDL_Quit();

	return 0;
}
