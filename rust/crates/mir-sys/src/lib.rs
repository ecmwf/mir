//! FFI bindings to ECMWF mir (Meteorological Interpolation and Regridding) library.
//!
//! mir pulls fields from a [`MIRInputWrapper`], transforms them according to a
//! [`Job`], and pushes them to a [`MIROutputWrapper`]. The job is not an action
//! but a description — a bag of key/value settings that mir compiles into an
//! action plan on each execute — so one job applies to any number of
//! input/output pairings.

use bindman::track_cpp_api;

// Auto-generated mir Error enum + From<cxx::Exception> impl
include!(concat!(env!("OUT_DIR"), "/mir_exceptions.rs"));

#[track_cpp_api(("mir/api/MIRJob.h", class = "MIRJob"),)]
#[cxx::bridge(namespace = "mir_bridge")]
pub mod ffi {
    unsafe extern "C++" {
        include!("MirBridge.h");

        // Cross-crate ExternType from eckit-sys
        #[namespace = "eckit_bridge"]
        type DataHandleWrapper = eckit_sys::DataHandleWrapper;

        // ==================== Library ====================

        type LibMir;

        #[Self = "LibMir"]
        fn version() -> Result<String>;
        #[Self = "LibMir"]
        fn git_sha1() -> Result<String>;
        #[Self = "LibMir"]
        fn home_dir() -> Result<String>;
        #[Self = "LibMir"]
        fn cache_dir() -> Result<String>;
        #[Self = "LibMir"]
        fn caching() -> Result<bool>;

        // ==================== Parametrisation ====================

        type Parametrisation;

        fn set_str(self: Pin<&mut Parametrisation>, name: &str, value: &str) -> Result<()>;
        fn set_f64(self: Pin<&mut Parametrisation>, name: &str, value: f64) -> Result<()>;
        fn set_i64(self: Pin<&mut Parametrisation>, name: &str, value: i64) -> Result<()>;
        fn set_bool(self: Pin<&mut Parametrisation>, name: &str, value: bool) -> Result<()>;
        fn set_f64_list(self: Pin<&mut Parametrisation>, name: &str, values: &[f64]) -> Result<()>;
        fn set_i64_list(self: Pin<&mut Parametrisation>, name: &str, values: &[i64]) -> Result<()>;

        fn to_json(self: &Parametrisation) -> Result<String>;

        #[Self = "Parametrisation"]
        #[must_use]
        fn create() -> UniquePtr<Parametrisation>;

        // ==================== MIRInput ====================

        type MIRInputWrapper;

        /// Advance to the next message; false once the stream is exhausted.
        fn next(self: Pin<&mut MIRInputWrapper>) -> Result<bool>;

        /// Fields carried per message — 1 for a scalar, 2 for a vector pair.
        fn dimensions(self: &MIRInputWrapper) -> Result<usize>;

        #[Self = "MIRInputWrapper"]
        fn from_data_handle(
            handle: Pin<&mut DataHandleWrapper>,
        ) -> Result<UniquePtr<MIRInputWrapper>>;

        #[Self = "MIRInputWrapper"]
        fn from_grib_file(path: &str) -> Result<UniquePtr<MIRInputWrapper>>;

        #[Self = "MIRInputWrapper"]
        fn from_grib_memory(message: &[u8]) -> Result<UniquePtr<MIRInputWrapper>>;

        /// Consecutive messages read as one N-dimensional field — how mir pairs
        /// components for the `vod2uv` and `uv2uv` job keys.
        #[Self = "MIRInputWrapper"]
        fn from_multi_dimensional_grib_file(
            path: &str,
            dimensions: usize,
            skip: usize,
        ) -> Result<UniquePtr<MIRInputWrapper>>;

        #[Self = "MIRInputWrapper"]
        fn from_gridspec(gridspec: &str, gridded: bool) -> Result<UniquePtr<MIRInputWrapper>>;

        #[Self = "MIRInputWrapper"]
        fn from_raw(
            values: &[f64],
            metadata: &Parametrisation,
        ) -> Result<UniquePtr<MIRInputWrapper>>;

        // ==================== MIROutput ====================

        type MIROutputWrapper;

        /// Interpolated values, for an output built by `to_resizable`.
        fn values(self: &MIROutputWrapper) -> Result<&[f64]>;

        /// The grid the field was interpolated onto, for an output built by
        /// `to_resizable`.
        fn metadata_json(self: &MIROutputWrapper) -> Result<String>;

        /// The encoded message, for an output built by `to_grib_memory`.
        fn message(self: &MIROutputWrapper) -> Result<&[u8]>;

        #[Self = "MIROutputWrapper"]
        fn to_callback(output: Box<OutputBox>) -> Result<UniquePtr<MIROutputWrapper>>;

        #[Self = "MIROutputWrapper"]
        fn to_grib_file(path: &str, append: bool) -> Result<UniquePtr<MIROutputWrapper>>;

        #[Self = "MIROutputWrapper"]
        fn to_grib_memory(capacity: usize) -> Result<UniquePtr<MIROutputWrapper>>;

        #[Self = "MIROutputWrapper"]
        fn to_resizable() -> Result<UniquePtr<MIROutputWrapper>>;

        #[Self = "MIROutputWrapper"]
        fn to_empty() -> Result<UniquePtr<MIROutputWrapper>>;

        // ==================== Job ====================

        type Job;

        fn set_str(self: Pin<&mut Job>, name: &str, value: &str) -> Result<()>;
        fn set_f64(self: Pin<&mut Job>, name: &str, value: f64) -> Result<()>;
        fn set_i64(self: Pin<&mut Job>, name: &str, value: i64) -> Result<()>;
        fn set_bool(self: Pin<&mut Job>, name: &str, value: bool) -> Result<()>;
        fn set_f64_list(self: Pin<&mut Job>, name: &str, values: &[f64]) -> Result<()>;
        fn set_i64_list(self: Pin<&mut Job>, name: &str, values: &[i64]) -> Result<()>;
        fn set_str_list(self: Pin<&mut Job>, name: &str, values: &Vec<String>) -> Result<()>;

        /// Parse `name=value` pairs the way the mir tool does; a bare `name`
        /// sets it to true.
        fn set_from_string(self: Pin<&mut Job>, args: &str) -> Result<()>;

        fn clear_key(self: Pin<&mut Job>, name: &str) -> Result<()>;

        /// Take the output grid from an input rather than stating it.
        fn representation_from(self: Pin<&mut Job>, input: &MIRInputWrapper) -> Result<()>;

        #[cxx_name = "to_json"]
        fn json_str(self: &Job) -> Result<String>;

        /// The equivalent mir-tool command line, for debugging.
        fn mir_tool_call(self: &Job) -> Result<String>;

        /// Transform the message the input is currently positioned on. Callers
        /// drive iteration themselves; see `execute_all`.
        fn execute_one(
            self: &Job,
            input: Pin<&mut MIRInputWrapper>,
            output: Pin<&mut MIROutputWrapper>,
        ) -> Result<()>;

        /// Drain the input, transforming every message, and return how many
        /// were processed. Single-field inputs yield one.
        fn execute_all(
            self: &Job,
            input: Pin<&mut MIRInputWrapper>,
            output: Pin<&mut MIROutputWrapper>,
        ) -> Result<usize>;

        #[Self = "Job"]
        #[must_use]
        fn create() -> UniquePtr<Job>;
    }

