#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(FrameWindowLayer, GJBaseGameLayer) {
	struct Fields {
		int clickLeft[2] = {0, 0};
		int releaseLeft[2] = {0, 0};
		bool held[2] = {false, false};
		bool resending = false;
	};

	static bool enabled() {
		return Mod::get()->getSettingValue<bool>("enabled");
	}

	void handleButton(bool down, int button, bool isPlayer1) {
		auto f = m_fields.self();

		if (f->resending || button != 1 || !enabled()) {
			return GJBaseGameLayer::handleButton(down, button, isPlayer1);
		}

		int i = isPlayer1 ? 0 : 1;
		if (down) {
			f->held[i] = true;
			f->releaseLeft[i] = 0;
			f->clickLeft[i] = static_cast<int>(Mod::get()->getSettingValue<int64_t>("click-window")) - 1;
		} else {
			f->held[i] = false;
			f->clickLeft[i] = 0;
			f->releaseLeft[i] = static_cast<int>(Mod::get()->getSettingValue<int64_t>("release-window")) - 1;
		}

		GJBaseGameLayer::handleButton(down, button, isPlayer1);
	}

	void processCommands(float dt) {
		if (enabled()) {
			auto f = m_fields.self();
			f->resending = true;
			for (int i = 0; i < 2; i++) {
				bool p1 = (i == 0);
				if (f->held[i] && f->clickLeft[i] > 0) {
					f->clickLeft[i]--;
					GJBaseGameLayer::handleButton(true, 1, p1);
				}
				if (!f->held[i] && f->releaseLeft[i] > 0) {
					f->releaseLeft[i]--;
					GJBaseGameLayer::handleButton(false, 1, p1);
				}
			}
			f->resending = false;
		}
		GJBaseGameLayer::processCommands(dt);
	}
};
