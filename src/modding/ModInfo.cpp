#include "poorcraft/modding/ModInfo.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/platform/DynamicLibrary.h"
#include "poorcraft/core/Logger.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace PoorCraft {

ModMetadata ModManifest::parseManifest(const std::string& manifestPath) {
    // Read file
    std::string content = Platform::read_file_text(manifestPath);
    if (content.empty()) {
        throw std::runtime_error("Failed to read manifest file: " + manifestPath);
    }

    ModMetadata metadata;

    // Extract fields using simple string parsing
    metadata.id = extractJsonString(content, "id");
    metadata.name = extractJsonString(content, "name");
    metadata.version = extractJsonString(content, "version");
    metadata.author = extractJsonString(content, "author");
    metadata.description = extractJsonString(content, "description");
    metadata.apiVersion = static_cast<uint32_t>(extractJsonInt(content, "apiVersion", 0));
    metadata.loadPriority = extractJsonInt(content, "loadPriority", 100);
    metadata.dependencies = extractJsonArray(content, "dependencies");

    std::string type = extractJsonString(content, "type");
    metadata.isNative = (type == "native");

    std::string entry = extractJsonString(content, "entry");
    
    // If entry lacks an extension, decorate it with platform-specific prefix/suffix
    std::string libraryName = entry;
    if (metadata.isNative && libraryName.find('.') == std::string::npos) {
        libraryName = DynamicLibrary::decorateLibraryName(libraryName);
    }
    
    // Construct library path (manifest directory + entry filename)
    std::string manifestDir = Platform::get_directory_name(manifestPath);
    metadata.libraryPath = Platform::join_path(manifestDir, libraryName);

    // Validate required fields
    if (metadata.id.empty() || metadata.name.empty() || metadata.version.empty() || 
        metadata.apiVersion == 0 || entry.empty()) {
        throw std::runtime_error("Missing required fields in manifest: " + manifestPath);
    }

    PC_INFO("Parsed mod manifest: {} v{} (API v{})", metadata.name, metadata.version, metadata.apiVersion);
    return metadata;
}

bool ModManifest::validateMetadata(const ModMetadata& metadata) {
    // Check ID is not empty and alphanumeric with underscores
    if (metadata.id.empty()) {
        PC_ERROR("Mod ID is empty");
        return false;
    }

    for (char c : metadata.id) {
        if (!std::isalnum(c) && c != '_') {
            PC_ERROR("Mod ID contains invalid characters: {}", metadata.id);
            return false;
        }
    }

    // Check version matches semantic version pattern (X.Y.Z)
    std::regex versionPattern(R"(^\d+\.\d+\.\d+$)");
    if (!std::regex_match(metadata.version, versionPattern)) {
        PC_ERROR("Invalid version format: {} (expected X.Y.Z)", metadata.version);
        return false;
    }

    // Check API version matches engine
    if (metadata.apiVersion != ENGINE_API_VERSION) {
        PC_ERROR("API version mismatch: mod requires {}, engine is {}", 
                 metadata.apiVersion, ENGINE_API_VERSION);
        return false;
    }

    return true;
}

std::string ModManifest::extractJsonString(const std::string& json, const std::string& key) {
    // Find key in JSON
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) {
        return "";
    }

    // Find colon after key
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) {
        return "";
    }

    // Find opening quote
    size_t openQuotePos = json.find('"', colonPos);
    if (openQuotePos == std::string::npos) {
        return "";
    }

    // Find closing quote
    size_t closeQuotePos = json.find('"', openQuotePos + 1);
    if (closeQuotePos == std::string::npos) {
        return "";
    }

    return json.substr(openQuotePos + 1, closeQuotePos - openQuotePos - 1);
}

int ModManifest::extractJsonInt(const std::string& json, const std::string& key, int defaultValue) {
    // Find key in JSON
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) {
        return defaultValue;
    }

    // Find colon after key
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) {
        return defaultValue;
    }

    // Skip whitespace
    size_t numStart = colonPos + 1;
    while (numStart < json.size() && std::isspace(json[numStart])) {
        numStart++;
    }

    // Extract number
    size_t numEnd = numStart;
    while (numEnd < json.size() && (std::isdigit(json[numEnd]) || json[numEnd] == '-')) {
        numEnd++;
    }

    if (numStart == numEnd) {
        return defaultValue;
    }

    try {
        return std::stoi(json.substr(numStart, numEnd - numStart));
    } catch (...) {
        return defaultValue;
    }
}

std::vector<std::string> ModManifest::extractJsonArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;

    // Find key in JSON
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) {
        return result;
    }

    // Find opening bracket
    size_t openBracketPos = json.find('[', keyPos);
    if (openBracketPos == std::string::npos) {
        return result;
    }

    // Find closing bracket
    size_t closeBracketPos = json.find(']', openBracketPos);
    if (closeBracketPos == std::string::npos) {
        return result;
    }

    // Extract array content
    std::string arrayContent = json.substr(openBracketPos + 1, closeBracketPos - openBracketPos - 1);

    // Parse array elements (simple comma-separated strings)
    size_t pos = 0;
    while (pos < arrayContent.size()) {
        // Find opening quote
        size_t openQuote = arrayContent.find('"', pos);
        if (openQuote == std::string::npos) {
            break;
        }

        // Find closing quote
        size_t closeQuote = arrayContent.find('"', openQuote + 1);
        if (closeQuote == std::string::npos) {
            break;
        }

        std::string element = arrayContent.substr(openQuote + 1, closeQuote - openQuote - 1);
        if (!element.empty()) {
            result.push_back(element);
        }

        pos = closeQuote + 1;
    }

    return result;
}

} // namespace PoorCraft
