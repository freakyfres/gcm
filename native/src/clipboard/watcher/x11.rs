use std::{fs, rc::Rc};

use anyhow::Result;
use x11rb::{
    connection::{Connection, RequestConnection},
    protocol::{
        xfixes::{SelectSelectionInputRequest, SelectionEventMask, SelectionNotifyEvent},
        xproto::{Atom, AtomEnum, ConnectionExt, Window},
        Event,
    },
    rust_connection::RustConnection,
};

use crate::clipboard::{
    types::{AppInfo, ClipboardData, IconData},
    watcher::ClipboardWatcher,
};

#[derive(Debug)]
pub struct Watcher {
    conn: RustConnection,
    atoms: Atoms,
    last_change_data: Option<ClipboardData>,
}

#[derive(Debug)]
#[allow(non_snake_case)]
struct Atoms {
    CLIPBOARD: Atom,
    _NET_ACTIVE_WINDOW: Atom,
    XA_WINDOW: Atom,
    XA_CARDINAL: Atom,
    XA_STRING: Atom,
    WM_CLASS: Atom,
    UTF8_STRING: Atom,
    _NET_WM_NAME: Atom,
}

impl Atoms {
    pub fn try_new(conn: &RustConnection) -> Result<Self> {
        let i = |name: &str| -> Result<u32> {
            Ok(conn
                .intern_atom(false, name.as_bytes())?
                .reply()?
                .atom)
        };
        let x = |atom: AtomEnum| -> Atom { atom.into() };
        Ok(Self {
            CLIPBOARD: i("CLIPBOARD")?,
            UTF8_STRING: i("UTF8_STRING")?,
            _NET_WM_NAME: i("_NET_WM_NAME")?,
            _NET_ACTIVE_WINDOW: i("_NET_ACTIVE_WINDOW")?,
            WM_CLASS: i("WM_CLASS")?,
            XA_WINDOW: x(AtomEnum::WINDOW),
            XA_CARDINAL: x(AtomEnum::CARDINAL),
            XA_STRING: x(AtomEnum::STRING),
        })
    }
}

struct WindowContext<'a> {
    conn: &'a RustConnection,
    atoms: Atoms,
    screen: usize,
    window: Window,
}

impl<'a> WindowContext<'a> {
    #[inline]
    fn default_root_window(conn: &RustConnection, screen_num: Option<usize>) -> Window {
        conn.setup().roots[screen_num.unwrap_or(0)].root
    }
    fn get_active_window(conn: &RustConnection) -> Result<Option<Window>> {
        let atoms = Atoms::try_new(conn)?;
        let root = Self::default_root_window(conn, None);
        Ok(conn
            .get_property(false, root, atoms._NET_ACTIVE_WINDOW, atoms.XA_WINDOW, 0, 1)?
            .reply()?
            .value32()
            .and_then(|mut i| i.next()))
    }

    fn try_new(conn: &'a RustConnection, window: Window) -> Result<Self> {
        Ok(Self {
            conn,
            atoms: Atoms::try_new(conn)?,
            window,
            screen: 0,
        })
    }

    fn set_screen(&mut self, screen: usize) {
        self.screen = screen;
    }

    fn get_window_pid(&self) -> Result<Option<u32>> {
        todo!()
    }
    fn get_window_name(&self) -> Result<String> {
        Ok(self
            .conn
            .get_property(
                false,
                self.window,
                self.atoms._NET_WM_NAME,
                self.atoms.UTF8_STRING,
                0,
                u32::MAX,
            )?
            .reply()?
            .value
            .try_into()?)
    }
    fn get_window_icon(&self) -> Result<IconData> {
        todo!()
    }
    fn get_window_class(&self) -> Result<(String, String)> {
        let binding = self
            .conn
            .get_property(
                false,
                self.window,
                self.atoms.WM_CLASS,
                self.atoms.XA_STRING,
                0,
                u32::MAX,
            )?
            .reply()?;
        let mut iter = binding.value.iter();
        let first: String = iter
            .by_ref()
            .take_while(|&&c| c != 0)
            .map(|c| *c)
            .collect::<Vec<_>>()
            .try_into()?;
        let second: String = iter
            .copied()
            .collect::<Vec<_>>()
            .try_into()?;
        Ok((first, second))
    }
    fn get_window_exe_path(&self, pid: u32) -> Result<Option<String>> {
        let exe_path = fs::read_link(format!("/proc/{pid}/exe"))?;
    }
    fn get_window_info(&self) -> Result<AppInfo> {
        todo!()
    }
}

impl Watcher {
    pub fn try_new() -> Result<Self> {
        let (conn, screen) = x11rb::connect(None)?;
        let atoms = Atoms::try_new(&conn)?;
        conn.send_trait_request_without_reply(SelectSelectionInputRequest {
            window: WindowContext::default_root_window(&conn, Some(screen)),
            selection: atoms.CLIPBOARD,
            event_mask: SelectionEventMask::SET_SELECTION_OWNER,
        })?;
        Ok(Self {
            conn,
            atoms,
            last_change_data: None,
        })
    }
    fn get_app_info(&mut self, event: SelectionNotifyEvent) -> Result<Option<AppInfo>> {
        let active_window: Window = match WindowContext::get_active_window(&self.conn)? {
            Some(w) => w,
            None => return Ok(None),
        };
        let w_ctx = WindowContext::try_new(&self.conn, active_window)?;
        Ok(None)
    }
    fn write_last_change_data(&mut self, event: SelectionNotifyEvent) -> Result<()> {
        self.last_change_data = Some(ClipboardData {
            change_reason: event.subtype.into(),
            change_timestamp: event.selection_timestamp,
            app_info: self.get_app_info(event)?,
        });
        Ok(())
    }
}

impl ClipboardWatcher for Watcher {
    fn sleep_until_next_change(&mut self) -> Result<()> {
        let ev: SelectionNotifyEvent;
        loop {
            if let Event::XfixesSelectionNotify(raw_ev) = self.conn.wait_for_event()? {
                ev = raw_ev;
                break;
            }
        }
        self.write_last_change_data(ev)?;
        Ok(())
    }

    fn get_last_change_data(&self) -> crate::clipboard::types::ClipboardData {
        todo!()
    }
}
