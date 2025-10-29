#include "config.h"
#include "util.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

KeyMapping default_mapping() {
    KeyMapping map;
    const KeyCode h = token_to_keycode("H");
    const KeyCode j = token_to_keycode("J");
    const KeyCode k = token_to_keycode("K");
    const KeyCode l = token_to_keycode("L");
    const KeyCode left = token_to_keycode("Left");
    const KeyCode down = token_to_keycode("Down");
    const KeyCode up = token_to_keycode("Up");
    const KeyCode right = token_to_keycode("Right");
    if (h && left) map[h] = left;
    if (j && down) map[j] = down;
    if (k && up) map[k] = up;
    if (l && right) map[l] = right;
    return map;
}

void ensure_default_config(const std::filesystem::path& path) {
    if (std::filesystem::exists(path)) return;
    std::ofstream out(path);
    if (!out) return;
    out << "# CapsUnlocked config\n";
    out << "# Map source=target using key names or hex codes.\n";
    out << "# Hold CapsLock to activate.\n\n";
    out << "h=Left\n";
    out << "j=Down\n";
    out << "k=Up\n";
    out << "l=Right\n";
}

} // namespace

KeyMapping load_config() {
    const std::filesystem::path dir = get_exe_dir();
    const std::filesystem::path cfg_path = dir / "capsunlocked.ini";
    ensure_default_config(cfg_path);

    KeyMapping map;
    std::ifstream in(cfg_path);
    if (!in) {
        return default_mapping();
    }

    std::string line;
    while (std::getline(in, line)) {
        trim(line);
        if (line.empty()) continue;
        const char first = line.front();
        if (first == '#' || first == ';') continue;
        const std::size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string left = line.substr(0, eq);
        std::string right = line.substr(eq + 1);
        trim(left);
        trim(right);
        if (left.empty() || right.empty()) continue;
        const KeyCode src = token_to_keycode(left);
        const KeyCode dst = token_to_keycode(right);
        if (src != 0 && dst != 0) {
            map[src] = dst;
        }
    }

    if (map.empty()) {
        map = default_mapping();
    }
    return map;
}
