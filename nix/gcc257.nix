{ pkgs }:
let
  recipeRevision = "b74211c9d959e9724802f3177c8229cd67202c87";
  recipeArchive = pkgs.fetchurl {
    name = "old-gcc-${recipeRevision}.tar.gz";
    url = "https://codeload.github.com/decompals/old-gcc/tar.gz/${recipeRevision}";
    hash = "sha256-De+MYfwfyABo2JUvsIWjxNu0Bp8fNi1wxpiWn8O5bMI=";
  };
  gccArchive = pkgs.fetchurl {
    url = "https://www.nic.funet.fi/index/gnu/funet/historical-funet-gnu-area-from-early-1990s/gcc-2.5.7.tar.gz";
    hash = "sha256-Y0W+QiNeXsTESNZYflUoPaPuV3YdtdINegpI1jmH/I4=";
  };
  # Match the release recipe's patch order, including its host portability
  # patches. This is a Decompals PSX target, not recovered Sony build source.
  source = pkgs.runCommand "gcc-2.5.7-psx-source-${recipeRevision}" {
    nativeBuildInputs = [ pkgs.gnutar pkgs.gzip pkgs.patch ];
  } ''
    mkdir recipe "$out"
    tar -xzf ${recipeArchive} -C recipe --strip-components=1
    tar -xzf ${gccArchive} -C "$out" --strip-components=1
    chmod -R u+w "$out"
    cd "$out"
    sed -i 's/include <varargs.h>/include <stdarg.h>/g' *.c
    for pair in \
      gvarargs.h:gvarargs-2.5.7.h.patch \
      sdbout.c:sdbout-2.6.0.c.patch \
      obstack.h:obstack-2.5.7.h.patch \
      collect2.c:collect2-2.6.0.c.patch \
      cccp.c:cccp-2.5.7.c.patch \
      gcc.c:gcc-2.5.7.c.patch \
      g++.c:g++-2.5.7.c.patch \
      config/mips/mips.h:mips-2.5.7.h.patch; do
      patch -u -p1 "''${pair%%:*}" -i "$NIX_BUILD_TOP/recipe/patches/''${pair#*:}"
    done
    patch -u -p1 < "$NIX_BUILD_TOP/recipe/patches/psx-2.5.7.patch"
    touch insn-config.h
  '';
  sourceFingerprint = traced: builtins.hashString "sha256" (builtins.toJSON {
    inherit recipeRevision;
    gcc = gccArchive.outputHash;
    recipe = recipeArchive.outputHash;
    target = "mips-sony-psx";
    instrumentation = if traced then {
      patch = builtins.readFile ../patches/gcc257-trace.patch;
      header = builtins.readFile ../tools/gcc257/kf-trace.h;
      logger = builtins.readFile ../tools/gcc257/kf-trace.c;
    } else null;
  });
  mkCompiler = traced: pkgs.pkgsi686Linux.stdenv.mkDerivation {
    pname = if traced then "cc1psx-257-trace" else "cc1psx-257-debug";
    version = "0.17-${recipeRevision}";
    src = source;
    patches = pkgs.lib.optionals traced [ ../patches/gcc257-trace.patch ];
    postPatch = pkgs.lib.optionalString traced ''
      cp --no-preserve=mode ${../tools/gcc257/kf-trace.h} kf-trace.h
      cp --no-preserve=mode ${../tools/gcc257/kf-trace.c} kf-trace.c
      echo '#define KF_COMPILER_SOURCE_SHA256 "${sourceFingerprint traced}"' >> kf-trace.h
    '';
    dontStrip = true;
    dontUpdateAutotoolsGnuConfigScripts = true;
    hardeningDisable = [ "all" ];
    dontAddStaticConfigureFlags = true;
    configurePhase = ''
      runHook preConfigure
      ./configure --target=mips-sony-psx --prefix="$out" \
        --with-endian-little --with-gnu-as \
        --host=i386-pc-linux --build=i386-pc-linux
      runHook postConfigure
    '';
    buildPhase = ''
      runHook preBuild
      gcc257_host_flags="-std=gnu89 -m32 -O0 -g3 -fcommon -fno-omit-frame-pointer -fdebug-prefix-map=$PWD=$out/share/gcc257/source -Dbsd4_4 -Dmips -march=i686 -DHAVE_STRERROR -Wno-error=implicit-function-declaration -Wno-error=implicit-int -Wno-error=incompatible-pointer-types -Wno-error=int-conversion"
      # The 1993 makefile omits some generated-header prerequisites. Complete
      # them first so parallel host compilation cannot race their creation.
      make -j "$NIX_BUILD_CORES" insn-config.h insn-codes.h insn-flags.h insn-attr.h \
        CFLAGS="$gcc257_host_flags"
      make -j "$NIX_BUILD_CORES" cc1 \
        CFLAGS="$gcc257_host_flags"
      runHook postBuild
    '';
    installPhase = ''
      runHook preInstall
      mkdir -p "$out/bin" "$out/share/gcc257"
      cp cc1 "$out/bin/cc1psx-257-${if traced then "trace" else "debug"}"
      mkdir "$out/share/gcc257/source"
      cp -r *.c *.h *.def config "$out/share/gcc257/source/"
      echo '${builtins.toJSON { inherit recipeRevision; compiler_source_sha256 = sourceFingerprint traced; }}' > "$out/share/gcc257/provenance.json"
      runHook postInstall
    '';
  };
in {
  inherit source recipeRevision;
  debug = mkCompiler false;
  instrumented = mkCompiler true;
}
