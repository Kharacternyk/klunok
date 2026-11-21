{
  description = "File versioning and automatic backup daemon";
  outputs = inputs: import ./nix inputs;
  inputs.nixpkgs.url = "nixpkgs";
}
