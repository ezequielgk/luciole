#include "app/library_setup.h"

#include "core/application_identity.h"
#include "core/files/resource_paths.h"
#include "core/ui_strings.h"
#include "imaging/pixbuf_image_decoder.h"
#include "luciole_build_info.h"
#include "ui/palette_file.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace luciole {

  namespace {

    std::filesystem::path relocatedAssetsDir() {
      // The binary is built for a fixed prefix, but a release tarball may be unpacked
      // anywhere: <prefix>/bin/luciole then finds <prefix>/share/<name>/assets.
      std::error_code ec;
      const std::filesystem::path executable = std::filesystem::read_symlink("/proc/self/exe", ec);
      if (ec || executable.empty()) {
        return {};
      }
      return executable.parent_path().parent_path() / "share" / applicationIdentity().name / "assets";
    }

  } // namespace

  void initializeLibrary() {
    setApplicationIdentity({.name = "luciole", .envPrefix = "LUCIOLE"});
    std::filesystem::path assets = LUCIOLE_INSTALL_ASSETSDIR;
    const char* overridePath = std::getenv("LUCIOLE_ASSETS_DIR");
    if (overridePath != nullptr && overridePath[0] != '\0') {
      assets = overridePath;
    } else {
      std::error_code ec;
      if (!std::filesystem::is_directory(assets, ec)) {
        const std::filesystem::path relocated = relocatedAssetsDir();
        if (!relocated.empty() && std::filesystem::is_directory(relocated, ec)) {
          assets = relocated;
        }
      }
    }
    paths::setAssetRoots({.application = assets, .library = assets});
    setUiStrings({.keyName = {}, .selectPlaceholder = [] { return std::string("Select…"); }});
    installPixbufImageDecoder();
    // PaletteWatcher applies colors.toml once the main loop owner exists.
    setPalette(defaultPalette());
  }
} // namespace luciole
