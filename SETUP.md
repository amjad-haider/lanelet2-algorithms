# SETUP.md

How Lanelet2 C++ is installed and linked on this machine (Ubuntu 24.04 /
Noble, GCC 13.3.0). Route: ROS 2 apt package.

## What worked

```
sudo apt install -y ros-jazzy-lanelet2 ros-jazzy-lanelet2-examples ros-jazzy-lanelet2-maps
```

`ros-jazzy-lanelet2` was verified present via `apt-cache policy
ros-jazzy-lanelet2` before installing (candidate `1.2.1-1noble...`), because
CLAUDE.md's own instruction is to verify the package exists rather than
assume it. It pulled in every submodule package (`lanelet2-core`, `-io`,
`-projection`, `-routing`, `-traffic-rules`, `-matching`, `-validation`,
`-python`) plus `ros-jazzy-mrt-cmake-modules`, the CMake helper library the
upstream source itself depends on (`find_package(mrt_cmake_modules
REQUIRED)` in every package's own CMakeLists — the reason a bare `cmake ..`
against the cloned source tree does not work; apt sidesteps that entirely by
shipping prebuilt binaries and config files).

Installed layout:

- Headers: `/opt/ros/jazzy/include/lanelet2_core/`, `lanelet2_io/`,
  `lanelet2_projection/`, `lanelet2_routing/`, `lanelet2_traffic_rules/`,
  `lanelet2_matching/`, `lanelet2_validation/`.
- Libraries: `/opt/ros/jazzy/lib/x86_64-linux-gnu/liblanelet2_*.so.1.2.1`.
- CMake config files: `/opt/ros/jazzy/share/lanelet2_*/cmake/`.

## CMake incantation for a downstream project

```cmake
find_package(lanelet2_io REQUIRED)
find_package(lanelet2_routing REQUIRED)
# ...one find_package per lanelet2 submodule actually used

add_executable(my_target main.cpp)
target_link_libraries(my_target PRIVATE ${lanelet2_io_LIBRARIES} ${lanelet2_routing_LIBRARIES})
```

Each submodule's `<name>Config.cmake` sets a `<name>_LIBRARIES` variable
(e.g. `lanelet2_io_LIBRARIES` = `lanelet2_io::lanelet2_io;lanelet2_io::lanelet2_io_compiler_flags`)
— link against that variable rather than guessing the imported target name
by hand, it is the documented interface.

Configure with the ROS prefix on `CMAKE_PREFIX_PATH`:

```
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=/opt/ros/jazzy
```

No need to `source /opt/ros/jazzy/setup.bash` for configure or build — only
`CMAKE_PREFIX_PATH` is needed for `find_package` to locate the config files.

## Runtime caveat: RPATH is not transitive (verified failure)

The installed `.so` files carry no `RPATH`/`RUNPATH` of their own. A binary
linked with `CMAKE_PREFIX_PATH=/opt/ros/jazzy` alone gets a `RUNPATH`
pointing at `/opt/ros/jazzy/lib/x86_64-linux-gnu` for its own *direct*
dependencies only. `DT_RUNPATH` (unlike the older `DT_RPATH`) is **not**
consulted when the dynamic linker resolves a *dependency's* own
dependencies. Concretely: `liblanelet2_io.so.1` depends on
`liblanelet2_core.so.1`; that second lookup ignores the executable's
`RUNPATH` and falls through to `LD_LIBRARY_PATH` / the system default
search path.

Verified failure, with a clean environment (`env -i`, so no ROS
`setup.bash` variables):

```
error while loading shared libraries: liblanelet2_core.so.1: cannot open shared object file: No such file or directory
```

This only went unnoticed during initial testing because this machine's
`~/.bashrc` sources ROS's `setup.bash`, which sets `LD_LIBRARY_PATH`
globally for every shell. A binary built here must not rely on that — it
should be runnable standalone. Fix, in the project's own top-level
`CMakeLists.txt`:

```cmake
set(CMAKE_BUILD_RPATH "/opt/ros/jazzy/lib/x86_64-linux-gnu")
set(CMAKE_INSTALL_RPATH "/opt/ros/jazzy/lib/x86_64-linux-gnu")
```

This bakes the search path into every binary this project builds, so it
runs correctly regardless of whether ROS's environment has been sourced.

## Dead end, recorded for the record: Conan

CLAUDE.md's stated preference order was ROS apt, then Conan, then Docker.
Conan was tried first (before discovering the ROS apt install already
existed on this machine) and hit two real, verified failures before being
abandoned in favor of the apt route:

1. Upstream's `conanfile.py` does `from distutils.sysconfig import
   get_python_lib`. `distutils` was removed from the Python standard library
   in 3.12 (this machine's default `python3`). `conan create .` failed
   immediately at recipe-load time with `ModuleNotFoundError: No module
   named 'distutils'`. Worked around locally by replacing the import with an
   equivalent `sysconfig.get_path('purelib', scheme='posix_prefix', ...)`
   call — a patch to the cloned reference source only, never upstreamed.
2. After that fix, `conan create . --build=missing` got much further —
   `lanelet2_core`, `lanelet2_io`, `lanelet2_projection` all built and
   linked successfully as shared libraries — but failed on Lanelet2's own
   internal gtest unit tests (`lanelet2_core-gtest-test_attribute`,
   `test_polygon`, `test_linestring`, ...) with `undefined reference to
   testing::internal::MakeAndRegisterTestInfo`. Root cause was not fully
   isolated, but the most likely culprit: upstream's own official Conan
   instructions specify `pip install conan catkin_pkg numpy` as
   prerequisites, and `mrt_cmake_modules`'s CMake macros likely shell out to
   Python helpers needing those packages to correctly register gtest
   targets. Conan was installed in this session via `uv tool install conan`,
   which gives Conan an isolated venv containing only Conan's own
   dependencies — `catkin_pkg` and `numpy` were confirmed absent from it.
   Not re-attempted once the working ROS apt route was found.

Given upstream's own README calls this route "experimental," and a working
route was already available, this was not pursued further. If ROS apt ever
stops being viable, retry Conan with `catkin_pkg` and `numpy` installed into
Conan's own interpreter first.

## Versions

- Ubuntu 24.04.3 LTS (Noble), GCC 13.3.0
- `ros-jazzy-lanelet2` 1.2.1-1noble
- `ros-jazzy-mrt-cmake-modules` 1.0.11-2noble
- Boost 1.83.0 (system apt version, resolved via the `mrt_cmake_modules`
  `FindBoost` — matches Ubuntu 24.04's `libboost-all-dev`)
