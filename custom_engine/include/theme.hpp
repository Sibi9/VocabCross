#pragma once
#include "raylib.h"
#include "arabic_shaper.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace Theme {

inline Font fontCartoon{};
inline Font fontArabic{};
inline int coins = 450;
inline int gems = 12;
inline bool soundOn = true;
inline float timeNow = 0.0f;
inline float screenShake = 0.0f;

// Core palette (tuned to the six reference screens).
inline Color ink = { 22, 28, 48, 255 };
inline Color navy = { 13, 21, 40, 255 };
inline Color navy2 = { 17, 28, 53, 255 };
inline Color skyLight = { 188, 227, 250, 255 };
inline Color mint = { 153, 213, 201, 255 };
inline Color gold = { 251, 191, 36, 255 };
inline Color goldDark = { 202, 138, 4, 255 };
inline Color goldLight = { 254, 240, 138, 255 };
inline Color orange = { 249, 115, 22, 255 };
inline Color orangeDark = { 194, 65, 12, 255 };
inline Color green = { 34, 197, 94, 255 };
inline Color greenDark = { 21, 128, 61, 255 };
inline Color blue = { 14, 165, 233, 255 };
inline Color blueDark = { 2, 132, 199, 255 };
inline Color purple = { 168, 85, 247, 255 };
inline Color purpleDark = { 126, 34, 206, 255 };
inline Color rose = { 244, 63, 94, 255 };
inline Color roseDark = { 190, 24, 93, 255 };
inline Color cream = { 255, 253, 245, 255 };
inline Color wood = { 140, 82, 43, 255 };
inline Color woodDark = { 92, 50, 22, 255 };
inline Color woodLight = { 205, 128, 72, 255 };

inline Color WithAlpha(Color c, float a) {
    c.a = static_cast<unsigned char>(std::clamp(a, 0.0f, 1.0f) * 255.0f);
    return c;
}

inline Color Mix(Color a, Color b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return Color{
        static_cast<unsigned char>(a.r + (b.r - a.r) * t),
        static_cast<unsigned char>(a.g + (b.g - a.g) * t),
        static_cast<unsigned char>(a.b + (b.b - a.b) * t),
        static_cast<unsigned char>(a.a + (b.a - a.a) * t)
    };
}

inline void loadFonts() {
    // Fredoka only needs Latin; keeping the atlas small guarantees it is crisp.
    std::vector<int> latin;
    for (int c = 32; c <= 126; ++c) latin.push_back(c);
    fontCartoon = LoadFontEx("assets/fonts/Fredoka.ttf", 56, latin.data(), static_cast<int>(latin.size()));
    SetTextureFilter(fontCartoon.texture, TEXTURE_FILTER_BILINEAR);

    // Raylib does not run OpenType GSUB.  We therefore draw Unicode Arabic
    // presentation forms, which must exist as real glyphs in the font.
    // Cairo.ttf omits 33 of the required isolated/initial forms, so Tahoma
    // (which covers all Arabic + Arabic Presentation Forms-B) is used for the
    // shaped Arabic layer.  All UI strings go through ArabicShaper::reshape().
    std::vector<int> arabic;
    for (int c = 32; c <= 126; ++c) arabic.push_back(c);
    arabic.push_back(0x060C);
    arabic.push_back(0x061F);
    for (int c = 0x0620; c <= 0x064A; ++c) arabic.push_back(c);
    for (int c = 0x064B; c <= 0x065F; ++c) arabic.push_back(c);
    for (int c = 0x0660; c <= 0x0669; ++c) arabic.push_back(c);
    for (int c = 0xFE70; c <= 0xFEFF; ++c) arabic.push_back(c);
    fontArabic = LoadFontEx("assets/fonts/tahoma.ttf", 44, arabic.data(), static_cast<int>(arabic.size()));
    SetTextureFilter(fontArabic.texture, TEXTURE_FILTER_BILINEAR);
}

