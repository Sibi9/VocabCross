#pragma once
#include "raylib.h"
#include "theme.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace Art {

// ---------------------------------------------------------------------------
// Texture helpers
// ---------------------------------------------------------------------------
inline Texture2D LoadRoundedTexture(const std::string& path, float radiusFactor = 0.12f) {
    Texture2D tex{};
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr) return tex;
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    float rad = std::min(img.width, img.height) * radiusFactor;
    Color* pixels = (Color*)img.data;
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            float cx = std::clamp((float)x, rad, (float)img.width - rad);
            float cy = std::clamp((float)y, rad, (float)img.height - rad);
            float dx = (float)x - cx;
            float dy = (float)y - cy;
            if (dx * dx + dy * dy > rad * rad + 1.0f) {
                pixels[y * img.width + x].a = 0;
            }
        }
    }
    tex = LoadTextureFromImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    UnloadImage(img);
    return tex;
}

inline void DrawTextureCover(Texture2D tex, Rectangle dst, Color tint = WHITE) {
    if (tex.id == 0) return;
    float srcAspect = (float)tex.width / (float)tex.height;
    float dstAspect = dst.width / dst.height;
    Rectangle src{ 0, 0, (float)tex.width, (float)tex.height };
    if (srcAspect > dstAspect) {
        float nw = (float)tex.height * dstAspect;
        src.x = ((float)tex.width - nw) * 0.5f;
        src.width = nw;
    } else {
        float nh = (float)tex.width / dstAspect;
        src.y = ((float)tex.height - nh) * 0.5f;
        src.height = nh;
    }
    DrawTexturePro(tex, src, dst, { 0, 0 }, 0.0f, tint);
}

inline void SoftGlow(Vector2 c, float r, Color col, int layers = 18) {
    for (int i = layers; i >= 1; --i) {
        float t = (float)i / layers;
        DrawCircleV(c, r * t, Theme::WithAlpha(col, 0.035f * (1.0f - t) + 0.012f));
    }
}

inline void Cloud(Vector2 c, float w, Color col, float alpha = 1.0f) {
    Color body = Theme::WithAlpha(col, alpha);
    Color shade = Theme::WithAlpha(Theme::Mix(col, Color{ 120, 160, 200, 255 }, 0.25f), alpha * 0.9f);
    DrawEllipse((int)c.x, (int)(c.y + w * 0.10f), w * 0.52f, w * 0.18f, Theme::WithAlpha(shade, alpha * 0.5f));
    DrawCircleV({ c.x - w * 0.28f, c.y }, w * 0.20f, body);
    DrawCircleV({ c.x, c.y - w * 0.08f }, w * 0.28f, body);
    DrawCircleV({ c.x + w * 0.26f, c.y + w * 0.01f }, w * 0.22f, body);
    DrawCircleV({ c.x + w * 0.44f, c.y + w * 0.06f }, w * 0.14f, body);
    DrawRectangleRounded({ c.x - w * 0.44f, c.y + w * 0.02f, w * 0.86f, w * 0.18f }, 0.9f, 10, body);
    DrawEllipse((int)(c.x - w * 0.10f), (int)(c.y - w * 0.16f), w * 0.22f, w * 0.07f,
                Theme::WithAlpha(WHITE, alpha * 0.35f));
}

// ---------------------------------------------------------------------------
// Academic pattern (Image Guess backdrop)
// ---------------------------------------------------------------------------
inline void CapIcon(Vector2 c, float s, Color col) {
    Vector2 diamond[5] = {
        { c.x, c.y - s * 0.55f }, { c.x + s * 0.90f, c.y - s * 0.10f },
        { c.x, c.y + s * 0.30f }, { c.x - s * 0.90f, c.y - s * 0.10f },
        { c.x, c.y - s * 0.20f }
    };
    DrawTriangleFan(diamond, 5, col);
    Color body = col;
    body.a = static_cast<unsigned char>(col.a * 0.85f);
    DrawRectangleRounded({ c.x - s * 0.44f, c.y + s * 0.12f, s * 0.88f, s * 0.32f }, 0.25f, 6, body);
    DrawLineEx({ c.x + s * 0.62f, c.y - s * 0.10f }, { c.x + s * 0.72f, c.y + s * 0.55f }, s * 0.06f, col);
    DrawCircleV({ c.x + s * 0.72f, c.y + s * 0.60f }, s * 0.09f, col);
}

inline void BookIcon(Vector2 c, float s, Color col) {
    Vector2 left[3] = { { c.x - s * 0.80f, c.y - s * 0.38f }, { c.x, c.y - s * 0.18f },
                        { c.x, c.y + s * 0.52f } };
    Vector2 right[3] = { { c.x + s * 0.80f, c.y - s * 0.38f }, { c.x, c.y - s * 0.18f },
                         { c.x, c.y + s * 0.52f } };
    Color leftCol = col;
    leftCol.a = static_cast<unsigned char>(col.a * 0.7f);
    DrawTriangleFan(left, 3, leftCol);
    DrawTriangleFan(right, 3, col);
    DrawLineEx({ c.x - s * 0.74f, c.y - s * 0.30f }, { c.x, c.y - s * 0.12f }, s * 0.05f, Theme::WithAlpha(WHITE, col.a / 255.0f * 0.35f));
    DrawLineEx({ c.x + s * 0.74f, c.y - s * 0.30f }, { c.x, c.y - s * 0.12f }, s * 0.05f, Theme::WithAlpha(WHITE, col.a / 255.0f * 0.35f));
}

inline void PencilIcon(Vector2 c, float s, Color col, float rot = -0.5f) {
    Rectangle body{ c.x - s * 0.65f, c.y - s * 0.16f, s * 1.10f, s * 0.32f };
    DrawRectanglePro(body, { body.width * 0.5f, body.height * 0.5f }, rot * RAD2DEG, col);
    Vector2 tip{ c.x + std::cos(rot) * s * 0.82f, c.y + std::sin(rot) * s * 0.82f };
    Vector2 a{ tip.x - std::cos(rot + 1.57f) * s * 0.16f, tip.y - std::sin(rot + 1.57f) * s * 0.16f };
    Vector2 b{ tip.x - std::cos(rot - 1.57f) * s * 0.16f, tip.y - std::sin(rot - 1.57f) * s * 0.16f };
    Vector2 p{ tip.x + std::cos(rot) * s * 0.28f, tip.y + std::sin(rot) * s * 0.28f };
    Color tipCol = col;
    tipCol.a = static_cast<unsigned char>(col.a * 0.65f);
    DrawTriangle(a, b, p, tipCol);
}

inline void BulbPatternIcon(Vector2 c, float s, Color col) {
    DrawCircleV(c, s * 0.42f, col);
    DrawRectangleRounded({ c.x - s * 0.18f, c.y + s * 0.28f, s * 0.36f, s * 0.26f }, 0.3f, 6, col);
    for (int i = 0; i < 4; ++i) {
        float a = -PI * 0.5f + (i - 1.5f) * 0.55f;
        DrawLineEx({ c.x + std::cos(a) * s * 0.55f, c.y + std::sin(a) * s * 0.55f },
                   { c.x + std::cos(a) * s * 0.80f, c.y + std::sin(a) * s * 0.80f },
                   std::max(1.0f, s * 0.07f), col);
    }
}

inline void PatternIcon(int type, Vector2 c, float s, Color col, float rot) {
    switch (type % 6) {
        case 0: CapIcon(c, s, col); break;
        case 1: BookIcon(c, s, col); break;
        case 2: BulbPatternIcon(c, s, col); break;
        case 3: UI::DrawStarShape(c, s * 0.55f, s * 0.24f, col, rot); break;
        case 4: PencilIcon(c, s, col, rot); break;
        default: {
            // flask
            DrawLineEx({ c.x - s * 0.12f, c.y - s * 0.55f }, { c.x - s * 0.12f, c.y - s * 0.15f }, s * 0.12f, col);
            DrawLineEx({ c.x + s * 0.12f, c.y - s * 0.55f }, { c.x + s * 0.12f, c.y - s * 0.15f }, s * 0.12f, col);
            Vector2 tri[4] = { { c.x, c.y - s * 0.05f }, { c.x + s * 0.44f, c.y + s * 0.52f },
                               { c.x - s * 0.44f, c.y + s * 0.52f }, { c.x, c.y + s * 0.20f } };
            DrawTriangleFan(tri, 4, col);
            break;
        }
    }
}

