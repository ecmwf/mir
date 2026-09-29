// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

use mir_sys::{Exception, Job, MIRInput, MIROutput, Parametrisation, UniquePtr};

#[test]
fn readback_on_wrong_output_is_serious_bug() -> Result<(), Exception> {
    mir_sys::init();

    let output = MIROutput::to_empty()?;
    let err = output.values().expect_err("values on an empty output");

    assert!(matches!(
        eckit_sys::Error::try_from_cxx(&err),
        Some(eckit_sys::Error::SeriousBug(_))
    ));

    Ok(())
}

#[test]
fn null_data_handle_is_assertion_failed() {
    mir_sys::init();

    let Err(err) = MIRInput::from_data_handle(UniquePtr::null()) else {
        panic!("null handle accepted");
    };

    assert!(matches!(
        eckit_sys::Error::try_from_cxx(&err),
        Some(eckit_sys::Error::AssertionFailed(_))
    ));
}

#[test]
fn wrongly_typed_metadata_is_cannot_convert() -> Result<(), Exception> {
    mir_sys::init();

    let mut meta = Parametrisation::make();
    meta.pin_mut().set_bool("gridded", true)?;
    meta.pin_mut().set_str("gridType", "reduced_gg")?;
    meta.pin_mut().set_f64("north", 90.)?;
    meta.pin_mut().set_f64("west", 0.)?;
    meta.pin_mut().set_f64("south", -90.)?;
    meta.pin_mut().set_f64("east", 360.)?;
    meta.pin_mut().set_str_list("N", &["four"])?;

    let mut input = MIRInput::from_raw(&[0.; 208], &meta)?;
    let mut output = MIROutput::to_empty()?;

    let mut job = Job::make();
    job.pin_mut().set_str("grid", "3/3")?;
    let err = job
        .execute_one(input.pin_mut(), output.pin_mut())
        .expect_err("N given as a list of strings");

    assert!(matches!(
        mir_sys::Error::try_from_cxx(&err),
        Some(mir_sys::Error::CannotConvert(_))
    ));

    Ok(())
}
