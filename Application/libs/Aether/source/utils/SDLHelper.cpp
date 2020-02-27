#include "SDLHelper.hpp"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>

// === SDL RENDERING ===
// SDL Renderer instance
static SDL_Renderer * renderer;
// SDL Window instance
static SDL_Window * window;

// === FONT OBJECTS ===
// Stores data about Extended font (icons)
static PlFontData ext_font_data;
// Stores data about Standard font
static PlFontData std_font_data;
// Map filled with TTF_Font * (standard) - referenced by font size and created if doesn't exist
// Also used for custom font data
static std::unordered_map<int, TTF_Font *> std_font;
// Same but for extended
static std::unordered_map<int, TTF_Font *> ext_font;
// Set true if a custom font is used
static bool customFont;
static std::string customFontPath;

// === MISCELLANEOUS ===
// Offset position
static int offsetX;
static int offsetY;
// Set to current blend mode
static SDL_BlendMode tex_blend_mode;

namespace SDLHelper {
    bool initSDL() {
        // Init main SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
            return false;
        }

        // Initialize SDL_ttf
        if (TTF_Init() == -1) {
            return false;
        }

        // Create SDL Window
        window = SDL_CreateWindow("window", 0, 0, 1280, 720, 0);
        if (!window) {
            return false;
        }

