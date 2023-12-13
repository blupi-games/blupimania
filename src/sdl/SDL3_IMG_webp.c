/*
  SDL_image:  An example image loading library for use with SDL
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* This is a WEBP image file loading framework */

#include <SDL2/SDL_image.h>

/*=============================================================================
        File: SDL_webp.c
     Purpose: A WEBP loader for the SDL library
    Revision:
  Created by: Michael Bonfils (Murlock) (26 November 2011)
              murlock42@gmail.com

=============================================================================*/

#include <SDL2/SDL_endian.h>

#ifdef macintosh
#define MACOS
#endif
#include <webp/decode.h>
#include <webp/demux.h>

static struct {
  int    loaded;
  void * handle_libwebpdemux;
  void * handle_libwebp;
  VP8StatusCode (*WebPGetFeaturesInternal) (
    const uint8_t * data, size_t data_size, WebPBitstreamFeatures * features,
    int decoder_abi_version);
  uint8_t * (*WebPDecodeRGBInto) (
    const uint8_t * data, size_t data_size, uint8_t * output_buffer,
    size_t output_buffer_size, int output_stride);
  uint8_t * (*WebPDecodeRGBAInto) (
    const uint8_t * data, size_t data_size, uint8_t * output_buffer,
    size_t output_buffer_size, int output_stride);
  WebPDemuxer * (*WebPDemuxInternal) (
    const WebPData * data, int allow_partial, WebPDemuxState * state,
    int version);
  int (*WebPDemuxGetFrame) (
    const WebPDemuxer * dmux, int frame_number, WebPIterator * iter);
  uint32_t (*WebPDemuxGetI) (
    const WebPDemuxer * dmux, WebPFormatFeature feature);
  void (*WebPDemuxDelete) (WebPDemuxer * dmux);
} lib;

#define FUNCTION_LOADER_LIBWEBP(FUNC, SIG)   \
  lib.FUNC = FUNC;                           \
  if (lib.FUNC == NULL)                      \
  {                                          \
    IMG_SetError ("Missing webp.framework"); \
    return -1;                               \
  }
#define FUNCTION_LOADER_LIBWEBPDEMUX(FUNC, SIG)   \
  lib.FUNC = FUNC;                                \
  if (lib.FUNC == NULL)                           \
  {                                               \
    IMG_SetError ("Missing webpdemux.framework"); \
    return -1;                                    \
  }

static int
SDL3_IMG_InitWEBP (void)
{
  if (lib.loaded == 0)
  {
    FUNCTION_LOADER_LIBWEBP (
      WebPGetFeaturesInternal,
      VP8StatusCode (*) (
        const uint8_t * data, size_t data_size,
        WebPBitstreamFeatures * features, int decoder_abi_version))
    FUNCTION_LOADER_LIBWEBP (
      WebPDecodeRGBInto,
      uint8_t *
        (*) (const uint8_t * data, size_t data_size, uint8_t * output_buffer, size_t output_buffer_size, int output_stride))
    FUNCTION_LOADER_LIBWEBP (
      WebPDecodeRGBAInto,
      uint8_t *
        (*) (const uint8_t * data, size_t data_size, uint8_t * output_buffer, size_t output_buffer_size, int output_stride))
    FUNCTION_LOADER_LIBWEBPDEMUX (
      WebPDemuxInternal,
      WebPDemuxer * (*) (const WebPData *, int, WebPDemuxState *, int) )
    FUNCTION_LOADER_LIBWEBPDEMUX (
      WebPDemuxGetFrame,
      int (*) (const WebPDemuxer * dmux, int frame_number, WebPIterator * iter))
    FUNCTION_LOADER_LIBWEBPDEMUX (
      WebPDemuxGetI,
      uint32_t (*) (const WebPDemuxer * dmux, WebPFormatFeature feature));
    FUNCTION_LOADER_LIBWEBPDEMUX (
      WebPDemuxDelete, void (*) (WebPDemuxer * dmux))
  }
  ++lib.loaded;

  return 0;
}

static int
webp_getinfo (SDL_RWops * src, size_t * datasize)
{
  Sint64 start, size;
  int    is_WEBP;
  Uint8  magic[20];

  if (!src)
  {
    return 0;
  }
  start   = SDL_RWtell (src);
  is_WEBP = 0;
  if (SDL_RWread (src, magic, sizeof (magic), 1) == 1)
  {
    if (
      magic[0] == 'R' && magic[1] == 'I' && magic[2] == 'F' &&
      magic[3] == 'F' && magic[8] == 'W' && magic[9] == 'E' &&
      magic[10] == 'B' && magic[11] == 'P' && magic[12] == 'V' &&
      magic[13] == 'P' && magic[14] == '8' &&
      (magic[15] == ' ' || magic[15] == 'X' || magic[15] == 'L'))
    {
      is_WEBP = 1;
      if (datasize)
      {
        size = SDL_RWsize (src);
        if (size > 0)
        {
          *datasize = (size_t) (size - start);
        }
        else
        {
          *datasize = 0;
        }
      }
    }
  }
  SDL_RWseek (src, start, RW_SEEK_SET);
  return (is_WEBP);
}

