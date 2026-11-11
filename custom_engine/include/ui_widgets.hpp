#pragma once
#include "raylib.h"
#include "theme.hpp"
#include "game_data.hpp"
#include "ui_asset_pack.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace UI {

// ---------------------------------------------------------------------------
// Low-level helpers
// ---------------------------------------------------------------------------
inline float Roundness(Rectangle r, float radius) {
    float m = std::min(r.width, r.height);
    if (m <= 0.0f) return 0.0f;
    return std::clamp(2.0f * radius / m, 0.0f, 1.0f);
}

inline void DropShadow(Rectangle r, float radius, float dy = 6.0f, float alpha = 0.30f, float grow = 2.0f) {
    Rectangle s{ r.x - grow, r.y + dy, r.width + grow * 2.0f, r.height + grow * 2.0f };
    DrawRectangleRounded(s, Roundness(s, radius + grow), 12, Theme::WithAlpha(BLACK, alpha * 0.45f));
    Rectangle s2{ r.x - grow * 0.5f, r.y + dy * 0.7f, r.width + grow, r.height + grow };
    DrawRectangleRounded(s2, Roundness(s2, radius + grow * 0.5f), 12, Theme::WithAlpha(BLACK, alpha));
}

// A rounded rectangle with a real vertical gradient (no flat programmer art).
inline void GradientRound(Rectangle r, float radius, Color top, Color bottom, int step = 2) {
    int h = (int)std::ceil(r.height);
    if (h <= 0) return;
    float rad = std::min(radius, std::min(r.width, r.height) * 0.5f);
    for (int i = 0; i < h; i += step) {
        float y = (float)i;
        float inset = 0.0f;
        if (y < rad) {
            float d = rad - y;
            inset = rad - std::sqrt(std::max(0.0f, rad * rad - d * d));
        } else if (y > (float)h - rad) {
            float d = y - ((float)h - rad);
            inset = rad - std::sqrt(std::max(0.0f, rad * rad - d * d));
        }
        Color c = Theme::Mix(top, bottom, y / std::max(1.0f, (float)h - 1.0f));
        int rows = std::min(step, h - i);
        DrawRectangle((int)std::floor(r.x + inset), (int)std::floor(r.y + y),
                      (int)std::ceil(r.width - inset * 2.0f), rows, c);
    }
}

inline void Rim(Rectangle r, float radius, Color outer, Color innerTop, Color innerBottom) {
    float rn = Roundness(r, radius);
    DrawRectangleRoundedLines(r, rn, 14, 2.4f, outer);
    Rectangle i1{ r.x + 2.0f, r.y + 2.0f, r.width - 4.0f, r.height - 4.0f };
    DrawRectangleRoundedLines(i1, Roundness(i1, radius - 1.0f), 14, 1.2f, Theme::WithAlpha(innerTop, 0.85f));
    DrawLineEx({ r.x + radius * 0.7f, r.y + r.height - 2.2f },
               { r.x + r.width - radius * 0.7f, r.y + r.height - 2.2f },
               1.4f, Theme::WithAlpha(innerBottom, 0.75f));
}

inline void GlassPanel(Rectangle r, float radius, Color tint, Color border = WHITE) {
    DropShadow(r, radius, 5.0f, 0.25f);
    GradientRound(r, radius, Theme::WithAlpha(tint, 0.92f), Theme::WithAlpha(Theme::Mix(tint, Theme::ink, 0.22f), 0.94f));
    DrawRectangleRoundedLines(r, Roundness(r, radius), 14, 2.0f, border);
    Rectangle gloss{ r.x + 5.0f, r.y + 4.0f, r.width - 10.0f, r.height * 0.38f };
    DrawRectangleRounded(gloss, Roundness(gloss, radius * 0.7f), 10, Theme::WithAlpha(WHITE, 0.16f));
    DrawLineEx({ r.x + radius * 0.8f, r.y + 3.0f }, { r.x + r.width - radius * 0.8f, r.y + 3.0f },
               1.5f, Theme::WithAlpha(WHITE, 0.45f));
}

inline void DrawStarShape(Vector2 c, float outer, float inner, Color fill, float rotation = -PI * 0.5f) {
    Vector2 pts[11];
    pts[0] = { c.x, c.y };
    for (int i = 0; i < 10; ++i) {
        float r = (i % 2 == 0) ? outer : inner;
        float a = rotation + (float)i * PI / 5.0f;
        pts[i + 1] = { c.x + std::cos(a) * r, c.y + std::sin(a) * r };
    }
    DrawTriangleFan(pts, 11, fill);
}

inline void Star5(Vector2 center, float outer, float inner, Color fill, Color outline, bool shadow = true) {
    if (shadow) {
        DrawStarShape({ center.x, center.y + 3.0f }, outer + 1.2f, inner + 0.8f,
                      Theme::WithAlpha(Color{ 120, 53, 15, 255 }, 0.55f));
    }
    if (outline.a > 0) {
        DrawStarShape(center, outer + 1.7f, inner + 1.2f, outline);
    }
    DrawStarShape(center, outer, inner, fill);
    // little specular glint on the upper left
    DrawCircleV({ center.x - outer * 0.25f, center.y - outer * 0.30f }, std::max(1.0f, outer * 0.13f),
                Theme::WithAlpha(WHITE, 0.75f));
}

inline void Sparkle(Vector2 c, float r, Color col) {
    DrawLineEx({ c.x - r, c.y }, { c.x + r, c.y }, 2.0f, col);
    DrawLineEx({ c.x, c.y - r }, { c.x, c.y + r }, 2.0f, col);
    DrawLineEx({ c.x - r * 0.55f, c.y - r * 0.55f }, { c.x + r * 0.55f, c.y + r * 0.55f }, 1.0f, col);
    DrawLineEx({ c.x + r * 0.55f, c.y - r * 0.55f }, { c.x - r * 0.55f, c.y + r * 0.55f }, 1.0f, col);
    DrawCircleV(c, std::max(1.5f, r * 0.18f), WHITE);
}

// ---------------------------------------------------------------------------
// 3D text
// ---------------------------------------------------------------------------
inline void draw3DTextOutlined(Font font, const std::string& text, Vector2 pos, float fontSize,
                               Color fillCol, Color outlineCol, float outlineWidth, Color shadowCol,
                               bool center = false) {
    Theme::text3D(font, text, pos, fontSize, fillCol, outlineCol, outlineWidth, shadowCol, center, center);
}

inline void Text3D(const std::string& text, Vector2 pos, float size, Color fill, Color outline,
                   float outlineWidth, Color shadow, bool centerX = false, bool centerY = false) {
    Theme::text3D(text, pos, size, fill, outline, outlineWidth, shadow, centerX, centerY);
}

inline void Text(const std::string& text, Vector2 pos, float size, Color color,
                 bool centerX = false, bool centerY = false) {
    Theme::text(text, pos, size, color, centerX, centerY);
}

inline void TextArabic(const std::string& text, Vector2 pos, float size, Color color,
                       bool centerX = false, bool centerY = false, Color outline = BLANK,
                       float outlineWidth = 0.0f) {
    Theme::textArabic(text, pos, size, color, centerX, centerY, outline, outlineWidth);
}

inline std::vector<std::string> WrapText(const std::string& text, float size, float maxWidth) {
    std::vector<std::string> lines;
    std::string word;
    std::string line;
    auto flushWord = [&]() {
        if (word.empty()) return;
        std::string candidate = line.empty() ? word : line + " " + word;
        if (Theme::measure(candidate, size).x > maxWidth && !line.empty()) {
            lines.push_back(line);
            line = word;
        } else {
            line = candidate;
        }
        word.clear();
    };
    for (char c : text) {
        if (c == ' ') flushWord();
        else word += c;
    }
    flushWord();
    if (!line.empty()) lines.push_back(line);
    if (lines.empty()) lines.push_back("");
    return lines;
}

// ---------------------------------------------------------------------------
// Capsules / buttons / badges
// ---------------------------------------------------------------------------
inline void Capsule(Rectangle r, Color top, Color bottom, Color border, Color gloss = WHITE) {
    float rad = r.height * 0.5f;
    DropShadow(r, rad, 4.0f, 0.22f);
    GradientRound(r, rad, top, bottom, 2);
    DrawRectangleRoundedLines(r, 1.0f, 18, 1.8f, border);
    Rectangle g{ r.x + r.height * 0.35f, r.y + 2.5f, r.width - r.height * 0.7f, r.height * 0.34f };
    DrawRectangleRounded(g, 0.9f, 8, Theme::WithAlpha(gloss, 0.22f));
}

