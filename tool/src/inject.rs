use std::process::Command;
use std::path::Path;
use std::ffi::CString;
use std::ptr::null_mut;
use std::mem;

use winapi::um::processthreadsapi::{CreateRemoteThread, OpenProcess};
use winapi::um::libloaderapi::{GetProcAddress, GetModuleHandleA};
use winapi::um::memoryapi::{VirtualAllocEx, WriteProcessMemory};
use winapi::um::handleapi::CloseHandle;
use winapi::um::winnt::{PROCESS_ALL_ACCESS, MEM_RESERVE, MEM_COMMIT, PAGE_READWRITE};

// http://resources.infosecinstitute.com/using-createremotethread-for-dll-injection-on-windows/
pub fn inject() {
    unsafe {
        let rtil = ::std::env::var("RTIL").unwrap_or("./rtil.dll".to_string());
        let mut path = Path::new(&rtil);
        if !path.is_file() {
            path = Path::new("../rtil/target/i686-pc-windows-msvc/release/rtil.dll");
        }
        let absolute = path.canonicalize().unwrap();
        let s = absolute.to_str().unwrap();
        println!("Trying to inject {}", s);
        let lib = CString::new(s).unwrap();
        let pid = pidof();
        let handle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
        if handle.is_null() {
            panic!("Could not open process");
        }
        let kernel = CString::new("kernel32.dll").unwrap();
        let load = CString::new("LoadLibraryA").unwrap();
        let addr = GetProcAddress(GetModuleHandleA(kernel.as_ptr()), load.as_ptr());
        if addr.is_null() {
            panic!("Cannot find LoadLibraryA in kernel32.dll");
        }
        println!("Found LoadLibraryA at {:?}", addr);
        let arg = VirtualAllocEx(handle, null_mut(), lib.to_bytes_with_nul().len(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if arg.is_null() {
            panic!("Cannot allocate memory in Refunct");
        }
        println!("Alloc'ed at {:?}", arg);
        let n = WriteProcessMemory(handle, arg, lib.as_ptr() as *const _, lib.to_bytes_with_nul().len(), null_mut());
        if n == 0 {
            panic!("Cannot write to Refunct");
        }
        let thread_id = CreateRemoteThread(handle, null_mut(), 0, Some(mem::transmute(addr)), arg, 0, null_mut());
        if thread_id.is_null() {
            panic!("Could not start remote thread");
        }
        println!("thread_id: {:?}", thread_id);
        CloseHandle(handle);
    }
}

fn pidof() -> u32 {
    let output = Command::new("powershell")
        .args(&[
            "-NoProfile",
            "-Command",
            "Get-Process -Name 'Refunct-Win32-Shipping' -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id"
        ])
        .output()
        .expect("Cannot get pid of Refunct");

    let s = String::from_utf8(output.stdout)
        .expect("Output of pidof is not utf8");

    // this constant needs to be rechecked and possibly updated if there is a new refunct build
    const KNOWN_SIZE: u64 = 34400;

    let candidates = s.trim()
        .split("\n")
        .map(|id| id.trim().parse::<u32>())
        .filter(|res| res.is_ok())
        .map(|res| res.unwrap())
        .filter(|id| {
            // use powershell magic to determine the memory size of the binary from which the process started
            // while not impossible this should be relatively unlikely to accidentally match anything else sharing the process name
            println!("checking if pid {} could be refunct", id);
            let res = Command::new("powershell")
                .args(&[
                    "-NoProfile",
                    "-Command",
                    format!(
                        "Get-Process -Id {} -Module | where-object {{ $_.ModuleName -eq 'Refunct-Win32-Shipping.exe' }} | Select-Object -ExpandProperty Size",
                        id
                    ).as_str()
                ])
                .output();
            match res {
                Err(_) => false,
                Ok(res_output) => {
                    match String::from_utf8(res_output.stdout) {
                        Err(_) => false,
                        Ok(size_str) => {
                            match size_str.trim().parse::<u64>() {
                                Err(_) => false,
                                Ok(size) => size == KNOWN_SIZE
                            }
                        }
                    }
                }
            }
        })
        .collect::<Vec<u32>>();
    if candidates.len() > 1 || candidates.is_empty() {
        println!("Found {} processes that match", candidates.len());
        panic!("Could not find or parse pid of Refunct");
    }
    candidates.first().unwrap().to_owned()
}
