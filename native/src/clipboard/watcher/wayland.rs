use crate::clipboard::{types::ClipboardData, watcher::ClipboardWatcher};

pub struct Watcher {}

impl Watcher {
    pub fn new() -> Self {
        Self {}
    }
}

impl ClipboardWatcher for Watcher {
    fn sleep_until_next_change(&mut self) -> Result<(), anyhow::Error> {
        todo!()
    }

    fn get_last_change_data(&self) -> Option<ClipboardData> {
        todo!()
    }
}
