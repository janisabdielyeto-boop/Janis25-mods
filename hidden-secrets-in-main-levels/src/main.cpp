#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace geode::prelude;

static bool g_orbGroup1Clicked = false;
static bool g_triggerGroup2Now = false;

struct RewindFrame {
    float x;
    float y;
    float rotation;
    bool isUpsideDown;
    bool isShip;
    unsigned int musicMS;
};

class $modify(ReverseOrbHook, GJBaseGameLayer) {
    void toggleGroup(int group, bool activate) {
        GJBaseGameLayer::toggleGroup(group, activate);
        if (group == 1 || group == 4) {
            g_orbGroup1Clicked = true;
            g_triggerGroup2Now = true;
        }
    }
};

class ReverseConfirmPopup : public FLAlertLayer {
protected:
    std::function<void(bool)> m_callback;

    bool init(std::function<void(bool)> cb) {
        if (!FLAlertLayer::init(0)) return false;
        m_callback = std::move(cb);

        auto win = CCDirector::sharedDirector()->getWinSize();
        m_mainLayer->setPosition({ 0.0f, win.height });

        auto bg = extension::CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({ 360.0f, 185.0f });
        bg->setPosition({ win.width * 0.5f, win.height * 0.5f });
        m_mainLayer->addChild(bg);

        auto title = CCLabelBMFont::create("Reverse Secret", "goldFont.fnt");
        title->setPosition({ win.width * 0.5f, win.height * 0.5f + 64.0f });
        title->setScale(0.75f);
        m_mainLayer->addChild(title);

        auto msg1 = CCLabelBMFont::create("You clicked on a toggle orb, which activated a secret.", "bigFont.fnt");
        msg1->setScale(0.32f);
        msg1->setPosition({ win.width * 0.5f, win.height * 0.5f + 18.0f });
        m_mainLayer->addChild(msg1);

        auto msg2 = CCLabelBMFont::create("Do you want to confirm the click ?", "bigFont.fnt");
        msg2->setScale(0.35f);
        msg2->setPosition({ win.width * 0.5f, win.height * 0.5f - 4.0f });
        m_mainLayer->addChild(msg2);

        auto menu = CCMenu::create();
        menu->setPosition({ 0.0f, 0.0f });
        m_mainLayer->addChild(menu);

        auto yesSpr = ButtonSprite::create("YES", 90, true, "goldFont.fnt", "GJ_button_01.png", 30.0f, 0.8f);
        auto yesBtn = CCMenuItemSpriteExtra::create(yesSpr, this, menu_selector(ReverseConfirmPopup::onYes));
        yesBtn->setPosition({ win.width * 0.5f - 68.0f, win.height * 0.5f - 54.0f });
        menu->addChild(yesBtn);

        auto noSpr = ButtonSprite::create("NO", 90, true, "goldFont.fnt", "GJ_button_06.png", 30.0f, 0.8f);
        auto noBtn = CCMenuItemSpriteExtra::create(noSpr, this, menu_selector(ReverseConfirmPopup::onNo));
        noBtn->setPosition({ win.width * 0.5f + 68.0f, win.height * 0.5f - 54.0f });
        menu->addChild(noBtn);

        m_mainLayer->runAction(CCEaseBackOut::create(CCMoveTo::create(0.32f, { 0.0f, 0.0f })));
        return true;
    }

    void closeWithChoice(bool yes) {
        auto win = CCDirector::sharedDirector()->getWinSize();
        m_mainLayer->runAction(CCSequence::create(
            CCEaseBackIn::create(CCMoveTo::create(0.28f, { 0.0f, win.height })),
            CCCallFunc::create(this, callfunc_selector(ReverseConfirmPopup::finishClose)),
            nullptr
        ));
        if (m_callback) m_callback(yes);
    }

