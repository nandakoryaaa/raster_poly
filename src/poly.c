#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_POLY_VERTICES 5
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

void bres_init(bres_step_t *bs, int x0, int y0, int x1, int y1) {
	int dx = x1 - x0;
	int dy = y1 - y0;

	bs->dist_x = abs(dx);
	bs->dist_y = abs(dy);

	if (bs->dist_y > bs->dist_x) {
		bs->dist = bs->dist_y;
	} else {
		bs->dist = bs->dist_x;
	}
	if (dx < 0) {
		bs->step_x = -1;
	} else {
		bs->step_x = 1;
	}
	if (dy < 0) {
		bs->step_y = -1;
	} else {
		bs->step_y = 1;
	}
	bs->err_x = 0;
	bs->err_y = 0;
    bs->count = bs->dist + 1;
	bs->x = x0;
	bs->y = y0;
}
	
int bres_step(bres_step_t *bs) {
	if (bs->count) {
        bs->err_x += bs->dist_x;
        bs->err_y += bs->dist_y;
        if (bs->err_x >= bs->dist) {
        	bs->err_x -= bs->dist;
        	bs->x += bs->step_x;
        }
        if (bs->err_y >= bs->dist) {
        	bs->err_y -= bs->dist;
            bs->y += bs->step_y;
        }
		bs->count--;
    }
	return bs->count;
}

void draw_rect(SDL_Surface *surf, int x, int y, int w, int h, int color) {
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(surf, &rect, color);
}

void bres_line2(SDL_Surface *surf, int x0, int y0, int x1, int y1, int color) {
	bres_step_t bres;
	bres_init(&bres, x0, y0, x1, y1);
	while(bres.count) {
		draw_rect(surf, bres.x, bres.y, 1, 1, color);
		bres_step(&bres);
	}
}

void bres_line(SDL_Surface *surf, int x0, int y0, int x1, int y1, int color) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int dist_x = abs(dx);
	int dist_y = abs(dy);
	int dist = dist_x;
	if (dist_y > dist_x) dist = dist_y;
	int step_x = 1;
	int step_y = 1;
	if (dx < 0) step_x = -1;
	if (dy < 0) step_y = -1;
	int err_x = 0;
    int err_y = 0;
    int dst = dist + 1;

    while(dst--) {
    	draw_rect(surf, x0, y0, 1, 1, color);
        err_x += dist_x;
        err_y += dist_y;
        if (err_x >= dist) {
        	err_x -= dist;
        	x0 += step_x;
        }
        if (err_y >= dist) {
        	err_y -= dist;
            y0 += step_y;
        }
    }
}

void draw_poly(SDL_Surface *surf, poly_t *poly, int color) {
	int *v = poly->vertices;
	int x0, y0;
	int x1 = v[MAX_POLY_VERTICES * 2 - 2];
	int y1 = v[MAX_POLY_VERTICES * 2 - 1];
	for (int i = 0; i < MAX_POLY_VERTICES * 2; i += 2) {
		x0 = x1;
		y0 = y1;
		x1 = v[i];
		y1 = v[i + 1];
		bres_line(surf, x0, y0, x1, y1, color);
	}
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
	int x0, y0, x1, y1, tmp;
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
	int phase = 0;
	int color;

    while(1) {

        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

        if (event.type == SDL_KEYDOWN) {
            if (phase == 0) {
				draw_rect(screenSurface, 0, 0, w, h, 0);
				init_poly(&poly);
				color = 0x0033cc; //(rnd(256) << 16) + (rnd(256) << 8) + rnd(256);
				draw_poly(screenSurface, &poly, 0xffffff);
			} else {
	            fill_poly(screenSurface, &poly, color);
			}
			phase = 1 - phase;
            SDL_UpdateWindowSurface(window);
        }        
    }

    SDL_FreeSurface(screenSurface);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
