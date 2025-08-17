use napi_derive::napi;

mod thread;
mod types;
mod watcher;


use crate::clipboard::types::WatcherCallback;


#[napi]
#[allow(dead_code)]
fn register_clipboard_watcher_callback(callback: WatcherCallback) {
    thread::add_callback(callback);
}
