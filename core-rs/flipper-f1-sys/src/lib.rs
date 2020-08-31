#![no_std]

#![allow(dead_code)]
#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]

pub mod cmsis_os {
    #[allow(non_camel_case_types)]
    pub use core::ffi::c_void;

    #[allow(non_camel_case_types)]
    pub type c_char = i8;

    include!(concat!(env!("OUT_DIR"), "/cmsis_os_bindings.rs"));
}

pub mod hal {
    include!(concat!(env!("OUT_DIR"), "/stm32_hal_bindings.rs"));
    include!(concat!(env!("OUT_DIR"), "/stm32_hal_statics.rs"));
}