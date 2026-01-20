## About this project

* These code are a part of a PhD work under DFG reference number: LE 4256/1-1 Project number: 419255448
* [License](https://git.physi.uni-heidelberg.de/HIT-PAT/HITDAQ/src/branch/master/LICENSE) under CC BY 4.0
* Analyse code for HITPAT data collected on **2024 August 8th**.
* The codes are built using **GNU MAKE**

## Environment
This project is developed and tested using the following environment:

- Operating system: Linux
- Shell environment for CVMFS users [LHCb Conda](https://gitlab.cern.ch/lhcb-core/lbcondawrappers/-/blob/master/README.md): `lb-conda default/2021-04-03` (CVMFS)

For users **not using CVMFS**, the following software and versions are required:

### Compilers and build tools
- GNU Make ≥ 4.x
- GCC / G++ ≥ 7 (C++11 support required)

### Libraries
- ROOT ≥ 6.22  
  (must provide `root-config`)
- GNU Scientific Library (GSL) ≥ 2.5  
  (must provide `gsl-config`)
- GMP ≥ 6.1 (including `libgmp` and `libgmpxx`)

### Environment requirements
- `root-config` and `gsl-config` must be available in `PATH`
- Corresponding library paths must be visible via `LD_LIBRARY_PATH`

The code is expected to build correctly with any reasonably recent Linux
distribution providing the above dependencies.

## How to run

Each folder contains one GNU Makefile to build the code.

For example, to run `beam_on_status`:

```bash
cd beam_on_status
make clean; make
./beam_on_status run5 4
```

or using Snakemake:
```bash
snakemake -r -j 4 maketestfile
```

## Folder structure

The `hitreader` folder contains the core headers for reading and analyzing beam data:

- **`hitreader/analyser.h`**  
  Top-level header for all folders; includes general analysis utilities.

- **`hitreader/hitreader.h`**  
  Interface to raw binary data (data type: `int`).  
  Defines the raw frame structure, which may vary between measurements.  
  As of **2024-08-08**, the raw binary files contain only calibrated data.

- **`hitreader/hitreader_float.h`**  
  Defines the float frame structure (data type: `float`).  
  Uses the same layout as the raw frame but stores floating-point values.  
  Primarily used for CPU analysize, hosting calibration factors, pedestal values, etc.

The `beam_on_status` folder contains code to determine the beam-on states using the clustering algorithm in the PhD thesis.

The `cal_fac_testbeam` folder contains the calibration factors used in the test beam from run5 onward.

The `cal_pre` folder contains the code for generating the calibration factor (where 1 is represented by 8192).

The `sum_signal1d` folder adds up the signals with careful pedestal subtraction.

