#include "../Rendering.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "../../WindowsWrapper.h"

#include "../Misc.h"
#include "../SDL2/Window.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct RenderBackend_Surface
{
	unsigned char *pixels;
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
} RenderBackend_Surface;

typedef struct RenderBackend_Glyph
{
	unsigned char *pixels;
	unsigned int width;
	unsigned int height;
} RenderBackend_Glyph;

static SDL_Renderer *renderer;
static SDL_Texture *texture;

static RenderBackend_Surface framebuffer;

static unsigned char glyph_colour_channels[3];
static RenderBackend_Surface *glyph_destination_surface;

SDL_Window *window;

RenderBackend_Surface* RenderBackend_Init(const char *window_title, int screen_width, int screen_height, BOOL fullscreen, BOOL vsync)
{
	puts("Available SDL2 render drivers:");

	for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		puts(info.name);
	}

	window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_RESIZABLE);

	if (window != NULL)
	{
		if (fullscreen)
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

	#if SDL_VERSION_ATLEAST(2,0,10)
		SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");	// We never interfere with the renderer, so don't let SDL implicitly disable batching
	#endif

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | (vsync ? SDL_RENDERER_PRESENTVSYNC : 0));

		if (renderer != NULL)
		{
			SDL_RendererInfo info;
			SDL_GetRendererInfo(renderer, &info);
			printf("Selected SDL2 render driver: %s\n", info.name);

			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

			if (texture != NULL)
			{
				SDL_LockTexture(texture, NULL, (void**)&framebuffer.pixels, (int*)&framebuffer.pitch);

				Backend_PostWindowCreation();

				framebuffer.width = screen_width;
				framebuffer.height = screen_height;

				return &framebuffer;
			}

			SDL_DestroyRenderer(renderer);
		}
		else
		{
			Backend_ShowMessageBox("Fatal error (software rendering backend)", "Could not create framebuffer surface");
		}

		SDL_DestroyWindow(window);
	}
	else
	{
		Backend_ShowMessageBox("Fatal error (software rendering backend)", "Could not create window");
	}

	return NULL;
}

void RenderBackend_Deinit(void)
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void RenderBackend_DrawScreen(void)
{
	SDL_UnlockTexture(texture);

	int renderer_width, renderer_height;
	SDL_GetRendererOutputSize(renderer, &renderer_width, &renderer_height);

	int texture_width, texture_height;
	SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);

	SDL_Rect dst_rect;
	if ((float)renderer_width / texture_width < (float)renderer_height / texture_height)
	{
		dst_rect.w = renderer_width;
		dst_rect.h = (int)(texture_height * (float)renderer_width / texture_width);
		dst_rect.x = 0;
		dst_rect.y = (renderer_height - dst_rect.h) / 2;
	}
	else
	{
		dst_rect.w = (int)(texture_width * (float)renderer_height / texture_height);
		dst_rect.h = renderer_height;
		dst_rect.x = (renderer_width - dst_rect.w) / 2;
		dst_rect.y = 0;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, &dst_rect);

	SDL_RenderPresent(renderer);

	SDL_LockTexture(texture, NULL, (void**)&framebuffer.pixels, (int*)&framebuffer.pitch);
}

RenderBackend_Surface* RenderBackend_CreateSurface(unsigned int width, unsigned int height)
{
	RenderBackend_Surface *surface = (RenderBackend_Surface*)malloc(sizeof(RenderBackend_Surface));

	if (surface == NULL)
		return NULL;

	surface->pixels = (unsigned char*)malloc(width * height * 4);

	if (surface->pixels == NULL)
	{
		free(surface);
		return NULL;
	}

	surface->width = width;
	surface->height = height;
	surface->pitch = width * 4;

	return surface;
}

void RenderBackend_FreeSurface(RenderBackend_Surface *surface)
{
	if (surface == NULL)
		return;

	free(surface->pixels);
	free(surface);
}

BOOL RenderBackend_IsSurfaceLost(RenderBackend_Surface *surface)
{
	(void)surface;

	return FALSE;
}

