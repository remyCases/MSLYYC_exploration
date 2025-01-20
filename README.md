# ModShardLauncher YYC Exploration

## Description

An attempt to port YYTK for MSL.

## Getting Started

### Prerequisites

The following tools are required to build the project:

- MSYS2 GCC 13.2.0
- MSYS2 G++ 13.2.0
- CMake 3.26.4
- Make 4.4.1
- Ninja 1.11.1

### Clone and Build

1. Clone the repository:
```bash
git clone git@github.com:remyCases/MSLYYC_exploration.git
cd MSLYYC_exploration
```

2. Initialize submodules:
```bash
git submodule update --init --recursive
```

3. Build using make:
```bash
make build_release
```

4. For testing, you can run with a specific file:
```bash
make run VAR=path/to/file
```

## Contributing

Contributions are welcomed, you can find the CONTRIBUTING document that sums up contribution guidelines [here](CONTRIBUTING.md).