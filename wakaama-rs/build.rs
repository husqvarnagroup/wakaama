fn main() {
    let wakaama_c_path = "../";

    build_wakaama(wakaama_c_path);
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
