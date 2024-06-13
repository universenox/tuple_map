{
  description = "boost-ext unit testing framework";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    src = {
      url = "github:boost-ext/ut/master";
      flake = false;
    };
  };
  outputs = { src, nixpkgs, ... }:
    let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in {
      defaultPackage.x86_64-linux = 
         pkgs.stdenv.mkDerivation {
          name = "boostext-ut";
          src = src;
          nativeBuildInputs = [ pkgs.cmake ];
          cmakeFlags = [
            "-DBOOST_UT_ALLOW_CPM_USE=OFF"
          ];
      };
    };
}
