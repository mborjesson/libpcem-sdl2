#include <stdio.h>
#include <unistd.h>

#include <SDL2/SDL.h>

extern "C" {
	#include "pcem.h"
	#include "pcem-config.h"
}

static SDL_Window *window = NULL;
static SDL_mutex* screen_mutex = NULL;

static int screen_width = 640;
static int screen_height = 480;

static int window_width = 800;
static int window_height = 600;
static char keys[272];

static uint8_t* image_buffer = NULL;

static int mousecapture = 0;

static SDL_AudioDeviceID audio_device;
static SDL_AudioStream* audio_streams[2];
static uint8_t* audio_buffer = 0;
static int audio_buffer_length = 0;

void on_video_size(int width, int height) {
	printf("Video size changed: %dx%d\n", width, height);
	screen_width = width;
	screen_height = height;
	//SDL_SetWindowSize(window, width, height);
}

void on_video_blit_draw(int x1, int x2, int y1, int y2, int offset_x, int offset_y, void *buffer, int buffer_width, int buffer_height, int buffer_channels) {
	SDL_LockMutex(screen_mutex);
	for (int y = y1; y < y2; ++y) {
		int start = y * 2048 * 4;
		if ((offset_y + y) >= 0 && (offset_y + y) < buffer_height) {
			for (int x = x1; x < x2; ++x) {
				int p = buffer_channels*(y*buffer_width+offset_x+x);

				for (int i = 0; i < 3; ++i) {
					image_buffer[start + 4*x + i] = ((uint8_t*)buffer)[p + i];
				}
				image_buffer[start + 4*x + 3] = 0xff;
			}
		}
	}
	SDL_UnlockMutex(screen_mutex);
}