void RenderBackend_RestoreSurface(RenderBackend_Surface *surface)
{
	(void)surface;
}

unsigned char* RenderBackend_LockSurface(RenderBackend_Surface *surface, unsigned int *pitch, unsigned int width, unsigned int height)
{
	(void)width;
	(void)height;

	if (surface == NULL)
		return NULL;

	*pitch = surface->pitch;
	return surface->pixels;
}

void RenderBackend_UnlockSurface(RenderBackend_Surface *surface, unsigned int width, unsigned int height)
{
	(void)surface;
	(void)width;
	(void)height;
}

void RenderBackend_Blit(RenderBackend_Surface *source_surface, const RECT *rect, RenderBackend_Surface *destination_surface, long x, long y, BOOL alpha_blend)
{
	if (source_surface == NULL || destination_surface == NULL)
		return;

	RECT rect_clamped;

	rect_clamped.left = rect->left;
	rect_clamped.top = rect->top;
	rect_clamped.right = rect->right;
	rect_clamped.bottom = rect->bottom;

	// Clamp the rect and coordinates so we don't write outside the pixel buffer
	long overflow;

	overflow = 0 - x;
	if (overflow > 0)
	{
		rect_clamped.left += overflow;
		x += overflow;
	}

	overflow = 0 - y;
	if (overflow > 0)
	{
		rect_clamped.top += overflow;
		y += overflow;
	}

	overflow = (x + (rect_clamped.right - rect_clamped.left)) - destination_surface->width;
	if (overflow > 0)
	{
		rect_clamped.right -= overflow;
	}

	overflow = (y + (rect_clamped.bottom - rect_clamped.top)) - destination_surface->height;
	if (overflow > 0)
	{
		rect_clamped.bottom -= overflow;
	}

	if (rect_clamped.bottom - rect_clamped.top <= 0)
		return;

	if (rect_clamped.right - rect_clamped.left <= 0)
		return;

	for (long j = 0; j < rect_clamped.bottom - rect_clamped.top; ++j)
	{
		unsigned char *source_pointer = &source_surface->pixels[((rect_clamped.top + j) * source_surface->pitch) + (rect_clamped.left * 4)];
		unsigned char *destination_pointer = &destination_surface->pixels[((y + j) * destination_surface->pitch) + (x * 4)];

		for (long i = 0; i < rect_clamped.right - rect_clamped.left; ++i)
		{
			if (source_pointer[3] == 0xFF)
			{
				*destination_pointer++ = *source_pointer++;
				*destination_pointer++ = *source_pointer++;
				*destination_pointer++ = *source_pointer++;
				*destination_pointer++ = *source_pointer++;
			}
			else if (source_pointer[3] != 0)
			{
				const float src_alpha = source_pointer[3] / 255.0f;
				const float dst_alpha = destination_pointer[3] / 255.0f;
				const float out_alpha = src_alpha + dst_alpha * (1.0f - src_alpha);

				for (unsigned int j = 0; j < 3; ++j)
					destination_pointer[j] = (unsigned char)((source_pointer[j] * src_alpha + destination_pointer[j] * dst_alpha * (1.0f - src_alpha)) / out_alpha);

				destination_pointer[3] = (unsigned char)(out_alpha * 255.0f);

				source_pointer += 4;
				destination_pointer += 4;
			}
			else
			{
				source_pointer += 4;
				destination_pointer += 4;
			}
		}
	}
}

