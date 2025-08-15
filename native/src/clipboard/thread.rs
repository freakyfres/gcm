use std::{
    sync::{LazyLock, Mutex},
    thread,
};

use anyhow::{anyhow, Error};
use napi::{threadsafe_function::ThreadsafeFunctionCallMode, Status};

use crate::clipboard::{types::WatcherCallback, watcher};

pub fn add_callback(callback: WatcherCallback) -> () {}
static THREAD: LazyLock<Mutex<Option<thread::JoinHandle<()>>>> = LazyLock::new(|| Mutex::new(None));
static CALLBACKS: LazyLock<Mutex<Vec<WatcherCallback>>> = LazyLock::new(|| Mutex::new(Vec::new()));
fn thread_impl() {
    let mut watcher = match watcher::get_clipboard_watcher() {
        Ok(w) => w,
        Err(e) => {
            eprintln!("Failed to create clipboard watcher. The thread will exit. Error: {e}");
            return;
        }
    };
    let mut err_count = 0i8;
    loop {
        match watcher.sleep_until_next_change() {
            Err(e) => {
                eprintln!("Error while waiting for clipboard change.");
                err_count += 1;
                if err_count > 3 {
                    eprintln!("Too many consecutive errors. The clipboard watcher thread will exit. Error: {e}");
                    return;
                }
                continue;
            }
            _ => {}
        }
        let change_data = watcher.get_last_change_data();
        let callbacks = CALLBACKS.lock().expect("Failed to lock CALLBACKS mutex");
        for callback in callbacks.iter() {
            match callback.call(
                Ok(change_data.clone()),
                ThreadsafeFunctionCallMode::NonBlocking,
            ) {
                Status::Ok => {}
                error_code => {
                    eprintln!("Failed to call callback with change data. Error: {:?}", error_code);
                }
            }
        }
    }
}
pub fn start_thread() -> Result<(), Error> {
    let mut current_thread = THREAD.lock().expect("Failed to lock THREAD mutex");
    if current_thread.is_some() {
        return Err(anyhow!("Clipboard watcher thread is already running"));
    }
    current_thread.replace(thread::spawn(thread_impl));

    Ok(())
}