inline bool Button3D(Rectangle r, const std::string& label, Color top, Color bottom,
                     Color lip, Color border, float textSize = 20.0f,
                     Color textCol = WHITE, bool hovered = false, bool pressed = false,
                     Color outline = Color{ 30, 20, 10, 255 }) {
    float lift = pressed ? 3.0f : (hovered ? 1.0f : 0.0f);
    Rectangle body{ r.x, r.y - lift, r.width, r.height };
    float radius = std::min(r.height * 0.42f, 22.0f);
    DropShadow(body, radius, 5.0f + lift, 0.30f);
    GradientRound({ body.x, body.y + 4.0f, body.width, body.height }, radius, lip, Theme::Mix(lip, BLACK, 0.22f));
    GradientRound(body, radius, top, bottom, 2);
    Rim(body, radius, border, Theme::WithAlpha(WHITE, 0.55f), Theme::WithAlpha(BLACK, 0.22f));
    Rectangle gloss{ body.x + 5.0f, body.y + 3.0f, body.width - 10.0f, body.height * 0.38f };
    DrawRectangleRounded(gloss, Roundness(gloss, radius * 0.7f), 8, Theme::WithAlpha(WHITE, 0.18f));
    if (!label.empty()) {
        Theme::text3D(Theme::fontCartoon, label, { body.x + body.width * 0.5f, body.y + body.height * 0.5f + 1.0f },
                      textSize, textCol, outline, std::max(1.0f, textSize * 0.08f),
                      Theme::WithAlpha(BLACK, 0.45f), true, true);
    }
    return hovered;
}

inline void IconButton3D(Rectangle r, Color top, Color bottom, Color border, bool hovered = false) {
    float rad = r.height * 0.34f;
    DropShadow(r, rad, 4.0f, 0.30f);
    GradientRound({ r.x, r.y + 4.0f, r.width, r.height }, rad, Theme::Mix(bottom, BLACK, 0.30f), bottom);
    GradientRound(r, rad, top, bottom, 2);
    Rim(r, rad, border, Theme::WithAlpha(WHITE, 0.6f), Theme::WithAlpha(BLACK, 0.25f));
    Rectangle g{ r.x + 4.0f, r.y + 3.0f, r.width - 8.0f, r.height * 0.34f };
    DrawRectangleRounded(g, Roundness(g, rad * 0.6f), 8, Theme::WithAlpha(WHITE, hovered ? 0.30f : 0.20f));
}

// ---------------------------------------------------------------------------
// Stateful Interactive Button Architecture (4-State Visual State Machine)
// ---------------------------------------------------------------------------
struct StatefulButton {
    Rectangle bounds{ 0, 0, 120, 48 };
    std::string label;
    std::string iconKey;
    float currentScale = 1.0f;
    float targetScale = 1.0f;
    float currentOffset = 0.0f;
    float targetOffset = 0.0f;
    float hoverGlow = 0.0f;
    float targetGlow = 0.0f;
    bool isHovered = false;
    bool isPressed = false;
    bool isDisabled = false;

    Color topColor{ 34, 197, 94, 255 };
    Color bottomColor{ 21, 128, 61, 255 };
    Color lipColor{ 20, 83, 45, 255 };
    Color borderColor{ 254, 240, 138, 255 };
    Color textColor{ WHITE };
    Color textOutline{ BLACK };
    float cornerRadius = 14.0f;
    float fontSize = 16.0f;

    void update(Vector2 mouse, bool mouseDown, float dt = 0.016f) {
        if (isDisabled) {
            targetScale = 1.0f;
            targetOffset = 0.0f;
            targetGlow = 0.0f;
            isHovered = false;
            isPressed = false;
            currentScale += (targetScale - currentScale) * std::min(1.0f, dt * 15.0f);
            currentOffset += (targetOffset - currentOffset) * std::min(1.0f, dt * 20.0f);
            hoverGlow += (targetGlow - hoverGlow) * std::min(1.0f, dt * 15.0f);
            return;
        }

        isHovered = CheckCollisionPointRec(mouse, bounds);
        if (isHovered) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (mouseDown) {
                isPressed = true;
                targetScale = 0.97f;
                targetOffset = 3.5f; // down press offset
                targetGlow = 0.85f;
            } else {
                isPressed = false;
                targetScale = 1.05f; // +5% smooth spring scale
                targetOffset = 0.0f;
                targetGlow = 1.0f;   // golden hover luminance
            }
        } else {
            isPressed = false;
            targetScale = 1.0f;
            targetOffset = 0.0f;
            targetGlow = 0.0f;
        }

        currentScale += (targetScale - currentScale) * std::min(1.0f, dt * 16.0f);
        currentOffset += (targetOffset - currentOffset) * std::min(1.0f, dt * 22.0f);
        hoverGlow += (targetGlow - hoverGlow) * std::min(1.0f, dt * 16.0f);
    }

    bool isClicked(Vector2 mouse, bool mouseReleased) const {
        if (isDisabled) return false;
        return mouseReleased && CheckCollisionPointRec(mouse, bounds);
    }

    void draw(Font font = Theme::fontCartoon) const {
        Vector2 center{ bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f };
        float w = bounds.width * currentScale;
        float h = bounds.height * currentScale;
        float x = center.x - w * 0.5f;
        float y = center.y - h * 0.5f + currentOffset;
        Rectangle r{ x, y, w, h };
        float rad = std::min(cornerRadius * currentScale, h * 0.5f);

        if (isDisabled) {
            Color disTop = Color{ 100, 116, 139, 230 };
            Color disBot = Color{ 71, 85, 105, 230 };
            DropShadow(r, rad, 2.0f, 0.2f);
            GradientRound(r, rad, disTop, disBot, 2);
            DrawRectangleRoundedLines(r, Roundness(r, rad), 14, 2.0f, Color{ 148, 163, 184, 180 });
            if (!label.empty()) {
                Theme::text3D(font, label, { center.x, y + h * 0.5f }, fontSize * currentScale,
                              Color{ 203, 213, 225, 255 }, BLACK, 1.0f, BLACK, true, true);
            }
            return;
        }

        float shadowDy = isPressed ? 2.0f : (4.0f + (currentScale - 1.0f) * 12.0f);
        float shadowAlpha = isPressed ? 0.20f : (0.32f + hoverGlow * 0.08f);
        DropShadow(r, rad, shadowDy, shadowAlpha);

        float lipHeight = isPressed ? 1.5f : 4.5f;
        Rectangle lipRect{ r.x, r.y + lipHeight, r.width, r.height };
        GradientRound(lipRect, rad, lipColor, Theme::Mix(lipColor, BLACK, 0.35f), 2);

        Color topGrad = topColor;
        Color botGrad = bottomColor;
        if (hoverGlow > 0.01f) {
            topGrad = Theme::Mix(topColor, Color{ 254, 240, 138, 255 }, hoverGlow * 0.22f);
            botGrad = Theme::Mix(bottomColor, Color{ 250, 204, 21, 255 }, hoverGlow * 0.18f);
        }
        GradientRound(r, rad, topGrad, botGrad, 2);

        Color curBorder = borderColor;
        if (hoverGlow > 0.01f) {
            curBorder = Theme::Mix(borderColor, WHITE, hoverGlow * 0.6f);
        }
        Rim(r, rad, curBorder, Theme::WithAlpha(WHITE, 0.65f + hoverGlow * 0.25f), Theme::WithAlpha(BLACK, 0.25f));

        Rectangle gloss{ r.x + rad * 0.4f, r.y + 2.5f, r.width - rad * 0.8f, r.height * 0.36f };
        DrawRectangleRounded(gloss, Roundness(gloss, rad * 0.6f), 8, Theme::WithAlpha(WHITE, 0.16f + hoverGlow * 0.10f));

        float contentX = center.x;
        if (!iconKey.empty()) {
            float iconSize = std::min(h * 0.62f, 32.0f * currentScale);
            if (label.empty()) {
                UIAssets::drawCentered(iconKey, { center.x, y + h * 0.5f }, (int)iconSize, (int)iconSize);
            } else {
                float iconX = r.x + rad + iconSize * 0.45f;
                UIAssets::drawCentered(iconKey, { iconX, y + h * 0.5f }, (int)iconSize, (int)iconSize);
                contentX = r.x + (r.width + iconSize * 0.8f) * 0.5f;
            }
        }

        if (!label.empty()) {
            float fSize = fontSize * currentScale;
            Theme::text3D(font, label, { contentX, y + h * 0.5f }, fSize,
                          textColor, textOutline, std::max(1.0f, fSize * 0.09f),
                          Theme::WithAlpha(BLACK, 0.5f), true, true);
        }
    }
};

// ---------------------------------------------------------------------------
// Stateful Map Node Button (No circles on top, spring scaling, stars)
// ---------------------------------------------------------------------------
struct StatefulNodeButton {
    Vector2 center{ 0, 0 };
    int levelNumber = 1;
    int stars = 0;
    bool unlocked = true;
    bool active = false;
    float currentScale = 1.0f;
    float targetScale = 1.0f;
    float currentOffset = 0.0f;
    float targetOffset = 0.0f;
    float hoverGlow = 0.0f;
    bool isHovered = false;
    bool isPressed = false;
    float radius = 30.0f;

