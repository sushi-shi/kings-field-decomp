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

    objdiff-src = {
      # v3.7.3: both front ends use our tested shared data comparator.
      url = "github:encounter/objdiff/6bcac60df8bb0b4de5b1cb98b033bdedb9ac6aa4";
      flake = false;
    };

    ghidra-psx-loader-src = {
      # Tag 2026.07.08; upstream CI explicitly covers Ghidra 12.0.4.
      url = "git+https://github.com/lab313ru/ghidra_psx_ldr.git?rev=85d9efaf5693418979152c2298f776734824035b&submodules=1";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, psy-k-src, maspsx-src, objdiff-src, ghidra-psx-loader-src }:
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

      # Native Decompals rebuild used only as a practical code-generation probe.
      # It corresponds to GCC 2.6.0's PSX target, but is not evidence that this
      # rebuilt host binary (or either staged historical 2.6.0 binary) built KF.
      gcc260NativeArchive = pkgs.fetchurl {
        name = "decompals-old-gcc-0.17-gcc-2.6.0-psx.tar.gz";
        url = "https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.6.0-psx.tar.gz";
        hash = "sha256-NY2slJ8PVmr5xq+Tu6DUrATVbKnfgMWL1m2Bu7UN7Os=";
      };
      gcc260Native = pkgs.runCommand "decompals-gcc-2.6.0-psx-0.17" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/bin"
        tar -xzf ${gcc260NativeArchive} -C "$out/bin"
        chmod +x "$out/bin"/*
      '';

      # Second native probe. GCC 2.5.7 is the closest available rebuild to the
      # 1994 Psy-Q compiler family; its text epilogue and load hoisting match
      # retail forms that 2.6.0 cannot emit, but it is still a probe.
      gcc257NativeArchive = pkgs.fetchurl {
        name = "decompals-old-gcc-0.17-gcc-2.5.7-psx.tar.gz";
        url = "https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.5.7-psx.tar.gz";
        hash = "sha256-DbH7QDx2blGwlDVLmhNqbB8BRXqkHMyiisRXt8tn2HE=";
      };
      gcc257Native = pkgs.runCommand "decompals-gcc-2.5.7-psx-0.17" {
        nativeBuildInputs = [ pkgs.gnutar pkgs.gzip ];
      } ''
        mkdir -p "$out/bin"
        tar -xzf ${gcc257NativeArchive} -C "$out/bin"
        chmod +x "$out/bin"/*
      '';

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

      runtime26Media = pkgs.fetchurl {
        name = "psyq-runtime-2.6.zip";
        url = "https://archive.org/download/ps1_sdks/Programmer%20Tool%20-%20Runtime%20Library%20Version%202.6%20%28Japan%29%20%28En%2CJa%29_DTL-S2170_redump.zip";
        hash = "sha256-eROgCPwsPjBZuKOJ9N+3Nzd6NbcrQ8sIsz8PVSR4W9M=";
      };

      runtime26Tools = pkgs.runCommand "kings-field-runtime26-host-tools" {
        nativeBuildInputs = [ pkgs.python3 pkgs.p7zip ];
      } ''
        python3 ${./scripts/stage-runtime26-tools.py} ${runtime26Media} "$out"
      '';

      psy-k = pkgs.rustPlatform.buildRustPackage {
        pname = "psy-k";
        version = "0.4.0-git";
        src = psy-k-src;
        cargoHash = "sha256-Afpg/HxxpweZv07VN9zIkqMEohRYHqkRvgzBs9ABUp0=";
        # Upstream's proprietary fixture submodule is intentionally absent.
        doCheck = false;
      };

      maspsxPatched = pkgs.applyPatches {
        name = "maspsx-private-bss-source";
        src = maspsx-src;
        patches = [ ./patches/maspsx-private-bss.patch ];
      };

      maspsx = pkgs.writeShellApplication {
        name = "maspsx";
        runtimeInputs = [ pkgs.python3 ];
        text = ''
          exec python3 ${maspsxPatched}/maspsx.py "$@"
        '';
      };

      cc1psx260 = pkgs.writeShellApplication {
        name = "cc1psx-260";
        text = ''exec ${gcc260Native}/bin/cc1 "$@"'';
      };

      cpppsx260 = pkgs.writeShellApplication {
        name = "cpppsx-260";
        text = ''exec ${gcc260Native}/bin/cpp "$@"'';
      };

      cc1psx257 = pkgs.writeShellApplication {
        name = "cc1psx-257";
        text = ''exec ${gcc257Native}/bin/cc1 "$@"'';
      };

      cpppsx257 = pkgs.writeShellApplication {
        name = "cpppsx-257";
        text = ''exec ${gcc257Native}/bin/cpp "$@"'';
      };

      gcc257Probe = import ./nix/gcc257.nix { inherit pkgs; };

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

      objdiffVersion = "3.7.3";
      objdiffGuiLibs = with pkgs; [
        libGL libxkbcommon wayland fontconfig freetype
        libx11 libxcursor libxi libxrandr libxcb
      ];
      objdiffBuild = pkgs.rustPlatform.buildRustPackage {
        pname = "objdiff-kf";
        version = objdiffVersion;
        src = objdiff-src;
        patches = [
          ./patches/objdiff-strict-data.patch
          ./patches/objdiff-bss-ownership.patch
          ./patches/objdiff-branch-destination.patch
        ];
        cargoHash = "sha256-Z9vyUj35nrHuUoOYM54RLCn7CzcQ6k3A6FsDYKCVqVM=";
        cargoBuildFlags = [ "-p" "objdiff-cli" "-p" "objdiff-gui" ];
        cargoInstallFlags = [ "-p" "objdiff-cli" "-p" "objdiff-gui" ];
        cargoTestFlags = [ "-p" "objdiff-core" "-p" "objdiff-cli" "-p" "objdiff-gui" ];
        nativeBuildInputs = [ pkgs.pkg-config pkgs.protobuf pkgs.makeWrapper ];
        buildInputs = objdiffGuiLibs;
        postInstall = ''
          wrapProgram $out/bin/objdiff \
            --prefix LD_LIBRARY_PATH : "${pkgs.lib.makeLibraryPath objdiffGuiLibs}"
        '';
      };
      objdiff-cli = objdiffBuild;
      objdiff = objdiffBuild;

      # A PATH wrapper also works in shells launched with nix develop --command.
      # The shared project groups objects under psx/, game/, and open/.
      objdiffShim = pkgs.writeShellApplication {
        name = "objdiff";
        text = ''
          for arg in "$@"; do
            case "$arg" in
              -p|--project-dir|--project-dir=*) exec ${objdiff}/bin/objdiff "$@" ;;
            esac
          done
          exec ${objdiff}/bin/objdiff \
            --project-dir "$KINGS_FIELD_DIR/build/objdiff" "$@"
        '';
      };

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
        unicorn
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

      vendoredSeed = pkgs.writeShellApplication {
        name = "kf-vendored-seed";
        runtimeInputs = [ analysisPython pkgs.git psy-k ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.seed_vendored_functions \
            --psyk ${psy-k}/bin/psyk \
            --sdk-lib-dir ${psyqToolchain}/psyq/lib \
            --signature-dir ${ghidraPsxLoader}/lib/ghidra/Ghidra/Extensions/ghidra_psx_ldr/data/psyq/260 \
            "$@"
        '';
      };

      fidCensus = pkgs.writeShellApplication {
        name = "kf-fid-census";
        runtimeInputs = [ analysisPython pkgs.git psy-k ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.fid_census \
            --psyk ${psy-k}/bin/psyk \
            --sdk-lib-dir ${psyqToolchain}/psyq/lib \
            "$@"
        '';
      };

      retailDelink = pkgs.writeShellApplication {
        name = "kf-delink";
        runtimeInputs = [ analysisPython pkgs.git ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.delink "$@"
        '';
      };

      objdiffProject = pkgs.writeShellApplication {
        name = "kf-objdiff-project";
        runtimeInputs = [ analysisPython pkgs.git ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.objdiff project "$@"
        '';
      };

      objdiffReport = pkgs.writeShellApplication {
        name = "kf-objdiff-report";
        runtimeInputs = [ analysisPython objdiff-cli pkgs.git ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.objdiff report "$@"
        '';
      };

      sourceCompile = pkgs.writeShellApplication {
        name = "kf-compile";
        runtimeInputs = [
          analysisPython
          cc1psx260
          cpppsx260
          cc1psx257
          cpppsx257
          maspsx
          mipsBinutilsAliases
          pkgs.git
        ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.compile "$@"
        '';
      };

      kfCli = pkgs.writeShellApplication {
        name = "kf";
        runtimeInputs = [
          analysisPython
          cc1psx260
          cpppsx260
          cc1psx257
          cpppsx257
          maspsx
          mipsBinutilsAliases
          objdiff-cli
          pkgs.llvmPackages.clang-unwrapped
          pkgs.git
          pkgs.ninja
        ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          cd "$repo"
          exec python3 -m scripts.kf.cli "$@"
        '';
      };

      shell = pkgs.mkShell {
        name = "kings-field";
        packages = [
          psyqToolchain
          psy-k
          pkgs.dosbox-x
          maspsx
          cc1psx260
          cpppsx260
          cc1psx257
          cpppsx257
          crossBinutils
          mipsBinutilsAliases
          analysisPython
          pythonSync
          splat
          retailValidate
          retailSeed
          functionAudit
          functionPropose
          vendoredSeed
          fidCensus
          retailDelink
          objdiffProject
          objdiffReport
          sourceCompile
          kfCli
          ghidraWithPlugins
          objdiff-cli
          objdiff
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
          cargo
          rustc
          rustfmt
        ]);

        shellHook = ''
          export KINGS_FIELD_DIR="$(git rev-parse --show-toplevel 2>/dev/null || echo "$PWD")"
          export PSYQ_DIR="${psyqToolchain}"
          export PSYQ_RUNTIME26_BIN="${runtime26Tools}/bin"
          export PSYQ_BIN="$PSYQ_DIR/psyq/bin"
          export PSYQ_INCLUDE="$PSYQ_DIR/psyq/include"
          export PSYQ_LIB="$PSYQ_DIR/psyq/lib"
          # Exact compiler attribution is not yet proven. Keep all three
          # candidates available for code-generation fingerprint tests; these
          # are alternatives, not a claim that King's Field used all of them.
          export PSYQ_GCC241_DIR="$PSYQ_DIR/compilers/gcc-2.4.1"
          export PSYQ_GCC260_RELEASE25_DIR="$PSYQ_DIR/compilers/gcc-2.6.0-release-2.5"
          export PSYQ_GCC260_DISK_DIR="$PSYQ_DIR/compilers/gcc-2.6.0-disk-1"
          export KF_GCC260_NATIVE="${gcc260Native}"
          export KF_GCC257_NATIVE="${gcc257Native}"
          export GHIDRA_INSTALL_DIR="${pkgs.ghidra}/lib/ghidra"
          export NIX_GHIDRAHOME="${ghidraWithPlugins}/lib/ghidra/Ghidra"
          export GHIDRA_PSX_LOADER="${ghidraPsxLoader}/lib/ghidra/Ghidra/Extensions/ghidra_psx_ldr"
          export JAVA_HOME="${pkgs.jdk21}/lib/openjdk"
          export UV_PROJECT_ENVIRONMENT="$KINGS_FIELD_DIR/build/python-env"
          export UV_PYTHON="${pkgs.python311}/bin/python3.11"
          export PATH="${objdiffShim}/bin:$PATH"

          ${kfCli}/bin/kf clangd >&2 || echo "[kings-field] clangd setup failed; run kf clangd after fixing the reported error" >&2

          echo "[kings-field] Psy-Q candidates: $PSYQ_DIR" >&2
          echo "[kings-field] compiler probes : GCC 2.4.1; two distinct GCC 2.6.0 builds" >&2
          echo "[kings-field] native C probes  : cc1psx-260/cpppsx-260 and cc1psx-257/cpppsx-257 (Decompals rebuild 0.17)" >&2
          echo "[kings-field] analysis        : ghidra + PSX loader, pyghidra, psy-k, radare2, mipsel binutils" >&2
          echo "[kings-field] assembly        : maspsx + mipsel-linux-gnu-as" >&2
          echo "[kings-field] Python RE stack : run 'kf-python-sync' once, then 'splat ...'" >&2
          echo "[kings-field] retail census   : kf-retail-validate; kf-function-audit/propose; kf-fid-census; kf-vendored-seed" >&2
          echo "[kings-field] matching        : kf init/build/match/status/check/bank; objdiff GUI" >&2
          echo "[kings-field] objdiff project : $KINGS_FIELD_DIR/build/objdiff (psx/game/open; override with -p)" >&2
        '';
      };

      toolchainTests = pkgs.runCommand "kings-field-toolchain-tests" {
        nativeBuildInputs = [
          analysisPython mipsBinutilsAliases psy-k objdiff-cli pkgs.dosbox-x
          pkgs.llvmPackages.clang-unwrapped
        ];
        GHIDRA_PSX_LOADER = "${ghidraPsxLoader}/lib/ghidra/Ghidra/Extensions/ghidra_psx_ldr";
        PSYQ_LIB = "${psyqToolchain}/psyq/lib";
        PSYQ_INCLUDE = "${psyqToolchain}/psyq/include";
        PSYQ_BIN = "${psyqToolchain}/psyq/bin";
        PSYQ_RUNTIME26_BIN = "${runtime26Tools}/bin";
      } ''
        mkdir project
        cp -r ${./scripts} project/scripts
        cp -r ${./tests} project/tests
        cp -r ${./config} project/config
        cp -r ${./include} project/include
        cp -r ${./src} project/src
        cd project
        python3 -m unittest discover -s tests -v
        touch "$out"
      '';

      gcc257TraceTests = pkgs.runCommand "kings-field-gcc257-trace-tests" {
        nativeBuildInputs = [
          analysisPython cc1psx257 cpppsx257 maspsx mipsBinutilsAliases
        ];
      } ''
        mkdir -p project/tests/fixtures
        cp -r ${./scripts} project/scripts
        cp ${./tests/gcc257_trace_smoke.py} project/tests/gcc257_trace_smoke.py
        cp ${./tests/fixtures/gcc257_trace_controls.c} project/tests/fixtures/gcc257_trace_controls.c
        cd project
        python3 tests/gcc257_trace_smoke.py \
          --debug ${gcc257Probe.debug}/bin/cc1psx-257-debug \
          --instrumented ${gcc257Probe.instrumented}/bin/cc1psx-257-trace \
          --output build/verification
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
        mkdir project
        cp -r ${./scripts} project/scripts
        cp -r ${./config} project/config
        cp -r ${./include} project/include
        cd project
        python3 -m scripts.kf.retail config/retail
        python3 -m scripts.kf.inventory check --config-dir config/retail
        touch "$out"
      '';

      objdiffMipsTests = pkgs.runCommand "kings-field-objdiff-mips-tests" {
        nativeBuildInputs = [
          analysisPython
          crossBinutils
          mipsBinutilsAliases
          objdiff-cli
          cc1psx260
          cpppsx260
          cc1psx257
          cpppsx257
          maspsx
        ];
      } ''
        mkdir project
        cp -r ${./scripts} project/scripts
        cp ${./tests/objdiff_mips_smoke.py} project/objdiff_mips_smoke.py
        cp ${./tests/objdiff_data_smoke.py} project/objdiff_data_smoke.py
        cp ${./tests/compiler_mips_smoke.py} project/compiler_mips_smoke.py
        cp ${./tests/test_compile_data_sizes.py} project/test_compile_data_sizes.py
        cd project
        python3 objdiff_mips_smoke.py
        python3 objdiff_data_smoke.py
        python3 compiler_mips_smoke.py
        python3 -m unittest test_compile_data_sizes -v
        touch "$out"
      '';

      psylinkOrderTests = pkgs.runCommand "kings-field-psylink-order-tests" {
        nativeBuildInputs = [
          pkgs.python3
          pkgs.dosbox-x
          psy-k
        ];
        PSYLINK_DOSBOX = "${pkgs.dosbox-x}/bin/dosbox-x";
        PSYQ_BIN = "${psyqToolchain}/psyq/bin";
        PSYQ_LIB = "${psyqToolchain}/psyq/lib";
      } ''
        export HOME="$TMPDIR/home"
        export XDG_CONFIG_HOME="$TMPDIR/config"
        mkdir -p "$HOME" "$XDG_CONFIG_HOME"
        mkdir project
        cp ${./tests/psylink_order_smoke.py} project/psylink_order_smoke.py
        cp ${./tests/psylink_alignment_smoke.py} project/psylink_alignment_smoke.py
        cp ${./tests/psylink_bss_smoke.py} project/psylink_bss_smoke.py
        python3 project/psylink_order_smoke.py
        python3 project/psylink_alignment_smoke.py
        python3 project/psylink_bss_smoke.py
        touch "$out"
      '';

      codecTests = pkgs.runCommand "kings-field-codec-tests" {
        nativeBuildInputs = [ pkgs.cargo pkgs.rustc pkgs.rustfmt pkgs.stdenv.cc ];
      } ''
        cp -r ${./tools} tools
        chmod -R u+w tools
        export CARGO_TARGET_DIR="$TMPDIR/cargo-target"
        cargo fmt --manifest-path tools/Cargo.toml --all -- --check
        cargo check --offline --manifest-path tools/Cargo.toml --lib
        cargo test --offline --manifest-path tools/Cargo.toml
        touch "$out"
      '';
    in {
      packages.${system} = {
        gcc257Debug = gcc257Probe.debug;
        gcc257Instrumented = gcc257Probe.instrumented;
        gcc257Source = gcc257Probe.source;
        inherit psyqToolchain psy-k maspsx gcc260Native cc1psx260 cpppsx260 gcc257Native cc1psx257 cpppsx257 mipsBinutilsAliases ghidraPsxLoader ghidraWithPlugins objdiff-cli objdiff retailValidate retailSeed functionAudit functionPropose vendoredSeed fidCensus retailDelink objdiffProject objdiffReport sourceCompile kfCli;
        default = psyqToolchain;
      };

      devShells.${system} = {
        default = shell;
        build = shell;
        instrumentation = pkgs.mkShell {
          inputsFrom = [ shell ];
          packages = [ gcc257Probe.debug gcc257Probe.instrumented ];
          shellHook = shell.shellHook;
        };
      };

      checks.${system} = {
        toolchain = psyqToolchain;
        toolchain-builder-tests = toolchainTests;
        gcc257-trace = gcc257TraceTests;
        ghidra-psx-loader = ghidraPsxLoader;
        ghidra-psx-loader-discovery = ghidraPluginTests;
        retail-config = retailConfigTests;
        objdiff-mips = objdiffMipsTests;
        psylink-order = psylinkOrderTests;
        codecs = codecTests;
      };
    };
}
