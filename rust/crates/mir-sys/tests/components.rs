// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

use mir_sys::{Exception, MIRInput, Parametrisation, UniquePtr};

fn raw() -> Result<UniquePtr<MIRInput>, Exception> {
    MIRInput::from_raw(&[0.; 4], &Parametrisation::make())
}

#[test]
fn components_step_together() -> Result<(), Exception> {
    mir_sys::init();

    let mut input = MIRInput::from_components()?;
    input.pin_mut().append(raw()?)?;
    input.pin_mut().append(raw()?)?;

    assert_eq!(input.dimensions()?, 2);
    assert!(input.pin_mut().next()?);
    assert!(!input.pin_mut().next()?);

    Ok(())
}

#[test]
fn append_to_other_input_is_user_error() -> Result<(), Exception> {
    mir_sys::init();

    let mut input = raw()?;
    let Err(err) = input.pin_mut().append(raw()?) else {
        panic!("append accepted by a raw input");
    };

    assert!(matches!(
        eckit_sys::Error::try_from_cxx(&err),
        Some(eckit_sys::Error::UserError(_))
    ));

    Ok(())
}

#[test]
fn append_null_is_assertion_failed() -> Result<(), Exception> {
    mir_sys::init();

    let mut input = MIRInput::from_components()?;
    let Err(err) = input.pin_mut().append(UniquePtr::null()) else {
        panic!("null component accepted");
    };

    assert!(matches!(
        eckit_sys::Error::try_from_cxx(&err),
        Some(eckit_sys::Error::AssertionFailed(_))
    ));

    Ok(())
}
