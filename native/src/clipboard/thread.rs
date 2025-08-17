use std::{
    sync::{LazyLock, Mutex},
    thread,
};

use anyhow::{anyhow, Error};
use napi::{threadsafe_function::ThreadsafeFunctionCallMode, Status};

use crate::clipboard::types::ClipboardData;

use super::{types::WatcherCallback, watcher};

static THREAD: LazyLock<Mutex<Option<thread::JoinHandle<()>>>> = LazyLock::new(|| Mutex::new(None));
static CALLBACKS: LazyLock<Mutex<Vec<WatcherCallback>>> = LazyLock::new(|| Mutex::new(Vec::new()));

pub fn add_callback(callback: WatcherCallback) -> () {
    let mut cbs = CALLBACKS
        .lock()
        .expect("Failed to lock CALLBACKS mutex");
    cbs.push(callback);
    if cbs.len() == 1 {
        if let Err(e) = start_thread() {
            eprintln!("Failed to start clipboard watcher thread: {e}");
        }
    }
}
fn thread_impl() {
    println!("Clipboard watcher thread started");
    let mut watcher = match watcher::get_clipboard_watcher() {
        Ok(w) => w,
        Err(e) => {
            eprintln!("Failed to create clipboard watcher. The thread will exit. Error: {e}");
            return;
        }
    };
    let mut err_count = 0i8;
    loop {
        println!("Waiting for clipboard change...");
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
        let change_data = match watcher.get_last_change_data() {
            Some(data) => data,
            None => {
                eprintln!("Failed to get last change data from clipboard watcher; using default");
                ClipboardData::default()
            }
        };
        let callbacks = CALLBACKS
            .lock()
            .expect("Failed to lock CALLBACKS mutex");
        for callback in callbacks.iter() {
            match callback.call(
                Ok(change_data.clone()),
                ThreadsafeFunctionCallMode::NonBlocking,
            ) {
                Status::Ok => {}
                error_code => {
                    eprintln!(
                        "Failed to call callback with change data. Error: {:?}",
                        error_code
                    );
                }
            }
        }
    }
}
fn start_thread() -> Result<(), Error> {
    let mut current_thread = THREAD
        .lock()
        .expect("Failed to lock THREAD mutex");
    if current_thread.is_some() {
        return Err(anyhow!("Clipboard watcher thread is already running"));
    }
    current_thread.replace(thread::spawn(thread_impl));

    Ok(())
}