static IMG_Animation *
BM_IMG_LoadWEBPAnimation_RW (SDL_RWops * src)
{
  Sint64                start;
  const char *          error = NULL;
  Uint32                format;
  WebPBitstreamFeatures features;
  struct WebPDemuxer *  dmuxer = NULL;
  WebPIterator          iter;
  IMG_Animation *       anim = NULL;
  size_t                raw_data_size;
  uint8_t *             raw_data = NULL;
  uint8_t *             ret;
  int                   frame_idx;
  WebPData              wd;

  if (!src)
  {
    /* The error message has been set in SDL_RWFromFile */
    return NULL;
  }

  start = SDL_RWtell (src);

  if (SDL3_IMG_InitWEBP () != 0)
  {
    goto error;
  }

  raw_data_size = -1;
  if (!webp_getinfo (src, &raw_data_size))
  {
    error = "Invalid WEBP Animation";
    goto error;
  }

  raw_data = (uint8_t *) SDL_malloc (raw_data_size);
  if (raw_data == NULL)
  {
    error = "Failed to allocate enough buffer for WEBP Animation";
    goto error;
  }

  if (SDL_RWread (src, raw_data, raw_data_size, 1) != 1)
  {
    error = "Failed to read WEBP Animation";
    goto error;
  }

  if (
    lib.WebPGetFeaturesInternal (
      raw_data, raw_data_size, &features, WEBP_DECODER_ABI_VERSION) !=
    VP8_STATUS_OK)
  {
    error = "WebPGetFeatures has failed";
    goto error;
  }

  if (features.has_alpha)
  {
    format = SDL_PIXELFORMAT_RGBA32;
  }
  else
  {
    format = SDL_PIXELFORMAT_RGB24;
  }

  wd.size     = raw_data_size;
  wd.bytes    = raw_data;
  dmuxer      = lib.WebPDemuxInternal (&wd, 0, NULL, WEBP_DEMUX_ABI_VERSION);
  anim        = (IMG_Animation *) SDL_malloc (sizeof (IMG_Animation));
  anim->w     = features.width;
  anim->h     = features.height;
  anim->count = lib.WebPDemuxGetI (dmuxer, WEBP_FF_FRAME_COUNT);
  anim->frames =
    (SDL_Surface **) SDL_calloc (anim->count, sizeof (*anim->frames));
  anim->delays = (int *) SDL_calloc (anim->count, sizeof (*anim->delays));
  for (frame_idx = 0; frame_idx < (anim->count); frame_idx++)
  {
    SDL_Surface * curr;
    if (lib.WebPDemuxGetFrame (dmuxer, frame_idx, &iter) == 0)
    {
      break;
    }
    curr = SDL_CreateRGBSurfaceWithFormat (0, features.width, features.height, features.has_alpha ? 32 : 24, format);
    if (curr == NULL)
    {
      error = "Failed to allocate SDL_Surface";
      goto error;
    }
    anim->frames[frame_idx] = curr;
    anim->delays[frame_idx] = iter.duration;
    if (features.has_alpha)
    {
      ret = lib.WebPDecodeRGBAInto (
        iter.fragment.bytes, iter.fragment.size, (uint8_t *) curr->pixels,
        curr->pitch * curr->h, curr->pitch);
    }
    else
    {
      ret = lib.WebPDecodeRGBInto (
        iter.fragment.bytes, iter.fragment.size, (uint8_t *) curr->pixels,
        curr->pitch * curr->h, curr->pitch);
    }
    if (ret == NULL)
    {
      break;
    }
  }
  if (dmuxer)
  {
    lib.WebPDemuxDelete (dmuxer);
  }

  if (raw_data)
  {
    SDL_free (raw_data);
  }
  return anim;
error:
  if (anim)
  {
    IMG_FreeAnimation (anim);
  }
  if (dmuxer)
  {
    lib.WebPDemuxDelete (dmuxer);
  }
  if (raw_data)
  {
    SDL_free (raw_data);
  }

  if (error)
  {
    IMG_SetError ("%s", error);
  }
  SDL_RWseek (src, start, RW_SEEK_SET);
  return NULL;
}

/* Load an animation from an SDL datasource, optionally specifying the type */
IMG_Animation *
BM_IMG_LoadWEBPAnimationTyped_RW (SDL_RWops *src, SDL_bool freesrc, const char *type)
{
    IMG_Animation *anim;

    /* Make sure there is something to do.. */
    if ( src == NULL ) {
        IMG_SetError("Passed a NULL data source");
        return(NULL);
    }

    /* See whether or not this data source can handle seeking */
    if ( SDL_RWseek(src, 0, RW_SEEK_CUR) < 0 ) {
        IMG_SetError("Can't seek in this data source");
        if (freesrc)
            SDL_RWclose(src);
        return(NULL);
    }

    anim = BM_IMG_LoadWEBPAnimation_RW (src);
    if (freesrc)
        SDL_RWclose(src);
    return anim;
}

/* Load an animation from a file */
IMG_Animation *
BM_IMG_LoadWEBPAnimation (const char *file)
{
    SDL_RWops *src = SDL_RWFromFile(file, "rb");
    const char *ext = SDL_strrchr(file, '.');
    if (ext) {
        ext++;
    }
    if (!src) {
        /* The error message has been set in SDL_RWFromFile */
        return NULL;
    }
    return BM_IMG_LoadWEBPAnimationTyped_RW (src, SDL_TRUE, ext);
}

