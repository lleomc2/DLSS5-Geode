#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <array>
#include <cmath>

using namespace geode::prelude;

namespace {
    constexpr std::size_t kTrailLength = 14;

    ccColor4F makeGlow(float r, float g, float b, float a) {
        return {r, g, b, a};
    }
}

class $modify(DLSS5PlayLayer, PlayLayer) {
    struct Fields {
        CCNode* visualLayer = nullptr;
        CCLayerColor* colorGrade = nullptr;
        CCLayerColor* bloomWash = nullptr;
        CCLayerColor* vignetteTop = nullptr;
        CCLayerColor* vignetteBottom = nullptr;
        CCLayerColor* vignetteLeft = nullptr;
        CCLayerColor* vignetteRight = nullptr;
        CCDrawNode* lightFX = nullptr;
        std::array<CCPoint, kTrailLength> player1Trail{};
        std::array<CCPoint, kTrailLength> player2Trail{};
        std::size_t player1TrailCount = 0;
        std::size_t player2TrailCount = 0;
        float elapsed = 0.f;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        const auto size = CCDirector::get()->getWinSize();

        m_fields->visualLayer = CCNode::create();
        m_fields->visualLayer->setID("dlss5-visual-layer");
        addChild(m_fields->visualLayer, 9999);

        // Subtle color grade and highlight lift. These are lightweight overlays,
        // so they remain compatible with the GLES 2.0 path used on Android.
        m_fields->colorGrade = CCLayerColor::create(ccc4(22, 160, 255, 255), size.width, size.height);
        m_fields->colorGrade->setAnchorPoint({0.f, 0.f});
        m_fields->colorGrade->setPosition({0.f, 0.f});
        m_fields->colorGrade->setOpacity(0);
        m_fields->visualLayer->addChild(m_fields->colorGrade, 1);

        m_fields->bloomWash = CCLayerColor::create(ccc4(210, 238, 255, 255), size.width, size.height);
        m_fields->bloomWash->setAnchorPoint({0.f, 0.f});
        m_fields->bloomWash->setPosition({0.f, 0.f});
        m_fields->bloomWash->setOpacity(0);
        m_fields->visualLayer->addChild(m_fields->bloomWash, 2);

        // Cinematic frame shading.
        m_fields->vignetteTop = CCLayerColor::create(ccc4(0, 0, 0, 255), size.width, 22.f);
        m_fields->vignetteTop->setPosition({0.f, size.height - 22.f});
        m_fields->visualLayer->addChild(m_fields->vignetteTop, 3);

        m_fields->vignetteBottom = CCLayerColor::create(ccc4(0, 0, 0, 255), size.width, 22.f);
        m_fields->vignetteBottom->setPosition({0.f, 0.f});
        m_fields->visualLayer->addChild(m_fields->vignetteBottom, 3);

        m_fields->vignetteLeft = CCLayerColor::create(ccc4(0, 0, 0, 255), 16.f, size.height);
        m_fields->vignetteLeft->setPosition({0.f, 0.f});
        m_fields->visualLayer->addChild(m_fields->vignetteLeft, 3);

        m_fields->vignetteRight = CCLayerColor::create(ccc4(0, 0, 0, 255), 16.f, size.height);
        m_fields->vignetteRight->setPosition({size.width - 16.f, 0.f});
        m_fields->visualLayer->addChild(m_fields->vignetteRight, 3);

        // Additive light strokes, player aura rings and motion trails.
        m_fields->lightFX = CCDrawNode::create();
        m_fields->lightFX->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        m_fields->visualLayer->addChild(m_fields->lightFX, 4);

        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!m_fields->visualLayer || !m_fields->colorGrade || !m_fields->bloomWash || !m_fields->lightFX) {
            return;
        }

        const bool enabled = Mod::get()->getSettingValue<bool>("enabled");
        m_fields->visualLayer->setVisible(enabled);
        if (!enabled) {
            m_fields->player1TrailCount = 0;
            m_fields->player2TrailCount = 0;
            return;
        }

        const double rawIntensity = Mod::get()->getSettingValue<double>("intensity");
        const float intensity = std::clamp(static_cast<float>(rawIntensity), 0.f, 100.f) / 100.f;
        const auto style = Mod::get()->getSettingValue<std::string>("color-style");
        const auto quality = Mod::get()->getSettingValue<std::string>("quality");
        const bool playerAura = Mod::get()->getSettingValue<bool>("player-aura");
        const bool lightRays = Mod::get()->getSettingValue<bool>("light-rays");

        ccColor3B tint = ccc3(22, 160, 255);
        ccColor4F glow = makeGlow(0.16f, 0.72f, 1.00f, 1.0f);
        if (style == "Violet") {
            tint = ccc3(145, 85, 255);
            glow = makeGlow(0.62f, 0.28f, 1.00f, 1.0f);
        } else if (style == "Warm") {
            tint = ccc3(255, 145, 70);
            glow = makeGlow(1.00f, 0.48f, 0.16f, 1.0f);
        } else if (style == "Neutral") {
            tint = ccc3(220, 230, 245);
            glow = makeGlow(0.72f, 0.86f, 1.00f, 1.0f);
        }

