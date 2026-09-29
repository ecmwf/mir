// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

use mir_sys::{Exception, Job, MIRInput, MIROutput, Parametrisation, UniquePtr};

/// The reduced Gaussian N4 field of mir's `tests/unit/raw_memory.cc`: 32 x 42
/// on the latitude north of the Equator, 32 x -42 south of it, 0 elsewhere.
fn reduced_gg_n4() -> Result<(Vec<f64>, UniquePtr<Parametrisation>), Exception> {
    let mut meta = Parametrisation::make();
    meta.pin_mut().set_bool("gridded", true)?;
    meta.pin_mut().set_str("gridType", "reduced_gg")?;
    meta.pin_mut().set_f64("north", 90.)?;
    meta.pin_mut().set_f64("west", 0.)?;
    meta.pin_mut().set_f64("south", -90.)?;
    meta.pin_mut().set_f64("east", 360.)?;
    meta.pin_mut().set_i64("N", 4)?;
    meta.pin_mut()
        .set_i64_list("pl", &[20, 24, 28, 32, 32, 28, 24, 20])?;

    let mut values = vec![0.; 208];
    values[72..104].fill(42.);
    values[104..136].fill(-42.);

    Ok((values, meta))
}

#[test]
fn raw_to_resizable() -> Result<(), Exception> {
    mir_sys::init();

    let (values, meta) = reduced_gg_n4()?;
    let mut input = MIRInput::from_raw(&values, &meta)?;
    let mut output = MIROutput::to_resizable()?;

    let mut job = Job::make();
    job.pin_mut().set_f64_list("grid", &[2., 2.])?;
    job.pin_mut().set_f64_list("area", &[1., -1., -1., 1.])?;
    job.pin_mut().set_str("interpolation", "nn")?;
    job.pin_mut().set_bool("caching", false)?;
    job.execute_one(input.pin_mut(), output.pin_mut())?;

    assert_eq!(output.values()?, [42., 42., -42., -42.]);
    assert_eq!(
        output.metadata()?.to_json()?,
        r#"{"grid":"{\"area\":[1,-1,-1,1],\"grid\":[2,2],\"reference\":[1,1]}"}"#
    );

    Ok(())
}

#[test]
fn gridspec_to_empty() -> Result<(), Exception> {
    mir_sys::init();

    let mut input = MIRInput::from_gridspec("{grid: o2}", true)?;
    let mut output = MIROutput::to_empty()?;

    let mut job = Job::make();
    job.pin_mut().set_str("grid", "3/3")?;
    job.pin_mut().set_bool("caching", false)?;
    job.execute_one(input.pin_mut(), output.pin_mut())?;

    Ok(())
}

#[test]
fn gridspec_to_resizable() -> Result<(), Exception> {
    mir_sys::init();

    let mut input = MIRInput::from_gridspec("{grid: o2}", true)?;
    let mut output = MIROutput::to_resizable()?;

    let mut job = Job::make();
    job.pin_mut().set_str("grid", "3/3")?;
    job.pin_mut().set_bool("caching", false)?;
    job.execute_one(input.pin_mut(), output.pin_mut())?;

    assert_eq!(output.values()?.len(), 120 * 61);
    assert_eq!(
        output.metadata()?.to_json()?,
        r#"{"grid":"{\"grid\":[3,3]}"}"#
    );

    Ok(())
}
