#include "native_assets.h"

#include <RmlUi/Core.h>

#include <cstdio>
#include <string>

namespace {

void load_font(const char *filename, bool fallback = false) {
  // load one bundled font face
  const std::string path =
      std::string(GUBSY_UI_BUILD_ASSET_DIR) + "/" + filename;
  if (!Rml::LoadFontFace(path, fallback))
    std::fprintf(stderr, "Could not load font: %s\n", path.c_str());
}

} // namespace

void load_trial_fonts() {
  // register primary and fallback faces
  load_font("LatoLatin-Regular.ttf");
  load_font("LatoLatin-Bold.ttf");
  load_font("DejaVuSans.ttf", true);
}
