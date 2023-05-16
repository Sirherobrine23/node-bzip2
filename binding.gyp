{
  "target_defaults": {
    "include_dirs" : [
      "<!(node -p \"require('node-addon-api').include_dir\")",
      "src/bzip2"
    ],
    "defines": [
      "NAPI_DISABLE_CPP_EXCEPTIONS"
    ],
    "sources": [
      "src/bzip2/blocksort.c",
      "src/bzip2/huffman.c",
      "src/bzip2/crctable.c",
      "src/bzip2/randtable.c",
      "src/bzip2/compress.c",
      "src/bzip2/decompress.c",
      "src/bzip2/bzlib.c"
    ]
  },
  "targets": [
    {
      "target_name": "compress",
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "sources": [
        "src/compress.cpp"
      ]
    },
    {
      "target_name": "descompress",
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "sources": [
        "src/descompress.cpp"
      ]
    }
  ],
}