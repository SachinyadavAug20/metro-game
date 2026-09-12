#pragma once
#include <raylib.h>

// Clean Liberation Sans typeface system for Metro Grid
// body text renders with the Regular weight; headings use the Bold weight.
void InitGameFont();
void CleanupGameFont();
Font GetGameFont();
bool HasGameFont();

void DrawGameText(const char* text, float x, float y, float size, Color color);
void DrawGameBoldText(const char* text, float x, float y, float size, Color color);
void DrawGameTextShadow(const char* text, float x, float y, float size, Color color, Color shadowColor = Color{0, 0, 0, 200});
void DrawGameBoldTextShadow(const char* text, float x, float y, float size, Color color, Color shadowColor = Color{0, 0, 0, 200});
void DrawGameTextCentered(const char* text, float centerX, float y, float size, Color color);
void DrawGameBoldTextCentered(const char* text, float centerX, float y, float size, Color color);
int MeasureGameText(const char* text, float size);
int MeasureGameBoldText(const char* text, float size);

#undef DrawText
#undef MeasureText
#define DrawText(txt, x, y, sz, ...) DrawGameText(txt, (float)(x), (float)(y), (float)(sz), __VA_ARGS__)
#define MeasureText(txt, sz) MeasureGameText(txt, (float)(sz))