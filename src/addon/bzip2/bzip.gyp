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
      ]
    }
  ]
}