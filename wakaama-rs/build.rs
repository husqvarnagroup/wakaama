use std::path::PathBuf;

fn main() {
    let wakaama_c_path = "../";

    build_wakaama(wakaama_c_path);
    generate_wrapper(wakaama_c_path);
}

fn build_wakaama(wakaama_c_path: &str) {
    let dst = cmake::Config::new(wakaama_c_path)
        .build_target("wakaama_static")
        .build();
    println!(
        "cargo:rustc-link-search=native={}",
        dst.join("build").display()
    );
    println!("cargo:rustc-link-lib=static=wakaama_static");
}

fn generate_wrapper(wakaama_c_path: &str) {
    let c_header_path = PathBuf::from(wakaama_c_path).join("include/liblwm2m.h");
    println!("cargo:rerun-if-changed={}", c_header_path.display());

    let bindings = bindgen::Builder::default()
        .header(c_header_path.to_str().unwrap())
        .generate()
        .expect("Unable to generate bindings");

    let out_path = std::path::PathBuf::from(std::env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("wakaama_bindings.rs"))
        .expect("Couldn't write bindings!");
}