        // Create SDL Renderer
        renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            return false;
        }

        // Prepare controller (only railed for now)
        if (SDL_JoystickOpen(0) == NULL) {
            return false;
        }

        // Set up blending
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
        tex_blend_mode = SDL_BLENDMODE_BLEND;

        // Ensure offset starts at (0, 0)
        offsetX = 0;
        offsetY = 0;

        // Load fonts
        customFont = false;
        Result rc = plInitialize();
        if (!R_SUCCEEDED(rc)) {
            return false;
        }
        plGetSharedFontByType(&std_font_data, PlSharedFontType_Standard);
        plGetSharedFontByType(&ext_font_data, PlSharedFontType_NintendoExt);
        return true;
    }

    void exitSDL() {
        // Delete created fonts
        plExit();
        for (auto it = std_font.begin(); it != std_font.end(); it++) {
            TTF_CloseFont(it->second);
        }
        for (auto it = ext_font.begin(); it != ext_font.end(); it++) {
            TTF_CloseFont(it->second);
        }

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }

    void clearScreen(SDL_Color c) {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderClear(renderer);
    }

    SDL_Texture * createTexture(int w, int h) {
        return SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
    }

    void destroySurface(SDL_Surface * s) {
        SDL_FreeSurface(s);
    }

    void destroyTexture(SDL_Texture * t) {
        SDL_DestroyTexture(t);
    }

    void getDimensions(SDL_Texture * t, int * w, int * h) {
        SDL_QueryTexture(t, nullptr, nullptr, w, h);
    }

    SDL_Texture * surfaceToTexture(SDL_Surface * s) {
        return SDL_CreateTextureFromSurface(renderer, s);
    }

    void getOffset(int * x, int * y) {
        *x = offsetX;
        *y = offsetY;
    }

    void setOffset(int x, int y) {
        offsetX = x;
        offsetY = y;
    }

    void setFont(std::string p) {
        if (p == "") {
            customFont = false;
        } else {
            customFontPath = p;
            customFont = true;
        }
        // Empty standard font cache
        for (auto it = std_font.begin(); it != std_font.end(); it++) {
            TTF_CloseFont(it->second);
        }
        std_font.clear();
    }

    void renderToScreen() {
        SDL_SetRenderTarget(renderer, nullptr);
    }

    void renderToTexture(SDL_Texture * t) {
        SDL_SetRenderTarget(renderer, t);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderClear(renderer);
    }

    SDL_BlendMode getBlendMode() {
        return tex_blend_mode;
    }

    void setBlendMode(SDL_BlendMode b) {
        tex_blend_mode = b;
    }

    void setRendererBlendMode(SDL_BlendMode b) {
        SDL_SetRenderDrawBlendMode(renderer, b);
    }

    // === DRAWING FUNCTIONS ===

    void draw() {
        SDL_RenderPresent(renderer);
    }

    void drawEllipse(SDL_Color c, int x, int y, unsigned int w, unsigned int h) {
        filledEllipseRGBA(renderer, x + offsetX, y + offsetY, w/2, h/2, c.r, c.g, c.b, c.a);
        aaellipseRGBA(renderer, x + offsetX, y + offsetY, w/2, h/2, c.r, c.g, c.b, c.a);
    }

    void drawFilledRect(SDL_Color c, int x, int y, int w, int h) {
        SDL_Rect rr = {x + offsetX, y + offsetY, w, h};
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(renderer, &rr);
    }

    void drawFilledRoundRect(SDL_Color c, int x, int y, int w, int h, unsigned int r) {
        roundedBoxRGBA(renderer, offsetX + (x + w - 2), offsetY + y, offsetX + x, offsetY + (y + h - 2), r, c.r, c.g, c.b, c.a);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    void drawRoundRect(SDL_Color c, int x, int y, int w, int h, unsigned int r, unsigned int b) {
        for (unsigned int i = 0; i < b; i++) {
            roundedRectangleRGBA(renderer, (x + offsetX + w) - i, y + offsetY + i, offsetX + x + i, (offsetY + y + h) - i, r, c.r, c.g, c.b, c.a);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    void drawRect(SDL_Color c, int x, int y, int w, int h, unsigned int b) {
        for (unsigned int i = 0; i < b; i++) {
            rectangleRGBA(renderer, (x + offsetX + w) - i, y + offsetY + i, x + offsetX + i, (y + offsetY + h) - i, c.r, c.g, c.b, c.a);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    void drawTexture(SDL_Texture * tex, SDL_Color c, int x, int y, int w, int h, int tx, int ty, int tw, int th) {
        // Set color
        SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
        SDL_SetTextureAlphaMod(tex, c.a);
        SDL_SetTextureBlendMode(tex, tex_blend_mode);

        // Get dimensions
        int width, height;
        SDL_QueryTexture(tex, nullptr, nullptr, &width, &height);
        if (w == -1) {
            w = width;
        }
        if (h == -1) {
            h = height;
        }

        // Scale if necessary + render
        SDL_Rect r = {x + offsetX, y + offsetY, w, h};
        if (tx != -1 && tw != -1 && ty != -1 && th != -1) {
            SDL_Rect s = {tx, ty, tw, th};
            SDL_RenderCopy(renderer, tex, &s, &r);
        } else {
            SDL_RenderCopy(renderer, tex, nullptr, &r);
        }

        // Reset color
        SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_SetTextureAlphaMod(tex, 255);
    }

    // === RENDERING FUNCTIONS ===

    SDL_Texture * renderEllipse(unsigned int xd, unsigned int yd) {
        SDL_Texture * tex = createTexture(xd + 4, yd + 4);
        renderToTexture(tex);
        drawEllipse(SDL_Color{255, 255, 255, 255}, xd/2 + 2, yd/2 + 2, xd, yd);
        renderToScreen();
        return tex;
    }

    SDL_Texture * renderFilledRect(int w, int h) {
        SDL_Texture * tex = createTexture(w, h);
        renderToTexture(tex);
        drawFilledRect(SDL_Color{255, 255, 255, 255}, 0, 0, w, h);
        renderToScreen();
        return tex;
    }

    SDL_Texture * renderFilledRoundRect(int w, int h, unsigned int c) {
        SDL_Texture * tex = createTexture(w, h);
        renderToTexture(tex);
        drawFilledRoundRect(SDL_Color{255, 255, 255, 255}, 0, 0, w, h, c);
        renderToScreen();
        return tex;
    }

    SDL_Texture * renderRoundRect(int w, int h, unsigned int r, unsigned int b) {
        SDL_Texture * tex = createTexture(w, h);
        renderToTexture(tex);
        drawRoundRect(SDL_Color{255, 255, 255, 255}, 0, 0, w, h, r, b);
        renderToScreen();
        return tex;
    }

    SDL_Texture * renderRect(int w, int h, unsigned int b) {
        SDL_Texture * tex = createTexture(w, h);
        renderToTexture(tex);
        drawRect(SDL_Color{255, 255, 255, 255}, 0, 0, w, h, b);
        renderToScreen();
        return tex;
    }

    SDL_Texture * renderImage(std::string path) {
        SDL_Surface * tmp = renderImageSurface(path);
        SDL_Texture * tex = surfaceToTexture(tmp);
        destroySurface(tmp);
        return tex;
    }

    SDL_Texture * renderImage(u8 * ptr, size_t size) {
        SDL_Surface * tmp = renderImageSurface(ptr, size);
        SDL_Texture * tex = surfaceToTexture(tmp);
        destroySurface(tmp);
        return tex;
    }

    SDL_Texture * renderImageShrinked(u8 * ptr, size_t size, int xF, int yF) {
        SDL_Surface * tmp = renderImageShrinkedSurface(ptr, size, xF, yF);
        SDL_Texture * tex = surfaceToTexture(tmp);
        destroySurface(tmp);
        return tex;
    }

    SDL_Texture * renderText(const char * str, int font_size, int style, bool ext) {
        SDL_Surface * tmp = renderTextSurface(str, font_size, style, ext);
        SDL_Texture * tex = surfaceToTexture(tmp);
        destroySurface(tmp);
        return tex;
    }

    SDL_Texture * renderTextWrapped(const char * str, int font_size, uint32_t max_w, int style, bool ext) {
        SDL_Surface * tmp = renderTextWrappedSurface(str, font_size, max_w, style, ext);
        SDL_Texture * tex = surfaceToTexture(tmp);
        destroySurface(tmp);
        return tex;
    }

    SDL_Surface * renderImageSurface(std::string path) {
        SDL_Surface * tmp = IMG_Load(path.c_str());
        SDL_Surface * tmp2 = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGBA32, 0);
        destroySurface(tmp);
        return tmp2;
    }

    SDL_Surface * renderImageSurface(u8 * ptr, size_t size) {
        SDL_Surface * tmp = IMG_Load_RW(SDL_RWFromMem(ptr, size), 1);
        SDL_Surface * tmp2 = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGBA32, 0);
        destroySurface(tmp);
        return tmp2;
    }

    SDL_Surface * renderImageShrinkedSurface(u8 * ptr, size_t size, int xF, int yF) {
        SDL_Surface * tmp = IMG_Load_RW(SDL_RWFromMem(ptr, size), 1);
        SDL_Surface * tmp2 = shrinkSurface(tmp, xF, yF);
        destroySurface(tmp);
        return tmp2;
    }

    SDL_Surface * renderTextSurface(const char * str, int font_size, int style, bool ext) {
        SDL_Surface * tmp;

        // If font doesn't exist create it
        // Use Extended font
        if (ext) {
            if (ext_font.find(font_size) == ext_font.end()) {
                TTF_Font * f = TTF_OpenFontRW(SDL_RWFromMem(ext_font_data.address, ext_font_data.size), 1, font_size);
                ext_font[font_size] = f;
            }
            tmp = TTF_RenderUTF8_Blended(ext_font[font_size], str, SDL_Color{255, 255, 255, 255});
        // Use standard font
        } else {
            if (std_font.find(font_size) == std_font.end()) {
                TTF_Font * f;
                if (customFont) {
                    f = TTF_OpenFont(customFontPath.c_str(), font_size);
                } else {
                    f = TTF_OpenFontRW(SDL_RWFromMem(std_font_data.address, std_font_data.size), 1, font_size);
                }
                if (TTF_GetFontStyle(f) != style) {
                    TTF_SetFontStyle(f, style);
                }
                std_font[font_size] = f;
            }
            tmp = TTF_RenderUTF8_Blended(std_font[font_size], str, SDL_Color{255, 255, 255, 255});
        }

        return tmp;
    }

    SDL_Surface * renderTextWrappedSurface(const char * str, int font_size, uint32_t max_w, int style, bool ext) {
        SDL_Surface * tmp;

        // If font doesn't exist create it
        // Use Extended font
        if (ext) {
            if (ext_font.find(font_size) == ext_font.end()) {
                TTF_Font * f = TTF_OpenFontRW(SDL_RWFromMem(ext_font_data.address, ext_font_data.size), 1, font_size);
                ext_font[font_size] = f;
            }
            tmp = TTF_RenderUTF8_Blended_Wrapped(ext_font[font_size], str, SDL_Color{255, 255, 255, 255}, max_w);
        // Use standard font
        } else {
            if (std_font.find(font_size) == std_font.end()) {
                TTF_Font * f;
                if (customFont) {
                    f = TTF_OpenFont(customFontPath.c_str(), font_size);
                } else {
                    f = TTF_OpenFontRW(SDL_RWFromMem(std_font_data.address, std_font_data.size), 1, font_size);
                }
                if (TTF_GetFontStyle(f) != style) {
                    TTF_SetFontStyle(f, style);
                }
                std_font[font_size] = f;
            }
            tmp = TTF_RenderUTF8_Blended_Wrapped(std_font[font_size], str, SDL_Color{255, 255, 255, 255}, max_w);
        }

        return tmp;
    }
};