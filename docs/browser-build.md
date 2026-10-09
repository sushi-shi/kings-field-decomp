# Manual browser build on Windows

This recipe builds the WebAssembly target. The port's native package and
Nix development shell target x86_64 Linux.

Run commands from the `port` checkout. The Linux browser workflow is in the
[README](../README.md#browser).

The browser version can be built directly on Windows. Install
[Git](https://git-scm.com/downloads/win), [Python 3](https://www.python.org/downloads/windows/),
[CMake 3.25+](https://cmake.org/download/) and [Ninja](https://ninja-build.org/),
with their commands available on `PATH`. In PowerShell, from your `port` checkout:

```powershell
git clone --depth 1 --branch 5.0.6 https://github.com/emscripten-core/emsdk.git build/emsdk
.\build\emsdk\emsdk.bat install 5.0.6
.\build\emsdk\emsdk.bat activate 5.0.6
Set-ExecutionPolicy -Scope Process RemoteSigned
.\build\emsdk\emsdk_env.ps1
python scripts/english_patch.py fetch --output build/wasm/english-v1.kfdelta
emcmake cmake --preset wasm
cmake --build --preset wasm
python -m http.server --directory build/wasm
```

CMake downloads SDL automatically. For later builds, repeat from
`Set-ExecutionPolicy`, skipping the translation download.

CMake's `wasm` preset writes to `build/wasm`; its Emscripten branch uses
SDL from `KF_SDL_SOURCE` when supplied, otherwise fetches the pinned SDL archive.
The optional translation delta is embedded by `scripts/english_patch.py`.
See [translation details](english-resources.md) for its source and local-use scope.

Serve `build/wasm`, open `http://localhost:8000/kings-field.html`, and select
your Japanese disc. Browser data and saves use local browser storage.
