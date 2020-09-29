use std::env;
use std::path::Path;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let pkg_name = env::var("CARGO_PKG_NAME").unwrap();

    cbindgen::generate(&crate_dir)
        .expect("Unable to generate cbindgen bindings")
        .write_to_file(
            Path::new(&crate_dir)
                .join("bindings")
                .join(format!("{}.h", pkg_name)),
        );
}
