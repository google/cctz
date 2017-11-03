# CMake workspace for CCTZ

This CMake project offers a complete build of `cctz` and its dependencies.
- it uses CMake `ExternalProject` feature
- it runs the cctz tests
- it tries to keep a consistent compile options between the projects,
  thanks to the toolchain file

# How to use

```
mkdir build
cd build
cmake ..
make
```
