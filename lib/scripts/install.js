const { readdirSync } = require("node:fs");
const { spawnSync } = require("node:child_process");
const { join, basename } = require("node:path");

/*
 * Patch the binary with the correct path of the libraries
 *
 * @param {string} binary - The binary to patch
 * @param {string[]} libraries - The libraries to add to the binary
 * */
function patchelf(binary, libraries) {
  const patchelf = join(__dirname, "patchelf");
  for (const library of libraries) {
    const lib = join(__dirname, "..", "deps", library);
    const args = ["--add-needed", `$ORIGIN/../deps/${basename(lib)}`, binary];
    spawnSync(patchelf, args);
  }
}

/*
 * Get the libraries from the deps folder
 *
 * @returns {string[]} - The libraries
 * */
function getLibraries() {
  const libraries = [];
  const files = readdirSync(join(__dirname, "..", "prebuilds", "deps"));
  for (const file of files) if (file.endsWith(".so")) libraries.push(file);

  return libraries;
}

/*
 * Get every .node file
 *
 * @returns {string[]} - The .node file paths
 * */
function getNodeFiles() {
  // Recursively search every file in the ../prebuilds/ folder
  // and return the .node files
  const files = [];
  const search = (dir) => {
    const filesInDir = readdirSync(dir);
    for (const file of filesInDir) {
      const path = join(dir, file);
      if (file.endsWith(".node")) files.push(path);
      else if (file !== "deps") search(path);
    }
  };
  search(join(__dirname, "..", "prebuilds"));

  return files;
}
function runNodeGypBuild() {
  spawnSync("node-gyp-build");
}

function main() {
  const libraries = getLibraries();
  const binaries = getNodeFiles();
  // TODO: Not every architecture will need this, I'll have to think better
  // this later
  patchelf(binaries[0], libraries);
  runNodeGypBuild();
}

main();
