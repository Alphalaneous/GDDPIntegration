//geode header
#include <Geode/Geode.hpp>

#include <Geode/modify/LevelInfoLayer.hpp>
#include "DPLayer.hpp"
#include "ListManager.hpp"

//geode namespace
using namespace geode::prelude;

//modify gddp level pages
class $modify(DemonProgression, LevelInfoLayer) {
	static void onModify(auto & self) {
		static_cast<void>(self.setHookPriority("LevelInfoLayer::init", -42));
	}

	void skillInfoPopup(CCObject* target) {
		auto btn = static_cast<CCMenuItemSpriteExtra*>(target);
		auto skillID = btn->getID();

		auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("skillset-info");

		FLAlertLayer::create(
			skillsetData[skillID]["display-name"].as_string().c_str(),
			skillsetData[skillID]["description"].as_string().c_str(),
			"OK"
		)->show();
	}

	bool init(GJGameLevel* p0, bool p1) {
		if (!LevelInfoLayer::init(p0, p1)) return false;

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}

		if (inGDDP && (data["level-data"].contains(std::to_string(p0->m_levelID.value())) || Mod::get()->getSettingValue<bool>("all-demons-rated"))) {

			//if not on the GDDP or GDDL, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && p0->m_stars == 10 && ListManager::getSpriteName(p0) == "") {
				if (!data["level-data"].contains(std::to_string(p0->m_levelID.value()))) {
					return true;
				}
			}

