#![no_std]

#[cfg(target_arch = "arm")]
use flipper_f1_sys::hal::{HAL_UART_Transmit_IT, huart1};

#[no_mangle]
pub extern "C" fn add(a: u32, b: u32) -> u32 {
    a + b
}


#[no_mangle]
pub extern "C" fn rust_uart_write() {
    let string = "Rust test string\n";
    let bytes = string.as_bytes();

    #[cfg(target_arch = "arm")]
    unsafe {
        HAL_UART_Transmit_IT(&mut huart1, bytes.as_ptr() as *mut _, bytes.len() as u16);
    }
    #[cfg(not(target_arch = "arm"))]
    unsafe {
        extern "C" {
            fn write(handle: i32, ptr: *const u8, size: usize) -> isize;
        }

        write(1, bytes.as_ptr(), bytes.len());
    }
}


mod aux {
    use core::panic::PanicInfo;

    #[panic_handler]
    fn panic(_info: &PanicInfo) -> ! {
        loop { continue }
    }
}
