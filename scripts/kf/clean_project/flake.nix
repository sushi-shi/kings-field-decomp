{
  description = "King's Field (SLPS-00017) source";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/64c08a7ca051951c8eae34e3e3cb1e202fe36786";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; config.allowUnfree = true; };
      sdk = import ./nix/psx-toolchain.nix {
        inherit pkgs;
        sdkBuilder = ./scripts/create-toolchain.py;
      };
      emulator = import ./nix/pcsx-redux.nix { inherit pkgs; };
      environment = {
        PSYQ_C_INCLUDE = "${sdk.gcc257Headers}/include";
        PSYQ_INCLUDE = "${sdk.psyqSdk}/release-2.5/isa board/PSXLIB/INCLUDE";
        PSYQ_BIN = "${sdk.psyqSdk}/release-2.5/isa board/PSXBIN/BIN";
        PSYQ_LIB = "${sdk.psyqSdk}/release-2.5/isa board/PSXLIB/LIB";
        PSYQ_H2000_LIB = "${sdk.psyqSdk}/release-2.5/H2000/LIB2000";
        PSYQ_ASPSX = "${sdk.aspsxNative}/1.07/ASPSX.EXE";
        PSYQ_ASMPSX = "${sdk.asmpsxNative}";
      };
      buildTools = [ pkgs.python3 pkgs.dosbox-x sdk.cpppsx257 sdk.cc1psx257 ];
      game = pkgs.stdenvNoCC.mkDerivation (environment // {
        pname = "kings-field";
        version = "1.0";
        src = pkgs.lib.cleanSourceWith {
          src = ./.;
          filter = path: type:
            let name = baseNameOf path;
            in !(builtins.elem name [ ".git" "build" "result" "__pycache__" "codecs" ]);
        };
        nativeBuildInputs = buildTools;
        dontConfigure = true;
        buildPhase = ''
          runHook preBuild
          python3 build.py
          runHook postBuild
        '';
        installPhase = ''
          runHook preInstall
          mkdir -p "$out"
          cp build/psx/PSX.EXE build/game/GAME.EXE build/open/OPEN.EXE "$out/"
          mkdir -p "$out/link"
          cp build/psx/PSX.CPE build/game/GAME.CPE build/open/OPEN.CPE "$out/link/"
          runHook postInstall
        '';
      });
      runner = pkgs.writeShellApplication {
        name = "kings-field";
        runtimeInputs = [ pkgs.python3 emulator ];
        text = ''
          export PCSX_REDUX_BIN="${emulator}/bin/pcsx-redux"
          export PCSX_REDUX_BIOS="${emulator.openbios}"
          exec python3 ${./scripts/psxbuild/disc.py} --executables ${game} "$@"
        '';
      };
      retailRunner = pkgs.writeShellApplication {
        name = "kf-run-retail";
        runtimeInputs = [ pkgs.python3 emulator ];
        text = ''
          export PCSX_REDUX_BIN="${emulator}/bin/pcsx-redux"
          export PCSX_REDUX_BIOS="${emulator.openbios}"
          exec python3 ${./scripts/psxbuild/disc.py} --retail "$@"
        '';
      };
    in {
      packages.${system} = { inherit game; default = game; };
      apps.${system} = {
        default = { type = "app"; program = "${runner}/bin/kings-field"; };
        retail = { type = "app"; program = "${retailRunner}/bin/kf-run-retail"; };
      };
      devShells.${system} = {
        default = pkgs.mkShell (environment // { packages = buildTools ++ [ retailRunner ]; });
        codecs = pkgs.mkShell { packages = [ pkgs.cargo pkgs.rustc ]; };
      };
    };
}
