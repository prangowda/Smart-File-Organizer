#include <iostream>
#include <filesystem>
#include <map>
#include <thread>
#include <mutex>
#include <regex>

namespace fs = std::filesystem;

// Mutex for thread-safe logging
std::mutex log_mutex;

// Function to categorize files based on extension
std::string categorizeFile(const fs::path& file) {
    static const std::map<std::string, std::string> extensionMap = {
        {".txt", "Documents"},
        {".docx", "Documents"},
        {".pdf", "Documents"},
        {".jpg", "Images"},
        {".png", "Images"},
        {".mp4", "Videos"},
        {".mp3", "Music"},
        {".log", "Logs"}
    };

    auto ext = file.extension().string();
    auto it = extensionMap.find(ext);
    return it != extensionMap.end() ? it->second : "Others";
}

// Function to move files into categorized folders
void organizeFile(const fs::path& file, const fs::path& targetDir) {
    try {
        std::string category = categorizeFile(file);
        fs::path categoryDir = targetDir / category;
        fs::create_directories(categoryDir);
        fs::path targetPath = categoryDir / file.filename();

        {
            std::lock_guard<std::mutex> lock(log_mutex);
            std::cout << "Moving: " << file << " -> " << targetPath << std::endl;
        }

        fs::rename(file, targetPath);
    } catch (const fs::filesystem_error& e) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Function to scan a directory recursively and organize files
void organizeDirectory(const fs::path& dir, const fs::path& targetDir) {
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            organizeFile(entry.path(), targetDir);
        }
    }
}

int main() {
    std::cout << "=== Smart File Organizer ===\n";

    // Input source and target directories
    std::string sourceDir, targetDir;
    std::cout << "Enter the source directory: ";
    std::getline(std::cin, sourceDir);
    std::cout << "Enter the target directory: ";
    std::getline(std::cin, targetDir);

    if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir)) {
        std::cerr << "Error: Source directory does not exist or is not a directory.\n";
        return 1;
    }

    try {
        std::cout << "Organizing files...\n";

        // Multi-threaded directory organization
        std::thread organizer(organizeDirectory, sourceDir, targetDir);
        organizer.join();

        std::cout << "Organization complete!\n";
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << "\n";
    }

    return 0;
}
