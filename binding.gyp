{
  "variables": {
    "compiler_checks": [
      "-Wall",
      "-Wextra",
      "-Weffc++",
      "-Wconversion",
      "-pedantic-errors",
      "-Wconversion",
      "-Wshadow",
      "-Wfloat-equal",
      "-Wuninitialized",
      "-Wunreachable-code",
      "-Wold-style-cast",
      "-Wno-error=unused-variable",
      "-fno-exceptions"
    ],
    "source_files": [ "<!@(find addon/src -name *.cpp)" ],
    "dependencies_include_dirs": [ "<!@(./scripts/deps.sh --emit-include-dirs)" ],
  },
  "targets": [
    {
      "target_name": "binding",
      "cflags_cc!": [ "<@(compiler_checks)" ],
      "sources": [ "<@(source_files)" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<(module_root_dir)/addon/include",
        "<@(dependencies_include_dirs)",
        "/usr/local/include/"
      ],
      "libraries": [ "<!@(./scripts/deps.sh --emit-libraries)" ]
    }
  ]
}
