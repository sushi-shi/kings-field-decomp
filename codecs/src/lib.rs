#![no_std]
#![deny(dead_code, unsafe_attr_outside_unsafe)]

#[forbid(unsafe_code)]
mod bytes;
#[forbid(unsafe_code)]
mod cast;

#[forbid(unsafe_code)]
mod audio;
mod ffi;
#[forbid(unsafe_code)]
mod formats;
#[forbid(unsafe_code)]
mod resources;
#[forbid(unsafe_code)]
mod tim;