inline void AcademicPattern(int w, int h) {
    DrawRectangleGradientV(0, 0, w, h, Color{ 18, 31, 60, 255 }, Color{ 12, 21, 46, 255 });
    // soft top glow
    SoftGlow({ w * 0.5f, -40.0f }, w * 0.9f, Color{ 30, 64, 175, 255 }, 12);
    struct Spot { float x, y, s, rot; int type; };
    static const Spot spots[] = {
        { 46, 75, 24, -0.3f, 0 }, { 190, 46, 18, 0.5f, 1 }, { 328, 82, 21, -0.2f, 2 },
        { 486, 58, 25, 0.4f, 0 }, { 92, 210, 18, 0.2f, 3 }, { 238, 170, 22, -0.4f, 4 },
        { 420, 208, 18, 0.3f, 5 }, { 510, 300, 23, -0.2f, 1 }, { 55, 360, 18, 0.4f, 0 },
        { 160, 330, 20, -0.5f, 2 }, { 330, 340, 16, 0.2f, 3 }, { 465, 410, 22, 0.4f, 4 },
        { 40, 520, 19, -0.3f, 5 }, { 120, 470, 16, 0.1f, 0 }, { 380, 500, 20, -0.2f, 1 },
        { 505, 560, 18, 0.5f, 2 }, { 70, 660, 21, -0.4f, 3 }, { 240, 640, 16, 0.2f, 4 },
        { 430, 660, 19, -0.1f, 5 }, { 510, 770, 21, 0.3f, 0 }, { 150, 780, 17, -0.2f, 1 },
        { 330, 800, 22, 0.4f, 2 }, { 65, 880, 18, 0.2f, 3 }, { 470, 900, 18, -0.3f, 4 },
    };
    for (const auto& sp : spots) {
        PatternIcon(sp.type, { sp.x, sp.y }, sp.s, Theme::WithAlpha(Color{ 148, 187, 233, 255 }, 0.085f), sp.rot);
    }
    // vignette
    for (int i = 0; i < 40; ++i) {
        float t = (float)i / 40.0f;
        DrawRectangleLinesEx({ -t * 20.0f, -t * 20.0f, (float)w + t * 40.0f, (float)h + t * 40.0f },
                             1.0f, Theme::WithAlpha(BLACK, 0.01f));
    }
}

// ---------------------------------------------------------------------------
// Ornate academic frame
// ---------------------------------------------------------------------------
inline void LaurelBranch(Vector2 origin, float angle, float length, Color col) {
    Vector2 dir{ std::cos(angle), std::sin(angle) };
    Vector2 tip{ origin.x + dir.x * length, origin.y + dir.y * length };
    DrawLineEx(origin, tip, std::max(1.2f, length * 0.035f), col);
    int leaves = 6;
    float side = 1.0f;
    for (int i = 0; i < leaves; ++i) {
        float t = 0.20f + 0.80f * (float)i / (float)(leaves - 1);
        Vector2 p{ origin.x + dir.x * length * t, origin.y + dir.y * length * t };
        Vector2 n{ -dir.y, dir.x };
        Vector2 off{ n.x * side * length * 0.09f, n.y * side * length * 0.09f };
        Vector2 tipL{ p.x + off.x + dir.x * length * 0.10f, p.y + off.y + dir.y * length * 0.10f };
        Vector2 back{ p.x - dir.x * length * 0.05f, p.y - dir.y * length * 0.05f };
        DrawEllipse((int)((p.x + tipL.x) * 0.5f), (int)((p.y + tipL.y) * 0.5f),
                    std::max(2.0f, length * 0.075f), std::max(1.0f, length * 0.032f),
                    Theme::WithAlpha(col, 0.92f));
        DrawLineEx(back, tipL, std::max(1.0f, length * 0.014f), Theme::WithAlpha(Theme::Mix(col, WHITE, 0.25f), 0.7f));
        side = -side;
    }
}

inline void CapCrest(Vector2 c, float s, Color gold, Color dark) {
    // small mortarboard with a tassel
    Vector2 diamond[5] = {
        { c.x, c.y - s * 0.62f }, { c.x + s * 0.95f, c.y - s * 0.10f },
        { c.x, c.y + s * 0.28f }, { c.x - s * 0.95f, c.y - s * 0.10f },
        { c.x, c.y - s * 0.25f }
    };
    DrawTriangleFan(diamond, 5, dark);
    Vector2 diamond2[5] = {
        { c.x, c.y - s * 0.54f }, { c.x + s * 0.82f, c.y - s * 0.10f },
        { c.x, c.y + s * 0.20f }, { c.x - s * 0.82f, c.y - s * 0.10f },
        { c.x, c.y - s * 0.22f }
    };
    DrawTriangleFan(diamond2, 5, gold);
    DrawRectangleRounded({ c.x - s * 0.40f, c.y + s * 0.08f, s * 0.80f, s * 0.28f }, 0.25f, 6, dark);
    DrawLineEx({ c.x + s * 0.60f, c.y - s * 0.14f }, { c.x + s * 0.74f, c.y + s * 0.46f }, s * 0.06f, gold);
    DrawCircleV({ c.x + s * 0.74f, c.y + s * 0.52f }, s * 0.09f, gold);
}

inline void AcademicFrame(Rectangle rec, Texture2D clueTex) {
    UI::DropShadow(rec, 28.0f, 9.0f, 0.42f, 3.0f);
    // gold outer body
    UI::GradientRound({ rec.x, rec.y + 6.0f, rec.width, rec.height }, 26.0f,
                      Color{ 180, 83, 9, 255 }, Color{ 120, 53, 15, 255 });
    UI::GradientRound(rec, 26.0f, Color{ 253, 186, 58, 255 }, Color{ 234, 130, 20, 255 });
    UI::Rim(rec, 26.0f, Color{ 146, 64, 14, 255 }, Theme::WithAlpha(WHITE, 0.65f), Theme::WithAlpha(BLACK, 0.25f));

    // blue band
    Rectangle band{ rec.x + 11.0f, rec.y + 11.0f, rec.width - 22.0f, rec.height - 22.0f };
    UI::GradientRound(band, 20.0f, Color{ 3, 105, 161, 255 }, Color{ 2, 132, 199, 255 });
    DrawRectangleRoundedLines(band, UI::Roundness(band, 20.0f), 16, 2.0f, Theme::WithAlpha(Theme::goldLight, 0.95f));
    Rectangle bandGloss{ band.x + 5.0f, band.y + 4.0f, band.width - 10.0f, band.height * 0.20f };
    DrawRectangleRounded(bandGloss, 0.5f, 8, Theme::WithAlpha(WHITE, 0.12f));

    // image (the reference leaves a taller blue/gold border above the clue)
    Rectangle img{ rec.x + 25.0f, rec.y + 60.0f, rec.width - 50.0f, rec.height - 86.0f };
    DrawRectangleRounded(img, UI::Roundness(img, 16.0f), 12, Color{ 8, 13, 28, 255 });
    Rectangle inner{ img.x + 3.0f, img.y + 3.0f, img.width - 6.0f, img.height - 6.0f };
    if (clueTex.id > 0) {
        Art::DrawTextureCover(clueTex, inner);
    } else {
        DrawRectangleRounded(inner, UI::Roundness(inner, 14.0f), 12, Color{ 20, 35, 60, 255 });
    }
    // inside shadow at edges
    DrawRectangleRoundedLines(img, UI::Roundness(img, 16.0f), 12, 2.4f, Theme::WithAlpha(BLACK, 0.40f));
    DrawRectangleRoundedLines(img, UI::Roundness(img, 16.0f), 12, 1.8f, Color{ 217, 119, 6, 255 });

    // crest arch / pediment
    float cx = rec.x + rec.width * 0.5f;
    float top = rec.y;
    DrawCircleSector({ cx, top + 7.0f }, 36.0f, 180.0f, 360.0f, 28, Color{ 120, 53, 15, 255 });
    DrawCircleSector({ cx, top + 5.0f }, 33.0f, 180.0f, 360.0f, 28, Color{ 234, 130, 20, 255 });
    DrawCircleSector({ cx, top + 4.0f }, 30.0f, 180.0f, 360.0f, 28, Color{ 251, 191, 36, 255 });
    DrawCircleSector({ cx, top + 3.0f }, 25.0f, 180.0f, 360.0f, 28, Color{ 2, 132, 199, 255 });
    DrawRing({ cx, top + 4.0f }, 29.0f, 31.5f, 180.0f, 360.0f, 40, Theme::WithAlpha(Theme::goldLight, 0.8f));
    DrawLineEx({ cx - 34.0f, top + 4.0f }, { cx + 34.0f, top + 4.0f }, 2.0f, Theme::WithAlpha(BLACK, 0.25f));
    CapCrest({ cx, top - 1.0f }, 13.0f, Theme::goldLight, Color{ 180, 83, 9, 255 });
    LaurelBranch({ cx - 22.0f, top + 5.0f }, PI * 0.92f, 38.0f, Theme::goldLight);
    LaurelBranch({ cx + 22.0f, top + 5.0f }, PI * 0.08f, 38.0f, Theme::goldLight);

    // corner laurels / chevrons
    LaurelBranch({ rec.x + 10.0f, rec.y + rec.height - 22.0f }, -PI * 0.72f, 38.0f, Theme::goldLight);
    LaurelBranch({ rec.x + rec.width - 10.0f, rec.y + rec.height - 22.0f }, -PI * 0.28f, 38.0f, Theme::goldLight);
    for (int i = 0; i < 3; ++i) {
        float ox = rec.x + 16.0f + i * 7.0f;
        DrawLineEx({ ox, rec.y + rec.height - 18.0f }, { ox + 5.0f, rec.y + rec.height - 13.0f }, 2.2f, Theme::goldLight);
        DrawLineEx({ rec.x + rec.width - 16.0f - i * 7.0f, rec.y + rec.height - 18.0f },
                   { rec.x + rec.width - 21.0f - i * 7.0f, rec.y + rec.height - 13.0f }, 2.2f, Theme::goldLight);
    }
    // little top chevrons
    for (int i = 0; i < 4; ++i) {
        float x = rec.x + 24.0f + i * 8.0f;
        DrawLineEx({ x, rec.y + 16.0f }, { x + 6.0f, rec.y + 21.0f }, 1.8f, Theme::goldLight);
        DrawLineEx({ rec.x + rec.width - 24.0f - i * 8.0f, rec.y + 16.0f },
                   { rec.x + rec.width - 30.0f - i * 8.0f, rec.y + 21.0f }, 1.8f, Theme::goldLight);
    }
}