inline void unloadFonts() {
    if (fontCartoon.texture.id > 0) UnloadFont(fontCartoon);
    if (fontArabic.texture.id > 0) UnloadFont(fontArabic);
}

inline Vector2 measure(const std::string& text, float size, Font font = Font{}) {
    if (font.texture.id == 0) font = fontCartoon;
    return MeasureTextEx(font, text.c_str(), size, 0.5f);
}

inline void text(const std::string& text, Vector2 pos, float size, Color color,
                 bool centerX = false, bool centerY = false, Font font = Font{}) {
    if (font.texture.id == 0) font = fontCartoon;
    Vector2 dim = MeasureTextEx(font, text.c_str(), size, 0.5f);
    if (centerX) pos.x -= dim.x * 0.5f;
    if (centerY) pos.y -= dim.y * 0.5f;
    DrawTextEx(font, text.c_str(), { pos.x + 1.0f, pos.y + 2.0f }, size, 0.5f, WithAlpha(BLACK, 0.28f));
    DrawTextEx(font, text.c_str(), pos, size, 0.5f, color);
}

inline void textRight(const std::string& text, Vector2 pos, float size, Color color, Font font = Font{}) {
    if (font.texture.id == 0) font = fontCartoon;
    Vector2 dim = MeasureTextEx(font, text.c_str(), size, 0.5f);
    Theme::text(text, { pos.x - dim.x, pos.y }, size, color, false, false, font);
}

// Cartoon text with a thick outline and a hard 3D drop lip.
inline void text3D(Font font, const std::string& text, Vector2 pos, float size,
                   Color fill, Color outline, float outlineWidth, Color shadow,
                   bool centerX = false, bool centerY = false, float shadowOffset = -1.0f) {
    if (text.empty()) return;
    if (font.texture.id == 0) font = fontCartoon;
    Vector2 dim = MeasureTextEx(font, text.c_str(), size, 0.5f);
    if (centerX) pos.x -= dim.x * 0.5f;
    if (centerY) pos.y -= dim.y * 0.5f;
    if (shadowOffset < 0.0f) shadowOffset = std::max(2.0f, outlineWidth * 1.25f + 1.0f);

    if (shadow.a > 0) {
        DrawTextEx(font, text.c_str(), { pos.x, pos.y + shadowOffset }, size, 0.5f, shadow);
    }
    if (outlineWidth > 0.0f && outline.a > 0) {
        float o = outlineWidth;
        float d = o * 0.7071f;
        for (int i = 0; i < 8; ++i) {
            float dx = (i == 0 ? -o : i == 1 ? o : i == 2 ? 0.0f : i == 3 ? 0.0f :
                        i == 4 ? -d : i == 5 ? d : i == 6 ? -d : d);
            float dy = (i == 0 ? 0.0f : i == 1 ? 0.0f : i == 2 ? -o : i == 3 ? o :
                        i == 4 ? -d : i == 5 ? -d : i == 6 ? d : d);
            DrawTextEx(font, text.c_str(), { pos.x + dx, pos.y + dy }, size, 0.5f, outline);
        }
    }
    DrawTextEx(font, text.c_str(), pos, size, 0.5f, fill);
}

inline void text3D(const std::string& text, Vector2 pos, float size, Color fill,
                   Color outline, float outlineWidth, Color shadow,
                   bool centerX = false, bool centerY = false) {
    text3D(fontCartoon, text, pos, size, fill, outline, outlineWidth, shadow, centerX, centerY);
}

inline std::string shapeArabic(const std::string& input) {
    return ArabicShaper::reshape(input);
}

