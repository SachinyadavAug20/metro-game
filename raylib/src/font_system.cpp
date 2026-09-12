#include <raylib.h>
#include <iostream>

static Font g_gameFont = {};
static Font g_boldFont = {};
static bool g_fontLoaded = false;

static bool LoadFontInto(const char* path, Font& out) {
    out = LoadFontEx(path, 48, 0, 0);
    if (out.texture.id <= 0) return false;
    SetTextureFilter(out.texture, TEXTURE_FILTER_BILINEAR);
    return true;
}

void InitGameFont() {
    if (g_fontLoaded) return;
    if (!LoadFontInto("assets/font.ttf", g_gameFont)) {
        g_fontLoaded = false;
        return;
    }
    if (!LoadFontInto("assets/font_bold.ttf", g_boldFont)) {
        g_boldFont = {};
    }
    g_fontLoaded = true;
    std::cout << "[METRO] Loaded clean Liberation Sans typeface (Regular + Bold)\n";
}

void CleanupGameFont() {
    if (g_fontLoaded) {
        UnloadFont(g_gameFont);
        if (g_boldFont.texture.id > 0) UnloadFont(g_boldFont);
        g_gameFont = {};
        g_boldFont = {};
        g_fontLoaded = false;
    }
}

Font GetGameFont() {
    if (g_fontLoaded) return g_gameFont;
    return GetFontDefault();
}

bool HasGameFont() {
    return g_fontLoaded;
}

static float RenderSize(float size) {
    // Slight lift at very small sizes keeps glyphs readable with the clean face.
    return (size < 9.5f) ? (size + 1.5f) : size;
}

static void DrawWith(Font f, const char* text, float x, float y, float size, Color color) {
    DrawTextEx(f, text, Vector2{x, y}, RenderSize(size), 0.35f, color);
}

void DrawGameText(const char* text, float x, float y, float size, Color color) {
    if (!text || text[0] == '\0') return;
    if (g_fontLoaded) DrawWith(g_gameFont, text, x, y, size, color);
    else ::DrawText(text, (int)x, (int)y, (int)size, color);
}

void DrawGameBoldText(const char* text, float x, float y, float size, Color color) {
    if (!text || text[0] == '\0') return;
    if (g_fontLoaded && g_boldFont.texture.id > 0) DrawWith(g_boldFont, text, x, y, size, color);
    else DrawGameText(text, x, y, size, color);
}

void DrawGameTextShadow(const char* text, float x, float y, float size, Color color, Color shadowColor) {
    DrawGameText(text, x + 1.0f, y + 1.0f, size, shadowColor);
    DrawGameText(text, x, y, size, color);
}

void DrawGameBoldTextShadow(const char* text, float x, float y, float size, Color color, Color shadowColor) {
    DrawGameBoldText(text, x + 1.0f, y + 1.0f, size, shadowColor);
    DrawGameBoldText(text, x, y, size, color);
}

int MeasureGameText(const char* text, float size) {
    if (!text || text[0] == '\0') return 0;
    if (g_fontLoaded) return (int)MeasureTextEx(g_gameFont, text, RenderSize(size), 0.35f).x;
    return ::MeasureText(text, (int)size);
}

int MeasureGameBoldText(const char* text, float size) {
    if (!text || text[0] == '\0') return 0;
    if (g_fontLoaded && g_boldFont.texture.id > 0) return (int)MeasureTextEx(g_boldFont, text, RenderSize(size), 0.35f).x;
    return MeasureGameText(text, size);
}

void DrawGameTextCentered(const char* text, float centerX, float y, float size, Color color) {
    int w = MeasureGameText(text, size);
    DrawGameText(text, centerX - (float)w * 0.5f, y, size, color);
}

void DrawGameBoldTextCentered(const char* text, float centerX, float y, float size, Color color) {
    int w = MeasureGameBoldText(text, size);
    DrawGameBoldText(text, centerX - (float)w * 0.5f, y, size, color);
}