// ---------------------------------------------------------------------------
// Mascots
// ---------------------------------------------------------------------------
inline void FoxMascot(Vector2 base, float s) {
    // shadow
    DrawEllipse((int)base.x, (int)(base.y + 4.0f), 46.0f * s, 12.0f * s, Theme::WithAlpha(BLACK, 0.28f));
    // tail
    Vector2 tail{ base.x - 44.0f * s, base.y - 34.0f * s };
    DrawEllipse((int)tail.x, (int)tail.y, 28.0f * s, 16.0f * s, Color{ 217, 108, 34, 255 });
    DrawEllipse((int)(tail.x - 22.0f * s), (int)(tail.y - 9.0f * s), 12.0f * s, 11.0f * s, Color{ 255, 245, 235, 255 });
    // legs
    DrawRectangleRounded({ base.x - 22.0f * s, base.y - 22.0f * s, 17.0f * s, 30.0f * s }, 0.4f, 8, Color{ 92, 55, 31, 255 });
    DrawRectangleRounded({ base.x + 5.0f * s, base.y - 22.0f * s, 17.0f * s, 30.0f * s }, 0.4f, 8, Color{ 92, 55, 31, 255 });
    DrawEllipse((int)(base.x - 14.0f * s), (int)(base.y + 2.0f * s), 12.0f * s, 6.0f * s, Color{ 52, 31, 17, 255 });
    DrawEllipse((int)(base.x + 14.0f * s), (int)(base.y + 2.0f * s), 12.0f * s, 6.0f * s, Color{ 52, 31, 17, 255 });
    // trench coat
    DrawEllipse((int)base.x, (int)(base.y - 60.0f * s), 34.0f * s, 45.0f * s, Color{ 120, 72, 40, 255 });
    DrawRectangleRounded({ base.x - 33.0f * s, base.y - 75.0f * s, 66.0f * s, 58.0f * s }, 0.3f, 10, Color{ 139, 85, 48, 255 });
    DrawRectangleRounded({ base.x - 35.0f * s, base.y - 46.0f * s, 70.0f * s, 12.0f * s }, 0.5f, 8, Color{ 74, 44, 25, 255 });
    DrawCircleV({ base.x - 2.0f * s, base.y - 40.0f * s }, 4.0f * s, Color{ 251, 191, 36, 255 });
    DrawLineEx({ base.x, base.y - 78.0f * s }, { base.x, base.y - 22.0f * s }, 2.0f * s, Color{ 90, 52, 28, 255 });
    // collar / scarf
    DrawRectangleRounded({ base.x - 26.0f * s, base.y - 88.0f * s, 52.0f * s, 12.0f * s }, 0.5f, 8, Color{ 180, 83, 9, 255 });
    // head
    Vector2 head{ base.x, base.y - 108.0f * s };
    DrawCircleV(head, 32.0f * s, Color{ 230, 126, 42, 255 });
    // ears
    DrawTriangle({ head.x - 28.0f * s, head.y - 16.0f * s }, { head.x - 38.0f * s, head.y - 54.0f * s },
                 { head.x - 6.0f * s, head.y - 30.0f * s }, Color{ 230, 126, 42, 255 });
    DrawTriangle({ head.x + 28.0f * s, head.y - 16.0f * s }, { head.x + 38.0f * s, head.y - 54.0f * s },
                 { head.x + 6.0f * s, head.y - 30.0f * s }, Color{ 230, 126, 42, 255 });
    DrawTriangle({ head.x - 26.0f * s, head.y - 22.0f * s }, { head.x - 33.0f * s, head.y - 46.0f * s },
                 { head.x - 13.0f * s, head.y - 30.0f * s }, Color{ 120, 61, 31, 255 });
    DrawTriangle({ head.x + 26.0f * s, head.y - 22.0f * s }, { head.x + 33.0f * s, head.y - 46.0f * s },
                 { head.x + 13.0f * s, head.y - 30.0f * s }, Color{ 120, 61, 31, 255 });
    // muzzle
    DrawEllipse((int)head.x, (int)(head.y + 12.0f * s), 22.0f * s, 16.0f * s, Color{ 255, 244, 230, 255 });
    DrawCircleV({ head.x, head.y + 8.0f * s }, 5.0f * s, Color{ 35, 22, 12, 255 });
    DrawLineEx({ head.x, head.y + 12.0f * s }, { head.x, head.y + 18.0f * s }, 1.6f * s, Color{ 60, 35, 18, 255 });
    DrawCircleSector({ head.x, head.y + 18.0f * s }, 8.0f * s, 20.0f, 160.0f, 12, Color{ 60, 35, 18, 255 });
    DrawCircleSector({ head.x, head.y + 18.0f * s }, 6.0f * s, 30.0f, 150.0f, 10, Color{ 255, 245, 235, 255 });
    // eyes
    DrawCircleV({ head.x - 12.0f * s, head.y - 2.0f * s }, 4.2f * s, Color{ 30, 20, 10, 255 });
    DrawCircleV({ head.x + 12.0f * s, head.y - 2.0f * s }, 4.2f * s, Color{ 30, 20, 10, 255 });
    DrawCircleV({ head.x - 13.5f * s, head.y - 3.5f * s }, 1.5f * s, WHITE);
    DrawCircleV({ head.x + 10.5f * s, head.y - 3.5f * s }, 1.5f * s, WHITE);
    // deerstalker hat
    DrawCircleSector({ head.x, head.y - 22.0f * s }, 30.0f * s, 180.0f, 360.0f, 18, Color{ 112, 68, 38, 255 });
    DrawRectangleRounded({ head.x - 34.0f * s, head.y - 25.0f * s, 68.0f * s, 10.0f * s }, 0.5f, 8, Color{ 87, 50, 27, 255 });
    DrawEllipse((int)head.x, (int)(head.y - 47.0f * s), 26.0f * s, 8.0f * s, Color{ 87, 50, 27, 255 });
    DrawLineEx({ head.x - 30.0f * s, head.y - 24.0f * s }, { head.x - 30.0f * s, head.y - 2.0f * s }, 3.0f * s, Color{ 87, 50, 27, 255 });
    DrawLineEx({ head.x + 30.0f * s, head.y - 24.0f * s }, { head.x + 30.0f * s, head.y - 2.0f * s }, 3.0f * s, Color{ 87, 50, 27, 255 });
    // arm + magnifier
    Vector2 m{ base.x + 46.0f * s, base.y - 70.0f * s };
    DrawLineEx({ base.x + 26.0f * s, base.y - 66.0f * s }, { m.x - 4.0f * s, m.y + 6.0f * s }, 9.0f * s, Color{ 139, 85, 48, 255 });
    UI::MagnifierIcon(m, 17.0f * s, Color{ 180, 83, 9, 255 }, Color{ 125, 211, 252, 255 }, 0.9f);
}

