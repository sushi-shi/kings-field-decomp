{
  description = "King's Field portable Linux and WebAssembly application";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/64c08a7ca051951c8eae34e3e3cb1e202fe36786";
  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      nativeTools = with pkgs; [ cmake ninja pkg-config cargo rustc clang python3 ];
      nativeLibraries = with pkgs; [ sdl3 libGL libglvnd ];
      sources = pkgs.lib.cleanSourceWith {
        src = ./.;
        filter = path: type:
          let
            relative = pkgs.lib.removePrefix "${toString ./.}/" (toString path);
            sourceDirectories = [ "cmake" "src" "include" "codecs" "codec-bridge" "web" "scripts" "resources" ];
          in pkgs.lib.cleanSourceFilter path type
            && !(builtins.elem (baseNameOf path) [ "build" "target" "__pycache__" ])
            && (builtins.elem relative [ "CMakeLists.txt" "build.json" ]
              || builtins.any (directory:
                relative == directory || pkgs.lib.hasPrefix "${directory}/" relative
              ) sourceDirectories);
      };
      # John Osborne's (weissvulf) English translation v1.0: the translator's own
      # PPF release, fetched from the Wayback Machine copy of their site. Nothing
      # translated is stored in this repository or re-hosted; the delta is derived
      # locally and only applied to the player's own Japanese disc resources.
      englishTranslation = pkgs.fetchurl {
        name = "Kings_Field_Jap_to_Eng_v1.0.rar";
        url = "https://web.archive.org/web/20211128030012id_/https://www.angelfire.com/art3/weissvulf/Kings_Field_Jap_to_Eng_v1.0.rar";
        hash = "sha256-1BMeSi4+nzwBiOTCxsLspjLcWpVNqNtK8pm8T74u224=";
      };
      englishDelta = pkgs.runCommand "kings-field-english-v1.kfdelta" {
        nativeBuildInputs = [ pkgs.libarchive pkgs.python3 ];
      } ''
        bsdtar -xOf ${englishTranslation} "KF Jap to Eng v1.0.ppf" > translation.ppf
        python3 ${./scripts/english_patch.py} from-ppf --ppf translation.ppf \
          --layout ${./resources/slps-00017-layout.tsv} --output "$out"
      '';
      unwrapped = pkgs.clangStdenv.mkDerivation {
        pname = "kings-field";
        version = "0.1.0";
        src = sources;
        nativeBuildInputs = nativeTools;
        buildInputs = nativeLibraries;
        cmakeFlags = [ "-DKF_ENGLISH_PATCH=${englishDelta}" ];
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
      packages.${system} = { inherit game unwrapped; english-delta = englishDelta; default = game; };
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
        KF_ENGLISH_PATCH = "${englishDelta}";
      };
    };
}