static const struct {
        SDL_Scancode sdl;
        int system;
} SDLScancodeToSystemScancode[] = {
                { SDL_SCANCODE_A, 0x1e },
                { SDL_SCANCODE_B, 0x30 },
                { SDL_SCANCODE_C, 0x2e },
                { SDL_SCANCODE_D, 0x20 },
                { SDL_SCANCODE_E, 0x12 },
                { SDL_SCANCODE_F, 0x21 },
                { SDL_SCANCODE_G, 0x22 },
                { SDL_SCANCODE_H, 0x23 },
                { SDL_SCANCODE_I, 0x17 },
                { SDL_SCANCODE_J, 0x24 },
                { SDL_SCANCODE_K, 0x25 },
                { SDL_SCANCODE_L, 0x26 },
                { SDL_SCANCODE_M, 0x32 },
                { SDL_SCANCODE_N, 0x31 },
                { SDL_SCANCODE_O, 0x18 },
                { SDL_SCANCODE_P, 0x19 },
                { SDL_SCANCODE_Q, 0x10 },
                { SDL_SCANCODE_R, 0x13 },
                { SDL_SCANCODE_S, 0x1f },
                { SDL_SCANCODE_T, 0x14 },
                { SDL_SCANCODE_U, 0x16 },
                { SDL_SCANCODE_V, 0x2f },
                { SDL_SCANCODE_W, 0x11 },
                { SDL_SCANCODE_X, 0x2d },
                { SDL_SCANCODE_Y, 0x15 },
                { SDL_SCANCODE_Z, 0x2c },
                { SDL_SCANCODE_0, 0x0B },
                { SDL_SCANCODE_1, 0x02 },
                { SDL_SCANCODE_2, 0x03 },
                { SDL_SCANCODE_3, 0x04 },
                { SDL_SCANCODE_4, 0x05 },
                { SDL_SCANCODE_5, 0x06 },
                { SDL_SCANCODE_6, 0x07 },
                { SDL_SCANCODE_7, 0x08 },
                { SDL_SCANCODE_8, 0x09 },
                { SDL_SCANCODE_9, 0x0A },
                { SDL_SCANCODE_GRAVE, 0x29 },
                { SDL_SCANCODE_MINUS, 0x0c },
                { SDL_SCANCODE_EQUALS, 0x0d },
                { SDL_SCANCODE_NONUSBACKSLASH, 0x56 },
                { SDL_SCANCODE_BACKSLASH, 0x2b },
                { SDL_SCANCODE_BACKSPACE, 0x0e },
                { SDL_SCANCODE_SPACE, 0x39 },
                { SDL_SCANCODE_TAB, 0x0f },
                { SDL_SCANCODE_CAPSLOCK, 0x3a },
                { SDL_SCANCODE_LSHIFT, 0x2a },
                { SDL_SCANCODE_LCTRL, 0x1d },
                { SDL_SCANCODE_LGUI, 0xdb },
                { SDL_SCANCODE_LALT, 0x38 },
                { SDL_SCANCODE_RSHIFT, 0x36 },
                { SDL_SCANCODE_RCTRL, 0x9d },
                { SDL_SCANCODE_RGUI, 0xdc },
                { SDL_SCANCODE_RALT, 0xb8 },
                { SDL_SCANCODE_SYSREQ, 0x54 },
                { SDL_SCANCODE_APPLICATION, 0xdd },
                { SDL_SCANCODE_RETURN, 0x1c },
                { SDL_SCANCODE_ESCAPE, 0x01 },
                { SDL_SCANCODE_F1, 0x3B },
                { SDL_SCANCODE_F2, 0x3C },
                { SDL_SCANCODE_F3, 0x3D },
                { SDL_SCANCODE_F4, 0x3e },
                { SDL_SCANCODE_F5, 0x3f },
                { SDL_SCANCODE_F6, 0x40 },
                { SDL_SCANCODE_F7, 0x41 },
                { SDL_SCANCODE_F8, 0x42 },
                { SDL_SCANCODE_F9, 0x43 },
                { SDL_SCANCODE_F10, 0x44 },
                { SDL_SCANCODE_F11, 0x57 },
                { SDL_SCANCODE_F12, 0x58 },
                { SDL_SCANCODE_SCROLLLOCK, 0x46 },
                { SDL_SCANCODE_LEFTBRACKET, 0x1a },
                { SDL_SCANCODE_RIGHTBRACKET, 0x1b },
                { SDL_SCANCODE_INSERT, 0xd2 },
                { SDL_SCANCODE_HOME, 0xc7 },
                { SDL_SCANCODE_PAGEUP, 0xc9 },
                { SDL_SCANCODE_DELETE, 0xd3 },
                { SDL_SCANCODE_END, 0xcf },
                { SDL_SCANCODE_PAGEDOWN, 0xd1 },
                { SDL_SCANCODE_UP, 0xc8 },
                { SDL_SCANCODE_LEFT, 0xcb },
                { SDL_SCANCODE_DOWN, 0xd0 },
                { SDL_SCANCODE_RIGHT, 0xcd },
                { SDL_SCANCODE_NUMLOCKCLEAR, 0x45 },
                { SDL_SCANCODE_KP_DIVIDE, 0xb5 },
                { SDL_SCANCODE_KP_MULTIPLY, 0x37 },
                { SDL_SCANCODE_KP_MINUS, 0x4a },
                { SDL_SCANCODE_KP_PLUS, 0x4e },
                { SDL_SCANCODE_KP_ENTER, 0x9c },
                { SDL_SCANCODE_KP_PERIOD, 0x53 },
                { SDL_SCANCODE_KP_0, 0x52 },
                { SDL_SCANCODE_KP_1, 0x4f },
                { SDL_SCANCODE_KP_2, 0x50 },
                { SDL_SCANCODE_KP_3, 0x51 },
                { SDL_SCANCODE_KP_4, 0x4b },
                { SDL_SCANCODE_KP_5, 0x4c },
                { SDL_SCANCODE_KP_6, 0x4d },
                { SDL_SCANCODE_KP_7, 0x47 },
                { SDL_SCANCODE_KP_8, 0x48 },
                { SDL_SCANCODE_KP_9, 0x49 },
                { SDL_SCANCODE_SEMICOLON, 0x27 },
                { SDL_SCANCODE_APOSTROPHE, 0x28 },
                { SDL_SCANCODE_COMMA, 0x33 },
                { SDL_SCANCODE_PERIOD, 0x34 },
                { SDL_SCANCODE_SLASH, 0x35 },
                { SDL_SCANCODE_PRINTSCREEN, 0xb7 }
};

