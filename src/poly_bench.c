#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_POLY_VERTICES 3
#define W 640
#define H 480

typedef struct {
	int vertices[MAX_POLY_VERTICES * 2];
} poly_t;

typedef struct {
	int x, y, step_x, step_y, err_x, err_y, dist_x, dist_y, dist, count;
} bres_step_t;

typedef struct {
	int count;
	int data[MAX_POLY_VERTICES * 2];
} scanline_t;

int rnd(int max) {
    return rand() % max;
}

void print_poly(poly_t *poly) {
	printf("-----------------------------\n");
	int *v = poly->vertices;
	for (int i = 0; i < MAX_POLY_VERTICES * 2; i += 2) {
		printf("%d, %d\n", v[i], v[i + 1]);
	}
}

void init_poly(poly_t *poly) {
	int *v = poly->vertices;
	int p = 0;
	for (int i = 0; i < MAX_POLY_VERTICES; i++) {
		v[p++] = rnd(W);
		v[p++] = rnd(H);
	}
}

void draw_rect(SDL_Surface *surf, int x, int y, int w, int h, int color) {
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(surf, &rect, color);
}


void insert_bubble(scanline_t *s, int bubble) {
	if (!s->count) {
		s->data[0] = bubble;
		s->count++;
		return;
	}

	int pos = s->count;
	int *d = s->data;
	while(pos) {
		if (d[pos - 1] <= bubble) {
			break;
		}
		d[pos] = d[pos - 1];
		pos--;
	}

	d[pos] = bubble;
	s->count++;
}

void fill_poly(SDL_Surface *surf, poly_t *poly, int color) {
	int *v = poly->vertices;
	int x0, y0, x1, y1;
	int p = 0;
 	int tmp;
	scanline_t scanline;

	int y = v[1];
	int max_y = v[1];
	int i;

	for (i = 3; i < MAX_POLY_VERTICES * 2; i += 2) {
		tmp = v[i];
		if (tmp > max_y) {
			max_y = tmp;
		}
		if (tmp < y) {
			y = tmp;
		}
	}

	x1 = v[MAX_POLY_VERTICES * 2 - 2];
	y1 = v[MAX_POLY_VERTICES * 2 - 1];

	for (; y < max_y; y++) {
		scanline.count = 0;

		for (i = 0; i < MAX_POLY_VERTICES * 2; i += 2) {
			x0 = x1;
			y0 = y1;
			x1 = v[i];
			y1 = v[i + 1];

			if (y0 == y) {
				if (y1 == y0) {
					insert_bubble(&scanline, x0);
					insert_bubble(&scanline, x1);
				}
				continue;
			}

			if (y1 == y) {
				if (i <= MAX_POLY_VERTICES * 2 - 4) {
					tmp = v[i + 3];
				} else {
					tmp = v[1];
				}
				if ((tmp < y && y0 < y) || (tmp > y && y0 > y)) {
					insert_bubble(&scanline, x1);
				}
				insert_bubble(&scanline, x1);
				continue;
			}

			if ((y0 < y1 && y >= y0 && y <= y1) || (y0 > y1 && y <= y0 && y >= y1)) {
				insert_bubble(&scanline, x0 + (x1 - x0) * (y - y0) / (y1 - y0));
			}
		}

		for (i = 0; i < scanline.count; i += 2) {
			x0 = scanline.data[i];
			draw_rect(surf, x0, y, scanline.data[i + 1] - x0, 1, color);
		}
	}
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    int w = W;
    int h = H;

    SDL_Window *window = SDL_CreateWindow(
        "Polygons",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w,
        h,
        SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Surface *screenSurface = SDL_GetWindowSurface(window);
    srand(time(0));

    SDL_Event event;
	poly_t poly;
	int color;

    int ticks = SDL_GetTicks();
    int count = 0;

    while(1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

		init_poly(&poly);
		color = (rnd(256) << 16) + (rnd(256) << 8) + rnd(256);
		fill_poly(screenSurface, &poly, color);

		SDL_UpdateWindowSurface(window);
		count++;

        if (count > 99999) {
           break;
        }
    }

    ticks = SDL_GetTicks() - ticks;

    printf("%d polys in %d ms\n", count, ticks);

    SDL_FreeSurface(screenSurface);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
