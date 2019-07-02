#ifdef __MINGW32__
#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <random>
#include <iostream>
#include <chrono>

using namespace std;

#define SCREEN_W 800
#define SCREEN_H 600
#define MAX_OBJS 1000
#define MAX_RAQUETE 2000

#define BALL_W 15.0
#define RACKET_w 70.0
#define RACKET_H 50.0

#define FIRST_STRIKE_SPEED_Y 300.0

#define BALL_ENERGY_DROP 0.01

struct point_t {
	double x, y;
};

typedef point_t vector_t;

class obj_t
{
public:
	point_t pos;
	vector_t speed;
	double w, h;
	Uint8 r, g, b;
};

struct audio_t {
	SDL_AudioSpec wavSpec;
	Uint32 wavLength;
	Uint8 *wavBuffer;
	SDL_AudioDeviceID deviceId;
};

SDL_Window *screen;
SDL_Renderer *renderer;
audio_t audio_pong;

obj_t *objs[MAX_OBJS];
int nobjs = 0;

obj_t *ball;
obj_t *raquete_1;
obj_t *raquete_2;

int alive = 1;

static void load_audio (audio_t *audio, char *fname)
{
	SDL_LoadWAV(fname, &audio->wavSpec, &audio->wavBuffer, &audio->wavLength);
	audio->deviceId = SDL_OpenAudioDevice(NULL, 0, &audio->wavSpec, NULL, 0);
}

static void destroy_audio (audio_t *audio)
{
	SDL_CloseAudioDevice(audio->deviceId);
	SDL_FreeWAV(audio->wavBuffer);
}

static void play_audio (audio_t *audio)
{
	int success;

	success = SDL_QueueAudio(audio->deviceId, audio->wavBuffer, audio->wavLength);
	SDL_PauseAudioDevice(audio->deviceId, 0);
}

static void render ()
{
	int i;
	obj_t *o;
	SDL_Rect rect;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for (i=0; i<nobjs; i++) {
		o = objs[i];

		rect.x = o->pos.x;
		rect.y = o->pos.y;
		rect.w = o->w;
		rect.h = o->h;

		SDL_SetRenderDrawColor(renderer, o->r, o->g, o->b, 255);
		SDL_RenderFillRect(renderer, &rect);
	}

	SDL_RenderPresent(renderer);
}

static void check_collision_boundaries (obj_t *o)
{
	if (o->pos.x < 0.0) {
		o->pos.x = 0.0;
		o->speed.x *= -1.0;
		play_audio(&audio_pong);
	}
	else if ((o->pos.x + o->w) > (double)SCREEN_W) {
		o->pos.x = (double)SCREEN_W - o->w;
		o->speed.x *= -1.0;
		play_audio(&audio_pong);
	}

	if (o->pos.y < 0.0) {
		o->pos.y = 0.0;
		o->speed.y *= -1.0;
		play_audio(&audio_pong);
	}
	else if ((o->pos.y + o->h) > (double)SCREEN_H) {
		o->pos.y = (double)SCREEN_H - o->h;
		o->speed.y *= -1.0;
		play_audio(&audio_pong);
	}
}

static void obj_lose_kinect_energy (obj_t *o, double t)
{
	o->speed.x *= (1.0-BALL_ENERGY_DROP)*t;
	o->speed.y *= (1.0-BALL_ENERGY_DROP)*t;
}

static void physics (double t)
{
	int i;
	obj_t *o;

	for (i=0; i<nobjs; i++) {
		o = objs[i];

		o->pos.x += o->speed.x * t;
		o->pos.y += o->speed.y * t;

		obj_lose_kinect_energy(o);

		check_collision_boundaries(o);
	}
}

static void add_obj (obj_t *o)
{
	assert(nobjs < MAX_OBJS);

	objs[ nobjs++ ] = o;
}

static void init_game ()
{
	ball = new obj_t();
	ball->pos.x = SCREEN_W / 2;
	ball->pos.y = SCREEN_H / 2;
	ball->speed.x = 0.0;
	ball->speed.y = 0.0;
	ball->w = 80.0;
	ball->h = 20.0;
	ball->r = 255;
	ball->g = 0;
	ball->b = 0;

	add_obj(ball);

	raquete_1 = new obj_t();
	raquete_1->pos.x = 0;
	raquete_1->pos.y = 270;
	raquete_1->speed.y = 0.0;
	raquete_1->w = 20.0;
	raquete_1->h = 80.0;
	raquete_1->r = 255;
	raquete_1->g = 255;
	raquete_1->b = 255;


	add_obj(raquete_1);

	raquete_2 = new obj_t();
	raquete_2->pos.x = 800;
	raquete_2->pos.y = 270;
	raquete_2->speed.y = 0.0;
	raquete_2->w = 20.0;
	raquete_2->h = 80.0;
	raquete_2->r = 255;
	raquete_2->g = 255;
	raquete_2->b = 255;

	add_obj(raquete_2);
}

int main (int argc, char **argv)
{
	SDL_Event event;
	const Uint8 *keyboard_state_array;
	chrono::high_resolution_clock::time_point tbegin, tend;
	double elapsed;

	cout << chrono::high_resolution_clock::period::den << endl;

	SDL_Init(SDL_INIT_EVERYTHING);

	screen = SDL_CreateWindow("My Game Window",
	SDL_WINDOWPOS_UNDEFINED,
	SDL_WINDOWPOS_UNDEFINED,
	SCREEN_W, SCREEN_H,
	SDL_WINDOW_OPENGL);

	renderer = SDL_CreateRenderer(screen, -1, 0);

	load_audio(&audio_pong, "pong.wav");

	init_game();

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	keyboard_state_array = SDL_GetKeyboardState(NULL);


	while (alive) {
		tbegin = chrono::high_resolution_clock::now();

		#define inc 500

		// if (keyboard_state_array[SDL_SCANCODE_UP])
		// ball->speed.y -= inc;
		// if (keyboard_state_array[SDL_SCANCODE_DOWN])
		// ball->speed.y += inc;
		// if (keyboard_state_array[SDL_SCANCODE_LEFT])
		// ball->speed.x -= inc;
		// if (keyboard_state_array[SDL_SCANCODE_RIGHT])
		// ball->speed.x += inc;

		if (keyboard_state_array[SDL_SCANCODE_UP])
		raquete_1->pos.y -= inc*elapsed;
		if (keyboard_state_array[SDL_SCANCODE_DOWN])
		raquete_1->pos.y += inc*elapsed;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
				alive = 0;
				break;

				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_SPACE: {
								ball->pos.x = raquete_1-> pos.x;
								ball->pos.y = raquete_1-> pos.y;
								ball->speed.y = FIRST_STRIKE_SPEED_Y;
							cout << "espaço apertado" << endl;
							break;
						}
					}
					break;
				}

			}
		}

		do {
			tend = chrono::high_resolution_clock::now();
			chrono::duration<double> elapsed_ = chrono::duration_cast<chrono::duration<double>>(tend - tbegin);
			elapsed = elapsed_.count();
		} while (elapsed < 0.01);

		physics(elapsed);
		render();
	}

	destroy_audio(&audio_pong);
	SDL_Quit();

	return 0;
}