inline void textArabic(const std::string& input, Vector2 pos, float size, Color color,
                       bool centerX = false, bool centerY = false, Color outline = BLANK,
                       float outlineWidth = 0.0f) {
    std::string shaped = ArabicShaper::reshape(input);
    if (shaped.empty()) return;
    Vector2 dim = MeasureTextEx(fontArabic, shaped.c_str(), size, 0.0f);
    if (centerX) pos.x -= dim.x * 0.5f;
    if (centerY) pos.y -= dim.y * 0.5f;
    if (outline.a > 0 && outlineWidth > 0) {
        for (int i = 0; i < 8; ++i) {
            float a = i * PI / 4.0f;
            DrawTextEx(fontArabic, shaped.c_str(),
                       { pos.x + cosf(a) * outlineWidth, pos.y + sinf(a) * outlineWidth },
                       size, 0.0f, outline);
        }
    }
    DrawTextEx(fontArabic, shaped.c_str(), { pos.x, pos.y + 1.5f }, size, 0.0f, WithAlpha(BLACK, 0.25f));
    DrawTextEx(fontArabic, shaped.c_str(), pos, size, 0.0f, color);
}

// ---------------------------------------------------------------------------
// Confetti / celebration particles
// ---------------------------------------------------------------------------
struct Particle {
    Vector2 pos;
    Vector2 vel;
    Color color;
    float size;
    float alpha;
    float life;
    float maxLife;
    float rot;
    float rotSpeed;
    int shape; // 0 circle, 1 star, 2 rectangle
};

inline std::vector<Particle> particles;

inline void spawnCelebration(Vector2 center, int count = 56) {
    static const Color palette[] = {
        { 251, 191, 36, 255 }, { 244, 63, 94, 255 }, { 34, 197, 94, 255 },
        { 56, 189, 248, 255 }, { 168, 85, 247, 255 }, { 255, 255, 255, 255 }
    };
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = { center.x + (float)(rand() % 80 - 40), center.y + (float)(rand() % 40 - 20) };
        float angle = (float)(rand() % 360) * DEG2RAD;
        float speed = 60.0f + (float)(rand() % 220);
        p.vel = { cosf(angle) * speed, sinf(angle) * speed - 90.0f };
        p.color = palette[i % 6];
        p.size = 3.0f + (float)(rand() % 6);
        p.alpha = 1.0f;
        p.maxLife = 1.3f + (float)(rand() % 90) / 100.0f;
        p.life = p.maxLife;
        p.rot = (float)(rand() % 360);
        p.rotSpeed = (float)(rand() % 400 - 200);
        p.shape = i % 3;
        particles.push_back(p);
    }
}

inline void updateParticles(float dt) {
    for (size_t i = 0; i < particles.size();) {
        auto& p = particles[i];
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.vel.y += 520.0f * dt;
        p.vel.x *= 0.995f;
        p.rot += p.rotSpeed * dt;
        p.life -= dt;
        p.alpha = std::max(0.0f, p.life / p.maxLife);
        if (p.life <= 0) particles.erase(particles.begin() + i);
        else ++i;
    }
}

inline void drawStarShape(Vector2 c, float outer, float inner, Color color) {
    Vector2 pts[10];
    for (int i = 0; i < 10; ++i) {
        float r = (i % 2 == 0) ? outer : inner;
        float a = -PI * 0.5f + (float)i * PI / 5.0f;
        pts[i] = { c.x + cosf(a) * r, c.y + sinf(a) * r };
    }
    for (int i = 0; i < 10; ++i) DrawTriangle(c, pts[(i + 1) % 10], pts[i], color);
}

inline void drawParticles() {
    for (const auto& p : particles) {
        Color c = p.color;
        c.a = static_cast<unsigned char>(std::clamp(p.alpha, 0.0f, 1.0f) * 255.0f);
        if (p.shape == 1) {
            drawStarShape(p.pos, p.size * 1.2f, p.size * 0.5f, c);
        } else if (p.shape == 2) {
            Rectangle r{ p.pos.x, p.pos.y, p.size * 1.4f, p.size * 0.7f };
            Vector2 origin{ r.width * 0.5f, r.height * 0.5f };
            DrawRectanglePro(r, origin, p.rot, c);
        } else {
            DrawCircleV(p.pos, p.size * 0.6f, c);
        }
    }
}

} // namespace Theme
