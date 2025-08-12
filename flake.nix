{
  description = "A basic flake with a shell";
  inputs = {
    nixpkgs = {
      url = "github:NixOS/nixpkgs/nixos-25.05";
    };
    systems = {
      url = "github:nix-systems/default";
    };
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
  };

  outputs =
    { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            gcc15
            cmake
            pnpm
            pkg-config
            xorg.libXfixes
            xorg.libX11
            xorg.libXau
            xorg.libXdmcp
            nodejs_24
            corepack_latest
            fish
            zsh
          ];
          LD_LIBRARY_PATH = pkgs.lib.strings.makeLibraryPath (
            with pkgs;
            [
              # node-canvas
              libgbm
              librsvg
              giflib
              #electron/chromium
              glib
              nspr
              nss
              dbus
              at-spi2-atk
              cups
              cairo
              gtk3
              pango
            ]
            ++ (with pkgs.xorg; [
              libX11
            ])
          );
          shellHook = ''
            #shell config here
          '';
        };
      }
    );
}
