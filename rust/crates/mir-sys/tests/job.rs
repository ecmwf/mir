// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

use mir_sys::{Exception, Job};

#[test]
fn interpolation_by_name_or_by_spec() -> Result<(), Exception> {
    mir_sys::init();

    let mut by_name = Job::make();
    by_name.pin_mut().set_str("interpolation", "k-nearest")?;
    by_name.pin_mut().set_i64("nclosest", 8)?;

    let mut by_spec = Job::make();
    by_spec
        .pin_mut()
        .set_str("interpolation", "{type: k-nearest, nclosest: 8}")?;

    assert_eq!(by_spec.to_json()?, by_name.to_json()?);
    assert_eq!(
        by_spec.to_json()?,
        r#"{"interpolation":"k-nearest","nclosest":8}"#
    );

    Ok(())
}

#[test]
fn interpolation_options_are_set_one_way() -> Result<(), Exception> {
    mir_sys::init();

    let mut by_spec = Job::make();
    by_spec
        .pin_mut()
        .set_str("interpolation", "{type: k-nearest, nclosest: 8}")?;
    let err = by_spec
        .pin_mut()
        .set_i64("nclosest", 4)
        .expect_err("option set by spec, then on its own");
    assert!(matches!(
        eckit_sys::Error::try_from_cxx(&err),
        Some(eckit_sys::Error::UserError(_))
    ));

    let mut by_name = Job::make();
    by_name.pin_mut().set_str("interpolation", "k-nearest")?;
    by_name.pin_mut().set_i64("nclosest", 8)?;
    let err = by_name
        .pin_mut()
        .set_str("interpolation", "{type: k-nearest, nclosest: 4}")
        .expect_err("option set on its own, then by spec");
    assert!(matches!(
        eckit_sys::Error::try_from_cxx(&err),
        Some(eckit_sys::Error::UserError(_))
    ));

    Ok(())
}

#[test]
fn spec_options_go_with_the_interpolation() -> Result<(), Exception> {
    mir_sys::init();

    let mut job = Job::make();
    job.pin_mut()
        .set_str("interpolation", "{type: k-nearest, nclosest: 8}")?;

    job.pin_mut().set_str("interpolation", "linear")?;
    assert_eq!(job.to_json()?, r#"{"interpolation":"linear"}"#);
    job.pin_mut().set_i64("nclosest", 4)?;
    job.pin_mut().clear("nclosest")?;

    job.pin_mut()
        .set_str("interpolation", "{type: k-nearest, nclosest: 8}")?;
    job.pin_mut().clear("interpolation")?;
    assert_eq!(job.to_json()?, "{}");

    Ok(())
}