inline void RobotMascot(Vector2 base, float s, float t) {
    float bob = std::sin(t * 2.0f) * 3.0f * s;
    Vector2 b{ base.x, base.y + bob };
    DrawEllipse((int)base.x, (int)(base.y + 52.0f * s), 32.0f * s, 9.0f * s, Theme::WithAlpha(BLACK, 0.30f));
    // jet glow
    for (int i = 3; i >= 1; --i) {
        DrawEllipse((int)b.x, (int)(b.y + 32.0f * s), 10.0f * s * i * 0.6f, 16.0f * s * i * 0.5f,
                    Theme::WithAlpha(Color{ 56, 189, 248, 255 }, 0.10f * (4 - i)));
    }
    // body
    UI::GradientRound({ b.x - 24.0f * s, b.y - 24.0f * s, 48.0f * s, 50.0f * s }, 16.0f * s,
                      Color{ 255, 255, 255, 255 }, Color{ 203, 213, 225, 255 });
    DrawRectangleRoundedLines({ b.x - 24.0f * s, b.y - 24.0f * s, 48.0f * s, 50.0f * s },
                              UI::Roundness({ 0, 0, 48 * s, 50 * s }, 16.0f * s), 12, 1.6f, Color{ 148, 163, 184, 255 });
    DrawCircleV({ b.x, b.y }, 8.0f * s, Color{ 14, 165, 233, 255 });
    DrawCircleV({ b.x, b.y }, 5.0f * s, Color{ 103, 232, 249, 255 });
    DrawCircleV({ b.x - 2.0f * s, b.y - 2.0f * s }, 2.0f * s, WHITE);
    // arms
    DrawLineEx({ b.x - 22.0f * s, b.y - 14.0f * s }, { b.x - 36.0f * s, b.y + 6.0f * s }, 8.0f * s, Color{ 226, 232, 240, 255 });
    DrawLineEx({ b.x + 22.0f * s, b.y - 14.0f * s }, { b.x + 34.0f * s, b.y + 8.0f * s }, 8.0f * s, Color{ 226, 232, 240, 255 });
    DrawCircleV({ b.x - 37.0f * s, b.y + 8.0f * s }, 5.0f * s, Color{ 186, 230, 253, 255 });
    DrawCircleV({ b.x + 35.0f * s, b.y + 10.0f * s }, 5.0f * s, Color{ 186, 230, 253, 255 });
    // head
    Vector2 h{ b.x, b.y - 45.0f * s };
    UI::GradientRound({ h.x - 26.0f * s, h.y - 22.0f * s, 52.0f * s, 44.0f * s }, 16.0f * s,
                      Color{ 255, 255, 255, 255 }, Color{ 203, 213, 225, 255 });
    DrawRectangleRoundedLines({ h.x - 26.0f * s, h.y - 22.0f * s, 52.0f * s, 44.0f * s },
                              UI::Roundness({ 0, 0, 52 * s, 44 * s }, 16.0f * s), 12, 1.6f, Color{ 148, 163, 184, 255 });
    Rectangle visor{ h.x - 19.0f * s, h.y - 13.0f * s, 38.0f * s, 26.0f * s };
    UI::GradientRound(visor, 9.0f * s, Color{ 30, 58, 95, 255 }, Color{ 8, 18, 38, 255 });
    DrawCircleV({ h.x - 8.0f * s, h.y }, 3.2f * s, Color{ 34, 211, 238, 255 });
    DrawCircleV({ h.x + 8.0f * s, h.y }, 3.2f * s, Color{ 34, 211, 238, 255 });
    DrawCircleV({ h.x - 8.8f * s, h.y - 0.8f * s }, 1.2f * s, WHITE);
    DrawCircleV({ h.x + 7.2f * s, h.y - 0.8f * s }, 1.2f * s, WHITE);
    // ears
    DrawCircleV({ h.x - 27.0f * s, h.y }, 5.0f * s, Color{ 14, 165, 233, 255 });
    DrawCircleV({ h.x + 27.0f * s, h.y }, 5.0f * s, Color{ 14, 165, 233, 255 });
    // antenna
    DrawLineEx({ h.x, h.y - 22.0f * s }, { h.x, h.y - 32.0f * s }, 2.0f * s, Color{ 100, 116, 139, 255 });
    DrawCircleV({ h.x, h.y - 34.0f * s }, 3.0f * s, Color{ 34, 211, 238, 255 });
    // graduation cap
    Vector2 cap{ h.x, h.y - 28.0f * s };
    Vector2 d[5] = { { cap.x, cap.y - 8.0f * s }, { cap.x + 20.0f * s, cap.y - 2.0f * s },
                     { cap.x, cap.y + 4.0f * s }, { cap.x - 20.0f * s, cap.y - 2.0f * s },
                     { cap.x, cap.y - 5.0f * s } };
    DrawTriangleFan(d, 5, Color{ 22, 28, 48, 255 });
    DrawLineEx({ cap.x + 14.0f * s, cap.y - 2.0f * s }, { cap.x + 16.0f * s, cap.y + 10.0f * s }, 1.8f * s,
               Theme::gold);
    DrawCircleV({ cap.x + 16.0f * s, cap.y + 12.0f * s }, 2.0f * s, Theme::gold);
    // shine
    DrawEllipse((int)(h.x - 12.0f * s), (int)(h.y - 17.0f * s), 7.0f * s, 3.0f * s, Theme::WithAlpha(WHITE, 0.7f));
}

inline void Astronaut(Vector2 base, float s, float t) {
    float bob = std::sin(t * 1.7f) * 2.0f * s;
    Vector2 b{ base.x, base.y + bob };
    // platform glow
    for (int i = 4; i >= 1; --i) {
        DrawEllipse((int)base.x, (int)(base.y + 48.0f * s), 40.0f * s * (0.45f + i * 0.12f), 9.0f * s * (0.45f + i * 0.12f),
                    Theme::WithAlpha(Color{ 56, 189, 248, 255 }, 0.05f * (5 - i)));
    }
    DrawEllipse((int)base.x, (int)(base.y + 45.0f * s), 34.0f * s, 7.0f * s, Theme::WithAlpha(Color{ 125, 211, 252, 255 }, 0.5f));
    // legs
    DrawLineEx({ b.x - 9.0f * s, b.y + 8.0f * s }, { b.x - 15.0f * s, b.y + 36.0f * s }, 10.0f * s, Color{ 241, 245, 249, 255 });
    DrawLineEx({ b.x + 9.0f * s, b.y + 8.0f * s }, { b.x + 17.0f * s, b.y + 36.0f * s }, 10.0f * s, Color{ 241, 245, 249, 255 });
    DrawCircleV({ b.x - 17.0f * s, b.y + 37.0f * s }, 6.0f * s, Color{ 148, 163, 184, 255 });
    DrawCircleV({ b.x + 19.0f * s, b.y + 37.0f * s }, 6.0f * s, Color{ 148, 163, 184, 255 });
    // backpack
    DrawRectangleRounded({ b.x - 18.0f * s, b.y - 22.0f * s, 36.0f * s, 34.0f * s }, 0.3f, 8, Color{ 203, 213, 225, 255 });
    // body
    UI::GradientRound({ b.x - 17.0f * s, b.y - 14.0f * s, 34.0f * s, 34.0f * s }, 12.0f * s,
                      Color{ 255, 255, 255, 255 }, Color{ 218, 230, 243, 255 });
    DrawCircleV({ b.x, b.y + 2.0f * s }, 5.0f * s, Color{ 14, 165, 233, 255 });
    // arms raised
    DrawLineEx({ b.x - 15.0f * s, b.y - 8.0f * s }, { b.x - 34.0f * s, b.y - 34.0f * s }, 8.0f * s, Color{ 241, 245, 249, 255 });
    DrawLineEx({ b.x + 15.0f * s, b.y - 8.0f * s }, { b.x + 34.0f * s, b.y - 34.0f * s }, 8.0f * s, Color{ 241, 245, 249, 255 });
    DrawCircleV({ b.x - 35.0f * s, b.y - 36.0f * s }, 5.5f * s, Color{ 226, 232, 240, 255 });
    DrawCircleV({ b.x + 35.0f * s, b.y - 36.0f * s }, 5.5f * s, Color{ 226, 232, 240, 255 });
    // helmet
    Vector2 h{ b.x, b.y - 34.0f * s };
    DrawCircleV(h, 22.0f * s, Color{ 241, 245, 249, 255 });
    DrawCircleV(h, 19.0f * s, Color{ 30, 41, 59, 255 });
    DrawCircleV({ h.x - 5.0f * s, h.y - 5.0f * s }, 6.0f * s, Theme::WithAlpha(WHITE, 0.35f));
    DrawCircleLines((int)h.x, (int)h.y, 22.0f * s, Color{ 148, 163, 184, 255 });
    DrawCircleV({ h.x + 12.0f * s, h.y - 14.0f * s }, 3.0f * s, Theme::gold);
    for (int i = 0; i < 3; ++i) {
        float a = t * 1.5f + i * 2.1f;
        Vector2 sp{ b.x + std::cos(a) * 42.0f * s, b.y - 24.0f * s + std::sin(a * 1.3f) * 24.0f * s };
        DrawCircleV(sp, (1.0f + (i % 2)) * s, Theme::WithAlpha(Theme::gold, 0.8f));
    }
}