    void update(Vector2 mouse, bool mouseDown, float dt = 0.016f) {
        if (!unlocked) {
            targetScale = 1.0f;
            targetOffset = 0.0f;
            hoverGlow = 0.0f;
            isHovered = false;
            isPressed = false;
            currentScale += (targetScale - currentScale) * std::min(1.0f, dt * 15.0f);
            return;
        }

        isHovered = CheckCollisionPointCircle(mouse, center, radius);
        if (isHovered) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (mouseDown) {
                isPressed = true;
                targetScale = 0.96f;
                targetOffset = 3.0f;
                hoverGlow = 0.8f;
            } else {
                isPressed = false;
                targetScale = 1.08f; // +8% smooth spring scale
                targetOffset = 0.0f;
                hoverGlow = 1.0f;
            }
        } else {
            isPressed = false;
            targetScale = active ? (1.0f + 0.06f * std::sin(Theme::timeNow * 5.0f)) : 1.0f;
            targetOffset = 0.0f;
            hoverGlow = 0.0f;
        }

        currentScale += (targetScale - currentScale) * std::min(1.0f, dt * 16.0f);
        currentOffset += (targetOffset - currentOffset) * std::min(1.0f, dt * 22.0f);
    }

    bool isClicked(Vector2 mouse, bool mouseReleased) const {
        if (!unlocked) return false;
        return mouseReleased && CheckCollisionPointCircle(mouse, center, radius * currentScale);
    }

    void draw() const {
        Vector2 c{ center.x, center.y + currentOffset };
        float r = radius * currentScale;

        // Dynamic ground shadow
        float sAlpha = isPressed ? 0.20f : (0.35f + hoverGlow * 0.12f);
        DrawCircle((int)c.x, (int)(c.y + 4.5f), r + 1.5f, Theme::WithAlpha(BLACK, sAlpha));

        if (!unlocked) {
            DrawCircle((int)c.x, (int)c.y, r, Color{ 100, 116, 139, 255 });
            DrawCircleLines((int)c.x, (int)c.y, r, Color{ 71, 85, 105, 255 });
            DrawCircleLines((int)c.x, (int)c.y, r - 2.0f, Color{ 148, 163, 184, 120 });
            // Lock icon
            DrawRectangle((int)(c.x - 7.0f), (int)(c.y - 2.0f), 14, 12, Color{ 203, 213, 225, 255 });
            DrawCircleLines((int)c.x, (int)(c.y - 4.0f), 6.0f, Color{ 203, 213, 225, 255 });
            return;
        }

        Color baseTop = active ? Color{ 59, 130, 246, 255 } : Color{ 245, 158, 11, 255 };
        Color baseBot = active ? Color{ 29, 78, 216, 255 } : Color{ 180, 83, 9, 255 };
        Color rimOuter = Color{ 254, 240, 138, 255 };

        if (hoverGlow > 0.01f) {
            baseTop = Theme::Mix(baseTop, Color{ 254, 240, 138, 255 }, hoverGlow * 0.25f);
            rimOuter = Theme::Mix(rimOuter, WHITE, hoverGlow * 0.6f);
        }

        // 3D bottom bevel
        DrawCircle((int)c.x, (int)(c.y + 3.0f), r, Theme::Mix(baseBot, BLACK, 0.35f));
        DrawCircleGradient((int)c.x, (int)c.y, r, baseTop, baseBot);

        // Double Rim
        DrawCircleLines((int)c.x, (int)c.y, r, rimOuter);
        DrawCircleLines((int)c.x, (int)c.y, r - 2.0f, Theme::WithAlpha(WHITE, 0.65f + hoverGlow * 0.25f));

        // Specular highlight
        DrawCircle((int)(c.x - r * 0.32f), (int)(c.y - r * 0.32f), r * 0.22f, Theme::WithAlpha(WHITE, 0.45f + hoverGlow * 0.25f));

        // Level Number
        std::string numStr = std::to_string(levelNumber);
        Theme::text3D(Theme::fontCartoon, numStr, c, 18.0f * currentScale, WHITE,
                      Color{ 20, 20, 20, 255 }, 2.0f, Theme::WithAlpha(BLACK, 0.5f), true, true);

        // Stars below node
        float starY = c.y + r + 10.0f;
        for (int s = 0; s < 3; ++s) {
            float sx = c.x + (s - 1) * 14.0f;
            bool earned = s < stars;
            Color starFill = earned ? Color{ 253, 224, 71, 255 } : Color{ 148, 163, 184, 180 };
            Color starOutline = earned ? Color{ 180, 83, 9, 255 } : Color{ 71, 85, 105, 255 };
            Star5({ sx, starY }, 5.5f, 2.5f, starFill, starOutline, false);
        }

        if (active) {
            float bounce = std::sin(Theme::timeNow * 6.0f) * 4.0f;
            Rectangle tag{ c.x - 30.0f, c.y - r - 24.0f + bounce, 60.0f, 20.0f };
            GradientRound(tag, 7.0f, Color{ 16, 185, 129, 255 }, Color{ 5, 150, 105, 255 });
            Rim(tag, 7.0f, Color{ 254, 240, 138, 255 }, WHITE, Color{ 6, 95, 70, 255 });
            Theme::text3D(Theme::fontCartoon, "PLAY", { c.x, c.y - r - 14.0f + bounce }, 11,
                          WHITE, BLACK, 1.5f, BLACK, true, true);
        }
    }
};

inline void CoinIcon(Vector2 c, float r) {
    DrawCircleV({ c.x, c.y + 2.0f }, r, Color{ 146, 64, 14, 255 });
    DrawCircleV(c, r, Color{ 234, 179, 8, 255 });
    DrawCircleV(c, r * 0.84f, Color{ 253, 224, 71, 255 });
    DrawCircleV(c, r * 0.64f, Color{ 250, 204, 21, 255 });
    // crown motif
    Color crown = Color{ 180, 83, 9, 255 };
    Vector2 a{ c.x - r * 0.42f, c.y + r * 0.26f };
    Vector2 b{ c.x + r * 0.42f, c.y + r * 0.26f };
    DrawLineEx(a, b, std::max(1.4f, r * 0.12f), crown);
    DrawLineEx({ c.x - r * 0.42f, c.y + r * 0.26f }, { c.x - r * 0.28f, c.y - r * 0.26f }, std::max(1.4f, r * 0.12f), crown);
    DrawLineEx({ c.x - r * 0.28f, c.y - r * 0.26f }, { c.x, c.y + r * 0.04f }, std::max(1.4f, r * 0.12f), crown);
    DrawLineEx({ c.x, c.y + r * 0.04f }, { c.x + r * 0.28f, c.y - r * 0.26f }, std::max(1.4f, r * 0.12f), crown);
    DrawLineEx({ c.x + r * 0.28f, c.y - r * 0.26f }, { c.x + r * 0.42f, c.y + r * 0.26f }, std::max(1.4f, r * 0.12f), crown);
    DrawCircleV({ c.x - r * 0.52f, c.y - r * 0.38f }, r * 0.12f, Theme::WithAlpha(WHITE, 0.7f));
}

inline void GemIcon(Vector2 c, float r) {
    Vector2 p[5] = {
        { c.x, c.y - r }, { c.x + r * 0.85f, c.y - r * 0.15f },
        { c.x + r * 0.55f, c.y + r * 0.9f }, { c.x - r * 0.55f, c.y + r * 0.9f },
        { c.x - r * 0.85f, c.y - r * 0.15f }
    };
    DrawTriangleFan(p, 5, Color{ 2, 132, 199, 255 });
    Vector2 p2[5] = {
        { c.x, c.y - r * 0.86f }, { c.x + r * 0.66f, c.y - r * 0.12f },
        { c.x + r * 0.42f, c.y + r * 0.68f }, { c.x - r * 0.42f, c.y + r * 0.68f },
        { c.x - r * 0.66f, c.y - r * 0.12f }
    };
    DrawTriangleFan(p2, 5, Color{ 56, 189, 248, 255 });
    DrawTriangleFan(p2, 5, Color{ 125, 211, 252, 160 });
    DrawLineEx({ c.x - r * 0.6f, c.y - r * 0.18f }, { c.x + r * 0.6f, c.y - r * 0.18f }, 1.2f, Theme::WithAlpha(WHITE, 0.8f));
    DrawCircleV({ c.x - r * 0.22f, c.y - r * 0.38f }, r * 0.10f, Theme::WithAlpha(WHITE, 0.9f));
}

inline void CoinsBadge(Vector2 pos, int value, float width = 118.0f, float height = 40.0f) {
    Rectangle r{ pos.x, pos.y, width, height };
    Capsule(r, Color{ 30, 43, 68, 245 }, Color{ 14, 22, 40, 250 }, Theme::WithAlpha(Theme::gold, 0.85f));
    float cr = height * 0.38f;
    CoinIcon({ r.x + cr + 7.0f, r.y + height * 0.48f }, cr);
    std::string s = std::to_string(value);
    Theme::text3D(Theme::fontCartoon, s, { r.x + cr * 2.0f + 17.0f, r.y + height * 0.5f }, height * 0.55f,
                  Color{ 254, 240, 138, 255 }, Color{ 90, 45, 10, 255 }, 1.2f,
                  Theme::WithAlpha(BLACK, 0.4f), false, true);
}