int sdl_scancode(SDL_Scancode scancode) {
        int i;
        for (i = 0; i < SDL_arraysize(SDLScancodeToSystemScancode); ++i) {
                if (SDLScancodeToSystemScancode[i].sdl == scancode) {
                        return SDLScancodeToSystemScancode[i].system;
                }
        }
        return -1;
}

void on_input_keyboard_poll(void *states) {
	memcpy(states, keys, 272);
}

void on_input_mouse_poll(int *x, int *y, int *z, int *buttons) {
	if (mousecapture) {
		uint32_t mb = SDL_GetRelativeMouseState(x, y);
		*buttons = 0;
		if (mb & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			*buttons |= 1;
		}
		if (mb & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			*buttons |= 2;
		}
		if (mb & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
			*buttons |= 4;
		}
	}
}

void on_audio_stream_create(int stream, int sample_rate, int sample_size_in_bits, int channels, int buffer_length) {
	printf("Create audio stream %d: %d hz, %d bits, %d channels (buffer length: %d)\n", stream, sample_rate, sample_size_in_bits, channels, buffer_length);
	if (audio_device) {
		SDL_AudioStream *s = SDL_NewAudioStream(AUDIO_S16, channels, sample_rate, AUDIO_S16, 2, 48000);
		if (s == NULL) {
			printf("Could not create audio stream: %s\n", SDL_GetError());
		} else {
			if (stream == PCEM_AUDIO_STREAM_DEFAULT) {
				audio_streams[0] = s;
			} else if (stream == PCEM_AUDIO_STREAM_CD) {
				audio_streams[1] = s;
			}
		}
	}
}

void on_audio_stream_data(int stream, void *buffer, int buffer_length) {
	if (audio_device) {
		SDL_AudioStream* s = NULL;

		if (stream == PCEM_AUDIO_STREAM_DEFAULT) {
			s = audio_streams[0];
		} else if (stream == PCEM_AUDIO_STREAM_CD) {
			s = audio_streams[1];
		}
		if (s) {
			int rc = SDL_AudioStreamPut(s, buffer, buffer_length);
			if (rc == -1) {
				printf("Could not put audio samples to the stream: %s\n", SDL_GetError());
			}
		}
	}
}

void audio_callback(void* userdata, Uint8* buf, int len) {
	memset(buf, 0, len);
	if (audio_buffer_length < len) {
		audio_buffer_length = len;
		if (audio_buffer) {
			free(audio_buffer);
		}
		audio_buffer = (uint8_t*) malloc(audio_buffer_length);
	}

	for (int i = 0; i < 2; ++i) {
		if (audio_streams[i]) {
			int read = SDL_AudioStreamGet(audio_streams[i], audio_buffer, len);
			if (read > 0) {
				SDL_MixAudioFormat(buf, audio_buffer, AUDIO_S16, read, SDL_MIX_MAXVOLUME);
			} else if (read == -1) {
				printf("Could not mix audio samples: %s\n", SDL_GetError());
			}
		}
	}
}

