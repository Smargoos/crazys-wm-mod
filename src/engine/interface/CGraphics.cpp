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
#include "interface/CGraphics.h"
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "CLog.h"
#include "utils/DirPath.h"
#include "interface/sColor.h"
#include "cTimer.h"
#include "interface/cFont.h"
#include "interface/cTheme.h"

CGraphics::CGraphics() : m_ImageCache(this)
{
    m_FPS = std::make_unique<cTimer>();
}

CGraphics::~CGraphics()
{
    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
    TTF_Quit();
    SDL_Quit();
}

void CGraphics::Begin()
{
    m_FPS->Start();
    if(SDL_RenderClear(m_Renderer)) {
        g_LogFile.error("interface", "Could not clear render: ", SDL_GetError());
    }
    if(m_BackgroundImage) {
        m_BackgroundImage.DrawSurface(0, 0);
    }
}

bool CGraphics::End()
{
    SDL_RenderPresent(m_Renderer);

    // Maintain framerate
    if((m_FPS->GetTicks() < 1000 / FRAMES_PER_SECOND))
    {
        //Sleep the remaining frame time
        SDL_Delay((1000/FRAMES_PER_SECOND)-m_FPS->GetTicks());
    }

    return true;
}

bool CGraphics::InitGraphics(const std::string& caption, int WindowWidth, int WindowHeight, bool Fullscreen)
{
    m_ScreenWidth = WindowWidth;
    m_ScreenHeight = WindowHeight;
    m_Fullscreen = Fullscreen;

    // init SDL
    g_LogFile.info("interface", "Initializing SDL");
    if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        g_LogFile.error("interface", "Poop. SDL_INIT_EVERYTHING failed.");
        g_LogFile.error("interface", SDL_GetError());
        return false;
    }

    // Setup the screen
    g_LogFile.info("interface", "Determining Fullscreen or Windowed Mode");
    if (m_Fullscreen) {
        // Get Desktop Mode
        SDL_DisplayMode desktop_mode;
        SDL_GetDesktopDisplayMode(0, &desktop_mode);
        g_LogFile.info("interface", "Desktop Display Mode = ", desktop_mode.w, " x ", desktop_mode.h);
        m_ScreenWidth  = desktop_mode.w;
        m_ScreenHeight = desktop_mode.h;

        SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &m_Window, &m_Renderer);
    }
    else {
        SDL_CreateWindowAndRenderer(WindowWidth, WindowHeight, 0, &m_Window, &m_Renderer);
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
    SDL_RenderSetLogicalSize(m_Renderer, m_ScreenWidth, m_ScreenHeight);

    if(!(m_Window && m_Renderer))    // check for error
    {
        g_LogFile.error("interface", "Could not SDL_CreateWindowAndRenderer");
        g_LogFile.error("interface", SDL_GetError());
        return false;
    }

    // set window caption
    g_LogFile.info("interface","Setting Window Caption");
    SDL_SetWindowTitle(m_Window, caption.c_str());

    // Init TTF
    g_LogFile.info("interface","Initializing TTF");
    if(TTF_Init() == -1)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL Error",
                                 "Could not initialize SDL_TTF", m_Window);
        g_LogFile.error("interface", "Could not initialize SDL_TTF");
        g_LogFile.error("interface", TTF_GetError());
        return false;
    }

    return true;
}

cSurface CGraphics::LoadImage(std::string filename, sLoadImageParams params)
{
    return m_ImageCache.LoadImage(std::move(filename), params);
}

cSurface CGraphics::CreateSurface(int width, int height, sColor color, bool transparent)
{
    return m_ImageCache.CreateSurface(width, height, color, transparent);
}

cFont CGraphics::LoadFont(const std::string& font, int size)
{
    cFont f{this};
    f.LoadFont(font, size);
    return f;
}

cSurface CGraphics::LoadImage(std::string filename, int width, int height, bool transparent) {
    sLoadImageParams params;
    params.MinWidth = width;
    params.MinHeight = height;
    params.MaxWidth = width;
    params.MaxHeight = height;
    params.Transparency = transparent;
    params.KeepRatio = true;
    return LoadImage(std::move(filename), params);
}

void CGraphics::SetTheme(cTheme& theme) {
    theme.set_screen_size(GetWidth(), GetHeight());

    // set window icon
    g_LogFile.info("interface", "Setting Window Icon");
    SDL_Surface* loadIcon = IMG_Load(theme.get_image("Icons", "window_icon.png").c_str());
    if(loadIcon) {
        SDL_SetWindowIcon(m_Window, loadIcon);
        SDL_FreeSurface(loadIcon);
    } else {
        g_LogFile.error("interface", "Error setting window icon (window_icon.png)");
        g_LogFile.error("interface", SDL_GetError());
    }

    // Load the universal background image
    sLoadImageParams lip;
    lip.Transparency = false;
    lip.KeepRatio = false;
    lip.MinWidth = GetWidth();
    lip.MinHeight = GetHeight();
    m_BackgroundImage = LoadImage(theme.get_image("Backdrops", "background.jpg"), lip);
    g_LogFile.info("interface", "Background Image Set");
}

void CGraphics::RenderTexture(SDL_Texture* texture, SDL_Rect* src, SDL_Rect* dst) {
    SDL_RenderCopy(m_Renderer, texture, src, dst);
}

SDL_Renderer* CGraphics::GetRenderer() {
    return m_Renderer;
}

