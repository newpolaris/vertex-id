#include "RenderHUD.h"

#include <glad/glad.h>

#include "nanovg.h"

namespace {
    // ndc to viewport
    // NOTE:
    //  what if 0.5*w*(x + 1), 0.5*h*(1 - y)
    glm::mat4 GetViewportTransform(glm::vec2 viewport) {
        auto w = viewport.x;
        auto h = viewport.y;

        // depth 0, 1 (not used :)
        return glm::mat4{
            w / 2, 0, 0, 0,
            0, -h / 2, 0, 0,
            0, 0, 1 - 0, 0,
            w / 2, h / 2, 0, 1
        };
    }
}

RenderHUD::RenderHUD(NVGcontext* context) :
    vg(context)
{
}

void RenderHUD::BeginFrame(const glm::mat4& transform, const glm::vec2& viewport)
{
    _transform = transform;
    _viewport = viewport;

    nvgBeginFrame(vg, viewport.x, viewport.y, 1.0);
    nvgSave(vg);
    nvgFontFace(vg, "sans");
    nvgFontSize(vg, 30.0f);
}

void RenderHUD::EndFrame()
{
    nvgRestore(vg);
    nvgEndFrame(vg);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderHUD::FlushDrawings()
{
    const auto toScreen = GetViewportTransform(_viewport);
    auto getSSPoint = [](const glm::mat4& mvp, const glm::mat4& sp, const glm::vec4& p) {
        auto ndc = mvp * p;
        ndc /= ndc.w;
        return glm::vec2(sp * ndc);
    };

    for (auto& [c, r, color] : _circles) {
        auto o = glm::vec4(c, 1.f);
        auto osp = getSSPoint(_transform, toScreen, o);

        nvgBeginPath(vg);
        nvgCircle(vg, osp.x, osp.y, 5);
        nvgFillColor(vg, nvgRGBAf(color.r, color.g, color.b, color.a));
        nvgFill(vg);
    }

    for (auto& [st, ed, w, color] : _lines) {
        auto sp = glm::vec4(st, 1.f);
        auto np = glm::vec4(ed, 1.f);
        auto osp = getSSPoint(_transform, toScreen, sp);
        auto onp = getSSPoint(_transform, toScreen, np);

        nvgBeginPath(vg);
        nvgMoveTo(vg, osp.x, osp.y);
        nvgLineTo(vg, onp.x, onp.y);
        nvgStrokeColor(vg, nvgRGBAf(color.r, color.g, color.b, color.a));
        nvgStrokeWidth(vg, w);
        nvgStroke(vg);
    }

    _lines.clear();
    _circles.clear();
}

void RenderHUD::Circle(const glm::vec3& position, float radius, const glm::vec4& color)
{
    _circles.push_back({position, radius, color});
}

void RenderHUD::Line(const glm::vec3& start, const glm::vec3& end, float width, const glm::vec4& color)
{
    _lines.push_back({start, end, width, color});
}