int main(int argc, char** argv) {
	int result;

	printf("libpcem SDL2\n");

	if (argc != 3) {
		printf("Usage: <global_config> <machine_config>\n");
		return 1;
	}

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    window = SDL_CreateWindow(
        "libpcem SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        window_width,
        window_height,
        SDL_WINDOW_OPENGL
    );

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 2;
    }

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("Could not create renderer: %s\n", SDL_GetError());
        return 3;
    }

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 2048, 2048);

	image_buffer = (uint8_t*) malloc(2048*2048*4);
	memset(image_buffer, 0, 2048*2048*4);

	screen_mutex = SDL_CreateMutex();

	SDL_AudioSpec want, have;

	SDL_memset(&want, 0, sizeof(want));
	want.freq = 48000;
	want.format = AUDIO_S16;
	want.channels = 2;
	want.samples = 4096;
	want.callback = audio_callback;

	audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (audio_device == 0) {
		printf("Failed to open audio: %s\n", SDL_GetError());
	} else {
		const char* driver_name = SDL_GetCurrentAudioDriver();

		if (driver_name) {
			printf("Audio initialized, driver: %s\n", driver_name);
		}

		SDL_PauseAudioDevice(audio_device, 0);
	}


	for (int i = 0; i < 2; ++i) {
		audio_streams[i] = 0;
	}

	// load configuration read only
	if (pcem_config_simple_init(argv[1], argv[2], 1)) {
		printf("Could not load configs.");
		return 4;
	}

	// set up callbacks
	pcem_callback_video_size(on_video_size);
	pcem_callback_video_blit_draw(on_video_blit_draw);
	pcem_callback_input_keyboard_poll(on_input_keyboard_poll);
	pcem_callback_input_mouse_poll(on_input_mouse_poll);
	pcem_callback_audio_stream_create(on_audio_stream_create);
	pcem_callback_audio_stream_data(on_audio_stream_data);

	// start emulation
	result = pcem_start();
	if (result != 0) {
		printf("Could not start PCem (%d)\n", result);
		return 5;
	}

	int running = 1;

	SDL_Event event;

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	SDL_Rect texture_rect, window_rect;

	texture_rect.x = 0;
	texture_rect.y = 0;
	window_rect.x = 0;
	window_rect.y = 0;

	void* pixels;
	int pitch;

	char title[256];
	memset(title, 0, 256);
	int start = SDL_GetTicks();
	while (running) {
        while(SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_WINDOWEVENT: {
					if (event.window.event == SDL_WINDOWEVENT_CLOSE) running = 0;
					break;
				}
				case SDL_KEYDOWN: {
					int key_idx = sdl_scancode(event.key.keysym.scancode);
					if (key_idx != -1)
						keys[key_idx] = 1;
					break;
                }
                case SDL_KEYUP: {
					int key_idx = sdl_scancode(event.key.keysym.scancode);
					if (key_idx != -1)
						keys[key_idx] = 0;
					break;
                }
				case SDL_MOUSEBUTTONUP: {
				    if (!mousecapture) {
						if (event.button.button == SDL_BUTTON_LEFT) {
							mousecapture = 1;
							SDL_GetRelativeMouseState(0, 0);
			                SDL_SetWindowGrab(window, SDL_TRUE);
                			SDL_SetRelativeMouseMode(SDL_TRUE);
						}
					}
				}
				}
		}

		if (keys[sdl_scancode(SDL_SCANCODE_END)] && keys[sdl_scancode(SDL_SCANCODE_LCTRL)] && mousecapture) {
			mousecapture = 0;
			SDL_SetWindowGrab(window, SDL_FALSE);
			SDL_SetRelativeMouseMode(SDL_FALSE);
		}

		SDL_RenderClear(renderer);

		texture_rect.w = screen_width;
		texture_rect.h = screen_height;

		SDL_LockTexture(texture, &texture_rect, &pixels, &pitch);
		SDL_LockMutex(screen_mutex);
		memcpy(pixels, image_buffer, pitch*texture_rect.h);
		SDL_UnlockMutex(screen_mutex);
		SDL_UnlockTexture(texture);

		window_rect.w = window_width;
		window_rect.h = window_height;

		SDL_RenderCopy(renderer, texture, &texture_rect, &window_rect);

		SDL_RenderPresent(renderer);

		int ticks = SDL_GetTicks();

		if (ticks-start >= 1000) {
			snprintf(title, 255, "libpcem SDL2 (%d%%)", pcem_get_emulation_speed());
			SDL_SetWindowTitle(window, title);
			start = ticks;
		}

		SDL_Delay(1);
	}

	printf("Shutting down...\n");

	// shut down
	pcem_stop();
	pcem_config_simple_close();

	SDL_DestroyMutex(screen_mutex);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
	free(image_buffer);

	for (int i = 0; i < 2; ++i) {
		if (audio_streams[i]) {
			SDL_FreeAudioStream(audio_streams[i]);
		}
	}

	SDL_CloseAudioDevice(audio_device);
	if (audio_buffer) {
		free(audio_buffer);
	}

    SDL_Quit();

	printf("All done.\n");

	return 0;
}
