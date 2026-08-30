{
  description = "King's Field (SLPS-00017) reverse-engineering and matching environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/64c08a7ca051951c8eae34e3e3cb1e202fe36786";

    psy-k-src = {
      url = "github:ttkb-oss/psy-k/e25c309c48cf5a1ae133e092f661fb874cc0f47f";
      flake = false;
    };

    maspsx-src = {
      url = "github:mkst/maspsx/746b895f02929ecd148af7b1f4ff05b69f973878";
      flake = false;
    };

    ghidra-psx-loader-src = {
      # Tag 2026.07.08; upstream CI explicitly covers Ghidra 12.0.4.
      url = "git+https://github.com/lab313ru/ghidra_psx_ldr.git?rev=85d9efaf5693418979152c2298f776734824035b&submodules=1";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, psy-k-src, maspsx-src, ghidra-psx-loader-src }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };

      floppies = pkgs.fetchurl {
        name = "psyq-release-2.5-floppies.rar";
        url = "https://archive.org/download/ps1_sdks/Floppies.rar";
        hash = "sha256-SaLzzryjqEIclPHeQ7nSDnN1CwQrCDHdKxcy0o+Ud4M=";
      };

      gcc260Disk = pkgs.fetchurl {
        name = "psyq-gnu-c-compiler-2.60-disk-1.img";
        url = "https://archive.org/download/ps1_sdks/GNU%20C%20Compiler%20Version%202.60%20%28World%29%20%28Disk%201%29.img";
        hash = "sha256-B6+f7NFIzUI0Ec65En7T8+kUmhFRoaJU3z4rjwwzuuc=";
      };

      # Keep the verifier/stager in the default shell so the historical tools
      # are an actual first-load dependency, not a README-only prerequisite.
      psyqToolchain = pkgs.runCommand "kings-field-toolchain-psyq-candidates" {
        nativeBuildInputs = with pkgs; [
          binutils
          coreutils
          file
          p7zip
          python3
          unar
        ];
      } ''
        export PSYQ_FLOPPIES_RAR="${floppies}"
        export PSYQ_GCC260_IMG="${gcc260Disk}"
        python3 ${./scripts/create-toolchain.py} \
          --work-dir "$TMPDIR/toolchain-work" \
          --stage-dir "$out"
      '';

      psy-k = pkgs.rustPlatform.buildRustPackage {
        pname = "psy-k";
        version = "0.4.0-git";
        src = psy-k-src;
        cargoHash = "sha256-Afpg/HxxpweZv07VN9zIkqMEohRYHqkRvgzBs9ABUp0=";
        # Upstream's proprietary fixture submodule is intentionally absent.
        doCheck = false;
      };

      maspsx = pkgs.writeShellApplication {
        name = "maspsx";
        runtimeInputs = [ pkgs.python3 ];
        text = ''
          exec python3 ${maspsx-src}/maspsx.py "$@"
        '';
      };

      ghidraPsxLoader = pkgs.stdenvNoCC.mkDerivation {
        pname = "ghidra-psx-loader";
        version = "2026.07.08";
        src = ghidra-psx-loader-src;
        nativeBuildInputs = with pkgs; [ gradle jdk21 unzip ];

        buildPhase = ''
          runHook preBuild
          export GHIDRA_INSTALL_DIR="${pkgs.ghidra}/lib/ghidra"
          export GRADLE_USER_HOME="$TMPDIR/gradle-home"
          export XDG_CONFIG_HOME="$TMPDIR/xdg-config"
          mkdir -p "$XDG_CONFIG_HOME"
          printf '\nrootProject.name = "ghidra_psx_ldr"\n' >> settings.gradle
          # Upstream tracks an old XML-form Sleigh artifact. Ghidra 12 expects
          # the current compiled SLA format, so force buildExtension to rebuild
          # it from the pinned .slaspec/.sinc sources.
          rm data/languages/mips32le.sla
          ${pkgs.ghidra}/lib/ghidra/support/sleigh \
            data/languages/mips32le.slaspec \
            data/languages/mips32le.sla
          gradle --offline --no-daemon buildExtension
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p unpacked "$out/lib/ghidra/Ghidra/Extensions/ghidra_psx_ldr"
          unzip -q dist/*.zip -d unpacked
          extension_root="$(find unpacked -mindepth 1 -maxdepth 1 -type d -print -quit)"
          if [ -z "$extension_root" ]; then
            echo "ghidra-psx-loader: extension root missing from Gradle output" >&2
            exit 1
          fi
          cp -R "$extension_root"/. "$out/lib/ghidra/Ghidra/Extensions/ghidra_psx_ldr/"
          runHook postInstall
        '';
      };

      # Nixpkgs' Ghidra wrapper exposes an extension-only application root via
      # NIX_GHIDRAHOME while retaining the base installation as the primary
      # root. This avoids copying Ghidra or colliding with its built-in modules.
      ghidraWithPlugins = pkgs.ghidra.withExtensions (_: [ ghidraPsxLoader ]);

      crossBinutils = pkgs.pkgsCross.mipsel-linux-gnu.buildPackages.binutils;
      mipsBinutilsAliases = pkgs.runCommand "mipsel-linux-gnu-binutils-aliases" { } ''
        mkdir -p "$out/bin"
        for tool in addr2line ar as c++filt elfedit gprof ld nm objcopy objdump ranlib readelf size strings strip; do
          source="${crossBinutils}/bin/mipsel-unknown-linux-gnu-$tool"
          if [ -e "$source" ]; then
            ln -s "$source" "$out/bin/mipsel-linux-gnu-$tool"
          fi
        done
      '';

      analysisPython = pkgs.python3.withPackages (pythonPackages: with pythonPackages; [
        capstone
        intervaltree
        pyelftools
        pyghidra
        pytest
        pyyaml
      ]);

      pythonSync = pkgs.writeShellApplication {
        name = "kf-python-sync";
        runtimeInputs = [ pkgs.git pkgs.uv pkgs.python311 ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          export UV_PROJECT_ENVIRONMENT="$repo/build/python-env"
          export UV_PYTHON="${pkgs.python311}/bin/python3.11"
          exec uv sync --frozen --no-install-project --project "$repo"
        '';
      };

      splat = pkgs.writeShellApplication {
        name = "splat";
        runtimeInputs = [ pkgs.git pkgs.uv pkgs.python311 ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          export UV_PROJECT_ENVIRONMENT="$repo/build/python-env"
          export UV_PYTHON="${pkgs.python311}/bin/python3.11"
          exec uv run --frozen --project "$repo" splat "$@"
        '';
      };

      retailValidate = pkgs.writeShellApplication {
        name = "kf-retail-validate";
        runtimeInputs = [ analysisPython pkgs.git ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.retail "$@"
        '';
      };

      retailSeed = pkgs.writeShellApplication {
        name = "kf-retail-seed";
        runtimeInputs = [ analysisPython pkgs.git ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.seed_retail "$@"
        '';
      };

      functionAudit = pkgs.writeShellApplication {
        name = "kf-function-audit";
        runtimeInputs = [ analysisPython pkgs.git ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.audit_functions "$@"
        '';
      };

      functionPropose = pkgs.writeShellApplication {
        name = "kf-function-propose";
        runtimeInputs = [ analysisPython pkgs.git ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.propose_function_admission "$@"
        '';
      };

      shell = pkgs.mkShell {
        name = "kings-field";
        packages = [
          psyqToolchain
          psy-k
          maspsx
          crossBinutils
          mipsBinutilsAliases
          analysisPython
          pythonSync
          splat
          retailValidate
          retailSeed
          functionAudit
          functionPropose
          ghidraWithPlugins
        ] ++ (with pkgs; [
          # Binary analysis and emulation.
          jdk21
          dosbox-staging
          mednafen
          radare2
          binwalk

          # Disc/image extraction, authoring, and comparison.
          bchunk
          cdrtools
          libcdio
          mame-tools
          mtools
          p7zip
          unar
          unrar
          xdelta

          # Native build, scripting, and inspection tools.
          git
          git-lfs
          gnumake
          cmake
          ninja
          pkg-config
          gcc
          llvmPackages.clang-unwrapped
          llvm
          gdb
          file
          jq
          ripgrep
          ruff
          uv
          python311
          xxd
          gnutar
          xz
        ]);

        shellHook = ''
          export KINGS_FIELD_DIR="$(git rev-parse --show-toplevel 2>/dev/null || echo "$PWD")"
          export PSYQ_DIR="${psyqToolchain}"
          export PSYQ_BIN="$PSYQ_DIR/psyq/bin"
          export PSYQ_INCLUDE="$PSYQ_DIR/psyq/include"
          export PSYQ_LIB="$PSYQ_DIR/psyq/lib"
          # Exact compiler attribution is not yet proven. Keep all three
          # candidates available for code-generation fingerprint tests; these
          # are alternatives, not a claim that King's Field used all of them.
          export PSYQ_GCC241_DIR="$PSYQ_DIR/compilers/gcc-2.4.1"
          export PSYQ_GCC260_RELEASE25_DIR="$PSYQ_DIR/compilers/gcc-2.6.0-release-2.5"
          export PSYQ_GCC260_DISK_DIR="$PSYQ_DIR/compilers/gcc-2.6.0-disk-1"
          export GHIDRA_INSTALL_DIR="${pkgs.ghidra}/lib/ghidra"
          export NIX_GHIDRAHOME="${ghidraWithPlugins}/lib/ghidra/Ghidra"
          export GHIDRA_PSX_LOADER="${ghidraPsxLoader}/lib/ghidra/Ghidra/Extensions/ghidra_psx_ldr"
          export JAVA_HOME="${pkgs.jdk21}/lib/openjdk"
          export UV_PROJECT_ENVIRONMENT="$KINGS_FIELD_DIR/build/python-env"
          export UV_PYTHON="${pkgs.python311}/bin/python3.11"

          echo "[kings-field] Psy-Q candidates: $PSYQ_DIR" >&2
          echo "[kings-field] compiler probes : GCC 2.4.1; two distinct GCC 2.6.0 builds" >&2
          echo "[kings-field] analysis        : ghidra + PSX loader, pyghidra, psy-k, radare2, mipsel binutils" >&2
          echo "[kings-field] assembly        : maspsx + mipsel-linux-gnu-as" >&2
          echo "[kings-field] Python RE stack : run 'kf-python-sync' once, then 'splat ...'" >&2
          echo "[kings-field] retail census   : kf-retail-validate; kf-function-audit/propose" >&2
        '';
      };

      toolchainTests = pkgs.runCommand "kings-field-toolchain-tests" {
        nativeBuildInputs = [ pkgs.python3 ];
      } ''
        mkdir project
        cp -r ${./scripts} project/scripts
        cp -r ${./tests} project/tests
        cd project
        python3 -m unittest discover -s tests -v
        touch "$out"
      '';

      ghidraPluginTests = pkgs.runCommand "kings-field-ghidra-plugin-tests" {
        nativeBuildInputs = [ analysisPython pkgs.jdk21 ];
        GHIDRA_INSTALL_DIR = "${pkgs.ghidra}/lib/ghidra";
        NIX_GHIDRAHOME = "${ghidraWithPlugins}/lib/ghidra/Ghidra";
      } ''
        export HOME="$TMPDIR/home"
        mkdir -p "$HOME"
        python3 ${./tests/ghidra_plugin_smoke.py}
        touch "$out"
      '';

      retailConfigTests = pkgs.runCommand "kings-field-retail-config-tests" {
        nativeBuildInputs = [ pkgs.python3 ];
      } ''
        python3 ${./scripts/kf/retail.py} ${./config/retail}
        touch "$out"
      '';
    in {
      packages.${system} = {
        inherit psyqToolchain psy-k maspsx mipsBinutilsAliases ghidraPsxLoader ghidraWithPlugins retailValidate retailSeed functionAudit functionPropose;
        default = psyqToolchain;
      };

      devShells.${system} = {
        default = shell;
        build = shell;
      };

      checks.${system} = {
        toolchain = psyqToolchain;
        toolchain-builder-tests = toolchainTests;
        ghidra-psx-loader = ghidraPsxLoader;
        ghidra-psx-loader-discovery = ghidraPluginTests;
        retail-config = retailConfigTests;
      };
    };
}
