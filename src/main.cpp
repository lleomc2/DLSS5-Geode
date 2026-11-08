#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

class $modify(DLSS5PlayLayer, PlayLayer) {
    struct Fields {
        CCNode* visualLayer = nullptr;
        CCLayerColor* colorGrade = nullptr;
        CCLayerColor* bloomWash = nullptr;
        CCLayerColor* vignetteTop = nullptr;
        CCLayerColor* vignetteBottom = nullptr;
        CCLayerColor* vignetteLeft = nullptr;
        CCLayerColor* vignetteRight = nullptr;
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

        m_fields->colorGrade = CCLayerColor::create(ccc4(20, 190, 255, 255), size.width, size.height);
        m_fields->colorGrade->setAnchorPoint({0.f, 0.f});
        m_fields->colorGrade->setPosition({0.f, 0.f});
        m_fields->colorGrade->setOpacity(0);
        m_fields->visualLayer->addChild(m_fields->colorGrade);

        m_fields->bloomWash = CCLayerColor::create(ccc4(220, 245, 255, 255), size.width, size.height);
        m_fields->bloomWash->setAnchorPoint({0.f, 0.f});
        m_fields->bloomWash->setPosition({0.f, 0.f});
        m_fields->bloomWash->setOpacity(0);
        m_fields->visualLayer->addChild(m_fields->bloomWash);

        m_fields->vignetteTop = CCLayerColor::create(ccc4(0, 0, 0, 255), size.width, 14.f);
        m_fields->vignetteTop->setPosition({0.f, size.height - 14.f});
        m_fields->visualLayer->addChild(m_fields->vignetteTop);

        m_fields->vignetteBottom = CCLayerColor::create(ccc4(0, 0, 0, 255), size.width, 14.f);
        m_fields->vignetteBottom->setPosition({0.f, 0.f});
        m_fields->visualLayer->addChild(m_fields->vignetteBottom);

        m_fields->vignetteLeft = CCLayerColor::create(ccc4(0, 0, 0, 255), 10.f, size.height);
        m_fields->vignetteLeft->setPosition({0.f, 0.f});
        m_fields->visualLayer->addChild(m_fields->vignetteLeft);

        m_fields->vignetteRight = CCLayerColor::create(ccc4(0, 0, 0, 255), 10.f, size.height);
        m_fields->vignetteRight->setPosition({size.width - 10.f, 0.f});
        m_fields->visualLayer->addChild(m_fields->vignetteRight);

        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!m_fields->colorGrade || !m_fields->bloomWash) {
            return;
        }

        const bool enabled = Mod::get()->getSettingValue<bool>("enabled");
        m_fields->visualLayer->setVisible(enabled);
        if (!enabled) {
            return;
        }

        const double rawIntensity = Mod::get()->getSettingValue<double>("intensity");
        const float intensity = std::clamp(static_cast<float>(rawIntensity), 0.f, 100.f) / 100.f;
        const auto style = Mod::get()->getSettingValue<std::string>("color-style");

        ccColor3B tint = ccc3(20, 190, 255);
        if (style == "Violet") {
            tint = ccc3(145, 85, 255);
        } else if (style == "Warm") {
            tint = ccc3(255, 145, 70);
        } else if (style == "Neutral") {
            tint = ccc3(220, 230, 245);
        }

        m_fields->elapsed += dt;
        const float pulse = 0.88f + 0.12f * std::sin(m_fields->elapsed * 1.7f);
        m_fields->colorGrade->setColor(tint);
        m_fields->colorGrade->setOpacity(static_cast<GLubyte>(std::round(18.f * intensity * pulse)));
        m_fields->bloomWash->setOpacity(static_cast<GLubyte>(std::round(7.f * intensity * pulse)));

        const auto vignette = static_cast<GLubyte>(std::round(42.f * intensity));
        m_fields->vignetteTop->setOpacity(vignette);
        m_fields->vignetteBottom->setOpacity(vignette);
        m_fields->vignetteLeft->setOpacity(vignette);
        m_fields->vignetteRight->setOpacity(vignette);
    }
};

$on_mod(Loaded) {
    log::info("DLSS5 Geode visual effects loaded.");
}
