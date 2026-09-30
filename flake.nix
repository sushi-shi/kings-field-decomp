{
  description = "King's Field portable Linux and WebAssembly application";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/64c08a7ca051951c8eae34e3e3cb1e202fe36786";
  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      nativeTools = with pkgs; [ cmake ninja pkg-config cargo rustc clang rust-bindgen rustfmt ];
      nativeLibraries = with pkgs; [ sdl3 libGL libglvnd ];
      sources = pkgs.lib.cleanSourceWith {
        src = ./.;
        filter = path: type:
          let
            relative = pkgs.lib.removePrefix "${toString ./.}/" (toString path);
            sourceDirectories = [ "src" "include" "codecs" "web" ];
          in pkgs.lib.cleanSourceFilter path type
            && !(builtins.elem (baseNameOf path) [ "build" "target" "__pycache__" ])
            && (builtins.elem relative [ "CMakeLists.txt" ]
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
      game = pkgs.lib.makeOverridable ({ disc ? null }:
        let
          resources = pkgs.runCommand "kings-field-resources" {
            preferLocalBuild = true;
            allowSubstitutes = false;
          } ''
            ${unwrapped}/bin/kings-field --disc ${pkgs.lib.escapeShellArg "${disc}"} \
              --extract-to "$out" --extract-only
          '';
        in pkgs.writeShellApplication {
          name = "kings-field";
          runtimeInputs = [ pkgs.coreutils pkgs.util-linux ];
          text = if disc == null then ''
            game_binary=${unwrapped}/bin/kings-field
            ${builtins.readFile ./scripts/launch.sh}
          '' else ''
            exec ${unwrapped}/bin/kings-field --data ${resources} "$@"
          '';
          meta.description = if disc == null
            then "King's Field launcher: set KF_DISC to your original Japanese ISO or BIN/CUE"
            else "King's Field with resources from your Japanese disc";
        }) {};
    in {
      packages.${system}.default = game;
      nixosModules.default = { config, lib, pkgs, ... }:
        let cfg = config.programs.kings-field;
        in {
          options.programs.kings-field = {
            enable = lib.mkEnableOption "King's Field";
            disc = lib.mkOption {
              type = lib.types.nullOr lib.types.path;
              default = null;
              description = "Japanese disc image to extract during installation. If unset, use KF_DISC when launching.";
            };
          };
          config = lib.mkIf cfg.enable {
            environment.systemPackages = [
              (self.packages.${pkgs.stdenv.hostPlatform.system}.default.override {
                inherit (cfg) disc;
              })
            ];
          };
        };
      apps.${system}.default = {
        type = "app";
        program = "${game}/bin/kings-field";
        meta.description = "King's Field direct source port";
      };
      checks.${system} = {
        native = unwrapped;
        launcher = game;
        codec-bindings = pkgs.runCommand "kf-codec-bindings" {
          nativeBuildInputs = with pkgs; [ rust-bindgen rustfmt cargo rustc clippy ];
          src = sources;
        } ''
          cp -r "$src" source
          chmod -R u+w source
          cd source
          bash codecs/bindings.sh --check
          export CARGO_HOME="$TMPDIR/kings-field-cargo"
          cargo clippy --offline --locked --release --manifest-path codecs/Cargo.toml -- \
            -D unfulfilled_lint_expectations
          touch "$out"
        '';
      };
      devShells.${system}.default = (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
        packages = nativeTools ++ nativeLibraries ++ (with pkgs; [
          emscripten nodejs chromium xvfb-run xdotool imagemagick python3 clippy
        ]);
        KF_SDL_SOURCE = "${pkgs.sdl3.src}";
        KF_RUST_SOURCE = "${pkgs.rustPlatform.rustLibSrc}";
      };
    };
}