inline void Telescope(Vector2 base, float s) {
    // tripod
    DrawLineEx({ base.x, base.y }, { base.x - 28.0f * s, base.y + 52.0f * s }, 4.0f * s, Color{ 51, 65, 85, 255 });
    DrawLineEx({ base.x, base.y }, { base.x + 27.0f * s, base.y + 52.0f * s }, 4.0f * s, Color{ 51, 65, 85, 255 });
    DrawLineEx({ base.x, base.y }, { base.x + 2.0f * s, base.y + 56.0f * s }, 4.0f * s, Color{ 51, 65, 85, 255 });
    DrawCircleV({ base.x, base.y }, 8.0f * s, Color{ 30, 41, 59, 255 });
    // tube
    float ang = -0.42f;
    Rectangle tube{ base.x, base.y - 8.0f * s, 120.0f * s, 26.0f * s };
    Vector2 origin{ 0.0f, tube.height * 0.5f };
    DrawRectanglePro({ tube.x - 4.0f * s, tube.y + 6.0f * s, tube.width, tube.height }, origin, ang * RAD2DEG,
                     Theme::WithAlpha(BLACK, 0.30f));
    DrawRectanglePro(tube, origin, ang * RAD2DEG, Color{ 71, 85, 105, 255 });
    DrawRectanglePro({ tube.x + 3.0f * s, tube.y + 2.0f * s, tube.width - 6.0f * s, tube.height * 0.45f },
                     origin, ang * RAD2DEG, Theme::WithAlpha(WHITE, 0.18f));
    // lens
    Vector2 lens{ base.x + std::cos(ang) * 122.0f * s, base.y - 8.0f * s + std::sin(ang) * 122.0f * s };
    DrawCircleV(lens, 16.0f * s, Color{ 245, 158, 11, 255 });
    DrawCircleV(lens, 12.0f * s, Color{ 30, 64, 175, 255 });
    DrawCircleV({ lens.x - 3.0f * s, lens.y - 3.0f * s }, 5.0f * s, Theme::WithAlpha(Color{ 56, 189, 248, 255 }, 0.65f));
    DrawCircleV({ lens.x - 5.0f * s, lens.y - 5.0f * s }, 2.0f * s, Theme::WithAlpha(WHITE, 0.8f));
    DrawCircleLines((int)lens.x, (int)lens.y, 16.0f * s, Theme::WithAlpha(Theme::goldLight, 0.9f));
    // focuser
    DrawRectanglePro({ base.x + 34.0f * s, base.y + 8.0f * s, 10.0f * s, 18.0f * s }, { 0, 0 }, ang * RAD2DEG,
                     Color{ 51, 65, 85, 255 });
}

// ---------------------------------------------------------------------------
// Environment props
// ---------------------------------------------------------------------------
inline void ScalesOfJustice(Vector2 base, float s, Color gold = Theme::gold) {
    Color dark = Theme::Mix(gold, BLACK, 0.28f);
    DrawEllipse((int)base.x, (int)(base.y + 2.0f * s), 30.0f * s, 8.0f * s, Theme::WithAlpha(BLACK, 0.25f));
    DrawRectangleRounded({ base.x - 22.0f * s, base.y - 4.0f * s, 44.0f * s, 10.0f * s }, 0.4f, 8, dark);
    DrawRectangleRounded({ base.x - 16.0f * s, base.y - 9.0f * s, 32.0f * s, 8.0f * s }, 0.4f, 8, gold);
    DrawRectangleRounded({ base.x - 3.0f * s, base.y - 76.0f * s, 6.0f * s, 68.0f * s }, 0.5f, 8, gold);
    DrawCircleV({ base.x, base.y - 78.0f * s }, 6.0f * s, Theme::goldLight);
    DrawLineEx({ base.x - 48.0f * s, base.y - 70.0f * s }, { base.x + 48.0f * s, base.y - 70.0f * s },
               4.0f * s, gold);
    for (int side = -1; side <= 1; side += 2) {
        Vector2 p{ base.x + side * 46.0f * s, base.y - 66.0f * s };
        DrawLineEx({ base.x + side * 46.0f * s, base.y - 68.0f * s }, { p.x, p.y + 18.0f * s }, 1.2f * s, dark);
        DrawLineEx({ base.x + side * 38.0f * s, base.y - 68.0f * s }, { p.x - side * 8.0f * s, p.y + 18.0f * s },
                   1.2f * s, dark);
        DrawEllipse((int)p.x, (int)(p.y + 22.0f * s), 20.0f * s, 5.0f * s, dark);
        DrawEllipse((int)p.x, (int)(p.y + 19.0f * s), 20.0f * s, 5.0f * s, gold);
        DrawEllipse((int)p.x, (int)(p.y + 17.0f * s), 14.0f * s, 3.0f * s, Theme::WithAlpha(Theme::goldLight, 0.65f));
    }
    DrawCircleV({ base.x, base.y - 76.0f * s }, 2.5f * s, Theme::WithAlpha(WHITE, 0.8f));
}

inline void Building(Vector2 base, float s, Color wall, Color roof, Color win) {
    DrawEllipse((int)base.x, (int)(base.y + 4.0f * s), 52.0f * s, 12.0f * s, Theme::WithAlpha(BLACK, 0.20f));
    // side face
    Vector2 side[4] = { { base.x + 30.0f * s, base.y - 84.0f * s }, { base.x + 57.0f * s, base.y - 72.0f * s },
                        { base.x + 57.0f * s, base.y + 2.0f * s }, { base.x + 30.0f * s, base.y - 2.0f * s } };
    DrawTriangleFan(side, 4, Theme::Mix(wall, BLACK, 0.18f));
    // front face
    Rectangle front{ base.x - 30.0f * s, base.y - 86.0f * s, 60.0f * s, 86.0f * s };
    UI::GradientRound(front, 3.0f * s, wall, Theme::Mix(wall, BLACK, 0.10f));
    // roof
    Vector2 roofPts[5] = { { base.x, base.y - 102.0f * s }, { base.x + 36.0f * s, base.y - 85.0f * s },
                           { base.x, base.y - 78.0f * s }, { base.x - 36.0f * s, base.y - 85.0f * s },
                           { base.x, base.y - 90.0f * s } };
    DrawTriangleFan(roofPts, 5, roof);
    // windows
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            Rectangle w{ base.x - 22.0f * s + col * 16.0f * s, base.y - 72.0f * s + row * 20.0f * s,
                         10.0f * s, 12.0f * s };
            DrawRectangleRec(w, win);
            DrawLineEx({ w.x + 5.0f * s, w.y }, { w.x + 5.0f * s, w.y + w.height }, 0.8f * s, Theme::WithAlpha(WHITE, 0.4f));
        }
    }
    DrawRectangleRounded({ base.x - 8.0f * s, base.y - 22.0f * s, 16.0f * s, 22.0f * s }, 0.2f, 4,
                         Theme::Mix(win, BLACK, 0.3f));
    DrawLineEx({ front.x, front.y }, { front.x + front.width, front.y }, 1.5f * s, Theme::WithAlpha(WHITE, 0.35f));
}

inline void GrassPatch(Rectangle r, Color grass, Color grassDark) {
    DrawEllipse((int)(r.x + r.width * 0.5f), (int)(r.y + r.height * 0.65f), r.width * 0.52f, r.height * 0.45f, grassDark);
    DrawEllipse((int)(r.x + r.width * 0.5f), (int)(r.y + r.height * 0.45f), r.width * 0.50f, r.height * 0.42f, grass);
    DrawEllipse((int)(r.x + r.width * 0.44f), (int)(r.y + r.height * 0.32f), r.width * 0.32f, r.height * 0.18f,
                Theme::WithAlpha(WHITE, 0.14f));
}

inline void Handshake(Vector2 c, float s) {
    // left sleeve
    DrawRectanglePro({ c.x - 70.0f * s, c.y - 8.0f * s, 60.0f * s, 26.0f * s }, { 0, 0 }, -12.0f, Color{ 37, 99, 235, 255 });
    // right sleeve
    DrawRectanglePro({ c.x + 10.0f * s, c.y - 8.0f * s, 60.0f * s, 26.0f * s }, { 0, 0 }, 12.0f, Color{ 15, 23, 42, 255 });
    // hands
    DrawRectangleRounded({ c.x - 24.0f * s, c.y - 10.0f * s, 32.0f * s, 26.0f * s }, 0.4f, 8, Color{ 224, 172, 123, 255 });
    DrawRectangleRounded({ c.x - 4.0f * s, c.y - 4.0f * s, 28.0f * s, 20.0f * s }, 0.4f, 8, Color{ 224, 172, 123, 255 });
    for (int i = 0; i < 4; ++i) {
        DrawLineEx({ c.x - 2.0f * s + i * 5.0f * s, c.y - 12.0f * s },
                   { c.x + 2.0f * s + i * 5.0f * s, c.y + 2.0f * s }, 3.0f * s, Color{ 198, 142, 94, 255 });
    }
    DrawLineEx({ c.x - 24.0f * s, c.y + 14.0f * s }, { c.x - 2.0f * s, c.y + 18.0f * s }, 3.0f * s, Color{ 198, 142, 94, 255 });
    DrawRectangleRounded({ c.x + 44.0f * s, c.y - 4.0f * s, 7.0f * s, 18.0f * s }, 0.3f, 6, Theme::WithAlpha(WHITE, 0.8f));
    // cuff links
    DrawCircleV({ c.x - 44.0f * s, c.y + 5.0f * s }, 2.0f * s, Theme::WithAlpha(WHITE, 0.75f));
}

