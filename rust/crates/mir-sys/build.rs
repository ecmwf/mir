fn main() {
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=cpp");
    println!("cargo:rerun-if-env-changed=MIR_DIR");
    println!("cargo:rerun-if-env-changed=DOCS_RS");

    if bindman_utils::is_docs_rs() {
        return;
    }

    bindman_utils::validate_build_mode(cfg!(feature = "system"), cfg!(feature = "vendored"));

    if cfg!(feature = "system") {
        build_system();
    } else {
        build_vendored();
    }
}

/// Generate `mir_exceptions.{h,rs}` for mir's own subclasses (`mir::exception::*`
/// in `mir/util/Exceptions.h`) plus eckit's exceptions inherited from upstream
/// `-sys` crates.
fn generate_exceptions(include: &std::path::Path) {
    use std::env;
    use std::path::PathBuf;

    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));

    let own = vec![bindman_build::ExceptionSource {
        header: include.join("mir/util/Exceptions.h"),
        include_path: "mir/util/Exceptions.h".to_string(),
        cpp_namespace: "mir::exception".to_string(),
        message_prefix: "mir".to_string(),
        base_class: "eckit::Exception".to_string(),
        recursive: true,
    }];

    let inherited = bindman_build::collect_dep_exception_sources();

    bindman_build::generate_exception_bridge(&bindman_build::ExceptionBridgeConfig {
        primary_namespace: "mir",
        out_dir: &out_dir,
        own: &own,
        inherited: &inherited,
    });

    bindman_build::publish_exception_sources(&own, &out_dir);
}

#[cfg(feature = "system")]
fn build_system() {
    use std::env;
    use std::path::PathBuf;

    let crate_dir =
        PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set"));
    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));

    let eckit_include = env::var("DEP_ECKIT_SYS_INCLUDE").expect("DEP_ECKIT_SYS_INCLUDE not set");
    let eckit_cpp_dir = env::var("DEP_ECKIT_SYS_CPP_DIR").expect("DEP_ECKIT_SYS_CPP_DIR not set");
    let metkit_include =
        env::var("DEP_METKIT_SYS_INCLUDE").expect("DEP_METKIT_SYS_INCLUDE not set");

    // Minimum supported system version; the crate version tracks the vendored release.
    let (root, mir_include, lib_dir) = bindman_utils::cmake_find_package("mir", "1.28.2");

    generate_exceptions(&mir_include);

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=dylib=mir");

    cxx_build::bridge("src/lib.rs")
        .file(crate_dir.join("cpp/Job.cc"))
        .file(crate_dir.join("cpp/LibMir.cc"))
        .file(crate_dir.join("cpp/MIRInput.cc"))
        .file(crate_dir.join("cpp/MIROutput.cc"))
        .file(crate_dir.join("cpp/Parametrisation.cc"))
        .include(&mir_include)
        .include(&eckit_include)
        .include(&eckit_cpp_dir)
        .include(&metkit_include)
        .include(crate_dir.join("cpp"))
        .include(&out_dir) // for mir_exceptions.h (generated)
        .flag_if_supported("-std=c++17")
        .compile("mir_sys_bridge");

    bindman_utils::link_cpp_stdlib();

    println!("cargo:root={}", root.display());
    println!("cargo:include={}", mir_include.display());

    bindman_build::check_cpp_api(&mir_include, &crate_dir.join("src/lib.rs"));
}

#[cfg(not(feature = "system"))]
fn build_system() {
    unreachable!("build_system called without system feature");
}

/// Locate the mir C++ sources: prefer the in-tree checkout when the crate
/// lives inside the mir repository (path or git dependency), falling back to
/// cloning the release tag (packaged crates.io case).
#[cfg(feature = "vendored")]
fn resolve_mir_src(src_dir: &std::path::Path) -> std::path::PathBuf {
    const MIR_REPO: &str = "https://github.com/ecmwf/mir.git";
    const MIR_TAG: &str = env!("CARGO_PKG_VERSION");

    let manifest_dir = std::path::PathBuf::from(
        std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set"),
    );
    if let Some(root) = manifest_dir.ancestors().nth(3)
        && root.join("CMakeLists.txt").exists()
        && root.join("VERSION").exists()
        && root.join("src/mir").is_dir()
    {
        eprintln!("mir-sys: building in-tree sources at {}", root.display());

        // Retrigger on C++ source edits.
        println!("cargo:rerun-if-changed={}", root.join("src").display());
        println!(
            "cargo:rerun-if-changed={}",
            root.join("CMakeLists.txt").display()
        );
        println!("cargo:rerun-if-changed={}", root.join("VERSION").display());

        // Diverging is fine mid-development, but should never go unnoticed.
        let tree_version = std::fs::read_to_string(root.join("VERSION"))
            .map(|s| s.trim().to_string())
            .unwrap_or_default();
        if tree_version != MIR_TAG {
            println!(
                "cargo:warning=mir-sys {MIR_TAG} is building in-tree mir {tree_version} (versions differ)"
            );
        }

        return root.to_path_buf();
    }
    bindman_utils::git_clone(MIR_REPO, MIR_TAG, &src_dir.join("mir"))
}

