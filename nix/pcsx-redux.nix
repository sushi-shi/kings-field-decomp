{ pkgs }:

let
  version = "b745534e";
  archive = pkgs.fetchurl {
    name = "PCSX-Redux-${version}-linux-x86_64.zip";
    url = "https://distrib.app/storage/assets/885/c95/a89/b75c1ea0e170c518a45153645887f6ec0c79ba3dbe87e991b40f735/PCSX-Redux-${version}-linux-x86_64.zip";
    hash = "sha256-g4QYdjFNm4kzEmlZZugqW+o6XKOMnhLi6zx2O3ZWyok=";
  };

  appImage = pkgs.runCommand "pcsx-redux-${version}-appimage" {
    nativeBuildInputs = [ pkgs.unzip ];
  } ''
    mkdir -p "$out"
    unzip -q ${archive} -d "$out"
  '';

  appImageArgs = {
    pname = "pcsx-redux";
    inherit version;
    src = "${appImage}/PCSX-Redux-HEAD-x86_64.AppImage";
  };
  contents = pkgs.appimageTools.extractType2 appImageArgs;
in
pkgs.appimageTools.wrapType2 (appImageArgs // {
  passthru = {
    inherit contents;
    openbios = "${contents}/usr/share/pcsx-redux/resources/openbios.bin";
  };
  meta = {
    description = "Development-oriented PlayStation emulator";
    homepage = "https://pcsx-redux.consoledev.net/";
    license = pkgs.lib.licenses.gpl2Plus;
    mainProgram = "pcsx-redux";
    platforms = [ "x86_64-linux" ];
    sourceProvenance = [ pkgs.lib.sourceTypes.binaryNativeCode ];
  };
})