void RenderBackend_ColourFill(RenderBackend_Surface *surface, const RECT *rect, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	if (surface == NULL)
		return;

	RECT rect_clamped;

	rect_clamped.left = rect->left;
	rect_clamped.top = rect->top;
	rect_clamped.right = rect->right;
	rect_clamped.bottom = rect->bottom;

	// Clamp the rect so it doesn't write outside the pixel buffer
	long overflow;

	overflow = 0 - rect_clamped.left;
	if (overflow > 0)
	{
		rect_clamped.left += overflow;
	}

	overflow = 0 - rect_clamped.top;
	if (overflow > 0)
	{
		rect_clamped.top += overflow;
	}

	overflow = rect_clamped.right - surface->width;
	if (overflow > 0)
	{
		rect_clamped.right -= overflow;
	}

	overflow = rect_clamped.bottom - surface->height;
	if (overflow > 0)
	{
		rect_clamped.bottom -= overflow;
	}

	if (rect_clamped.bottom - rect_clamped.top <= 0)
		return;

	if (rect_clamped.right - rect_clamped.left <= 0)
		return;

	for (long j = 0; j < rect_clamped.bottom - rect_clamped.top; ++j)
	{
		unsigned char *destination_pointer = &surface->pixels[((rect_clamped.top + j) * surface->pitch) + (rect_clamped.left * 4)];

		for (long i = 0; i < rect_clamped.right - rect_clamped.left; ++i)
		{
			*destination_pointer++ = red;
			*destination_pointer++ = green;
			*destination_pointer++ = blue;
			*destination_pointer++ = alpha;
		}
	}
}

RenderBackend_Glyph* RenderBackend_LoadGlyph(const unsigned char *pixels, unsigned int width, unsigned int height, int pitch)
{
	RenderBackend_Glyph *glyph = (RenderBackend_Glyph*)malloc(sizeof(RenderBackend_Glyph));

	if (glyph == NULL)
		return NULL;

	glyph->pixels = (unsigned char*)malloc(width * height);

	if (glyph->pixels == NULL)
	{
		free(glyph);
		return NULL;
	}

	for (unsigned int y = 0; y < height; ++y)
		memcpy(&glyph->pixels[y * width], &pixels[y * pitch], width);

	glyph->width = width;
	glyph->height = height;

	return glyph;
}

void RenderBackend_UnloadGlyph(RenderBackend_Glyph *glyph)
{
	if (glyph == NULL)
		return;

	free(glyph->pixels);
	free(glyph);
}

void RenderBackend_PrepareToDrawGlyphs(RenderBackend_Surface *destination_surface, const unsigned char *colour_channels)
{
	if (destination_surface == NULL)
		return;

	glyph_destination_surface = destination_surface;

	memcpy(glyph_colour_channels, colour_channels, sizeof(glyph_colour_channels));
}

void RenderBackend_DrawGlyph(RenderBackend_Glyph *glyph, long x, long y)
{
	if (glyph == NULL)
		return;

	for (unsigned int iy = MAX(-y, 0); y + iy < MIN(y + glyph->height, glyph_destination_surface->height); ++iy)
	{
		for (unsigned int ix = MAX(-x, 0); x + ix < MIN(x + glyph->width, glyph_destination_surface->width); ++ix)
		{
			const unsigned char src_alpha_int = glyph->pixels[iy * glyph->width + ix];

			if (src_alpha_int != 0)
			{
				const float src_alpha = src_alpha_int / 255.0f;

				unsigned char *bitmap_pixel = glyph_destination_surface->pixels + (y + iy) * glyph_destination_surface->pitch + (x + ix) * 4;

				const float dst_alpha = bitmap_pixel[3] / 255.0f;
				const float out_alpha = src_alpha + dst_alpha * (1.0f - src_alpha);

				for (unsigned int j = 0; j < 3; ++j)
					bitmap_pixel[j] = (unsigned char)((glyph_colour_channels[j] * src_alpha + bitmap_pixel[j] * dst_alpha * (1.0f - src_alpha)) / out_alpha);	// Alpha blending			// Gamma-corrected alpha blending

				bitmap_pixel[3] = (unsigned char)(out_alpha * 255.0f);
			}
		}
	}
}

void RenderBackend_FlushGlyphs(void)
{
	
}

void RenderBackend_HandleRenderTargetLoss(void)
{
	// No problem for us
}

void RenderBackend_HandleWindowResize(unsigned int width, unsigned int height)
{
	// No problem for us
}
