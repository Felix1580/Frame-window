#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

static bool modEnabled() {
	return Mod::get()->getSettingValue<bool>("enabled");
}

static int intSetting(char const* key) {
	return static_cast<int>(Mod::get()->getSettingValue<int64_t>(key));
}

class $modify(FrameWindowLayer, GJBaseGameLayer) {
	struct Fields {
		int clickLeft[2] = {0, 0};     // frames left to keep re-sending the click
		int holdFrames[2] = {0, 0};    // how long the current hold has lasted
		bool held[2] = {false, false}; // is the button logically down
		bool pendingRelease[2] = {false, false};
		bool resending = false;
	};

	void handleButton(bool down, int button, bool isPlayer1) {
		auto f = m_fields.self();

		if (f->resending || button != 1 || !modEnabled()) {
			return GJBaseGameLayer::handleButton(down, button, isPlayer1);
		}

		int i = isPlayer1 ? 0 : 1;

		if (down) {
			// New press while a release was still being held back:
			// let the old hold end first, then start the new one.
			if (f->pendingRelease[i]) {
				f->pendingRelease[i] = false;
				GJBaseGameLayer::handleButton(false, button, isPlayer1);
			}
			f->held[i] = true;
			f->holdFrames[i] = 0;
			f->clickLeft[i] = intSetting("click-window") - 1;
			GJBaseGameLayer::handleButton(true, button, isPlayer1);
		} else {
			f->clickLeft[i] = 0;
			if (f->held[i] && f->holdFrames[i] < intSetting("min-hold")) {
				// Released too early: hold it back until the minimum is reached.
				f->pendingRelease[i] = true;
			} else {
				f->held[i] = false;
				GJBaseGameLayer::handleButton(false, button, isPlayer1);
			}
		}
	}

	void processCommands(float dt, bool isHalfTick, bool isLastTick) {
		if (modEnabled() && isLastTick) {
			auto f = m_fields.self();
			f->resending = true;
			for (int i = 0; i < 2; i++) {
				bool p1 = (i == 0);
				if (f->held[i]) {
					f->holdFrames[i]++;

					// Click buffering: keep re-sending the press.
					if (!f->pendingRelease[i] && f->clickLeft[i] > 0) {
						f->clickLeft[i]--;
						GJBaseGameLayer::handleButton(true, 1, p1);
					}

					// Delayed release once the minimum hold is reached.
					if (f->pendingRelease[i] && f->holdFrames[i] >= intSetting("min-hold")) {
						f->pendingRelease[i] = false;
						f->held[i] = false;
						GJBaseGameLayer::handleButton(false, 1, p1);
					}
				}
			}
			f->resending = false;
		}
		GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
	}
};