    void finishClose() { this->removeFromParentAndCleanup(true); }
    void onYes(CCObject*) { closeWithChoice(true); }
    void onNo(CCObject*) { closeWithChoice(false); }
    void keyBackClicked() override {}

public:
    static ReverseConfirmPopup* create(std::function<void(bool)> cb) {
        auto ret = new ReverseConfirmPopup();
        if (ret && ret->init(std::move(cb))) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
class $modify(StereoReversePlayHook, PlayLayer) {
    struct Fields {
        bool m_isReverseEligible = false;
        bool m_isOfficialSM = false;
        int m_activeLevelId = 0;
        std::vector<RewindFrame> m_history;
        int m_state = 0;
        float m_timer = 0.0f;
        float m_rewindCursor = 0.0f;
        float m_realMaxX = 0.0f;
        CCNode* m_completeTextNode = nullptr;
    };

    void spawnObjectString(const char* setupStr) {
        if (auto arr = this->createObjectsFromSetup(setupStr)) {
            for (int i = 0; i < arr->count(); ++i) {
                if (auto obj = typeinfo_cast<GameObject*>(arr->objectAtIndex(i))) {
                    this->addObject(obj);
                }
            }
        }
    }

    void configureGroup2ReverseCourse() {
        this->activateObjectsOnGroup(2);
        if (!m_fields->m_isOfficialSM) return;
        if (m_objects) {
            for (int i = 0; i < m_objects->count(); ++i) {
                auto obj = typeinfo_cast<GameObject*>(m_objects->objectAtIndex(i));
                if (!obj) continue;
                int id = obj->m_objectID;
                if (id == 12 || id == 13) {
                    obj->addToGroup(2);
                    obj->setPosition({ obj->getPositionX() + 90.0f, obj->getPositionY() + 15.0f });
                }
            }
        }
        spawnObjectString("1,35,2,23800,3,135;1,36,2,21600,3,195;1,35,2,19450,3,135;1,141,2,17900,3,225;1,35,2,16200,3,105;1,36,2,14600,3,195;1,35,2,11800,3,105;1,140,2,9600,3,105;1,36,2,7800,3,195;1,35,2,5400,3,105;1,36,2,3200,3,180;1,35,2,1650,3,105;");
    }

    bool init(GJGameLevel* level, bool useReplay) {
        if (!PlayLayer::init(level, useReplay)) return false;
        if (!level) return true;
        std::string name = level->m_levelName;
        bool isOfficialSM = (level->m_levelID == 1 || name == "Stereo Madness");
        bool isCustomSM = (name == "Secret Madness");
        bool isDryOut = (level->m_levelID == 4 || name == "Dry Out");
        if (isOfficialSM || isCustomSM || isDryOut) {
            m_fields->m_isReverseEligible = true;
            m_fields->m_isOfficialSM = isOfficialSM && !isCustomSM;
            m_fields->m_activeLevelId = isDryOut ? 4 : 1;
        }
        if (m_objects) {
            for (int i = 0; i < m_objects->count(); ++i) {
                if (auto obj = typeinfo_cast<GameObject*>(m_objects->objectAtIndex(i))) {
                    if (obj->getPositionX() > m_fields->m_realMaxX) m_fields->m_realMaxX = obj->getPositionX();
                }
            }
        }
        if (m_fields->m_isOfficialSM) {
            spawnObjectString("1,1594,2,25155,3,165,51,1;1,901,2,25185,3,165,51,2,28,90,29,15,10,0.4;");
        }
        return true;
    }

    void resetRewindState() {
        g_orbGroup1Clicked = false;
        g_triggerGroup2Now = false;
        m_fields->m_history.clear();
        m_fields->m_state = 0;
        m_fields->m_timer = 0.0f;
        m_fields->m_rewindCursor = 0.0f;
        m_fields->m_completeTextNode = nullptr;
        if (m_player1) {
            m_player1->reverseDirection(false);
            m_player1->setFlipX(false);
        }
    }

    void resetLevel() { resetRewindState(); PlayLayer::resetLevel(); }
    void onQuit() { resetRewindState(); PlayLayer::onQuit(); }
    void showCompleteText() {
        if (g_orbGroup1Clicked && m_fields->m_isReverseEligible) {
            PlayLayer::showCompleteText();
            m_fields->m_state = 1;
            m_fields->m_timer = 1.0f;
            if (auto children = this->getChildren()) {
                for (int i = children->count() - 1; i >= 0; --i) {
                    auto node = static_cast<CCNode*>(children->objectAtIndex(i));
                    if (node && node->getZOrder() >= 10 && node != m_player1) {
                        m_fields->m_completeTextNode = node;
                        break;
                    }
                }
            }
            return;
        }
        PlayLayer::showCompleteText();
    }

    void showEndLayer() {
        if (g_orbGroup1Clicked && m_fields->m_isReverseEligible && m_fields->m_state != 99) return;
        PlayLayer::showEndLayer();
    }

    void startFullRewind() {
        m_fields->m_state = 3;
        m_fields->m_timer = 0.35f;
        if (auto fmod = FMODAudioEngine::sharedEngine()) fmod->playEffect("sfx_endStart_02_rev.ogg"_spr);
        if (m_fields->m_completeTextNode) {
            m_fields->m_completeTextNode->stopAllActions();
            m_fields->m_completeTextNode->runAction(CCSequence::create(CCEaseBackIn::create(CCScaleTo::create(0.32f, 0.0f)), CCHide::create(), nullptr));
        }
    }

    void postUpdate(float dt) {
        if (!m_fields->m_isReverseEligible || !m_player1) { PlayLayer::postUpdate(dt); return; }
        if (g_triggerGroup2Now) { g_triggerGroup2Now = false; this->configureGroup2ReverseCourse(); }

        if (m_fields->m_state == 0) {
            PlayLayer::postUpdate(dt);
            unsigned int ms = 0;
            if (auto fmod = FMODAudioEngine::sharedEngine()) ms = fmod->getMusicTimeMS(0);
            m_fields->m_history.push_back({ m_player1->getPositionX(), m_player1->getPositionY(), m_player1->getRotation(), m_player1->m_isUpsideDown, m_player1->m_isShip, ms });
            return;
        }
        if (m_fields->m_state == 1) {
            m_fields->m_timer -= dt;
            if (m_fields->m_timer <= 0.0f) {
                m_fields->m_state = 2;
                auto pop = ReverseConfirmPopup::create([this](bool confirmed) {
                    if (!confirmed) { m_fields->m_state = 99; this->showEndLayer(); }
                    else { this->startFullRewind(); }
                });
                if (pop) { pop->setZOrder(1000); this->addChild(pop); }
            }
            return;
        }
        if (m_fields->m_state == 2) return;
        if (m_fields->m_state == 3) {
            m_fields->m_timer -= dt;
            if (m_fields->m_timer <= 0.0f) {
                m_fields->m_state = 4;
                m_fields->m_timer = 1.15f;
                m_fields->m_rewindCursor = static_cast<float>(m_fields->m_history.size()) - 1.0f;
                this->m_hasCompletedLevel = false;
                m_player1->setVisible(true);
                m_player1->setFlipX(true);
                if (auto fmod = FMODAudioEngine::sharedEngine()) {
                    const char* revTrack = (m_fields->m_activeLevelId == 4) ? "04_DryOut_rev.mp3"_spr : "01_StereoMadness_rev.mp3"_spr;
                    fmod->playMusic(revTrack, false, 0.0f, 0);
                    unsigned int endMS = m_fields->m_history.empty() ? 0 : m_fields->m_history.back().musicMS;
                    unsigned int totalMS = fmod->getMusicLengthMS(0);
                    if (totalMS > endMS) fmod->setMusicTimeMS(totalMS - endMS, true, 0);
                }
            }
            return;
        }
        if (m_fields->m_state == 4) {
            m_fields->m_timer -= dt;
            m_fields->m_rewindCursor -= dt * 60.0f;
            int idx = static_cast<int>(m_fields->m_rewindCursor);
            if (m_fields->m_timer > 0.0f && idx > 0 && !m_fields->m_history.empty()) {
                const auto& f = m_fields->m_history[std::min(idx, static_cast<int>(m_fields->m_history.size()) - 1)];
                m_player1->setPosition({ f.x, f.y });
                m_player1->m_position = CCPoint(f.x, f.y);
                m_player1->setRotation(f.rotation);
                m_player1->setFlipX(true);
                this->m_gameState.m_cameraPosition.x = std::max(0.0f, f.x - 140.0f);
            } else {
                m_fields->m_state = 5;
                m_player1->reverseDirection(true);
                m_player1->setFlipX(true);
            }
            return;
        }
        if (m_fields->m_state == 5) {
            PlayLayer::postUpdate(dt);
            m_player1->setFlipX(true);
            float curX = m_player1->getPositionX();
            if (m_fields->m_realMaxX > 300.0f) {
                float revPct = std::clamp((curX / m_fields->m_realMaxX) * 100.0f, 0.0f, 100.0f);
                if (m_percentageLabel) m_percentageLabel->setString(fmt::format("{:.2f}%", revPct).c_str());
                this->updateProgressbar();
            }
            if (curX <= 65.0f) {
                m_fields->m_state = 6;
                m_fields->m_timer = 0.55f;
                if (auto fmod = FMODAudioEngine::sharedEngine()) {
                    fmod->stopAllMusic(true);
                    fmod->playEffect("sfx_playSound_01_rev.ogg"_spr);
                }
            }
            return;
        }
        if (m_fields->m_state == 6) {
            m_fields->m_timer -= dt;
            if (m_fields->m_timer <= 0.0f) { m_fields->m_state = 99; this->onQuit(); }
        }
    }
};