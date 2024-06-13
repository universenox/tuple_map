{
  description = "Generic Modern C++ dev env";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    boostext-ut.url = "path:boostext-ut/";
    # boostext-mp.url = "path:boostext-mp/";
  };
  outputs = { nixpkgs, boostext-ut, ... }:
    let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
      llvmPackages = pkgs.llvmPackages_18;

      # comment this out to use default gcc one.
      stdenv = llvmPackages.libcxxStdenv;

      compileInputs = with pkgs; [ 
          cmake
          boost183
          boostext-ut.defaultPackage.x86_64-linux
          fmt
          ( gbenchmark.override{ stdenv=stdenv; })
          ( abseil-cpp.override{ stdenv=stdenv; })
      ];

      devShellPackages = with pkgs; [
          ninja

          clang-tools_18 # This clangd works. clangd in other package bad.
          llvmPackages.lldb
          llvmPackages.clang-manpages
          llvmPackages.llvm-manpages
          llvmPackages.bintools

          cmake-format
      ];
    in
    with pkgs;{
      # `nix build`
      defaultPackage.x86_64-linux = stdenv.mkDerivation {
          name = "tuple_map";
          src = lib.cleanSource ./.;
          nativeBuildInputs = [cmake] ++ compileInputs;

          installPhase = ''
            ls
            mkdir -p $out
            cp ./test $out
            cp ./bench $out
          '';
      };
      # `nix develop` shell
      devShells.x86_64-linux.default = 
      ( mkShell.override { stdenv=stdenv; }) 
      {
        name = "cppshell";
        packages = devShellPackages;
        buildInputs = compileInputs;

        CMAKE_EXPORT_COMPILE_COMMANDS=''1'';
        CMAKE_GENERATOR              =''Ninja'';
      };
    };
}
