// Copyright 2026 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   https://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

// Initializes the TZDIR environment variable before main() when running under
// Bazel/Blaze test runners so that tests load hermetic zoneinfo files from
// testdata/zoneinfo rather than the host system's /usr/share/zoneinfo.

#include <cstdlib>
#include <fstream>
#include <string>

#if !defined(_WIN32)
#include <stdlib.h>
#endif

namespace cctz {

namespace {

bool GetEnv(const char* name, std::string* out) {
#if defined(_MSC_VER)
  char* val = nullptr;
  if (_dupenv_s(&val, nullptr, name) != 0 || val == nullptr) return false;
  if (out != nullptr) *out = val;
  std::free(val);
  return true;
#else
  const char* val = std::getenv(name);
  if (val == nullptr) return false;
  if (out != nullptr) *out = val;
  return true;
#endif
}

void InitZoneInfoEnv() {
  // Honor any explicit TZDIR (e.g., from `bazel test --test_env=TZDIR=...`).
  if (GetEnv("TZDIR", nullptr)) return;

  // Bazel sets TEST_SRCDIR to the root of the runfiles tree for `bazel test`.
  std::string srcdir;
  if (!GetEnv("TEST_SRCDIR", &srcdir) || srcdir.empty()) return;

  const std::string zoneinfo_dir = "testdata/zoneinfo";

  // Bazel sets TEST_WORKSPACE to the workspace directory name under
  // TEST_SRCDIR (e.g., "_main" with Bzlmod). Internal Blaze does not set
  // TEST_WORKSPACE, but starts tests with the working directory at the
  // workspace root inside TEST_SRCDIR, where a relative path works directly.
  std::string workspace;
  std::string tzdir =
      (GetEnv("TEST_WORKSPACE", &workspace) && !workspace.empty())
          ? srcdir + "/" + workspace + "/" + zoneinfo_dir
          : zoneinfo_dir;

  // On Windows, Bazel disables runfiles symlink trees by default and instead
  // sets RUNFILES_MANIFEST_ONLY=1 and RUNFILES_MANIFEST_FILE, which maps
  // space-delimited "<workspace>/<path> <real_path>" entries. Look up the UTC
  // zoneinfo file in the manifest and strip "/UTC" to get its directory.
  std::string manifest_only;
  if (GetEnv("RUNFILES_MANIFEST_ONLY", &manifest_only) &&
      manifest_only == "1") {
    tzdir.clear();
    std::string manifest_path;
    if (workspace.empty() ||
        !GetEnv("RUNFILES_MANIFEST_FILE", &manifest_path)) {
      return;
    }
    const std::string key = workspace + "/" + zoneinfo_dir + "/UTC ";
    const std::string suffix = "/UTC";
    std::ifstream manifest(manifest_path);
    for (std::string line; std::getline(manifest, line);) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (line.size() > key.size() + suffix.size() &&
          line.compare(0, key.size(), key) == 0 &&
          line.compare(line.size() - suffix.size(), suffix.size(), suffix) ==
              0) {
        tzdir =
            line.substr(key.size(), line.size() - key.size() - suffix.size());
        break;
      }
    }
    if (tzdir.empty()) return;
  }

#if defined(_WIN32)
  (void)_putenv_s("TZDIR", tzdir.c_str());
#else
  (void)::setenv("TZDIR", tzdir.c_str(), 0);
#endif
}

struct ZoneInfoEnvInitializer {
  ZoneInfoEnvInitializer() { InitZoneInfoEnv(); }
} zone_info_env_initializer;

}  // namespace

}  // namespace cctz
