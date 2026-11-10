#pragma once
#include "raylib.h"

// The game renders into a fixed 540x960 canvas which is then letterboxed into
// the actual OS window.  Raw mouse positions are in window pixels, so every
// screen must use this transform instead of GetMousePosition() directly.
namespace Input {

inline float scaleX = 1.0f;
inline float scaleY = 1.0f;
inline float offsetX = 0.0f;
inline float offsetY = 0.0f;

inline void SetCanvasTransform(float sx, float sy, float ox, float oy) {
    scaleX = (sx > 0.0001f) ? sx : 1.0f;
    scaleY = (sy > 0.0001f) ? sy : 1.0f;
    offsetX = ox;
    offsetY = oy;
}

inline Vector2 Mouse() {
    Vector2 m = GetMousePosition();
    return { (m.x - offsetX) / scaleX, (m.y - offsetY) / scaleY };
}

// Recompute the letterbox rectangle that preserves the 9:16 canvas aspect.
inline Rectangle CanvasDestination(int windowW, int windowH) {
    const float aspect = 540.0f / 960.0f;
    float w = (float)windowW;
    float h = w / aspect;
    if (h > (float)windowH) {
        h = (float)windowH;
        w = h * aspect;
    }
    return { ((float)windowW - w) * 0.5f, ((float)windowH - h) * 0.5f, w, h };
}

inline void UpdateFromWindow() {
    Rectangle d = CanvasDestination(GetScreenWidth(), GetScreenHeight());
    SetCanvasTransform(d.width / 540.0f, d.height / 960.0f, d.x, d.y);
}

} // namespace Input