inline void GemsBadge(Vector2 pos, int value, float width = 102.0f, float height = 40.0f) {
    Rectangle r{ pos.x, pos.y, width, height };
    Capsule(r, Color{ 30, 43, 68, 245 }, Color{ 14, 22, 40, 250 }, Theme::WithAlpha(Color{ 56, 189, 248, 255 }, 0.9f));
    GemIcon({ r.x + height * 0.42f + 5.0f, r.y + height * 0.5f }, height * 0.37f);
    std::string s = std::to_string(value);
    Theme::text3D(Theme::fontCartoon, s, { r.x + height * 0.95f + 15.0f, r.y + height * 0.5f }, height * 0.55f,
                  Color{ 224, 242, 254, 255 }, Color{ 7, 89, 133, 255 }, 1.2f,
                  Theme::WithAlpha(BLACK, 0.4f), false, true);
}

inline void ProgressCapsule(Rectangle r, float progress, Color fillA, Color fillB, int stars = 0) {
    Capsule(r, Color{ 22, 33, 56, 240 }, Color{ 10, 17, 32, 250 }, Theme::WithAlpha(Color{ 96, 165, 250, 255 }, 0.9f));
    Rectangle track{ r.x + 4.0f, r.y + 4.0f, r.width - 8.0f, r.height - 8.0f };
    DrawRectangleRounded(track, 0.9f, 10, Color{ 12, 24, 45, 255 });
    float p = std::clamp(progress, 0.0f, 1.0f);
    if (p > 0.01f) {
        Rectangle fill{ track.x + 1.0f, track.y + 1.0f, (track.width - 2.0f) * p, track.height - 2.0f };
        GradientRound(fill, (track.height - 2.0f) * 0.5f, fillB, fillA);
        Rectangle gl{ fill.x + 3.0f, fill.y + 1.5f, std::max(0.0f, fill.width - 6.0f), fill.height * 0.38f };
        if (gl.width > 2.0f) DrawRectangleRounded(gl, 0.8f, 6, Theme::WithAlpha(WHITE, 0.35f));
    }
    (void)stars;
}

// ---------------------------------------------------------------------------
// Wooden crossword tiles + letter tray
// ---------------------------------------------------------------------------
inline void WoodGrain(Rectangle r, float radius) {
    Color dark = Color{ 96, 50, 22, 120 };
    Color light = Color{ 232, 166, 112, 90 };
    for (int i = 0; i < 3; ++i) {
        float y = r.y + r.height * (0.24f + i * 0.22f);
        float wob = (i % 2 == 0) ? 1.6f : -1.4f;
        Vector2 pts[3] = {
            { r.x + radius * 0.55f, y },
            { r.x + r.width * 0.5f, y + wob },
            { r.x + r.width - radius * 0.55f, y - wob }
        };
        DrawSplineSegmentCatmullRom(pts[0], pts[1], pts[2], pts[1], 1.1f, dark);
        DrawSplineSegmentCatmullRom(pts[0], { pts[1].x, pts[1].y + 2.0f }, { pts[2].x, pts[2].y + 2.0f }, pts[1], 0.8f, light);
    }
    // small knot
    DrawEllipse((int)(r.x + r.width * 0.70f), (int)(r.y + r.height * 0.68f),
                std::max(1.5f, r.width * 0.055f), std::max(1.0f, r.height * 0.035f), dark);
}

inline void drawWoodenTile3D(Rectangle rec, bool isActiveSlot, bool isEmptyRecessed,
                             const std::string& letter, bool isGiven = false) {
    float radius = rec.width * 0.20f;
    DropShadow(rec, radius, 5.0f, 0.36f, 1.0f);

    if (isActiveSlot) {
        for (int i = 3; i >= 1; --i) {
            Rectangle g{ rec.x - i * 2.0f, rec.y - i * 2.0f, rec.width + i * 4.0f, rec.height + i * 4.0f };
            DrawRectangleRoundedLines(g, Roundness(g, radius + 2.0f), 12, 2.0f,
                                      Theme::WithAlpha(Theme::goldLight, 0.13f * (4 - i)));
        }
    }

    if (isEmptyRecessed) {
        GradientRound(rec, radius, Color{ 72, 34, 14, 255 }, Color{ 108, 57, 26, 255 });
        Rectangle inner{ rec.x + 3.0f, rec.y + 3.0f, rec.width - 6.0f, rec.height - 6.0f };
        GradientRound(inner, radius * 0.85f, Color{ 53, 24, 9, 255 }, Color{ 88, 44, 18, 255 });
        DrawLineEx({ inner.x + 4.0f, inner.y + 2.0f }, { inner.x + inner.width - 4.0f, inner.y + 2.0f },
                   2.0f, Theme::WithAlpha(BLACK, 0.35f));
        DrawLineEx({ inner.x + 4.0f, inner.y + inner.height - 2.0f },
                   { inner.x + inner.width - 4.0f, inner.y + inner.height - 2.0f },
                   1.2f, Theme::WithAlpha(Color{ 210, 140, 90, 255 }, 0.45f));
        DrawRectangleRoundedLines(rec, Roundness(rec, radius), 12, 1.4f, Theme::WithAlpha(Color{ 60, 28, 12, 255 }, 0.9f));
        if (isActiveSlot) {
            DrawRectangleRoundedLines({ rec.x - 1.0f, rec.y - 1.0f, rec.width + 2.0f, rec.height + 2.0f },
                                      Roundness(rec, radius), 12, 2.6f, Theme::goldLight);
        }
        if (!letter.empty()) {
            Theme::text3D(Theme::fontCartoon, letter, { rec.x + rec.width * 0.5f, rec.y + rec.height * 0.5f },
                          rec.height * 0.64f, WHITE, Color{ 40, 18, 8, 255 }, 1.8f,
                          Theme::WithAlpha(BLACK, 0.5f), true, true);
        }
        return;
    }

    // thick 3D bottom lip
    GradientRound({ rec.x, rec.y + 5.0f, rec.width, rec.height }, radius, Color{ 95, 49, 20, 255 }, Color{ 62, 30, 12, 255 });
    // warm oak body
    Rectangle body{ rec.x, rec.y, rec.width, rec.height - 2.0f };
    GradientRound(body, radius, Color{ 216, 145, 88, 255 }, Color{ 176, 101, 48, 255 });
    // grain + bevel
    BeginScissorMode((int)body.x, (int)body.y, (int)body.width, (int)body.height);
    WoodGrain(body, radius);
    EndScissorMode();
    DrawRectangleRoundedLines(body, Roundness(body, radius), 12, 2.6f, Color{ 120, 63, 28, 255 });
    DrawRectangleRoundedLines({ body.x + 2.0f, body.y + 2.0f, body.width - 4.0f, body.height - 4.0f },
                              Roundness(body, radius), 12, 1.2f, Theme::WithAlpha(Color{ 250, 205, 160, 255 }, 0.75f));
    // top gloss
    Rectangle gloss{ body.x + 4.0f, body.y + 3.0f, body.width - 8.0f, body.height * 0.34f };
    DrawRectangleRounded(gloss, Roundness(gloss, radius * 0.7f), 8, Theme::WithAlpha(WHITE, 0.22f));

    if (isActiveSlot) {
        DrawRectangleRoundedLines({ rec.x - 1.0f, rec.y - 1.0f, rec.width + 2.0f, rec.height + 2.0f },
                                  Roundness(rec, radius), 12, 3.0f, Theme::goldLight);
    }

    if (!letter.empty()) {
        Color fill = isGiven ? Color{ 255, 251, 235, 255 } : WHITE;
        Theme::text3D(Theme::fontCartoon, letter, { rec.x + rec.width * 0.5f, rec.y + rec.height * 0.51f },
                      rec.height * 0.68f, fill, Color{ 74, 37, 17, 255 }, 2.6f,
                      Theme::WithAlpha(Color{ 40, 18, 8, 255 }, 0.85f), true, true);
    }
}

inline void drawGlossyWhiteTile3D(Rectangle rec, const std::string& letter, bool hasGlow = false) {
    float radius = rec.width * 0.22f;
    DropShadow(rec, radius, 5.0f, 0.36f, 1.0f);
    GradientRound({ rec.x, rec.y + 4.0f, rec.width, rec.height }, radius, Color{ 116, 137, 164, 255 }, Color{ 80, 99, 124, 255 });
    Rectangle body{ rec.x, rec.y, rec.width, rec.height - 2.0f };
    GradientRound(body, radius, Color{ 255, 255, 255, 255 }, Color{ 218, 230, 243, 255 });
    DrawRectangleRoundedLines(body, Roundness(body, radius), 12, 1.8f, Color{ 174, 195, 216, 255 });
    DrawRectangleRoundedLines({ body.x + 2.0f, body.y + 2.0f, body.width - 4.0f, body.height - 4.0f },
                              Roundness(body, radius), 12, 1.0f, Theme::WithAlpha(WHITE, 0.95f));
    Rectangle gloss{ body.x + 5.0f, body.y + 3.0f, body.width - 10.0f, body.height * 0.38f };
    DrawRectangleRounded(gloss, Roundness(gloss, radius), 8, Theme::WithAlpha(WHITE, 0.75f));
    if (hasGlow) {
        for (int i = 1; i <= 3; ++i) {
            DrawRectangleRoundedLines({ rec.x - i * 2.0f, rec.y - i * 2.0f, rec.width + i * 4.0f, rec.height + i * 4.0f },
                                      Roundness(rec, radius + 2.0f), 12, 2.0f,
                                      Theme::WithAlpha(Color{ 56, 189, 248, 255 }, 0.18f * (4 - i)));
        }
    }
    if (!letter.empty()) {
        Theme::text3D(Theme::fontCartoon, letter, { body.x + body.width * 0.5f, body.y + body.height * 0.5f },
                      body.height * 0.60f, Color{ 30, 41, 59, 255 }, Color{ 148, 163, 184, 255 }, 1.2f,
                      Theme::WithAlpha(Color{ 71, 85, 105, 255 }, 0.35f), true, true);
    }
}

