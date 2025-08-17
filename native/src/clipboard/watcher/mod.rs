#[cfg(target_os = "linux")]
use std::env;

use crate::clipboard::types;
mod x11;
mod wayland;
pub trait ClipboardWatcher {
    fn sleep_until_next_change(&mut self) -> Result<(), anyhow::Error>;
    fn get_last_change_data(&self) -> Option<types::ClipboardData>;
}

pub fn get_clipboard_watcher() -> Result<Box<dyn ClipboardWatcher>, anyhow::Error> {
    let wayland_display = env::var("WAYLAND_DISPLAY").is_ok();
    #[cfg(target_os = "linux")]
    if env::var("DISPLAY").is_ok() && !wayland_display {
        return Ok(Box::new(x11::Watcher::try_new()?));
    } else if wayland_display {
        return Ok(Box::new(wayland::Watcher::new()));
    }
    panic!("Unsupported platform for clipboard watcher. Only X11 and Wayland are supported on Linux.");
}