inline void LawBookProps(Vector2 c, float s) {
    DrawEllipse((int)c.x, (int)(c.y + 16.0f * s), 46.0f * s, 12.0f * s, Theme::WithAlpha(BLACK, 0.20f));
    // book
    DrawRectangleRounded({ c.x - 36.0f * s, c.y - 8.0f * s, 62.0f * s, 16.0f * s }, 0.2f, 6, Color{ 30, 64, 175, 255 });
    DrawRectangleRounded({ c.x - 33.0f * s, c.y - 12.0f * s, 60.0f * s, 14.0f * s }, 0.2f, 6, Color{ 37, 99, 235, 255 });
    DrawLineEx({ c.x - 24.0f * s, c.y - 5.0f * s }, { c.x + 18.0f * s, c.y - 5.0f * s }, 2.0f * s, Theme::WithAlpha(WHITE, 0.7f));
    DrawLineEx({ c.x - 24.0f * s, c.y - 1.0f * s }, { c.x + 8.0f * s, c.y - 1.0f * s }, 1.5f * s, Theme::WithAlpha(WHITE, 0.5f));
    // gavel
    DrawLineEx({ c.x + 6.0f * s, c.y + 10.0f * s }, { c.x + 30.0f * s, c.y - 12.0f * s }, 5.0f * s, Color{ 120, 72, 40, 255 });
    DrawRectanglePro({ c.x + 22.0f * s, c.y - 22.0f * s, 20.0f * s, 14.0f * s }, { 10.0f * s, 7.0f * s }, -30.0f,
                     Color{ 146, 64, 14, 255 });
    // coins
    for (int i = 0; i < 3; ++i) {
        DrawEllipse((int)(c.x - 44.0f * s + i * 9.0f * s), (int)(c.y + 14.0f * s - i * 2.0f * s),
                    6.0f * s, 3.0f * s, Color{ 251, 191, 36, 255 });
    }
}

inline void PapersProp(Vector2 c, float s) {
    DrawRectanglePro({ c.x - 24.0f * s, c.y - 28.0f * s, 46.0f * s, 56.0f * s }, { 0, 0 }, -12.0f,
                     Color{ 226, 232, 240, 255 });
    DrawRectanglePro({ c.x - 18.0f * s, c.y - 26.0f * s, 46.0f * s, 56.0f * s }, { 0, 0 }, 6.0f,
                     Color{ 248, 250, 252, 255 });
    for (int i = 0; i < 4; ++i) {
        DrawLineEx({ c.x - 8.0f * s, c.y - 14.0f * s + i * 10.0f * s },
                   { c.x + 18.0f * s, c.y - 16.0f * s + i * 10.0f * s }, 1.6f * s, Color{ 148, 163, 184, 255 });
    }
    UI::CheckBadge({ c.x + 22.0f * s, c.y + 18.0f * s }, 12.0f * s);
}

inline void BriefcaseProp(Vector2 c, float s) {
    DrawEllipse((int)c.x, (int)(c.y + 22.0f * s), 38.0f * s, 8.0f * s, Theme::WithAlpha(BLACK, 0.20f));
    DrawRectangleRounded({ c.x - 32.0f * s, c.y - 12.0f * s, 64.0f * s, 40.0f * s }, 0.18f, 8, Color{ 120, 72, 40, 255 });
    DrawRectangleRounded({ c.x - 32.0f * s, c.y - 18.0f * s, 64.0f * s, 26.0f * s }, 0.18f, 8, Color{ 146, 64, 14, 255 });
    DrawRectangleRoundedLines({ c.x - 12.0f * s, c.y - 30.0f * s, 24.0f * s, 16.0f * s }, 0.4f, 8, 3.0f * s,
                              Color{ 92, 50, 22, 255 });
    DrawRectangleRounded({ c.x - 4.0f * s, c.y - 2.0f * s, 8.0f * s, 6.0f * s }, 0.2f, 4, Theme::gold);
    DrawLineEx({ c.x - 32.0f * s, c.y + 2.0f * s }, { c.x + 32.0f * s, c.y + 2.0f * s }, 1.5f * s,
               Theme::WithAlpha(WHITE, 0.22f));
}

// ---------------------------------------------------------------------------
// Space backdrop (Wheel stage)
// ---------------------------------------------------------------------------
inline void StarField(int w, int h, float t, bool withConstellations = true) {
    for (int i = 0; i < 150; ++i) {
        float x = (float)((i * 97 + 31) % w);
        float y = (float)((i * 173 + 47) % h);
        float tw = 0.55f + 0.45f * std::sin(t * 2.0f + i * 1.7f);
        float r = (i % 11 == 0) ? 1.8f : ((i % 3 == 0) ? 1.2f : 0.8f);
        Color c = (i % 9 == 0) ? Color{ 253, 224, 71, 255 } : Color{ 226, 232, 240, 255 };
        DrawCircleV({ x, y }, r, Theme::WithAlpha(c, 0.35f + 0.5f * tw));
        if (i % 17 == 0) {
            DrawLineEx({ x - r * 3.0f, y }, { x + r * 3.0f, y }, 1.0f, Theme::WithAlpha(c, 0.35f));
            DrawLineEx({ x, y - r * 3.0f }, { x, y + r * 3.0f }, 1.0f, Theme::WithAlpha(c, 0.35f));
        }
    }
    if (withConstellations) {
        static const Vector2 c1[] = { { 32, 168 }, { 78, 122 }, { 146, 138 }, { 198, 98 }, { 250, 142 }, { 306, 112 } };
        static const Vector2 c2[] = { { 330, 70 }, { 386, 40 }, { 440, 84 }, { 498, 52 } };
        for (int i = 0; i + 1 < 6; ++i) {
            DrawLineEx(c1[i], c1[i + 1], 1.0f, Theme::WithAlpha(Color{ 125, 211, 252, 255 }, 0.35f));
            DrawCircleV(c1[i], 1.8f, Color{ 125, 211, 252, 255 });
        }
        for (int i = 0; i + 1 < 4; ++i) {
            DrawLineEx(c2[i], c2[i + 1], 1.0f, Theme::WithAlpha(Color{ 125, 211, 252, 255 }, 0.35f));
            DrawCircleV(c2[i], 1.6f, Color{ 125, 211, 252, 255 });
        }
    }
}

inline void SpaceBackdrop(int w, int h, float t) {
    DrawRectangleGradientV(0, 0, w, h, Color{ 10, 17, 40, 255 }, Color{ 20, 13, 50, 255 });
    SoftGlow({ 110, 250 }, 260.0f, Color{ 147, 51, 234, 255 }, 20);
    SoftGlow({ 300, 470 }, 180.0f, Color{ 14, 116, 144, 255 }, 12);
    StarField(w, h, t, true);
    // a few meteors
    for (int i = 0; i < 3; ++i) {
        float x = 70.0f + i * 170.0f + std::sin(t * 0.4f + i) * 20.0f;
        float y = 300.0f + i * 140.0f;
        DrawLineEx({ x, y }, { x + 54.0f, y - 30.0f }, 2.2f, Theme::WithAlpha(Color{ 253, 224, 71, 255 }, 0.7f));
        DrawCircleV({ x, y }, 4.0f, Theme::WithAlpha(Theme::goldLight, 0.9f));
    }
    // observatory arch window
    DrawRing({ (float)w * 0.5f, 520.0f }, 430.0f, 485.0f, 180.0f, 360.0f, 90, Color{ 11, 20, 40, 255 });
    DrawRingLines({ (float)w * 0.5f, 520.0f }, 428.0f, 486.0f, 180.0f, 360.0f, 90,
                  Theme::WithAlpha(Color{ 96, 165, 250, 255 }, 0.45f));
    DrawRingLines({ (float)w * 0.5f, 520.0f }, 420.0f, 425.0f, 180.0f, 360.0f, 90,
                  Theme::WithAlpha(Color{ 191, 219, 254, 255 }, 0.25f));
    // dark floor
    DrawRectangleGradientV(0, 555, w, h - 555, Color{ 14, 20, 38, 255 }, Color{ 9, 13, 26, 255 });
    DrawLineEx({ 0, 556 }, { (float)w, 556 }, 2.0f, Theme::WithAlpha(Color{ 96, 165, 250, 255 }, 0.22f));
    DrawLineEx({ 0, 560 }, { (float)w, 560 }, 1.0f, Theme::WithAlpha(BLACK, 0.35f));
    // faint floor shading (kept as a horizontal band, not a hard oval)
    DrawRectangleGradientV(0, 700, w, 260, Theme::WithAlpha(Color{ 30, 64, 175, 255 }, 0.0f),
                           Theme::WithAlpha(Color{ 30, 64, 175, 255 }, 0.05f));
    Telescope({ 78.0f, 445.0f }, 1.00f);
    Astronaut({ 448.0f, 455.0f }, 1.05f, t);
    // little fireworks / celebratory sparks around the astronaut
    for (int i = 0; i < 10; ++i) {
        float a = (float)i * 0.63f + t * 0.2f;
        float rr = 54.0f + (i % 3) * 13.0f;
        Vector2 p{ 448.0f + std::cos(a) * rr, 415.0f + std::sin(a) * rr * 0.7f };
        Color c = (i % 3 == 0) ? Theme::gold : ((i % 3 == 1) ? Color{ 244, 114, 182, 255 } : Color{ 125, 211, 252, 255 });
        DrawCircleV(p, 2.2f, Theme::WithAlpha(c, 0.85f));
        DrawLineEx({ p.x - 4.0f, p.y }, { p.x + 4.0f, p.y }, 1.0f, Theme::WithAlpha(c, 0.5f));
        DrawLineEx({ p.x, p.y - 4.0f }, { p.x, p.y + 4.0f }, 1.0f, Theme::WithAlpha(c, 0.5f));
    }
}