inline void TrayHole(Vector2 center, float radius) {
    DrawCircleV({ center.x, center.y + 1.5f }, radius + 2.0f, Color{ 104, 56, 24, 255 });
    DrawCircleV(center, radius + 1.0f, Color{ 58, 28, 11, 255 });
    DrawCircleV(center, radius - 1.0f, Color{ 78, 39, 17, 255 });
    DrawCircleSector({ center.x, center.y - 2.0f }, radius - 2.0f, 200.0f, 340.0f, 12,
                     Theme::WithAlpha(BLACK, 0.30f));
    DrawCircleLines((int)center.x, (int)center.y, radius - 0.5f, Theme::WithAlpha(Color{ 230, 160, 110, 255 }, 0.35f));
}

inline void drawCandyOrb(Vector2 center, float radius, Color mainCol, Color shadowCol,
                         const std::string& letter, bool isPressed = false) {
    float yOff = isPressed ? 3.0f : 0.0f;
    Vector2 c{ center.x, center.y + yOff };
    DrawEllipse((int)center.x, (int)(center.y + radius * 0.75f), radius * 0.95f, radius * 0.35f,
                Theme::WithAlpha(BLACK, 0.35f));
    DrawCircleV({ c.x, c.y + 2.5f }, radius, shadowCol);
    DrawCircleV(c, radius - 0.5f, mainCol);
    // radial shading: darker rim + lit upper-left
    DrawCircleGradient((int)(c.x - radius * 0.22f), (int)(c.y - radius * 0.28f), radius * 0.95f,
                       Theme::WithAlpha(Theme::Mix(mainCol, WHITE, 0.55f), 0.85f),
                       Theme::WithAlpha(Theme::Mix(mainCol, shadowCol, 0.55f), 0.0f));
    // top glossy crescent
    DrawCircleSector({ c.x, c.y + radius * 0.05f }, radius * 0.80f, 200.0f, 340.0f, 18,
                     Theme::WithAlpha(WHITE, 0.34f));
    DrawEllipse((int)(c.x - radius * 0.28f), (int)(c.y - radius * 0.45f),
                radius * 0.28f, radius * 0.16f, Theme::WithAlpha(WHITE, 0.55f));
    DrawCircleLines((int)c.x, (int)c.y, radius - 0.5f, Theme::WithAlpha(shadowCol, 0.9f));
    if (!letter.empty()) {
        Theme::text3D(Theme::fontCartoon, letter, { c.x, c.y + 1.0f }, radius * 1.14f,
                      WHITE, Color{ 35, 22, 10, 255 }, 2.0f,
                      Theme::WithAlpha(Color{ 10, 5, 0, 255 }, 0.55f), true, true);
    }
}

inline void drawWoodenTray(Rectangle trayRec) {
    DropShadow(trayRec, 22.0f, 8.0f, 0.42f, 2.0f);
    GradientRound({ trayRec.x, trayRec.y + 7.0f, trayRec.width, trayRec.height }, 22.0f,
                  Color{ 92, 50, 22, 255 }, Color{ 52, 27, 11, 255 });
    GradientRound(trayRec, 22.0f, Color{ 158, 96, 51, 255 }, Color{ 116, 66, 33, 255 });
    // plank seams
    for (int i = 1; i < 3; ++i) {
        float y = trayRec.y + trayRec.height * (i / 3.0f);
        DrawLineEx({ trayRec.x + 16.0f, y }, { trayRec.x + trayRec.width - 16.0f, y },
                   1.2f, Theme::WithAlpha(Color{ 83, 44, 20, 255 }, 0.55f));
    }
    DrawRectangleRoundedLines(trayRec, Roundness(trayRec, 22.0f), 18, 3.0f, Color{ 74, 38, 16, 255 });
    Rectangle inner{ trayRec.x + 4.0f, trayRec.y + 4.0f, trayRec.width - 8.0f, trayRec.height - 8.0f };
    DrawRectangleRoundedLines(inner, Roundness(inner, 19.0f), 18, 1.4f, Theme::WithAlpha(Color{ 235, 171, 120, 255 }, 0.55f));
    Rectangle gloss{ trayRec.x + 12.0f, trayRec.y + 5.0f, trayRec.width - 24.0f, trayRec.height * 0.17f };
    DrawRectangleRounded(gloss, 0.8f, 8, Theme::WithAlpha(WHITE, 0.10f));
}

// ---------------------------------------------------------------------------
// Pencil progress
// ---------------------------------------------------------------------------
inline void drawPencilProgressBar(Rectangle rec, int currentLevel, int maxLevel) {
    float x = rec.x, y = rec.y, w = rec.width, h = rec.height;
    DrawRectangleRounded({ x - 2.0f, y + 5.0f, w + 4.0f, h }, 0.5f, 10, Theme::WithAlpha(BLACK, 0.35f));

    float eraserW = h * 0.82f;
    // pink eraser
    DrawCircleSector({ x + eraserW * 0.55f, y + h * 0.5f }, h * 0.48f, 90.0f, 270.0f, 18, Color{ 244, 63, 94, 255 });
    DrawRectangleRec({ x + eraserW * 0.55f, y + 2.0f, eraserW * 0.45f, h - 4.0f }, Color{ 244, 63, 94, 255 });
    DrawRectangleRec({ x + 3.0f, y + 3.0f, eraserW * 0.4f, h * 0.34f }, Theme::WithAlpha(WHITE, 0.22f));
    // ferrule
    float ferruleX = x + eraserW;
    float ferruleW = h * 0.38f;
    GradientRound({ ferruleX, y, ferruleW, h }, 3.0f, Color{ 241, 245, 249, 255 }, Color{ 148, 163, 184, 255 });
    for (int i = 0; i < 3; ++i) {
        DrawLineEx({ ferruleX + 3.0f + i * 4.0f, y + 2.0f }, { ferruleX + 3.0f + i * 4.0f, y + h - 2.0f },
                   1.2f, Color{ 100, 116, 139, 200 });
    }
    // tip cone
    float tipW = h * 0.95f;
    float coneStartX = x + w - tipW;
    float bodyX = ferruleX + ferruleW;
    float bodyW = coneStartX - bodyX;
    // barrel
    GradientRound({ bodyX, y, bodyW, h }, 3.0f, Color{ 251, 191, 36, 255 }, Color{ 217, 119, 6, 255 });
    Rectangle track{ bodyX + 2.0f, y + 4.0f, bodyW - 4.0f, h - 8.0f };
    GradientRound(track, 2.5f, Color{ 6, 78, 59, 255 }, Color{ 4, 47, 36, 255 });
    float p = std::clamp((float)currentLevel / (float)std::max(1, maxLevel), 0.04f, 1.0f);
    float fillW = (track.width - 4.0f) * p;
    if (fillW > 3.0f) {
        Rectangle fill{ track.x + 2.0f, track.y + 2.0f, fillW, track.height - 4.0f };
        GradientRound(fill, fill.height * 0.45f, Color{ 134, 239, 172, 255 }, Color{ 22, 163, 74, 255 });
        DrawRectangleRounded({ fill.x + 3.0f, fill.y + 1.0f, std::max(1.0f, fill.width - 6.0f), fill.height * 0.33f },
                             0.8f, 6, Theme::WithAlpha(WHITE, 0.45f));
    }
    // wooden cone + lead
    Vector2 pTop{ coneStartX, y + 1.0f };
    Vector2 pBot{ coneStartX, y + h - 1.0f };
    Vector2 pTip{ coneStartX + tipW, y + h * 0.5f };
    DrawTriangle(pTop, pTip, pBot, Color{ 253, 230, 138, 255 });
    DrawTriangle({ coneStartX, y + h * 0.2f }, { coneStartX + tipW * 0.68f, y + h * 0.5f },
                 { coneStartX, y + h * 0.8f }, Color{ 245, 158, 11, 255 });
    DrawTriangle({ coneStartX + tipW * 0.62f, y + h * 0.34f }, pTip,
                 { coneStartX + tipW * 0.62f, y + h * 0.66f }, Color{ 30, 41, 59, 255 });
    DrawTriangle({ coneStartX + tipW * 0.78f, y + h * 0.44f }, { coneStartX + tipW * 0.94f, y + h * 0.5f },
                 { coneStartX + tipW * 0.78f, y + h * 0.56f }, Color{ 75, 85, 99, 255 });
    DrawRectangleRoundedLines({ bodyX, y, bodyW, h }, 0.2f, 8, 1.8f, Color{ 180, 83, 9, 255 });
    DrawLineEx({ bodyX + 2.0f, y + 2.0f }, { bodyX + bodyW - 2.0f, y + 2.0f }, 1.5f, Theme::WithAlpha(WHITE, 0.55f));

    // Level pill below
    Rectangle pill{ x + w * 0.5f - 66.0f, y + h + 3.0f, 132.0f, 25.0f };
    DropShadow(pill, 12.0f, 2.0f, 0.25f, 0.5f);
    GradientRound(pill, 12.0f, Color{ 27, 39, 64, 250 }, Color{ 12, 19, 35, 250 });
    DrawRectangleRoundedLines(pill, 1.0f, 12, 1.2f, Theme::WithAlpha(Color{ 71, 85, 105, 255 }, 0.9f));
    std::string lvlStr = "LEVEL " + std::to_string(currentLevel) + " / " + std::to_string(maxLevel);
    Theme::text3D(Theme::fontCartoon, lvlStr, { pill.x + pill.width * 0.5f, pill.y + pill.height * 0.5f },
                  14.0f, Color{ 253, 224, 71, 255 }, Color{ 60, 30, 10, 255 }, 1.0f,
                  Theme::WithAlpha(BLACK, 0.5f), true, true);
}

