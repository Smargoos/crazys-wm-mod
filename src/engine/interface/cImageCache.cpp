/*
* Copyright 2009, 2010, The Pink Petal Development Team.
* The Pink Petal Devloment Team are defined as the game's coders
* who meet on http://pinkpetal.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "interface/cImageCache.h"
#include "interface/cSurface.h"
#include "interface/sColor.h"
#include "interface/CGraphics.h"
#include "CLog.h"
#include "ffmpeg.h"

#include <string>
#include <stdexcept>
#include <memory>
#include <cassert>
#include <utility>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

// hashing (https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine)
std::size_t combine() { return 0; }

template<class T, class... Args>
std::size_t combine(const T& b, const Args&... args)
{
    std::size_t seed = combine(args...);
    return seed ^ std::hash<T>{}(b) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);
}


std::size_t std::hash<sImageCacheKey>::operator()(sImageCacheKey const& s) const noexcept
{
    return combine(s.file_name, s.transparency, s.width, s.height);
}

bool sImageCacheKey::operator==(const sImageCacheKey& o) const
{
    return std::tie(file_name, transparency, width, height) == std::tie(
            o.file_name, o.transparency, o.width, o.height);
}

cSurface cImageCache::CreateSurface(int width, int height, sColor color, bool transparent)
{
    std::string    fake_name = color.to_hex();
    sImageCacheKey key{fake_name, width, height, transparent};
    auto           lookup = m_SurfaceCache.find(key);
    if(lookup != m_SurfaceCache.end()) {
        // otherwise, return from cache
        m_ImageFoundCount++;
        return cSurface{lookup->second, m_GFX};
    }

    // CreateSDLSurface guarantees that sf is not nullptr, so we don't need to do any additional
    // checking here.
    auto sf = CreateSDLSurface(width, height, transparent);
    if(transparent)
        SDL_FillRect(sf.get(), nullptr, SDL_MapRGBA(sf->format, color.r, color.g, color.b, SDL_ALPHA_TRANSPARENT));
    else
        SDL_FillRect(sf.get(), nullptr, SDL_MapRGB(sf->format, color.r, color.g, color.b));

    return AddToCache(std::move(key), std::move(sf), std::move(fake_name));
}

cSurface cImageCache::LoadDefaultSize(std::string filename, bool transparent) {
    sImageCacheKey key{filename, -1, -1, transparent};
    auto lookup = m_SurfaceCache.find(key);
    if(lookup != m_SurfaceCache.end()) {
        // OK, we have found the original image in the cache
        return cSurface{lookup->second, m_GFX};
    } else {
        auto loaded = surface_ptr_t{IMG_Load(filename.c_str())};
        if (!loaded) {
            throw std::runtime_error("Could not load image '" + filename + "': " + IMG_GetError());
        }

        // convert everything to 32 bit pixel format
        if(loaded->format->BitsPerPixel != 32) {
            loaded.reset(SDL_ConvertSurfaceFormat(loaded.get(), SDL_PIXELFORMAT_ARGB8888, 0));
        }
        ++m_ImageLoadCount;

        // set blend mode based on transparency
        SDL_BlendMode mode;
        SDL_GetSurfaceBlendMode(loaded.get(), &mode);
        int status = SDL_SetSurfaceBlendMode(loaded.get(), transparent ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);

        return AddToCache(std::move(key), std::move(loaded), std::move(filename));
    }
}

namespace {
    void keep_ratio_rescaling(int width, int height, int& target_width, int& target_height, bool use_minimum=true) {
        double scale_x = (double) target_width / (double) width;
        double scale_y = (double) target_height / (double) height;
        double s = use_minimum ? std::min(scale_x, scale_y) : std::max(scale_x, scale_y);
        target_width = width * s;
        target_height = height * s;
    }

    struct sImageSize {
        int Width;
        int Height;
    };

    sImageSize ensure_minimum_dimensions(int width, int height, int min_width, int min_height, bool keep_ratio) {
        int target_width = width;
        int target_height = height;
        if(min_width > 0 && width < min_width) {
            target_width = min_width;
        }

        if(min_height > 0 && height < min_height) {
            target_height = min_height;
        }

        if(keep_ratio) {
            keep_ratio_rescaling(width, height, target_width, target_height, false);
        }

        return {target_width, target_height};
    }

    sImageSize ensure_maximum_dimensions(int width, int height, int max_width, int max_height, bool keep_ratio) {
        int target_width = width;
        int target_height = height;
        if(max_width > 0 && width > max_width) {
            target_width = max_width;
        }

        if(max_height > 0 && height > max_height) {
            target_height = max_height;
        }

        if(keep_ratio) {
            keep_ratio_rescaling(width, height, target_width, target_height, true);
        }

        return {target_width, target_height};
    }
}

cSurface cImageCache::LoadImage(std::string filename, sLoadImageParams params)
{
    auto def_image = LoadDefaultSize(filename, params.Transparency);
    auto& surface = def_image.RawSurface();

    // now, we need to check whether the original image is compatible with the size constraints
    // is it too small?
    sImageSize target = ensure_minimum_dimensions(surface->width(), surface->height(), params.MinWidth, params.MinHeight, params.KeepRatio);
    target = ensure_maximum_dimensions(target.Width, target.Height, params.MaxWidth, params.MaxHeight, params.KeepRatio);

    // If everything fits, we are done here and can return the surface as is
    if(surface->width() == target.Width && surface->height() == target.Height) {
        m_ImageFoundCount++;
        return std::move(def_image);
    }

    // otherwise, we can look up whether we already have the correctly sized image
    sImageCacheKey key{filename, target.Width, target.Height, params.Transparency};
    auto lookup = m_SurfaceCache.find(key);
    if(lookup != m_SurfaceCache.end()) {
        m_ImageFoundCount++;
        return cSurface{lookup->second, m_GFX};
    }

    // not cached, need to resize
    auto resized = RawResize(surface->surface(), target.Width, target.Height);

    // set blend mode based on transparency
    SDL_SetSurfaceBlendMode(resized.get(), params.Transparency ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);

    return AddToCache(std::move(key), std::move(resized), std::move(filename));
}

surface_ptr_t cImageCache::RawResize(SDL_Surface* source, int width, int height) {
    auto target = CreateSDLSurface(width, height, true);
    SDL_Rect inp{0, 0, source->w, source->h};
    SDL_Rect tgt{0, 0, target->w, target->h};
    if (source->format->format != target->format->format) {
        // First adapt pixel format, then scale
        auto intermed = CreateSDLSurface(source->w, source->h, true);
        if (SDL_BlitSurface(source, &inp, intermed.get(), nullptr)) {
            g_LogFile.error("interface", "Could not adapt pixel format: ", SDL_GetError());
        };

        if (SDL_BlitScaled(intermed.get(), &inp, target.get(), &tgt)) {
            g_LogFile.error("interface", "Could not scale surface: ", SDL_GetError());
        }
    } else {
        // Scale directly
        if (SDL_BlitScaled(source, &inp, target.get(), &tgt)) {
            g_LogFile.error("interface", "Could not scale surface: ", SDL_GetError());
        }
    }
    SDL_BlendMode mode = SDL_BLENDMODE_INVALID;
    assert(SDL_GetSurfaceBlendMode(source, &mode) == 0);
    assert(SDL_SetSurfaceBlendMode(target.get(), mode) == 0);
    m_ImageResizeCount++;
    return target;
}

surface_ptr_t cImageCache::ResizeImage(surface_ptr_t input, int width, int height, bool keep_ratio)
{
    if (width != input->w || height != input->h)
    {
        if(width  == -1) { width = input->w; }
        if(height == -1) { height = input->h; }
        if (keep_ratio)  {
            keep_ratio_rescaling(input->w, input->h, width, height);
        }

        return RawResize(input.get(), width, height);
    }

    return input;
}

cImageCache::~cImageCache()
{
    PrintStats();
}

void cImageCache::PrintStats() const
{
    g_LogFile.debug_table("interface", "ImageCache Stats",
            "Current", m_SurfaceCache.size(),
            "Loaded", m_ImageLoadCount,
            "Found", m_ImageFoundCount,
            "Resized", m_ImageResizeCount,
            "Created", m_ImageCreateCount,
            "Culled", m_ImageCullCount
            );
}

void cImageCache::Cull()
{
    std::size_t freeable_size = 0;
    std::size_t total = 0;
    for(auto& entry : m_SurfaceCache) {
        if(entry.second.use_count() == 1) {
            freeable_size += entry.second->bytes();
        }
        total += entry.second->bytes();
    }

    // if we cannot free more than 50MB, leave stuff in cache.
    if(freeable_size < 1024*1024*50)
        return;

    g_LogFile.info("interface", "Culling ", freeable_size / 1024 / 1024, "MB of ", total / 1024 / 1024);

    // TODO keep some cullable surfaces in cache based on usage
    for(auto entry = m_SurfaceCache.begin(); entry != m_SurfaceCache.end();) {
        if(entry->second.use_count() == 1) {
            entry = m_SurfaceCache.erase(entry);
            m_ImageCullCount++;
        } else {
            ++entry;
        }
    }
}

cSurface cImageCache::AddToCache(sImageCacheKey key, surface_ptr_t surface, std::string name)
{
    auto cached = std::make_shared<cCachedSurface>(std::move(surface), std::move(name));

    if(m_AddedSize > 10'000'000) {
        Cull();
        m_AddedSize = 0;
    }
    m_AddedSize += cached->bytes();
    m_SurfaceCache.emplace(std::move(key), cached);
    return cSurface(std::move(cached), m_GFX);
}

surface_ptr_t cImageCache::CreateSDLSurface(int width, int height, bool transparent)
{
    assert(width > 0);
    assert(height > 0);
    if(width <= 0 || height <= 0) {
        throw std::logic_error("Invalid size specified for SDLSurface");
    }
    m_ImageCreateCount++;
    auto sf = SDL_CreateRGBSurface(0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if(!sf) {
        throw std::runtime_error("Could not create SDL surface");
    }
    if(transparent) {
        SDL_SetSurfaceBlendMode(sf, SDL_BLENDMODE_BLEND);
    }
    return surface_ptr_t{sf};
}

surface_ptr_t cImageCache::CloneSDLSurface(SDL_Surface * sf)
{
    m_ImageCreateCount++;
    return surface_ptr_t{SDL_ConvertSurface(sf, sf->format, sf->flags)};
}

cSurface cImageCache::FillRect(const cSurface& source, SDL_Rect area, sColor color)
{
    surface_ptr_t surface;
    const auto& raw = *source.RawSurface();
    // make a copy of the old surface
    auto target = CloneSDLSurface(raw.surface());
    // draw the rect
    SDL_FillRect(target.get(), &area, SDL_MapRGB(raw.surface()->format, color.r, color.g, color.b));

    // and create a new cache entry
    std::stringstream name;
    name << "CustomSurface#";
    name << m_ImageCreateCount;
    name << "@FillRect";
    return AddToCache(sImageCacheKey{std::move(name.str()), (int)raw.width(), (int)raw.height(), false}, std::move(target), source.GetFileName());
}

cSurface cImageCache::BlitSurface(const cSurface& target, SDL_Rect* target_area, const cSurface& source, SDL_Rect* src_area)
{
    surface_ptr_t surface;
    const auto& raw = *target.RawSurface();
    // make a copy of the old surface
    auto new_surface = CloneSDLSurface(raw.surface());
    // blit the other surface
    SDL_BlitSurface(source.RawSurface()->surface(), src_area, new_surface.get(), target_area);
    auto ccs =  std::make_shared<cCachedSurface>(std::move(new_surface), target.GetFileName());
    return cSurface(std::move(ccs), m_GFX);
}

struct sFrameData {
    unsigned char* Data;
    int Wrap;
    int Width;
    int Height;
    long Time;
};

// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decode_video.c
// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gae80afec6f26df6607eaacf39b561c315

class sConverter {
public:
    sConverter(int src_w, int src_h, AVPixelFormat src_fmt, int dst_w, int dst_h, AVPixelFormat dst_fmt) :
            m_Context(sws_getContext(src_w, src_h, src_fmt,
                                     dst_w, dst_h, dst_fmt,
                                     SWS_BILINEAR | SWS_ACCURATE_RND, nullptr, nullptr, nullptr)) {
        if (!m_Context) {
            g_LogFile.error("ffmpeg", "Impossible to create scale context for the conversion fmt: ",
                            av_get_pix_fmt_name(src_fmt), " s: ", src_w, "x", src_h, " ->  fmt: ",
                            av_get_pix_fmt_name(dst_fmt), " s: ", dst_w, "x", dst_h);
        }
    }
    ~sConverter() {
        sws_freeContext(m_Context);
    }

    void scale(const AVFrame* src, ffmpeg::sImageBuffer& target) {
        sws_scale(m_Context, src->data, src->linesize, 0, src->height, target.Data, target.Linesize);
    }
private:
    SwsContext* m_Context;
};

template<class F>
void ReadMovie(std::string& file_name, sLoadImageParams params, F&& callback) {
    auto format = ffmpeg::open_format(file_name.c_str());
    find_stream_info(format);

    AVCodec* dec = nullptr;
    int stream_index = av_find_best_stream(format.get(), AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (stream_index < 0) {
        throw std::runtime_error("Could not find video stream");
    }

    auto ctx = ffmpeg::alloc_context(dec);
    auto stream = format->streams[stream_index];
    load_parameters(ctx, stream->codecpar);
    open(ctx, dec);

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    sImageSize target = ensure_minimum_dimensions(ctx->width, ctx->height, params.MinWidth, params.MinHeight, params.KeepRatio);
    target = ensure_maximum_dimensions(target.Width, target.Height, params.MaxWidth, params.MaxHeight, params.KeepRatio);

    keep_ratio_rescaling(ctx->width, ctx->height, target.Width, target.Height);

    AVPixelFormat src_fmt = ctx->pix_fmt;
    if(src_fmt == AV_PIX_FMT_NONE) {
        g_LogFile.template warning("ffmpeg", "Pixel format not set for file ", file_name, " Note that animated webp's are currently unsupported.");
        return;
    }
    sConverter converter{ctx->width, ctx->height, src_fmt,
                         target.Width, target.Height, AV_PIX_FMT_RGBA};

    ffmpeg::sImageBuffer converted(target.Width, target.Height, AV_PIX_FMT_RGBA);
    auto converter_callback = [&](ffmpeg::Frame& frame) {

        converter.scale(frame.get(), converted);
        sFrameData data{converted.Data[0], converted.Linesize[0],
                        target.Width, target.Height, (1000 * frame->best_effort_timestamp * stream->time_base.num) / stream->time_base.den};
        callback(data);
    };

    auto frame = ffmpeg::alloc_frame();
    while (av_read_frame(format.get(), &pkt) >= 0) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (pkt.stream_index == stream_index) {
            decode_packet(ctx, frame, &pkt, converter_callback);
        }
        av_packet_unref(&pkt);
    }
    decode_packet(ctx, frame, nullptr, converter_callback);
}

cAnimatedSurface cImageCache::LoadFfmpeg(std::string movie, sLoadImageParams params) {
    std::vector<sAnimationFrame> surfaces;
    auto start = std::chrono::steady_clock::now();
    int frame_num = 0;
    long last_frame = 0;

    auto frame_handler = [&](const sFrameData& frame) {
        std::stringstream name;

        // create SDL surface from image data
        m_ImageCreateCount++;
        // this does not create a copy of the data!
        auto sf = SDL_CreateRGBSurfaceFrom(frame.Data, frame.Width, frame.Height, 32, frame.Wrap,
                                           0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        if(!sf) {
            throw std::runtime_error("Could not create SDL surface");
        }
        // but this does
        auto surface_ptr = surface_ptr_t(SDL_ConvertSurfaceFormat(sf, SDL_PIXELFORMAT_ARGB8888, 0));
        SDL_SetSurfaceBlendMode(surface_ptr.get(), SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(sf);

        name << movie << "@" << frame_num;
        auto surface = AddToCache(sImageCacheKey{std::move(name.str()), frame.Width, frame.Height, false},
                                  std::move(surface_ptr), movie);
        // TODO is this off by one? first frame seems to be at time "0"
        surfaces.push_back({std::move(surface), int(frame.Time - last_frame)});
        last_frame = frame.Time;
        ++frame_num;
    };
    try {
        ReadMovie(movie, params, frame_handler);
    } catch (std::exception& error) {
        g_LogFile.error("ffmpeg", "Error when trying to load image/video from '", movie, "': ", error.what());
        return {};
    }

    long dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    g_LogFile.info("ffmpeg", "Loaded ", movie, " with ", surfaces.size(), " frames in ", dur, "ms");
    return cAnimatedSurface{std::move(movie), std::move(surfaces)};
}

cSurface cImageCache::CreateTextSurface(TTF_Font* font, std::string text, sColor color, bool antialias)
{
    assert(font);

    std::string id = text + color.to_hex();
    sImageCacheKey key{id, -1, TTF_FontHeight(font), antialias};
    auto           lookup = m_SurfaceCache.find(key);
    if(lookup != m_SurfaceCache.end()) {
        // otherwise, return from cache
        m_ImageFoundCount++;
        return cSurface{lookup->second, m_GFX};
    }

    auto sdlColor = SDL_Color{color.r, color.g, color.b, 0};
    auto new_image = antialias ? TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor)
                               : TTF_RenderUTF8_Solid(font, text.c_str(), sdlColor);
    if(new_image)
        return AddToCache(std::move(key), surface_ptr_t{new_image}, std::move(text));
    else
        return cSurface();
}

cImageCache::cImageCache(CGraphics* gfx) : m_GFX(gfx) {
}

void SDLDeleter::operator()(SDL_Surface * surface) const {
    SDL_FreeSurface(surface);
}

void SDLDeleter::operator()(SDL_Texture* texture) const {
    SDL_DestroyTexture(texture);
}


std::size_t cCachedSurface::width() const
{
    return surface()->w;
}

std::size_t cCachedSurface::height() const
{
    return surface()->h;
}

cCachedSurface::cCachedSurface(surface_ptr_t surface, std::string name) :
    m_Name(std::move(name)),
    m_Surface(std::move(surface))
{

}

size_t cCachedSurface::bytes() const
{
    return m_Surface->w * m_Surface->h * m_Surface->format->BytesPerPixel;
}

SDL_Surface* cCachedSurface::surface() const
{
    return m_Surface.get();
}

SDL_Texture* cCachedSurface::texture(SDL_Renderer* renderer) const {
    if(!m_Texture) {
        const_cast<texture_ptr_t&>(m_Texture).reset(SDL_CreateTextureFromSurface(renderer, m_Surface.get()));
    }
    return m_Texture.get();
}
