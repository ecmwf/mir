// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

//! Interpolate a GRIB field onto a 1x1 regular lat/lon grid.
//!
//! Run with: `cargo run --example regrid -- <input.grib>`
//!
//! An input is something like: `echo retrieve,param=z,target=z.grib | mars`

use std::error::Error;
use std::fs;

use mir_sys::{Job, MIRInput, MIROutput};

fn main() -> Result<(), Box<dyn Error>> {
    let Some(path) = std::env::args().nth(1) else {
        eprintln!("usage: regrid <input.grib>");
        return Ok(());
    };

    eckit_sys::init();

    let mut job = Job::make();
    job.pin_mut().set_str("grid", "1/1")?;

    // file to file
    let mut input = MIRInput::from_grib_file(&path)?;
    let mut output = MIROutput::to_grib_file("z_ll_ff.grib", false)?;
    let fields = job.execute_all(input.pin_mut(), output.pin_mut())?;
    println!("file to file: {fields} field(s) written to z_ll_ff.grib");

    // memory to file
    let message = fs::read(&path)?;
    let mut input = MIRInput::from_grib_memory(&message)?;
    let mut output = MIROutput::to_grib_file("z_ll_mf.grib", false)?;
    job.execute_one(input.pin_mut(), output.pin_mut())?;
    println!("memory to file: z_ll_mf.grib");

    // file to memory
    let mut input = MIRInput::from_grib_file(&path)?;
    let mut output = MIROutput::to_grib_memory(64 * 1024 * 1024)?;
    job.execute_all(input.pin_mut(), output.pin_mut())?;
    fs::write("z_ll_fm.grib", output.message()?)?;
    println!("file to memory: {} bytes", output.message()?.len());

    Ok(())
}
