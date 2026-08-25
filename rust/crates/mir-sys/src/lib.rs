//! FFI bindings to ECMWF mir (Meteorological Interpolation and Regridding) library.
//!
//! mir pulls fields from a [`MIRInput`], transforms them according to a
//! [`Job`], and pushes them to a [`MIROutput`]. A job is a description
//! rather than an action, so one job applies to any number of input/output
//! pairings.

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

        type MIRInput;

        /// Advance to the next message; false once the stream is exhausted.
        fn next(self: Pin<&mut MIRInput>) -> Result<bool>;

        /// Fields carried per message: 1 for a scalar, 2 for a vector pair.
        fn dimensions(self: &MIRInput) -> Result<usize>;

        #[Self = "MIRInput"]
        fn from_data_handle(
            handle: Pin<&mut DataHandleWrapper>,
        ) -> Result<UniquePtr<MIRInput>>;

        #[Self = "MIRInput"]
        fn from_grib_file(path: &str) -> Result<UniquePtr<MIRInput>>;

        #[Self = "MIRInput"]
        fn from_grib_memory(message: &[u8]) -> Result<UniquePtr<MIRInput>>;

        /// Consecutive messages read as one N-dimensional field, as mir pairs
        /// components for the `vod2uv` and `uv2uv` job keys.
        #[Self = "MIRInput"]
        fn from_multi_dimensional_grib_file(
            path: &str,
            dimensions: usize,
            skip: usize,
        ) -> Result<UniquePtr<MIRInput>>;

        #[Self = "MIRInput"]
        fn from_gridspec(gridspec: &str, gridded: bool) -> Result<UniquePtr<MIRInput>>;

        #[Self = "MIRInput"]
        fn from_raw(
            values: &[f64],
            metadata: &Parametrisation,
        ) -> Result<UniquePtr<MIRInput>>;

        // ==================== MIROutput ====================

        type MIROutput;

        /// Interpolated values, for an output built by `to_resizable`.
        fn values(self: &MIROutput) -> Result<&[f64]>;

        /// The grid the field was interpolated onto, for an output built by
        /// `to_resizable`.
        fn metadata_json(self: &MIROutput) -> Result<String>;

        /// The encoded message, for an output built by `to_grib_memory`.
        fn message(self: &MIROutput) -> Result<&[u8]>;

        #[Self = "MIROutput"]
        fn to_callback(output: Box<OutputBox>) -> Result<UniquePtr<MIROutput>>;

        #[Self = "MIROutput"]
        fn to_grib_file(path: &str, append: bool) -> Result<UniquePtr<MIROutput>>;

        #[Self = "MIROutput"]
        fn to_grib_memory(capacity: usize) -> Result<UniquePtr<MIROutput>>;

        #[Self = "MIROutput"]
        fn to_resizable() -> Result<UniquePtr<MIROutput>>;

        #[Self = "MIROutput"]
        fn to_empty() -> Result<UniquePtr<MIROutput>>;

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
        fn representation_from(self: Pin<&mut Job>, input: &MIRInput) -> Result<()>;

        #[cxx_name = "to_json"]
        fn json_str(self: &Job) -> Result<String>;

        /// The equivalent mir-tool command line, for debugging.
        fn mir_tool_call(self: &Job) -> Result<String>;

        /// Transform the message the input is currently positioned on. Callers
        /// drive iteration themselves; see `execute_all`.
        fn execute_one(
            self: &Job,
            input: Pin<&mut MIRInput>,
            output: Pin<&mut MIROutput>,
        ) -> Result<()>;

        /// Drain the input, transforming every message, and return how many
        /// were processed. Single-field inputs yield one.
        fn execute_all(
            self: &Job,
            input: Pin<&mut MIRInput>,
            output: Pin<&mut MIROutput>,
        ) -> Result<usize>;

        #[Self = "Job"]
        #[must_use]
        fn create() -> UniquePtr<Job>;
    }

    extern "Rust" {
        type OutputBox;

        fn invoke_output(output: &mut OutputBox, data: &[u8]);
    }
}

pub use cxx::{Exception, UniquePtr};
pub use ffi::*;

// ==================== Output callback adapter ====================

type OutputFn = Box<dyn FnMut(&[u8]) + Send>;

/// Holds the closure a `to_callback` output hands each encoded message to.
///
/// The C++ `CallbackOutput` carries this by `rust::Box<OutputBox>` and forwards
/// every `out` call through [`invoke_output`].
pub struct OutputBox(OutputFn);

/// Wrap a closure for [`ffi::MIROutput::to_callback`].
///
/// The `'static` bound is what keeps this safe: the output owns the box and may
/// outlive the call that created it, so the closure cannot borrow from its
/// caller.
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
