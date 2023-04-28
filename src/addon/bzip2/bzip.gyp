{
  "targets": [
    {
      "target_name": "lib",
      "type": "static_library",
      "standlone_static_library": 1,
      "defines": [],
      "sources" : [
        "blocksort.c",
        "huffman.c",
        "crctable.c",
        "randtable.c",
        "compress.c",
        "decompress.c",
        "bzlib.c",
        "bzip2recover.c"
      ],
      "cflags": [
        "-Wall",
        "-Winline",
        "-O2",
        "-g",
        "-D_FILE_OFFSET_BITS=64"
      ],
      "conditions": [
        [
          "OS == 'mac'", {
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "MACOSX_DEPLOYMENT_TARGET": "10.11"
            }
          }
        ]
      ]
    }
  ]
}