#[cfg(feature = "vendored")]
fn build_vendored() {
    use std::env;
    use std::fs;
    use std::path::PathBuf;
    use std::process::Command;

    const ECBUILD_REPO: &str = "https://github.com/ecmwf/ecbuild.git";
    const ECBUILD_TAG: &str = "3.13.1";

    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));
    let src_dir = out_dir.join("src");
    let build_dir = out_dir.join("build");
    let install_dir = out_dir.join("install");

    fs::create_dir_all(&src_dir).expect("Failed to create src directory");
    fs::create_dir_all(&build_dir).expect("Failed to create build directory");

    let eckit_root =
        env::var("DEP_ECKIT_SYS_ROOT").expect("DEP_ECKIT_SYS_ROOT not set - eckit-sys dependency");
    let eccodes_root = env::var("DEP_ECCODES_SYS_ROOT")
        .expect("DEP_ECCODES_SYS_ROOT not set - eccodes-sys dependency");
    let atlas_root =
        env::var("DEP_ATLAS_SYS_ROOT").expect("DEP_ATLAS_SYS_ROOT not set - atlas-sys dependency");
    let metkit_root = env::var("DEP_METKIT_SYS_ROOT")
        .expect("DEP_METKIT_SYS_ROOT not set - metkit-sys dependency");
    let eckit_cpp_dir = env::var("DEP_ECKIT_SYS_CPP_DIR").expect("DEP_ECKIT_SYS_CPP_DIR not set");

    let ecbuild_src = bindman_utils::git_clone(ECBUILD_REPO, ECBUILD_TAG, &src_dir.join("ecbuild"));
    let mir_src = resolve_mir_src(&src_dir);

    // cmake hard-errors if the source path recorded in CMakeCache.txt changes
    // (e.g. cloned <-> in-tree); wipe the build dir when it is stale.
    if let Ok(cache) = fs::read_to_string(build_dir.join("CMakeCache.txt")) {
        let cached_src = cache
            .lines()
            .find_map(|l| l.strip_prefix("CMAKE_HOME_DIRECTORY:INTERNAL="));
        if cached_src != mir_src.to_str() {
            fs::remove_dir_all(&build_dir).expect("Failed to remove stale mir build directory");
            fs::create_dir_all(&build_dir).expect("Failed to create build directory");
        }
    }

    let ecbuild_bin = ecbuild_src.join("bin/ecbuild");
    let num_jobs = bindman_utils::build_parallelism();

    let cmake_prefix_path = format!("{eckit_root};{eccodes_root};{atlas_root};{metkit_root}");

    let mut cmd = Command::new(&ecbuild_bin);
    cmd.current_dir(&build_dir)
        .arg(format!("--prefix={}", install_dir.display()))
        .arg("--")
        .arg(&mir_src)
        .arg(format!("-DCMAKE_PREFIX_PATH={cmake_prefix_path}"))
        .arg(format!(
            "-DCMAKE_BUILD_TYPE={}",
            bindman_utils::cmake_build_type()
        ))
        .arg("-DENABLE_TESTS=OFF")
        .arg("-DBUILD_TESTING=OFF")
        .arg("-DENABLE_DOCS=OFF")
        .arg("-DENABLE_BUILD_TOOLS=OFF")
        .arg("-DENABLE_MIR_TOOLS=OFF");

    #[cfg(target_os = "macos")]
    cmd.arg("-DCMAKE_INSTALL_NAME_DIR=@rpath");

    bindman_utils::run_command(&mut cmd, "ecbuild configure mir");

    bindman_utils::run_command(
        Command::new("cmake")
            .args(["--build", ".", "--parallel", &num_jobs])
            .current_dir(&build_dir),
        "cmake build mir",
    );

    bindman_utils::run_command(
        Command::new("cmake")
            .args(["--install", "."])
            .current_dir(&build_dir),
        "cmake install mir",
    );

    let include_dir = install_dir.join("include");
    let crate_dir =
        PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set"));

    let lib_dir = bindman_utils::resolve_lib_dir(&install_dir);

    generate_exceptions(&include_dir);

    cxx_build::bridge("src/lib.rs")
        .file(crate_dir.join("cpp/Job.cc"))
        .file(crate_dir.join("cpp/LibMir.cc"))
        .file(crate_dir.join("cpp/MIRInput.cc"))
        .file(crate_dir.join("cpp/MIROutput.cc"))
        .file(crate_dir.join("cpp/Parametrisation.cc"))
        .include(&include_dir)
        .include(format!("{eckit_root}/include"))
        .include(&eckit_cpp_dir)
        .include(format!("{metkit_root}/include"))
        .include(crate_dir.join("cpp"))
        .include(&out_dir) // for mir_exceptions.h (generated)
        .flag_if_supported("-std=c++17")
        .compile("mir_sys_bridge");

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=dylib=mir");
    bindman_utils::link_cpp_stdlib();

    println!("cargo:root={}", install_dir.display());
    println!("cargo:include={}", include_dir.display());

    let crate_dir =
        PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set"));
    bindman_build::check_cpp_api(&include_dir, &crate_dir.join("src/lib.rs"));
}

#[cfg(not(feature = "vendored"))]
fn build_vendored() {
    unreachable!("build_vendored called without vendored feature");
}