// ---------------------------------------------------------------------------
// Answer slots / keyboard
// ---------------------------------------------------------------------------
inline Vector2 PointOnRoundedRect(Rectangle r, float radius, float d) {
    float rad = std::min(radius, std::min(r.width, r.height) * 0.5f);
    float sw = std::max(0.0f, r.width - 2.0f * rad);
    float sh = std::max(0.0f, r.height - 2.0f * rad);
    float arc = PI * 0.5f * rad;
    if (d < sw) return { r.x + rad + d, r.y };
    d -= sw;
    if (d < arc) {
        float a = -PI * 0.5f + d / std::max(1.0f, rad);
        return { r.x + r.width - rad + std::cos(a) * rad, r.y + rad + std::sin(a) * rad };
    }
    d -= arc;
    if (d < sh) return { r.x + r.width, r.y + rad + d };
    d -= sh;
    if (d < arc) {
        float a = d / std::max(1.0f, rad);
        return { r.x + r.width - rad + std::cos(a) * rad, r.y + r.height - rad + std::sin(a) * rad };
    }
    d -= arc;
    if (d < sw) return { r.x + r.width - rad - d, r.y + r.height };
    d -= sw;
    if (d < arc) {
        float a = PI * 0.5f + d / std::max(1.0f, rad);
        return { r.x + rad + std::cos(a) * rad, r.y + r.height - rad + std::sin(a) * rad };
    }
    d -= arc;
    if (d < sh) return { r.x, r.y + r.height - rad - d };
    d -= sh;
    float a = PI + d / std::max(1.0f, rad);
    return { r.x + rad + std::cos(a) * rad, r.y + rad + std::sin(a) * rad };
}

inline void DashedRoundedBorder(Rectangle r, float radius, Color col, float thick, float dash, float gap) {
    float rad = std::min(radius, std::min(r.width, r.height) * 0.5f);
    float perim = 2.0f * std::max(0.0f, r.width - 2.0f * rad) +
                  2.0f * std::max(0.0f, r.height - 2.0f * rad) + 2.0f * PI * rad;
    if (perim <= 1.0f) return;
    for (float d = 0.0f; d < perim; d += dash + gap) {
        float end = std::min(perim, d + dash);
        Vector2 prev = PointOnRoundedRect(r, rad, d);
        for (float t = d + 2.0f; t <= end; t += 2.0f) {
            Vector2 p = PointOnRoundedRect(r, rad, t);
            DrawLineEx(prev, p, thick, col);
            prev = p;
        }
    }
}

inline void drawDashedRoundedSlot(Rectangle rec, const std::string& letter, bool isCursor,
                                  bool showUnderscore = false) {
    float radius = rec.width * 0.20f;
    DropShadow(rec, radius, 4.0f, 0.30f, 1.0f);
    GradientRound(rec, radius, Color{ 13, 23, 43, 255 }, Color{ 7, 12, 26, 255 });
    DrawRectangleRoundedLines(rec, Roundness(rec, radius), 12, 1.6f, Theme::WithAlpha(Color{ 51, 65, 85, 255 }, 0.6f));
    if (isCursor) {
        for (int i = 1; i <= 2; ++i) {
            DrawRectangleRoundedLines({ rec.x - i * 2.0f, rec.y - i * 2.0f, rec.width + i * 4.0f, rec.height + i * 4.0f },
                                      Roundness(rec, radius + 2.0f), 12, 2.0f,
                                      Theme::WithAlpha(Color{ 56, 189, 248, 255 }, 0.16f * (3 - i)));
        }
    }
    DashedRoundedBorder({ rec.x + 3.0f, rec.y + 3.0f, rec.width - 6.0f, rec.height - 6.0f }, radius,
                        isCursor ? Color{ 103, 232, 249, 255 } : Theme::WithAlpha(Color{ 226, 232, 240, 240 }, 0.95f),
                        2.2f, 7.0f, 5.0f);
    if (!letter.empty()) {
        Theme::text3D(Theme::fontCartoon, letter, { rec.x + rec.width * 0.5f, rec.y + rec.height * 0.48f },
                      rec.height * 0.62f, WHITE, Color{ 30, 41, 59, 255 }, 1.8f,
                      Theme::WithAlpha(BLACK, 0.5f), true, true);
    } else if (showUnderscore) {
        DrawLineEx({ rec.x + rec.width * 0.24f, rec.y + rec.height * 0.78f },
                   { rec.x + rec.width * 0.76f, rec.y + rec.height * 0.78f },
                   3.0f, Theme::WithAlpha(WHITE, 0.85f));
    }
}

inline void drawCreamKeyboardTile(Rectangle rec, const std::string& letter, bool isHovered, bool isPressed) {
    float lift = isPressed ? 3.0f : (isHovered ? 2.0f : 0.0f);
    Rectangle body{ rec.x, rec.y - lift, rec.width, rec.height };
    DropShadow(body, 12.0f, 5.0f + lift, 0.33f, 1.0f);
    GradientRound({ body.x, body.y + 4.0f, body.width, body.height }, 12.0f, Color{ 180, 83, 9, 255 }, Color{ 146, 64, 14, 255 });
    GradientRound(body, 12.0f, isHovered ? WHITE : Color{ 255, 253, 245, 255 }, Color{ 250, 235, 205, 255 });
    Rim(body, 12.0f, Color{ 217, 119, 6, 255 }, Theme::WithAlpha(WHITE, 0.95f), Theme::WithAlpha(Color{ 202, 138, 4, 255 }, 0.6f));
    Rectangle gloss{ body.x + 4.0f, body.y + 3.0f, body.width - 8.0f, body.height * 0.35f };
    DrawRectangleRounded(gloss, 0.4f, 8, Theme::WithAlpha(WHITE, 0.6f));
    if (!letter.empty()) {
        Theme::text3D(Theme::fontCartoon, letter, { body.x + body.width * 0.5f, body.y + body.height * 0.51f },
                      body.height * 0.56f, Color{ 15, 23, 42, 255 }, Color{ 71, 85, 105, 210 }, 1.1f,
                      Theme::WithAlpha(Color{ 148, 163, 184, 255 }, 0.55f), true, true);
    }
}

// ---------------------------------------------------------------------------
// Icons
// ---------------------------------------------------------------------------
inline void LockIcon(Vector2 c, float s, Color body = Color{ 251, 191, 36, 255 }) {
    DrawCircleSector({ c.x, c.y - s * 0.18f }, s * 0.40f, 180.0f, 360.0f, 14, Theme::Mix(body, BLACK, 0.25f));
    DrawCircleSector({ c.x, c.y - s * 0.18f }, s * 0.26f, 180.0f, 360.0f, 12, Theme::WithAlpha(BLACK, 0.3f));
    Rectangle r{ c.x - s * 0.52f, c.y - s * 0.12f, s * 1.04f, s * 0.82f };
    DrawRectangleRounded({ r.x, r.y + 2.0f, r.width, r.height }, 0.3f, 8, Theme::Mix(body, BLACK, 0.35f));
    DrawRectangleRounded(r, 0.3f, 8, body);
    DrawRectangleRounded({ r.x + 2.0f, r.y + 2.0f, r.width - 4.0f, r.height * 0.38f }, 0.35f, 6,
                         Theme::WithAlpha(WHITE, 0.35f));
    DrawCircleV({ c.x, c.y + s * 0.22f }, s * 0.10f, Theme::WithAlpha(BLACK, 0.5f));
}