        m_fields->elapsed += std::clamp(dt, 0.f, 0.05f);
        const float pulse = 0.90f + 0.10f * std::sin(m_fields->elapsed * 1.35f);
        m_fields->colorGrade->setColor(tint);
        m_fields->colorGrade->setOpacity(static_cast<GLubyte>(std::round(25.f * intensity * pulse)));
        m_fields->bloomWash->setOpacity(static_cast<GLubyte>(std::round(13.f * intensity * pulse)));

        const auto vignette = static_cast<GLubyte>(std::round(38.f * intensity));
        m_fields->vignetteTop->setOpacity(vignette);
        m_fields->vignetteBottom->setOpacity(vignette);
        m_fields->vignetteLeft->setOpacity(static_cast<GLubyte>(vignette * 0.82f));
        m_fields->vignetteRight->setOpacity(static_cast<GLubyte>(vignette * 0.82f));

        m_fields->lightFX->clear();

        if (intensity <= 0.001f) {
            return;
        }

        const auto size = CCDirector::get()->getWinSize();
        const unsigned int rayCount = quality == "Ultra" ? 4u : (quality == "Balanced" ? 3u : 1u);
        const float rayAlpha = (quality == "Ultra" ? 0.045f : 0.028f) * intensity;

        // Soft, slowly moving volumetric-style shafts. They are deliberately
        // translucent so gameplay objects and hazards stay readable.
        if (lightRays) {
            for (unsigned int i = 0; i < rayCount; ++i) {
                const float phase = m_fields->elapsed * 0.12f + static_cast<float>(i) * 1.9f;
                const float x = size.width * (0.12f + 0.22f * static_cast<float>(i)) +
                    std::sin(phase) * size.width * 0.035f;
                const float spread = size.width * (0.10f + 0.015f * static_cast<float>(i));
                CCPoint beam[4] = {
                    {x, size.height},
                    {x + spread * 0.24f, size.height},
                    {x + spread, 0.f},
                    {x + spread * 0.66f, 0.f}
                };
                const auto beamColor = makeGlow(glow.r, glow.g, glow.b, rayAlpha);
                m_fields->lightFX->drawPolygon(
                    beam, 4, beamColor, 0.f, makeGlow(0.f, 0.f, 0.f, 0.f)
                );
            }
        }

        auto drawPlayerAura = [&](PlayerObject* player,
                                  std::array<CCPoint, kTrailLength>& trail,
                                  std::size_t& trailCount) {
            if (!player || !player->isVisible()) {
                trailCount = 0;
                return;
            }

            const CCPoint worldPoint = player->convertToWorldSpace({0.f, 0.f});
            const CCPoint point = m_fields->visualLayer->convertToNodeSpace(worldPoint);

            for (std::size_t i = kTrailLength - 1; i > 0; --i) {
                trail[i] = trail[i - 1];
            }
            trail[0] = point;
            trailCount = std::min(trailCount + 1, kTrailLength);

            if (playerAura) {
                const unsigned int rings = quality == "Ultra" ? 4u : (quality == "Balanced" ? 3u : 2u);
                for (unsigned int i = 0; i < rings; ++i) {
                    const float radius = 18.f + static_cast<float>(i) * 8.f;
                    const float alpha = (0.17f / (1.f + static_cast<float>(i) * 0.85f)) * intensity * pulse;
                    const auto ring = makeGlow(glow.r, glow.g, glow.b, alpha);
                    m_fields->lightFX->drawCircle(
                        point, radius, makeGlow(0.f, 0.f, 0.f, 0.f),
                        2.8f - static_cast<float>(i) * 0.45f, ring, 28
                    );
                }

                // A short horizontal specular streak gives the player a crisp,
                // high-contrast highlight without changing collision or physics.
                const float streak = 17.f + 4.f * pulse;
                m_fields->lightFX->drawSegment(
                    {point.x - streak, point.y + 1.f},
                    {point.x + streak, point.y + 1.f},
                    0.9f, makeGlow(glow.r, glow.g, glow.b, 0.22f * intensity)
                );
            }

            if (quality != "Performance") {
                for (std::size_t i = 1; i < trailCount; ++i) {
                    const float fade = (1.f - static_cast<float>(i) / static_cast<float>(kTrailLength)) *
                        0.20f * intensity;
                    const float width = quality == "Ultra" ? 2.6f : 1.8f;
                    m_fields->lightFX->drawSegment(
                        trail[i - 1], trail[i], width,
                        makeGlow(glow.r, glow.g, glow.b, fade)
                    );
                }
            }
        };

        drawPlayerAura(m_player1, m_fields->player1Trail, m_fields->player1TrailCount);
        drawPlayerAura(m_player2, m_fields->player2Trail, m_fields->player2TrailCount);
    }
};

$on_mod(Loaded) {
    log::info("DLSS5 Geode visual engine loaded (mobile-friendly cinematic effects).");
}
