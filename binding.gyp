{
  "targets": [
    {
      "target_name": "bzip2",
      "dependencies": [
        "src/addon/cpp_bzip2/bzip.gyp:lib"
      ],
      "sources": [
        "src/addon/main.cpp"
      ],
      "include_dirs" : [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "src/addon"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ]
    }
  ],
}