// ---------------------------------------------------------------------------
// Carnival / emotions island (fourth map, fully procedural)
// ---------------------------------------------------------------------------
inline void Balloon(Vector2 c, float s, Color col) {
    DrawLineEx({ c.x, c.y + 22.0f * s }, { c.x + 6.0f * s, c.y + 58.0f * s }, 1.2f * s,
               Theme::WithAlpha(Color{ 255, 255, 255, 255 }, 0.45f));
    DrawEllipse((int)c.x, (int)c.y, 14.0f * s, 18.0f * s, col);
    DrawEllipse((int)(c.x - 4.0f * s), (int)(c.y - 6.0f * s), 4.0f * s, 7.0f * s,
                Theme::WithAlpha(WHITE, 0.35f));
    DrawTriangle({ c.x - 4.0f * s, c.y + 16.0f * s }, { c.x + 4.0f * s, c.y + 16.0f * s },
                 { c.x, c.y + 22.0f * s }, col);
}

inline void Heart(Vector2 c, float s, Color col) {
    DrawCircleV({ c.x - s * 0.42f, c.y - s * 0.25f }, s * 0.45f, col);
    DrawCircleV({ c.x + s * 0.42f, c.y - s * 0.25f }, s * 0.45f, col);
    DrawTriangle({ c.x - s * 0.84f, c.y - s * 0.10f }, { c.x + s * 0.84f, c.y - s * 0.10f },
                 { c.x, c.y + s * 0.90f }, col);
    DrawCircleV({ c.x - s * 0.38f, c.y - s * 0.42f }, s * 0.12f, Theme::WithAlpha(WHITE, 0.45f));
}

inline void FerrisWheel(Vector2 c, float s, float t) {
    Color frame = Color{ 236, 72, 153, 255 };
    Color frame2 = Color{ 168, 85, 247, 255 };
    // legs
    DrawLineEx({ c.x, c.y }, { c.x - 44.0f * s, c.y + 150.0f * s }, 5.0f * s, frame2);
    DrawLineEx({ c.x, c.y }, { c.x + 44.0f * s, c.y + 150.0f * s }, 5.0f * s, frame2);
    DrawLineEx({ c.x - 40.0f * s, c.y + 148.0f * s }, { c.x + 40.0f * s, c.y + 148.0f * s }, 5.0f * s, frame2);
    DrawCircleLines((int)c.x, (int)c.y, 72.0f * s, frame);
    DrawCircleLines((int)c.x, (int)c.y, 64.0f * s, Theme::WithAlpha(frame2, 0.8f));
    for (int i = 0; i < 10; ++i) {
        float a = t * 0.20f + i * 2.0f * PI / 10.0f;
        Vector2 p{ c.x + std::cos(a) * 68.0f * s, c.y + std::sin(a) * 68.0f * s };
        DrawLineEx(c, p, 2.0f * s, Theme::WithAlpha(frame, 0.7f));
        Rectangle cab{ p.x - 8.0f * s, p.y - 6.0f * s, 16.0f * s, 14.0f * s };
        DrawRectangleRounded(cab, 0.4f, 6, (i % 2 == 0) ? Theme::gold : Color{ 56, 189, 248, 255 });
        DrawRectangleRoundedLines(cab, 0.4f, 6, 1.0f * s, Theme::WithAlpha(BLACK, 0.25f));
    }
    DrawCircleV(c, 10.0f * s, Theme::gold);
    DrawCircleV(c, 5.0f * s, Theme::goldDark);
}

inline void CircusTent(Vector2 c, float s) {
    Color red = Color{ 244, 63, 94, 255 };
    Color cream = Color{ 255, 251, 235, 255 };
    DrawEllipse((int)c.x, (int)(c.y + 30.0f * s), 55.0f * s, 12.0f * s, Theme::WithAlpha(BLACK, 0.20f));
    for (int i = 0; i < 6; ++i) {
        float a0 = -PI + i * PI / 6.0f;
        float a1 = -PI + (i + 1) * PI / 6.0f;
        Vector2 p0{ c.x + std::cos(a0) * 52.0f * s, c.y + 26.0f * s + std::sin(a0) * 22.0f * s };
        Vector2 p1{ c.x + std::cos(a1) * 52.0f * s, c.y + 26.0f * s + std::sin(a1) * 22.0f * s };
        Vector2 top{ c.x, c.y - 62.0f * s };
        DrawTriangle(top, p0, p1, (i % 2 == 0) ? red : cream);
    }
    DrawTriangle({ c.x - 10.0f * s, c.y - 60.0f * s }, { c.x + 10.0f * s, c.y - 60.0f * s },
                 { c.x, c.y - 82.0f * s }, Theme::gold);
    DrawLineEx({ c.x, c.y - 82.0f * s }, { c.x, c.y - 92.0f * s }, 2.0f * s, Color{ 51, 65, 85, 255 });
    DrawTriangle({ c.x, c.y - 94.0f * s }, { c.x + 14.0f * s, c.y - 88.0f * s },
                 { c.x, c.y - 82.0f * s }, Theme::gold);
    // door
    DrawRectangleRounded({ c.x - 12.0f * s, c.y, 24.0f * s, 28.0f * s }, 0.5f, 8, Color{ 88, 28, 135, 255 });
}

inline void TheaterMasks(Vector2 c, float s) {
    // happy mask
    DrawCircleV({ c.x - 14.0f * s, c.y }, 18.0f * s, Color{ 255, 251, 235, 255 });
    DrawCircleV({ c.x - 20.0f * s, c.y - 5.0f * s }, 2.5f * s, Color{ 30, 41, 59, 255 });
    DrawCircleV({ c.x - 8.0f * s, c.y - 5.0f * s }, 2.5f * s, Color{ 30, 41, 59, 255 });
    DrawCircleSector({ c.x - 14.0f * s, c.y + 3.0f * s }, 7.0f * s, 20.0f, 160.0f, 12, Color{ 30, 41, 59, 255 });
    // sad mask
    DrawCircleV({ c.x + 16.0f * s, c.y - 4.0f * s }, 18.0f * s, Color{ 253, 224, 71, 255 });
    DrawCircleV({ c.x + 10.0f * s, c.y - 9.0f * s }, 2.5f * s, Color{ 30, 41, 59, 255 });
    DrawCircleV({ c.x + 22.0f * s, c.y - 9.0f * s }, 2.5f * s, Color{ 30, 41, 59, 255 });
    DrawCircleSector({ c.x + 16.0f * s, c.y + 14.0f * s }, 7.0f * s, 200.0f, 340.0f, 12, Color{ 30, 41, 59, 255 });
}