			//if not a demon level that's registered on the gddp, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && p0->m_stars != 10) {
				if (!data["level-data"].contains(std::to_string(p0->m_levelID.value()))) {
					return true;
				}
			}

			log::info("{}", Mod::get()->getSavedValue<bool>("in-gddp"));

			if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
				auto bg = typeinfo_cast<CCSprite*>(this->getChildByID("background"));
				bg->setColor({ 18, 18, 86 });
			}

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);
			auto reqLevels = Mod::get()->getSavedValue<int>("current-pack-requirement", 0);
			auto totalLevels = Mod::get()->getSavedValue<int>("current-pack-totalLvls", 0);

			auto hasRank = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(data[type][id]["listID"].as_int())).hasRank;

			auto diffSpr = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("difficulty-sprite"));
			
			auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("skillset-info", matjson::parse("{\"unknown\": {\"display-name\": \"Unknown\",\"description\": \"This skill does not have a description.\",\"sprite\": \"DP_Skill_Unknown\"}}"));

			int gddpDiff = 0;
			matjson::Array skillsets = {};

			if (data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
				gddpDiff = data["level-data"][std::to_string(this->m_level->m_levelID.value())]["difficulty"].as_int();
				skillsets = data["level-data"][std::to_string(this->m_level->m_levelID.value())]["skillsets"].as_array();
			}

			//skillset badges
			if (Mod::get()->getSettingValue<bool>("skillset-badges") && skillsets.size() > 0) {

				//create the skillset menu
				auto skillMenu = CCMenu::create();
				auto skillLayout = AxisLayout::create();
				skillLayout->setAxis(Axis::Column);
				skillMenu->setLayout(skillLayout, true, false);
				skillMenu->setID("skillset-menu"_spr);
				skillMenu->setPosition({ diffSpr->getPositionX() + 14, diffSpr->getPositionY() - 26 });
				skillMenu->setZOrder(42);
				skillMenu->setContentSize({ 31.5f, 65.0f });
				skillMenu->setAnchorPoint({ 1.0f, 0.5f });
				skillMenu->setScale(0.75f);

				//add skillset buttons
				for (int i = 0; i < skillsets.size(); i++) {

					std::string skillID = skillsets[i].as_string();

					//check data entry
					if (!skillsetData.contains(skillID)) {
						skillID = "unknown";
					}

					//get data
					auto name = skillsetData[skillID]["display-name"].as_string();
					auto desc = skillsetData[skillID]["description"].as_string();
					auto spriteName = fmt::format("{}.png", skillsetData[skillID]["sprite"].as_string());

					CCSprite* sprite;
					if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr) {
						spriteName = fmt::format("{}.png", skillsetData["unknown"]["sprite"].as_string());
						sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
					}
					else {
						sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
					}

					auto skillsetBtn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(DemonProgression::skillInfoPopup));
					skillsetBtn->setID(skillID);
					skillMenu->addChild(skillsetBtn);
				}

				skillMenu->updateLayout(false);

				this->addChild(skillMenu);

			}

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {
				diffSpr->setOpacity(0);
			}

			std::string sprite = "DP_Beginner";
			std::string plusSprite = "DP_BeginnerPlus";

			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && !data["level-data"].contains(std::to_string(p0->m_levelID.value()))) {
				sprite = ListManager::getSpriteName(p0);
				plusSprite = fmt::format("{}Plus", sprite);
			}
			else {
				sprite = data["main"][gddpDiff]["sprite"].as_string();
				plusSprite = data["main"][gddpDiff]["plusSprite"].as_string();
			}

			//fallbacks
			if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", sprite)).data()) == nullptr) {
				sprite = "DP_Invisible";
			}

			if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", plusSprite)).data()) == nullptr) {
				plusSprite = "DP_Invisible";
			}

			std::string fullSpr = fmt::format("{}Text.png", sprite);
			std::string fullPlusSpr = fmt::format("{}Text.png", plusSprite);

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && sprite != "DP_Invisible") {
				auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr).data());

				if (p0->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible") {
					typeinfo_cast<CCSprite*>(diffSpr->getChildren()->objectAtIndex(0))->setVisible(false);
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
				}

				if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible") {
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
				}

				customSpr->setID("gddp-difficulty");
				customSpr->setAnchorPoint({ 0.5f, 1 });
				customSpr->setPosition({ diffSpr->getPositionX() + 0.25f, diffSpr->getPositionY() + 30 });
				customSpr->setZOrder(5);

				this->addChild(customSpr);

				if (this->getChildByID("grd-difficulty") && !Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
					customSpr->setVisible(false);
				}
			}

			//GrD Effects
			if (Mod::get()->getSettingValue<bool>("disable-grandpa-demon-effects") && Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {
				if (Loader::get()->isModLoaded("itzkiba.grandpa_demon")) {
					int num = 0;

					for (int i = 0; i < this->getChildrenCount(); i++) {
						if (getChildOfType<CCSprite>(this, i)) {
							if (!(getChildOfType<CCSprite>(this, i)->getID() != "") && (getChildOfType<CCSprite>(this, i)->getTag() != 69420) && (getChildOfType<CCSprite>(this, i)->getContentHeight() >= 750.0f)) {
								num += 1;
								getChildOfType<CCSprite>(this, i)->setID(fmt::format("grd-bg-{}", num));
							}
						}

						if (num == 2) {
							break;
						}
					}

					num = 0;

					for (int i = 0; i < this->getChildrenCount(); i++) {
						if (getChildOfType<CCParticleSystemQuad>(this, i)) {
							if (!(getChildOfType<CCParticleSystemQuad>(this, i)->getID() != "") && (getChildOfType<CCParticleSystemQuad>(this, i)->getPositionY() >= 230)) {
								num += 1;
								getChildOfType<CCParticleSystemQuad>(this, i)->setID(fmt::format("grd-particles-{}", num));
							}
						}

						if (num == 2) {
							break;
						}
					}

					if (this->getChildByID("grd-bg-1")) {
						this->getChildByID("grd-bg-1")->setVisible(false);
						

						typeinfo_cast<CCSprite*>(this->getChildByID("background"))->setOpacity(255);
						typeinfo_cast<CCSprite*>(this->getChildByID("bottom-left-art"))->setOpacity(255);
						typeinfo_cast<CCSprite*>(this->getChildByID("bottom-right-art"))->setOpacity(255);
					}

					if (this->getChildByID("grd-bg-2")) {
						this->getChildByID("grd-bg-2")->setVisible(false);
					}

					if (this->getChildByID("grd-particles-1")) {
						this->getChildByID("grd-particles-1")->setVisible(false);
					}

					if (this->getChildByID("grd-particles-2")) {
						this->getChildByID("grd-particles-2")->setVisible(false);
					}
				}
			}

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && Mod::get()->getSettingValue<bool>("override-grandpa-demon") && (type == "main" || type == "legacy")) {
				if (Loader::get()->isModLoaded("itzkiba.grandpa_demon") && this->getChildByID("grd-difficulty")) {
					this->getChildByID("grd-difficulty")->setVisible(false);
					if (this->getChildByID("grd-infinity")) { this->getChildByID("grd-infinity")->setVisible(false); }

					this->getChildByID("grd-difficulty")->removeMeAndCleanup();
				}
			}

		}

		return true;
	}

	void updateLabelValues() {
		LevelInfoLayer::updateLabelValues();

		if (this->m_level->m_stars != 10) {
			return;
		}

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}

		if (inGDDP && (data["level-data"].contains(std::to_string(this->m_level->m_levelID.value())) || Mod::get()->getSettingValue<bool>("all-demons-rated"))) {
			
			//if not on the GDDP or GDDL, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars == 10 && ListManager::getSpriteName(this->m_level) == "") {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return;
				}
			}

			//if not a demon level that's registered on the gddp, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars != 10) {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return;
				}
			}
			
			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
				if (Loader::get()->isModLoaded("itzkiba.grandpa_demon") && this->getChildByID("grd-difficulty")) {
					this->getChildByID("grd-difficulty")->setVisible(false);
					if (this->getChildByID("grd-infinity")) { this->getChildByID("grd-infinity")->setVisible(false); }

					this->getChildByID("grd-difficulty")->removeMeAndCleanup();
				}
			}
		}
	}
};