## About this project

* These code are a part of a PhD work under DFG reference number: LE 4256/1-1 Project number: 419255448
* Project: [A Scintillating fiber-based Beam Profile Monitor for Ion Therapy](https://git.physi.uni-heidelberg.de/HIT-PAT/HITDAQ/src/branch/master)
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

There are two applications here. One under folder`cal_pre, and one under `app`
Each folder contains one GNU Makefile to build the code.

For example, to run `app/main`:

```bash
cd app
make clean; make
./main dataset_path runID
```

<!-- or using Snakemake:
```bash
snakemake -r -j 4 maketestfile
``` -->

## Folder structure

The `app/main.cpp` file is the main application. Different analysis processes can be easily pipelined in `main.cpp`.

- **`src/analysis`**  
  Contains different analysis processes.  
  All analyses follow the `IAnalyzer.h` interface, enabling an easy pipeline structure in `main.cpp`.  
  Detailed analysis modules are located under `src/analysis/modules/`.

- **`src/algo`**  
  Contains clustering, RMS, and other algorithms for beam reconstruction.


- **`src/io`**  
  Provides the interface to the binary dataset. Data are returned as `double`.

The `cal_fac_testbeam` folder contains the calibration factors used in test beam runs from run 5 onward.

The `cal_pre` folder contains the code for generating the calibration factors (where a value of 1 is represented by 8192).