inline void CarnivalIsland(int w, int h, float t) {
    // sunset sky
    DrawRectangleGradientV(0, 0, w, h, Color{ 67, 26, 132, 255 }, Color{ 253, 164, 90, 255 });
    SoftGlow({ 270, 340 }, 220.0f, Color{ 253, 224, 71, 255 }, 22);
    DrawCircleV({ 270, 335 }, 92.0f, Theme::WithAlpha(Color{ 255, 247, 214, 255 }, 0.90f));
    DrawCircleV({ 270, 335 }, 76.0f, Color{ 254, 240, 138, 255 });
    Cloud({ 86, 150 }, 130, Color{ 255, 228, 230, 255 }, 0.78f);
    Cloud({ 462, 200 }, 116, Color{ 255, 228, 230, 255 }, 0.68f);
    Cloud({ 280, 90 }, 90, Color{ 255, 245, 245, 255 }, 0.45f);

    // floating rock cliff (jagged, layered)
    Vector2 cliff[20] = {
        { 64, 438 }, { 92, 408 }, { 126, 400 }, { 178, 404 }, { 228, 396 },
        { 300, 402 }, { 348, 392 }, { 402, 400 }, { 454, 408 }, { 492, 438 },
        { 502, 512 }, { 492, 580 }, { 506, 650 }, { 492, 736 }, { 500, 796 },
        { 470, 872 }, { 414, 926 }, { 330, 950 }, { 208, 942 }, { 118, 900 }
    };
    DrawTriangleFan(cliff, 20, Color{ 74, 52, 48, 255 });
    Vector2 cliff2[20] = {
        { 78, 450 }, { 104, 420 }, { 138, 412 }, { 188, 414 }, { 238, 408 },
        { 302, 414 }, { 350, 404 }, { 398, 412 }, { 444, 420 }, { 480, 450 },
        { 488, 520 }, { 480, 590 }, { 492, 660 }, { 480, 740 }, { 486, 796 },
        { 458, 862 }, { 404, 910 }, { 326, 934 }, { 214, 926 }, { 130, 886 }
    };
    DrawTriangleFan(cliff2, 20, Color{ 132, 94, 76, 255 });
    // warm front face
    Vector2 cliff3[8] = { { 120, 470 }, { 230, 440 }, { 410, 448 }, { 466, 492 },
                          { 452, 790 }, { 388, 886 }, { 226, 898 }, { 138, 834 } };
    DrawTriangleFan(cliff3, 8, Color{ 183, 133, 103, 255 });
    // rock strata
    for (int i = 0; i < 6; ++i) {
        float yy = 530.0f + i * 58.0f;
        Vector2 p[5] = {
            { 100.0f, yy }, { 190.0f, yy - 8.0f }, { 290.0f, yy + 4.0f },
            { 390.0f, yy - 6.0f }, { 488.0f, yy + 2.0f }
        };
        DrawSplineCatmullRom(p, 5, 2.0f, Theme::WithAlpha(Color{ 104, 66, 52, 255 }, 0.35f));
    }
    // side shading
    Vector2 shadeL[5] = { { 64, 438 }, { 120, 470 }, { 138, 834 }, { 118, 900 }, { 78, 850 } };
    DrawTriangleFan(shadeL, 5, Theme::WithAlpha(Color{ 70, 45, 38, 255 }, 0.45f));
    Vector2 shadeR[5] = { { 492, 438 }, { 466, 492 }, { 452, 790 }, { 470, 872 }, { 500, 796 } };
    DrawTriangleFan(shadeR, 5, Theme::WithAlpha(Color{ 70, 45, 38, 255 }, 0.30f));

    // grass cap with thick 3D edge
    DrawEllipse(272, 452, 232.0f, 66.0f, Color{ 16, 122, 72, 255 });
    DrawEllipse(272, 438, 230.0f, 62.0f, Color{ 34, 197, 94, 255 });
    DrawEllipse(268, 428, 224.0f, 54.0f, Color{ 74, 222, 128, 255 });
    DrawEllipse(250, 418, 160.0f, 30.0f, Theme::WithAlpha(Color{ 190, 242, 100, 255 }, 0.45f));
    // grass tufts along the front edge
    for (int i = 0; i < 12; ++i) {
        float x = 96.0f + i * 31.0f;
        float y = 466.0f + std::sin(i * 1.7f) * 5.0f;
        DrawLineEx({ x, y }, { x - 3.0f, y + 11.0f }, 2.2f, Color{ 21, 128, 61, 255 });
        DrawLineEx({ x + 3.0f, y }, { x + 5.0f, y + 9.0f }, 2.0f, Color{ 34, 197, 94, 255 });
        DrawLineEx({ x + 6.0f, y }, { x + 10.0f, y + 7.0f }, 1.8f, Color{ 74, 222, 128, 255 });
    }

    // waterfall on the left cliff
    Vector2 fall[4] = { { 108, 474 }, { 96, 560 }, { 118, 650 }, { 106, 748 } };
    DrawSplineCatmullRom(fall, 4, 26.0f, Color{ 30, 64, 175, 255 });
    DrawSplineCatmullRom(fall, 4, 18.0f, Color{ 56, 189, 248, 255 });
    DrawSplineCatmullRom(fall, 4, 9.0f, Theme::WithAlpha(Color{ 224, 242, 254, 255 }, 0.8f));
    for (int i = 0; i < 8; ++i) {
        float yy = 520.0f + i * 30.0f;
        DrawCircleV({ 101.0f + std::sin(i * 1.7f) * 8.0f, yy }, 3.0f + (i % 3), Theme::WithAlpha(WHITE, 0.55f));
    }

    // upper-left grass terrace (with a visible rock side)
    DrawEllipse(148, 574, 112.0f, 40.0f, Color{ 103, 68, 50, 255 });
    DrawEllipse(148, 560, 112.0f, 40.0f, Color{ 17, 122, 70, 255 });
    DrawEllipse(148, 548, 110.0f, 35.0f, Color{ 45, 200, 105, 255 });
    DrawEllipse(146, 540, 98.0f, 27.0f, Color{ 74, 222, 128, 255 });
    // upper-right terrace
    DrawEllipse(398, 574, 110.0f, 38.0f, Color{ 103, 68, 50, 255 });
    DrawEllipse(398, 560, 110.0f, 38.0f, Color{ 17, 122, 70, 255 });
    DrawEllipse(398, 548, 108.0f, 33.0f, Color{ 45, 200, 105, 255 });
    DrawEllipse(396, 540, 96.0f, 26.0f, Color{ 74, 222, 128, 255 });

    // boulders and vines at the cliff base
    for (int i = 0; i < 5; ++i) {
        float x = 128.0f + i * 66.0f;
        float y = 872.0f + (i % 3) * 10.0f;
        DrawEllipse((int)x, (int)y, 22.0f - i * 2.0f, 12.0f - i, Color{ 96, 64, 47, 255 });
        DrawEllipse((int)x, (int)(y - 4.0f), 18.0f - i * 2.0f, 8.0f - i, Color{ 150, 108, 82, 255 });
    }
    for (int i = 0; i < 7; ++i) {
        float x = 118.0f + i * 52.0f;
        float y = 466.0f + (i % 2) * 3.0f;
        float len = 22.0f + (i % 3) * 7.0f;
        DrawSplineSegmentBezierQuadratic({ x, y }, { x + 7.0f, y + len * 0.65f }, { x + 2.0f, y + len }, 2.2f,
                                         Color{ 21, 128, 61, 255 });
        DrawCircleV({ x + 4.0f, y + len * 0.62f }, 2.2f, Color{ 74, 222, 128, 255 });
        DrawCircleV({ x + 2.0f, y + len * 0.88f }, 2.0f, Color{ 74, 222, 128, 255 });
    }

    // winding stone path (spline, three layers)
    Vector2 pathPts[] = {
        { 270, 862 }, { 300, 800 }, { 350, 760 }, { 330, 700 }, { 260, 660 },
        { 215, 610 }, { 225, 550 }, { 265, 505 }, { 265, 465 }
    };
    DrawSplineCatmullRom(pathPts, 9, 46.0f, Color{ 104, 72, 45, 255 });
    DrawSplineCatmullRom(pathPts, 9, 38.0f, Color{ 228, 194, 148, 255 });
    DrawSplineCatmullRom(pathPts, 9, 30.0f, Color{ 247, 223, 186, 255 });
    // stone joints
    for (int i = 0; i < 13; ++i) {
        float tt = (float)i / 13.0f;
        int seg = std::min(7, (int)(tt * 8.0f));
        float lt = tt * 8.0f - seg;
        Vector2 a = pathPts[seg], b = pathPts[seg + 1];
        Vector2 p{ a.x + (b.x - a.x) * lt, a.y + (b.y - a.y) * lt };
        DrawLineEx({ p.x - 13.0f, p.y + 4.0f }, { p.x + 13.0f, p.y - 2.0f }, 1.6f,
                   Theme::WithAlpha(Color{ 170, 132, 92, 255 }, 0.65f));
    }

    // landmarks
    FerrisWheel({ 128, 330 }, 1.00f, t);
    CircusTent({ 408, 414 }, 1.02f);
    // heart arch over the path
    Heart({ 235, 448 }, 17.0f, Theme::WithAlpha(Color{ 244, 63, 94, 255 }, 0.95f));
    Heart({ 305, 448 }, 17.0f, Theme::WithAlpha(Color{ 244, 63, 94, 255 }, 0.95f));
    for (int i = 0; i < 5; ++i) {
        Heart({ 270.0f, 416.0f - i * 7.0f }, 8.0f - i * 0.8f, Theme::WithAlpha(Color{ 251, 113, 133, 255 }, 0.9f));
    }
    TheaterMasks({ 86, 650 }, 1.15f);
    Balloon({ 452, 610 }, 1.0f, Color{ 244, 63, 94, 255 });
    Balloon({ 480, 650 }, 0.85f, Color{ 56, 189, 248, 255 });
    Balloon({ 430, 660 }, 0.75f, Color{ 251, 191, 36, 255 });
    // fountain on the right terrace
    DrawEllipse(420, 588, 30.0f, 10.0f, Color{ 30, 64, 175, 255 });
    DrawEllipse(420, 584, 26.0f, 8.0f, Color{ 125, 211, 252, 255 });
    DrawLineEx({ 420, 584 }, { 420, 552 }, 4.0f, Theme::WithAlpha(Color{ 186, 230, 253, 255 }, 0.9f));
    DrawCircleV({ 420, 550 }, 5.0f, Theme::WithAlpha(WHITE, 0.85f));

    // bunting
    for (int i = 0; i < 12; ++i) {
        float x = 55.0f + i * 39.0f;
        float y = 386.0f + std::sin(i * 0.8f) * 10.0f;
        Vector2 a{ x, y }, b{ x + 24.0f, y + 10.0f };
        DrawLineEx(a, b, 1.2f, Theme::WithAlpha(WHITE, 0.45f));
        DrawTriangle(a, b, { x + 12.0f, y + 24.0f },
                     (i % 3 == 0) ? Theme::gold : ((i % 3 == 1) ? Color{ 244, 63, 94, 255 } : Color{ 56, 189, 248, 255 }));
    }
    // fireflies
    for (int i = 0; i < 26; ++i) {
        float x = 60.0f + (i * 53 % 420);
        float y = 430.0f + (i * 71 % 430);
        float a = 0.25f + 0.30f * std::sin(t * 2.0f + i);
        DrawCircleV({ x, y }, 1.8f, Theme::WithAlpha(Color{ 253, 224, 71, 255 }, a));
    }
}

} // namespace Art
