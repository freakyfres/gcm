use napi::threadsafe_function::ThreadsafeFunctionCallMode;
use napi_derive::napi;

mod thread;
mod types;
mod watcher;

use types::{ChangeReason, ClipboardData};

use crate::clipboard::types::WatcherCallback;


#[napi]
#[allow(dead_code)]
fn register_clipboard_watcher_callback(callback: WatcherCallback) {
    callback.call(
        Ok(ClipboardData {
            app_info: None,
            change_reason: ChangeReason::NewOwner,
            change_timestamp: 0,
        }),
        ThreadsafeFunctionCallMode::NonBlocking,
    );
}
