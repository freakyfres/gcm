#include "Atoms.hpp"

#include <X11/X.h>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <X11/extensions/Xfixes.h>
#include <X11/Xlib.h>

namespace gcm::clipboard {
	struct WindowInfo {
		std::optional<pid_t> pid;
		std::vector<char> iconData;
		std::vector<std::string> windowClass;
		std::string windowTitle;
		std::optional<std::string> exePath;
		void debug() const;
	};

	/**
	 * @brief
	 *
	 * NEW_OWNER: some other app claimed the ownership
	 * DESTROY: the window was destroyed
	 * CLOSE: the client was closed
	 */
	enum ChangeReason {
		NEW_OWNER,
		DESTROY,
		CLOSE,
		LAST
	};

	struct ClipboardState {
		ChangeReason changeReason;
		std::optional<WindowInfo> appInfo;
		uint32_t changeTimestamp;
	};

	class Watcher {
	  public:
		Watcher();
		~Watcher() noexcept;

		Watcher(const Watcher&) = delete;
		Watcher& operator=(const Watcher&) = delete;
		Watcher(Watcher&&) = delete;
		Watcher& operator=(Watcher&&) = delete;

		void sleepUntilNextChange();

		/**
		 * @brief Get the Latest ClipboardState object
		 *
		 * @param state output parameter to hold the latest clipboard state
		 * @return true failed to get the latest clipboard state
		 * @return false the latest clipboard state was successfully retrieved
		 * and written to state
		 */
		bool getLatestClipboardState(ClipboardState& state) const;

	  private:
		Atoms atoms;
		void writeLatestState(XFixesSelectionNotifyEvent* event);
		WindowInfo getWindowInfo(Window window) const;
		std::expected<Window, std::string> getActiveWindow() const;
		Display* display;
		int xfixesEventBase;
		std::unique_ptr<ClipboardState> lastState {nullptr};
	};
} // namespace gcm::clipboard
