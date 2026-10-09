{
  description = "King's Field (SLPS-00017) reverse-engineering and matching environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/64c08a7ca051951c8eae34e3e3cb1e202fe36786";

    psy-k-src = {
      url = "github:ttkb-oss/psy-k/e25c309c48cf5a1ae133e092f661fb874cc0f47f";
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

  outputs = { self, nixpkgs, psy-k-src, objdiff-src, ghidra-psx-loader-src }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };

      toolchain = import ./nix/psx-toolchain.nix {
        inherit pkgs;
        sdkBuilder = ./scripts/create-toolchain.py;
      };
      inherit (toolchain) psyqSdk gcc257Native gcc257Headers gcc260Native cc1psx257 cpppsx257
        cc1psx260 cpppsx260 aspsxNative asmpsxNative;

      psy-k = pkgs.rustPlatform.buildRustPackage {
        pname = "psy-k";
        version = "0.4.0-git";
        src = psy-k-src;
        cargoHash = "sha256-Afpg/HxxpweZv07VN9zIkqMEohRYHqkRvgzBs9ABUp0=";
        # Upstream's proprietary fixture submodule is intentionally absent.
        doCheck = false;
      };



      gcc257Probe = import ./nix/gcc257.nix { inherit pkgs; };
      pcsxRedux = import ./nix/pcsx-redux.nix { inherit pkgs; };

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

      pylibclang = pkgs.python3Packages.buildPythonPackage {
        pname = "pylibclang";
        version = "318.1.1";
        format = "wheel";
        src = pkgs.fetchurl {
          url = "https://files.pythonhosted.org/packages/99/89/cd76c442cb55ae3767a61c8c1590255452aeb75a5e8b2d25be05b5aea177/pylibclang-318.1.1-cp313-cp313-manylinux_2_27_x86_64.manylinux_2_28_x86_64.whl";
          sha256 = "39f8b1f97bedd042d5c51348b28cea91311dc554d36ebdebd6761bd7b21301ac";
        };
        nativeBuildInputs = [ pkgs.autoPatchelfHook ];
        buildInputs = [ pkgs.stdenv.cc.cc.lib pkgs.zlib ];
        pythonImportsCheck = [ "pylibclang.cindex" ];
      };

      analysisPython = pkgs.python3.withPackages (pythonPackages: with pythonPackages; [
        capstone
        intervaltree
        libclang
        pylibclang
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
            --sdk-lib-dir "${psyqSdk}/release-2.5/isa board/PSXLIB/LIB" \
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
            --sdk-lib-dir "${psyqSdk}/release-2.5/isa board/PSXLIB/LIB" \
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
          mipsBinutilsAliases
          pkgs.git
          pkgs.dosbox-x
        ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          export PSYQ_ASPSX="${aspsxNative}/1.07/ASPSX.EXE"
          export PSYQ_INCLUDE="${psyqSdk}/release-2.5/isa board/PSXLIB/INCLUDE"
          export PSYQ_C_INCLUDE="${gcc257Headers}/include"
          cd "$repo"
          exec python3 -m scripts.kf.compile "$@"
        '';
      };

      emulatorCommand = mode: pkgs.writeShellApplication {
        name = "kf-run-${mode}";
        runtimeInputs = [ analysisPython pcsxRedux pkgs.git ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          export KINGS_FIELD_DIR="$repo"
          export PCSX_REDUX_BIN="${pcsxRedux}/bin/pcsx-redux"
          export PCSX_REDUX_BIOS="${pcsxRedux.openbios}"
          cd "$repo"
          exec python3 -m scripts.kf.emulator ${mode} "$@"
        '';
      };
      runRetail = emulatorCommand "retail";
      runCandidate = emulatorCommand "candidate";

      kfCli = pkgs.writeShellApplication {
        name = "kf";
        runtimeInputs = [
          analysisPython
          cc1psx260
          cpppsx260
          cc1psx257
          cpppsx257
          mipsBinutilsAliases
          objdiff-cli
          pkgs.dosbox-x
          pkgs.llvmPackages.clang-unwrapped
          pkgs.git
          pkgs.ninja
        ];
        text = ''
          repo="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
          export PSYQ_ASPSX="${aspsxNative}/1.07/ASPSX.EXE"
          export PSYQ_SDK="${psyqSdk}/release-2.5"
          export PSYQ_ASMPSX="${asmpsxNative}"
          export PSYQ_BIN="$PSYQ_SDK/isa board/PSXBIN/BIN"
          export PSYQ_INCLUDE="$PSYQ_SDK/isa board/PSXLIB/INCLUDE"
          export PSYQ_C_INCLUDE="${gcc257Headers}/include"
          export PSYQ_LIB="$PSYQ_SDK/isa board/PSXLIB/LIB"
          export PSYQ_H2000_LIB="$PSYQ_SDK/H2000/LIB2000"
          cd "$repo"
          exec python3 -m scripts.kf.cli "$@"
        '';
      };

      shell = pkgs.mkShell {
        name = "kings-field";
        packages = [
          psyqSdk
          psy-k
          pkgs.dosbox-x
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
          runRetail
          runCandidate
          kfCli
          ghidraWithPlugins
          objdiff-cli
          objdiff
          pcsxRedux
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
          unset PSYQ_DIR PSYQ_GCC241_DIR PSYQ_GCC260_RELEASE25_DIR
          unset PSYQ_GCC260_DISK_DIR PSYQ_GCC260_IMG
          export PSYQ_SDK="${psyqSdk}/release-2.5"
          export PSYQ_BIN="$PSYQ_SDK/isa board/PSXBIN/BIN"
          export PSYQ_INCLUDE="$PSYQ_SDK/isa board/PSXLIB/INCLUDE"
          export PSYQ_C_INCLUDE="${gcc257Headers}/include"
          export PSYQ_LIB="$PSYQ_SDK/isa board/PSXLIB/LIB"
          export PSYQ_H2000_LIB="$PSYQ_SDK/H2000/LIB2000"
          export PSYQ_COMPILER="$PSYQ_SDK/compiler"
          export PSYQ_ASPSX="${aspsxNative}/1.07/ASPSX.EXE"
          export KF_GCC260_NATIVE="${gcc260Native}"
          export PSYQ_ASMPSX="${asmpsxNative}"
          export KF_GCC257_NATIVE="${gcc257Native}"
          export GHIDRA_INSTALL_DIR="${pkgs.ghidra}/lib/ghidra"
          export NIX_GHIDRAHOME="${ghidraWithPlugins}/lib/ghidra/Ghidra"
          export GHIDRA_PSX_LOADER="${ghidraPsxLoader}/lib/ghidra/Ghidra/Extensions/ghidra_psx_ldr"
          export JAVA_HOME="${pkgs.jdk21}/lib/openjdk"
          export UV_PROJECT_ENVIRONMENT="$KINGS_FIELD_DIR/build/python-env"
          export UV_PYTHON="${pkgs.python311}/bin/python3.11"
          export PATH="${objdiffShim}/bin:$PATH"

          ${kfCli}/bin/kf clangd >&2 || echo "[kings-field] clangd setup failed; run kf clangd after fixing the reported error" >&2

          echo "[kings-field] SDK             : Psy-Q Release 2.5; one complete pinned media tree" >&2
          echo "[kings-field] SDK assembler   : ASPSX 1.07 hash-pinned; Release 2.5 copy remains key-locked" >&2
          echo "[kings-field] compiler probes : cc1psx-260/cpppsx-260 and cc1psx-257/cpppsx-257" >&2
          echo "[kings-field] analysis        : ghidra + PSX loader, pyghidra, psy-k, radare2, mipsel binutils" >&2
          echo "[kings-field] PSX emulator    : pcsx-redux ${pcsxRedux.version}" >&2
          echo "[kings-field] runtime checks  : kf-run-retail / kf-run-candidate" >&2
          echo "[kings-field] executable chain: source -> ASPSX -> PSYLINK -> CPE2X; run kf build" >&2
          echo "[kings-field] objdiff views   : native Psy-Q objects -> ELF reader" >&2
          echo "[kings-field] Python RE stack : run 'kf-python-sync' once, then 'splat ...'" >&2
          echo "[kings-field] retail census   : kf-retail-validate; kf-function-audit/propose; kf-fid-census; kf-vendored-seed" >&2
          echo "[kings-field] matching        : kf init/build/match/status/check/bank; objdiff GUI" >&2
          echo "[kings-field] objdiff project : $KINGS_FIELD_DIR/build/objdiff (psx/game/open; override with -p)" >&2
        '';
      };

      sdkBuilderTests = pkgs.runCommand "kings-field-sdk-builder-tests" {
        nativeBuildInputs = [
          analysisPython mipsBinutilsAliases psy-k objdiff-cli pkgs.dosbox-x pkgs.git
          cc1psx257 cpppsx257
          pkgs.llvmPackages.clang-unwrapped
        ];
        GHIDRA_PSX_LOADER = "${ghidraPsxLoader}/lib/ghidra/Ghidra/Extensions/ghidra_psx_ldr";
        PSYQ_SDK = "${psyqSdk}/release-2.5";
        PSYQ_LIB = "${psyqSdk}/release-2.5/isa board/PSXLIB/LIB";
        PSYQ_H2000_LIB = "${psyqSdk}/release-2.5/H2000/LIB2000";
        PSYQ_INCLUDE = "${psyqSdk}/release-2.5/isa board/PSXLIB/INCLUDE";
        PSYQ_C_INCLUDE = "${gcc257Headers}/include";
        PSYQ_BIN = "${psyqSdk}/release-2.5/isa board/PSXBIN/BIN";
        PSYQ_ASPSX = "${aspsxNative}/1.07/ASPSX.EXE";
        PSYQ_ASMPSX = "${asmpsxNative}";
      } ''
        mkdir project
        cp -r ${./scripts} project/scripts
        cp -r ${./tests} project/tests
        cp -r ${./config} project/config
        cp -r ${./include} project/include
        cp -r ${./src} project/src
        cp -r ${./vendor} project/vendor
        cd project
        python3 -m unittest discover -s tests -v
        touch "$out"
      '';

      gcc257TraceTests = pkgs.runCommand "kings-field-gcc257-trace-tests" {
        nativeBuildInputs = [ pkgs.dosbox-x
          analysisPython cc1psx257 cpppsx257 mipsBinutilsAliases
        ];

        PSYQ_ASPSX = "${aspsxNative}/1.07/ASPSX.EXE";
      } ''
        mkdir -p project/tests/fixtures project/config
        cp -r ${./scripts} project/scripts
        cp ${./config/native_reloc_referents.tsv} project/config/native_reloc_referents.tsv
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
        cp -r ${./vendor} project/vendor
        cd project
        python3 -m scripts.kf.retail config/retail
        python3 -m scripts.kf.inventory check --config-dir config/retail
        touch "$out"
      '';

      objdiffMipsTests = pkgs.runCommand "kings-field-objdiff-mips-tests" {
        nativeBuildInputs = [ pkgs.dosbox-x
          analysisPython
          crossBinutils
          mipsBinutilsAliases
          objdiff-cli
          cc1psx260
          cpppsx260
          cc1psx257
          cpppsx257
        ];

        PSYQ_ASPSX = "${aspsxNative}/1.07/ASPSX.EXE";
      } ''
        mkdir project
        cp -r ${./scripts} project/scripts
        mkdir project/config
        cp ${./config/native_reloc_referents.tsv} project/config/native_reloc_referents.tsv
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
        PSYQ_BIN = "${psyqSdk}/release-2.5/isa board/PSXBIN/BIN";
        PSYQ_LIB = "${psyqSdk}/release-2.5/isa board/PSXLIB/LIB";
        PSYQ_H2000_LIB = "${psyqSdk}/release-2.5/H2000/LIB2000";
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
        inherit psyqSdk psy-k gcc260Native cc1psx260 cpppsx260 gcc257Native cc1psx257 cpppsx257 mipsBinutilsAliases ghidraPsxLoader ghidraWithPlugins objdiff-cli objdiff retailValidate retailSeed functionAudit functionPropose vendoredSeed fidCensus retailDelink objdiffProject objdiffReport sourceCompile runRetail runCandidate kfCli;
        aspsx107 = aspsxNative;
        pcsx-redux = pcsxRedux;
        default = psyqSdk;
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
        sdk = psyqSdk;
        sdk-builder-tests = sdkBuilderTests;
        gcc257-trace = gcc257TraceTests;
        ghidra-psx-loader = ghidraPsxLoader;
        ghidra-psx-loader-discovery = ghidraPluginTests;
        retail-config = retailConfigTests;
        objdiff-mips = objdiffMipsTests;
        psylink-order = psylinkOrderTests;
        codecs = codecTests;
        pcsx-redux = pcsxRedux;
      };
    };
}