    extern "Rust" {
        /// Opaque Rust box holding the closure a `to_callback` output writes to.
        ///
        /// Constructed via [`make_output_box`]; the C++ `CallbackOutput` holds
        /// it by `rust::Box<OutputBox>` and forwards each encoded message
        /// through [`invoke_output`].
        type OutputBox;

        /// Called by the C++ `CallbackOutput::out` shim with one encoded GRIB
        /// message.
        fn invoke_output(output: &mut OutputBox, data: &[u8]);
    }
}

// Public re-exports for the safe wrapper crate
pub use cxx::{Exception, UniquePtr};
pub use ffi::*;

// ==================== Output callback adapter ====================

/// Opaque wrapper holding the closure each interpolated message is handed to.
///
/// The C++ `CallbackOutput` (declared in `MIROutput.h` as `struct OutputBox`)
/// carries this by `rust::Box<OutputBox>` and forwards every
/// `out(const void*, size_t, bool)` call via [`invoke_output`].
pub struct OutputBox(Box<dyn FnMut(&[u8]) + Send>);

/// Wrap a closure for [`ffi::MIROutputWrapper::to_callback`].
///
/// The `'static` bound is what keeps this safe: the output owns the box and may
/// outlive the call that created it, so the closure cannot borrow from its
/// caller. Collect into an `Arc<Mutex<..>>` or send over a channel instead.
pub fn make_output_box<F>(f: F) -> Box<OutputBox>
where
    F: FnMut(&[u8]) + Send + 'static,
{
    Box::new(OutputBox(Box::new(f)))
}

/// Called from C++ `CallbackOutput::out` to deliver one encoded message.
fn invoke_output(output: &mut OutputBox, data: &[u8]) {
    (output.0)(data);
}
