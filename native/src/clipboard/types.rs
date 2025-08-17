use napi::threadsafe_function::{ThreadsafeFunction, UnknownReturnValue};
use napi_derive::napi;
use x11rb::protocol::xfixes::SelectionEvent;

#[napi(string_enum)]
#[derive(Clone, Debug)]
pub enum ChangeReason {
    NewOwner,
    Destroy,
    Close,
}

impl From<SelectionEvent> for ChangeReason {
    fn from(reason: SelectionEvent) -> Self {
        match reason {
            SelectionEvent::SELECTION_CLIENT_CLOSE => Self::Close,
            SelectionEvent::SELECTION_WINDOW_DESTROY => Self::Destroy,
            SelectionEvent::SET_SELECTION_OWNER => Self::NewOwner,
            _ => panic!("Unknown selection event: {:?}", reason)
        }
    }
}

#[napi(object)]
#[derive(Clone, Debug, Default)]
pub struct IconData {
    pub width: u32,
    pub height: u32,
    pub data: Vec<u8>,
}

#[napi(object)]
#[derive(Clone, Debug, Default)]
pub struct AppInfo {
    pub pid: Option<i32>,
    pub icon_data: IconData,
    pub window_class: (String, String),
    pub window_title: String,
    pub exe_path: Option<String>,
}

#[napi(object)]
#[derive(Clone, Debug)]
pub struct ClipboardData {
    pub change_reason: ChangeReason,
    pub change_timestamp: u32,
    pub app_info: Option<AppInfo>,
}
impl Default for ClipboardData {
    fn default() -> Self {
        Self {
            change_reason: ChangeReason::NewOwner,
            change_timestamp: 0,
            app_info: Some(AppInfo::default()),
        }
    }
}

pub type WatcherCallback = ThreadsafeFunction<ClipboardData, UnknownReturnValue>;