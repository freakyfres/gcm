#pragma once
#include "Atoms.hpp"
#include "IHasJsObj.hpp"
#include "napi.h"

#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <X11/extensions/Xfixes.h>
#include <X11/X.h>
#include <X11/Xlib.h>

namespace gcm {
	struct WindowInfo;

	class WindowContext {
	  public:
		WindowContext(std::shared_ptr<Display> dpy, Window window);
		~WindowContext() noexcept = default;
		WindowInfo getWindowInfo() const;
		std::string getWindowName() const;
		std::vector<uint8_t> getWindowIcon() const;
		std::vector<std::string> getWindowClass() const;
		std::optional<pid_t> getWindowPid() const;
		std::optional<std::string> getWindowExePath(pid_t pid) const;
		static std::expected<Window, std::string>
		getActiveWindow(std::shared_ptr<Display> display);

	  private:
		std::shared_ptr<Display> display;
		Window window;
		Atoms atoms {display};
	};

	struct WindowInfo: public IHasJsObj {
		std::optional<pid_t> pid;
		std::vector<uint8_t> iconData;
		std::vector<std::string> windowClass;
		std::string windowTitle;
		std::optional<std::string> exePath;
		void debug() const;
		virtual Napi::Value toJsObject(Napi::Env env) const override;
		static WindowInfo getWindowInfo(Display* dpy, Window window);
	};

	std::shared_ptr<Display> openDisplay() noexcept;

	namespace clipboard {

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

		std::string toString(ChangeReason reason) noexcept;

		struct ClipboardState: public IHasJsObj {
			ChangeReason changeReason;
			std::optional<WindowInfo> appInfo;
			uint32_t changeTimestamp;
			virtual Napi::Value toJsObject(Napi::Env env) const override;
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
			 * @return false the latest clipboard state was successfully
			 * retrieved and written to state
			 */
			void getLatestClipboardState(std::unique_ptr<ClipboardState>& state) const;

		  private:
			Atoms atoms;
			void writeLatestState(XFixesSelectionNotifyEvent* event);
			std::shared_ptr<Display> display;
			int xfixesEventBase;
			std::unique_ptr<ClipboardState> lastState {nullptr};
		};

	} // namespace clipboard
} // namespace gcm