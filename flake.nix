{
  description = "King's Field portable Linux and WebAssembly application";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/64c08a7ca051951c8eae34e3e3cb1e202fe36786";
  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      nativeTools = with pkgs; [ cmake ninja pkg-config cargo rustc clang ];
      nativeLibraries = with pkgs; [ sdl3 libGL libglvnd ];
      sources = pkgs.lib.cleanSourceWith {
        src = ./.;
        filter = path: type:
          let
            relative = pkgs.lib.removePrefix "${toString ./.}/" (toString path);
            sourceDirectories = [ "cmake" "src" "include" "codecs" "codec-bridge" "web" ];
          in pkgs.lib.cleanSourceFilter path type
            && !(builtins.elem (baseNameOf path) [ "build" "target" "__pycache__" ])
            && (builtins.elem relative [ "CMakeLists.txt" "build.json" ]
              || builtins.any (directory:
                relative == directory || pkgs.lib.hasPrefix "${directory}/" relative
              ) sourceDirectories);
      };
      unwrapped = pkgs.clangStdenv.mkDerivation {
        pname = "kings-field";
        version = "0.1.0";
        src = sources;
        nativeBuildInputs = nativeTools;
        buildInputs = nativeLibraries;
        preBuild = ''
          export CARGO_HOME="$TMPDIR/kings-field-cargo"
        '';
        meta = {
          description = "King's Field direct source port (requires original Japanese disc data)";
          mainProgram = "kings-field";
          platforms = [ system ];
        };
      };
      game = pkgs.writeShellApplication {
        name = "kings-field";
        runtimeInputs = [ pkgs.coreutils pkgs.util-linux ];
        text = ''
          game_binary=${unwrapped}/bin/kings-field
          ${builtins.readFile ./scripts/launch.sh}
        '';
        meta.description = "King's Field launcher: set KF_DISC to your original Japanese ISO or BIN/CUE";
      };
    in {
      packages.${system} = { inherit game unwrapped; default = game; };
      apps.${system}.default = {
        type = "app";
        program = "${game}/bin/kings-field";
        meta.description = "King's Field direct source port";
      };
      checks.${system} = { native = unwrapped; launcher = game; };
      devShells.${system}.default = (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
        packages = nativeTools ++ nativeLibraries ++ (with pkgs; [
          emscripten nodejs chromium xvfb-run xdotool imagemagick rustfmt python3
        ]);
        KF_SDL_SOURCE = "${pkgs.sdl3.src}";
        KF_RUST_SOURCE = "${pkgs.rustPlatform.rustLibSrc}";
      };
    };
}
