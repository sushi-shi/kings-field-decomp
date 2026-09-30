#![no_std]
#![deny(unsafe_attr_outside_unsafe)]

#[forbid(unsafe_code)]
mod bytes;

#[forbid(unsafe_code)]
pub mod audio;
mod ffi;
#[forbid(unsafe_code)]
mod resources;
#[forbid(unsafe_code)]
pub mod tim;
