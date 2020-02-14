{
  "targets": [
    {
      "target_name": "fontmanager",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "src/FontManager.cc" ],
      "include_dirs" : [
        "<!@(node -p \"require('node-addon-api').include\")",
      ],
      "conditions": [
        ['OS=="mac"', {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
          },
          "sources": ["src/FontManagerMac.mm"],
          "link_settings": {
            "libraries": ["CoreText.framework", "Foundation.framework"]
          }
        }],
        ['OS=="win"', {
          "sources": ["src/FontManagerWindows.cc"],
          "link_settings": {
            "libraries": ["Dwrite.lib"]
          }
        }],
        ['OS=="linux"', {
          "sources": ["src/FontManagerLinux.cc"],
          "link_settings": {
            "libraries": ["-lfontconfig"]
          }
        }]
      ]
    }
  ]
}
