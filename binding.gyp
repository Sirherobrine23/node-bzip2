{
  "targets": [
    {
      "target_name": "bzip2",
      "sources": [
        "src/addon/bzip2/blocksort.c",
        "src/addon/bzip2/huffman.c",
        "src/addon/bzip2/crctable.c",
        "src/addon/bzip2/randtable.c",
        "src/addon/bzip2/compress.c",
        "src/addon/bzip2/decompress.c",
        "src/addon/bzip2/bzlib.c",
        "src/addon/main.cpp"
      ],
      "include_dirs" : [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "src/addon"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "conditions": [
        [
          "OS=='linux'", {
            "cflags": [ "-fpermissive" ],
          }
        ]
      ]
    }
  ],
}