inline void CheckBadge(Vector2 c, float r, Color fill = Color{ 34, 197, 94, 255 }) {
    DrawCircleV({ c.x, c.y + 2.0f }, r, Theme::WithAlpha(BLACK, 0.35f));
    DrawCircleV(c, r, fill);
    DrawCircleGradient((int)c.x, (int)(c.y - r * 0.25f), r, Theme::WithAlpha(WHITE, 0.25f), Theme::WithAlpha(WHITE, 0.0f));
    DrawCircleLines((int)c.x, (int)c.y, r, Theme::WithAlpha(WHITE, 0.85f));
    DrawLineEx({ c.x - r * 0.45f, c.y + r * 0.02f }, { c.x - r * 0.10f, c.y + r * 0.38f }, r * 0.26f, WHITE);
    DrawLineEx({ c.x - r * 0.10f, c.y + r * 0.38f }, { c.x + r * 0.48f, c.y - r * 0.38f }, r * 0.26f, WHITE);
}

inline void GearIcon(Vector2 c, float r, Color col) {
    // A clean vector gear: 8 teeth + a ring + a hub.  Drawing it as strokes
    // (instead of a concave polygon) keeps it crisp at every size.
    for (int i = 0; i < 8; ++i) {
        float a = (float)i * 2.0f * PI / 8.0f;
        Vector2 inner{ c.x + std::cos(a) * r * 0.72f, c.y + std::sin(a) * r * 0.72f };
        Vector2 outer{ c.x + std::cos(a) * r * 1.05f, c.y + std::sin(a) * r * 1.05f };
        DrawLineEx(inner, outer, std::max(2.2f, r * 0.24f), col);
    }
    DrawRing(c, r * 0.43f, r * 0.72f, 0.0f, 360.0f, 32, col);
    DrawCircleV(c, r * 0.20f, col);
    // tiny top-left shine
    DrawCircleV({ c.x - r * 0.30f, c.y - r * 0.36f }, std::max(1.0f, r * 0.075f),
                Theme::WithAlpha(WHITE, 0.55f));
}

inline void MagnifierIcon(Vector2 c, float r, Color rim, Color glass, float handleAngle = 0.78f) {
    DrawCircleV({ c.x, c.y + 2.0f }, r, Theme::WithAlpha(BLACK, 0.35f));
    DrawRing(c, r * 0.70f, r, 0.0f, 360.0f, 28, rim);
    DrawCircleV(c, r * 0.72f, Theme::WithAlpha(glass, 0.4f));
    DrawCircleGradient((int)(c.x - r * 0.2f), (int)(c.y - r * 0.25f), r * 0.7f,
                       Theme::WithAlpha(WHITE, 0.65f), Theme::WithAlpha(WHITE, 0.0f));
    Vector2 h0{ c.x + std::cos(handleAngle) * r * 0.95f, c.y + std::sin(handleAngle) * r * 0.95f };
    Vector2 h1{ c.x + std::cos(handleAngle) * r * 1.75f, c.y + std::sin(handleAngle) * r * 1.75f };
    DrawLineEx(h0, h1, r * 0.36f, Theme::Mix(rim, BLACK, 0.25f));
    DrawLineEx(h0, h1, r * 0.20f, rim);
}

inline void BulbIcon(Vector2 c, float r, Color glass = Color{ 255, 250, 210, 255 }) {
    Color outline = Color{ 120, 53, 15, 255 };
    // rays behind the bulb
    for (int i = 0; i < 7; ++i) {
        float a = -PI * 0.5f + (i - 3.0f) * 0.34f;
        DrawLineEx({ c.x + std::cos(a) * r * 0.92f, c.y + std::sin(a) * r * 0.92f - r * 0.30f },
                   { c.x + std::cos(a) * r * 1.28f, c.y + std::sin(a) * r * 1.28f - r * 0.30f },
                   std::max(1.4f, r * 0.12f), Theme::WithAlpha(outline, 0.55f));
    }
    // bulb outline
    DrawCircleV({ c.x, c.y - r * 0.28f }, r * 0.76f, outline);
    DrawCircleV({ c.x, c.y - r * 0.28f }, r * 0.67f, glass);
    DrawCircleGradient((int)(c.x - r * 0.22f), (int)(c.y - r * 0.58f), r * 0.45f,
                       Theme::WithAlpha(WHITE, 0.95f), Theme::WithAlpha(WHITE, 0.0f));
    // screw base
    DrawRectangleRounded({ c.x - r * 0.30f, c.y + r * 0.27f, r * 0.60f, r * 0.30f }, 0.3f, 6, outline);
    DrawRectangleRounded({ c.x - r * 0.26f, c.y + r * 0.29f, r * 0.52f, r * 0.24f }, 0.3f, 6,
                         Color{ 203, 213, 225, 255 });
    DrawLineEx({ c.x - r * 0.20f, c.y + r * 0.36f }, { c.x + r * 0.20f, c.y + r * 0.36f }, 1.4f,
               Theme::WithAlpha(outline, 0.45f));
    DrawLineEx({ c.x - r * 0.18f, c.y + r * 0.44f }, { c.x + r * 0.18f, c.y + r * 0.44f }, 1.4f,
               Theme::WithAlpha(outline, 0.45f));
}

inline void ShuffleIcon(Vector2 c, float r, Color col) {
    auto arrow = [&](Vector2 p0, Vector2 ctrl, Vector2 p1, float th, Color cc) {
        DrawSplineSegmentBezierQuadratic(p0, ctrl, p1, th, cc);
        Vector2 d{ p1.x - ctrl.x, p1.y - ctrl.y };
        float len = std::sqrt(d.x * d.x + d.y * d.y);
        if (len < 0.001f) return;
        d.x /= len; d.y /= len;
        Vector2 n{ -d.y, d.x };
        float h = th * 2.8f;
        Vector2 a{ p1.x - d.x * h + n.x * h * 0.55f, p1.y - d.y * h + n.y * h * 0.55f };
        Vector2 b{ p1.x - d.x * h - n.x * h * 0.55f, p1.y - d.y * h - n.y * h * 0.55f };
        DrawTriangle(p1, a, b, cc);
    };
    Vector2 top0{ c.x - r * 0.82f, c.y - r * 0.30f };
    Vector2 topC{ c.x, c.y - r * 0.92f };
    Vector2 top1{ c.x + r * 0.82f, c.y - r * 0.30f };
    Vector2 bot0{ c.x + r * 0.82f, c.y + r * 0.30f };
    Vector2 botC{ c.x, c.y + r * 0.92f };
    Vector2 bot1{ c.x - r * 0.82f, c.y + r * 0.30f };
    // shadow
    arrow({ top0.x, top0.y + 2.0f }, { topC.x, topC.y + 2.0f }, { top1.x, top1.y + 2.0f }, std::max(2.4f, r * 0.19f),
          Theme::WithAlpha(BLACK, 0.28f));
    arrow({ bot0.x, bot0.y + 2.0f }, { botC.x, botC.y + 2.0f }, { bot1.x, bot1.y + 2.0f }, std::max(2.4f, r * 0.19f),
          Theme::WithAlpha(BLACK, 0.28f));
    float th = std::max(2.0f, r * 0.17f);
    arrow(top0, topC, top1, th, col);
    arrow(bot0, botC, bot1, th, col);
}

inline void MapPinIcon(Vector2 c, float r, Color pin = Color{ 239, 68, 68, 255 }) {
    DrawCircleV({ c.x, c.y - r * 0.25f }, r * 0.72f, pin);
    DrawTriangle({ c.x - r * 0.62f, c.y + r * 0.02f }, { c.x, c.y + r * 0.98f },
                 { c.x + r * 0.62f, c.y + r * 0.02f }, pin);
    DrawCircleV({ c.x, c.y - r * 0.25f }, r * 0.30f, Color{ 255, 241, 242, 255 });
    DrawCircleGradient((int)(c.x - r * 0.22f), (int)(c.y - r * 0.55f), r * 0.38f,
                       Theme::WithAlpha(WHITE, 0.55f), Theme::WithAlpha(WHITE, 0.0f));
}

inline void MenuIcon(Vector2 c, float r, Color col) {
    for (int i = -1; i <= 1; ++i) {
        DrawRectangleRounded({ c.x - r * 0.7f, c.y + i * r * 0.42f - r * 0.10f, r * 1.4f, r * 0.20f },
                             0.9f, 6, col);
    }
}

inline void PauseIcon(Vector2 c, float r, Color col) {
    DrawRectangleRounded({ c.x - r * 0.42f, c.y - r * 0.55f, r * 0.30f, r * 1.10f }, 0.4f, 6, col);
    DrawRectangleRounded({ c.x + r * 0.12f, c.y - r * 0.55f, r * 0.30f, r * 1.10f }, 0.4f, 6, col);
}

