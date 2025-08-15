#![deny(clippy::all)]

use napi_derive::napi;
mod clipboard;

#[napi]
pub fn plus_100(input: u32) -> u32 {
  input + 102
}