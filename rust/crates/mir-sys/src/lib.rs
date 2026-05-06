//! FFI bindings to ECMWF mir (Meteorological Interpolation and Regridding) library.

use bindman::track_cpp_api;

#[track_cpp_api(("mir/api/MIRJob.h", class = "MIRJob"),)]
#[cxx::bridge(namespace = "mir_bridge")]
pub mod ffi {
    unsafe extern "C++" {
        include!("mir_bridge.h");

        // Cross-crate ExternType from eckit-sys
        #[namespace = "eckit_bridge"]
        type DataHandleWrapper = eckit_sys::DataHandleWrapper;

        // ==================== GribDataHandleInput ====================

        type GribInputWrapper;

        fn next(self: Pin<&mut GribInputWrapper>) -> Result<bool>;

        fn grib_input_create(
            handle: Pin<&mut DataHandleWrapper>,
        ) -> Result<UniquePtr<GribInputWrapper>>;

        // ==================== VectorInput ====================

        type VectorInputWrapper;

        fn vector_input_create(
            component1: Pin<&mut GribInputWrapper>,
            component2: Pin<&mut GribInputWrapper>,
        ) -> Result<UniquePtr<VectorInputWrapper>>;

        // ==================== MIRJob ====================

        type MIRJobWrapper;

        fn set_string(self: Pin<&mut MIRJobWrapper>, name: &str, value: &str) -> Result<()>;
        fn set_double(self: Pin<&mut MIRJobWrapper>, name: &str, value: f64) -> Result<()>;
        fn set_long(self: Pin<&mut MIRJobWrapper>, name: &str, value: i64) -> Result<()>;
        fn set_bool(self: Pin<&mut MIRJobWrapper>, name: &str, value: bool) -> Result<()>;
        fn set_double_list(self: Pin<&mut MIRJobWrapper>, name: &str, values: &[f64])
        -> Result<()>;
        fn set_from_string(self: Pin<&mut MIRJobWrapper>, args: &str) -> Result<()>;
        fn clear(self: Pin<&mut MIRJobWrapper>, name: &str) -> Result<()>;
        fn representation_from(
            self: Pin<&mut MIRJobWrapper>,
            input: Pin<&mut GribInputWrapper>,
        ) -> Result<()>;
        fn json_str(self: &MIRJobWrapper) -> Result<String>;
        fn mir_tool_call(self: &MIRJobWrapper) -> Result<String>;

        /// Execute with DataHandle input (creates GribDataHandleInput internally).
        fn execute(
            self: Pin<&mut MIRJobWrapper>,
            input: Pin<&mut DataHandleWrapper>,
            output: Box<OutputBox>,
        ) -> Result<()>;

        /// Execute with GribInput (for scalar interpolation with pre-created input).
        fn execute_input(
            self: Pin<&mut MIRJobWrapper>,
            input: Pin<&mut GribInputWrapper>,
            output: Box<OutputBox>,
        ) -> Result<()>;

        /// Execute with VectorInput (for VO/D → U/V conversion).
        fn execute_vector(
            self: Pin<&mut MIRJobWrapper>,
            input: Pin<&mut VectorInputWrapper>,
            output: Box<OutputBox>,
        ) -> Result<()>;

        #[must_use]
        fn mir_job_create() -> UniquePtr<MIRJobWrapper>;
    }

    extern "Rust" {
        /// Opaque Rust box holding the output callback.
        type OutputBox;

        /// Called from C++ `RustGribOutput::out()` with interpolated message data.
        fn invoke_output(output: &OutputBox, data: &[u8]);
    }
}

pub use cxx::{Exception, UniquePtr};
pub use ffi::*;

// SAFETY: All mir wrapper types own their data with no thread-local or global mutable state.
#[allow(clippy::non_send_fields_in_send_ty)]
mod send_impls {
    use super::ffi::{GribInputWrapper, MIRJobWrapper, VectorInputWrapper};
    unsafe impl Send for GribInputWrapper {}
    unsafe impl Send for MIRJobWrapper {}
    unsafe impl Send for VectorInputWrapper {}
}

type OutputFn = Box<dyn FnMut(&[u8])>;

/// Holds a closure that receives interpolated GRIB messages.
///
/// The closure runs synchronously within `MIRJob::execute()` — never sent across threads.
pub struct OutputBox(OutputFn);

impl OutputBox {
    /// Create an `OutputBox` from a closure.
    ///
    /// # Safety
    /// The closure must remain valid for the duration of the `execute()` call.
    /// This is guaranteed when used via `MIRJob::execute()`.
    pub unsafe fn new<'a>(f: impl FnMut(&[u8]) + 'a) -> Self {
        // Erase the lifetime — cxx requires 'static for Box<Opaque>,
        // but the closure runs synchronously within execute().
        type BorrowedFn<'b> = Box<dyn FnMut(&[u8]) + 'b>;
        let f: BorrowedFn<'a> = Box::new(f);
        // Safety: caller guarantees the closure outlives the execute() call
        let erased: OutputFn = unsafe { std::mem::transmute::<BorrowedFn<'a>, OutputFn>(f) };
        Self(erased)
    }
}

/// Called from C++ to deliver interpolated data.
fn invoke_output(output: &OutputBox, data: &[u8]) {
    // Safety: OutputBox is only accessed from C++ callback which is single-threaded
    // within a single execute() call. The &self is actually &mut through the C++ side.
    let ptr = (&raw const output.0) as *mut OutputFn;
    unsafe { (*ptr)(data) };
}