inline void SoundIcon(Vector2 c, float r, bool on, Color col) {
    Vector2 p[4] = { { c.x - r * 0.75f, c.y - r * 0.30f }, { c.x - r * 0.28f, c.y - r * 0.30f },
                     { c.x + r * 0.12f, c.y - r * 0.70f }, { c.x + r * 0.12f, c.y + r * 0.70f } };
    DrawTriangleFan(p, 4, col);
    DrawRectangleRec({ c.x - r * 0.75f, c.y - r * 0.30f, r * 0.45f, r * 0.60f }, col);
    if (on) {
        DrawRing(c, r * 0.45f, r * 0.58f, -0.8f, 0.8f, 12, col);
        DrawRing(c, r * 0.72f, r * 0.85f, -0.8f, 0.8f, 12, col);
    } else {
        DrawLineEx({ c.x + r * 0.35f, c.y - r * 0.35f }, { c.x + r * 0.85f, c.y + r * 0.35f }, 2.4f, col);
        DrawLineEx({ c.x + r * 0.85f, c.y - r * 0.35f }, { c.x + r * 0.35f, c.y + r * 0.35f }, 2.4f, col);
    }
}

// ---------------------------------------------------------------------------
// Map nodes
// ---------------------------------------------------------------------------
struct MapNodeStyle {
    Color faceA = { 56, 189, 248, 255 };
    Color faceB = { 2, 132, 199, 255 };
    Color lip = { 3, 105, 161, 255 };
    Color ring = { 224, 242, 254, 255 };
    Color lockedA = { 148, 163, 184, 255 };
    Color lockedB = { 100, 116, 139, 255 };
    bool lockBelow = false;
};

inline void MapNode(Vector2 center, float radius, int levelNumber, bool isUnlocked, bool isCurrent,
                    int stars, bool isCheckmarked, bool twoDigits, const MapNodeStyle& style = {}) {
    // ground shadow / halo
    DrawEllipse((int)center.x, (int)(center.y + radius * 0.58f), radius * 1.15f, radius * 0.42f,
                Theme::WithAlpha(BLACK, 0.30f));
    if (isCurrent) {
        for (int i = 3; i >= 1; --i) {
            DrawCircleV(center, radius + i * 5.0f, Theme::WithAlpha(style.faceA, 0.10f * (4 - i)));
        }
    }
    Color faceA = isUnlocked ? style.faceA : style.lockedA;
    Color faceB = isUnlocked ? style.faceB : style.lockedB;
    Color lip = isUnlocked ? style.lip : Theme::Mix(style.lockedB, BLACK, 0.2f);

    DrawCircleV({ center.x, center.y + 5.0f }, radius + 1.0f, lip);
    DrawCircleV({ center.x, center.y + 2.0f }, radius, faceB);
    DrawCircleGradient((int)center.x, (int)(center.y - radius * 0.18f), radius,
                       Theme::Mix(faceA, WHITE, 0.25f), faceB);
    // rim + bevel
    DrawCircleLines((int)center.x, (int)center.y, radius, style.ring);
    DrawCircleLines((int)center.x, (int)center.y, radius - 2.0f, Theme::WithAlpha(WHITE, 0.55f));
    DrawCircleLines((int)center.x, (int)(center.y + 3.0f), radius - 0.5f, Theme::WithAlpha(BLACK, 0.25f));
    DrawCircleSector({ center.x, center.y - radius * 0.05f }, radius * 0.84f, 200.0f, 340.0f, 18,
                     Theme::WithAlpha(WHITE, 0.30f));
    DrawEllipse((int)(center.x - radius * 0.28f), (int)(center.y - radius * 0.50f),
                radius * 0.26f, radius * 0.12f, Theme::WithAlpha(WHITE, 0.45f));

    // number
    std::string label = std::to_string(levelNumber);
    if (twoDigits && levelNumber < 10) label = "0" + label;
    Theme::text3D(Theme::fontCartoon, label, { center.x, center.y + 1.0f }, radius * 0.94f,
                  WHITE, Theme::Mix(lip, BLACK, 0.25f), 1.8f, Theme::WithAlpha(BLACK, 0.5f), true, true);

    // stars beneath
    float starY = center.y + radius + 9.0f;
    for (int s = -1; s <= 1; ++s) {
        bool earned = isUnlocked && (s + 1) < stars;
        Color sc = earned ? Theme::gold : Theme::WithAlpha(Theme::gold, isUnlocked ? 0.32f : 0.55f);
        Star5({ center.x + s * 13.0f, starY }, 7.0f, 3.1f, sc,
              Theme::WithAlpha(Color{ 120, 53, 15, 255 }, earned ? 0.9f : 0.45f), false);
    }

    if (!isUnlocked) {
        if (style.lockBelow) {
            LockIcon({ center.x, starY + 17.0f }, 13.0f);
        } else {
            Vector2 badge{ center.x + radius * 0.72f, center.y - radius * 0.68f };
            DrawCircleV({ badge.x, badge.y + 1.5f }, 12.0f, Theme::WithAlpha(BLACK, 0.35f));
            DrawCircleV(badge, 11.5f, Color{ 245, 158, 11, 255 });
            DrawCircleLines((int)badge.x, (int)badge.y, 11.5f, Theme::WithAlpha(WHITE, 0.75f));
            LockIcon({ badge.x, badge.y - 1.0f }, 11.0f, Theme::goldLight);
        }
    }
    if (isCheckmarked) {
        CheckBadge({ center.x + radius * 0.72f, center.y - radius * 0.68f }, 11.5f);
    }
}

// ---------------------------------------------------------------------------
// Wooden sign
// ---------------------------------------------------------------------------
inline void WoodenSign(Rectangle board, const std::string& leftWord, const std::string& rightWord,
                       const std::vector<Color>& palette) {
    // posts (behind the board, but extending visibly below it)
    for (int p = 0; p < 2; ++p) {
        float x = board.x + (p == 0 ? board.width * 0.16f : board.width * 0.72f);
        float postW = board.width * 0.12f;
        GradientRound({ x, board.y + board.height * 0.55f, postW, board.height * 0.95f }, 5.0f,
                      Color{ 139, 90, 43, 255 }, Color{ 88, 52, 24, 255 });
        DrawRectangleRoundedLines({ x, board.y + board.height * 0.55f, postW, board.height * 0.95f },
                                  0.2f, 6, 1.4f, Color{ 60, 32, 14, 255 });
        DrawLineEx({ x + postW * 0.30f, board.y + board.height * 0.65f },
                   { x + postW * 0.30f, board.y + board.height * 1.42f }, 1.2f,
                   Theme::WithAlpha(Color{ 220, 165, 112, 255 }, 0.45f));
    }
    DropShadow(board, 18.0f, 7.0f, 0.42f, 2.0f);
    GradientRound({ board.x, board.y + 7.0f, board.width, board.height }, 18.0f,
                  Color{ 130, 74, 35, 255 }, Color{ 84, 45, 20, 255 });
    GradientRound(board, 18.0f, Color{ 205, 133, 74, 255 }, Color{ 166, 101, 53, 255 });
    for (int i = 1; i < 3; ++i) {
        float y = board.y + board.height * (i / 3.0f);
        DrawLineEx({ board.x + 14.0f, y }, { board.x + board.width - 14.0f, y }, 1.4f,
                   Theme::WithAlpha(Color{ 110, 58, 26, 255 }, 0.55f));
    }
    DrawRectangleRoundedLines(board, Roundness(board, 18.0f), 16, 3.0f, Color{ 100, 52, 22, 255 });
    DrawRectangleRoundedLines({ board.x + 3.0f, board.y + 3.0f, board.width - 6.0f, board.height - 6.0f },
                              Roundness(board, 15.0f), 16, 1.2f, Theme::WithAlpha(Color{ 248, 190, 140, 255 }, 0.55f));
    // nails
    for (int i = 0; i < 4; ++i) {
        float nx = board.x + 10.0f + (i % 2) * (board.width - 20.0f);
        float ny = board.y + 9.0f + (i / 2) * (board.height - 18.0f);
        DrawCircleV({ nx, ny }, 3.2f, Color{ 180, 150, 110, 255 });
        DrawCircleV({ nx - 0.8f, ny - 0.8f }, 1.4f, Theme::WithAlpha(WHITE, 0.65f));
    }

    // two lines of colorful letters
    auto drawLine = [&](const std::string& s, float y, float size) {
        float total = 0.0f;
        for (char c : s) total += Theme::measure(std::string(1, c), size).x + 1.5f;
        float x = board.x + (board.width - total) * 0.5f;
        for (size_t i = 0; i < s.size(); ++i) {
            std::string ch(1, s[i]);
            Color col = palette[i % palette.size()];
            Theme::text3D(Theme::fontCartoon, ch, { x, y }, size, col,
                          Theme::WithAlpha(Color{ 70, 34, 12, 255 }, 0.95f), 2.0f,
                          Theme::WithAlpha(Color{ 40, 18, 8, 255 }, 0.8f), false, true);
            x += Theme::measure(ch, size).x + 1.5f;
        }
    };
    drawLine(leftWord, board.y + board.height * 0.26f, 31.0f);
    drawLine(rightWord, board.y + board.height * 0.62f, 28.0f);
}

} // namespace UI
