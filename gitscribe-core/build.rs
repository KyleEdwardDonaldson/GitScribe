use std::env;
use std::path::PathBuf;

fn main() {
    // Tell Cargo to link against Windows system libraries needed by libgit2
    #[cfg(target_os = "windows")]
    {
        println!("cargo:rustc-link-lib=advapi32");
        println!("cargo:rustc-link-lib=crypt32");
        println!("cargo:rustc-link-lib=secur32");
    }

    // Generate C header file for FFI
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let config = cbindgen::Config::from_file("cbindgen.toml")
        .unwrap_or_else(|_| cbindgen::Config::default());

    cbindgen::Builder::new()
        .with_crate(&crate_dir)
        .with_config(config)
        .generate()
        .expect("Unable to generate C bindings")
        .write_to_file(PathBuf::from(&crate_dir).join("include").join("gitscribe_core.h"));

    println!("cargo:rerun-if-changed=src/ffi.rs");
}
