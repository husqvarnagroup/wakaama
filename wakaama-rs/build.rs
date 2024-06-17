
// https://rust-lang.github.io/rust-bindgen/tutorial-3.html

fn main() {
    println!("cargo:rerun-if-changed=../include/liblwm2m.h");

    let bindings = bindgen::Builder::default()
        .header("../include/liblwm2m.h")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = std::path::PathBuf::from(std::env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("wakaama_bindings.rs"))
        .expect("Couldn't write bindings!");
}
