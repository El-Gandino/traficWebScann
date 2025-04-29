#ifndef JSON_H
#define JSON_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <map>
#include <variant>
#include <vector>
#include <memory>

// Définir un type générique pour représenter les valeurs JSON
class JsonValue; // Déclaration anticipée

using JsonObject = std::map<std::string, std::shared_ptr<JsonValue>>;
using JsonArray = std::vector<std::shared_ptr<JsonValue>>;
using JsonValueVariant = std::variant<std::string, JsonObject, JsonArray>;

class JsonValue {
public:
    JsonValue() = default;
    explicit JsonValue(const std::string& value) : value(value) {}
    explicit JsonValue(const JsonObject& object) : value(object) {}
    explicit JsonValue(const JsonArray& array) : value(array) {}

    JsonValueVariant value;
};

class JsonHandler {
public:
    // Lit un fichier JSON et le retourne sous forme de JsonValue
    static std::shared_ptr<JsonValue> readJsonFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filePath);
        }

        std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        size_t index = 0;
        return parseJson(jsonContent, index);
    }

private:
    // Fonction pour analyser une chaîne JSON
    static std::shared_ptr<JsonValue> parseJson(const std::string& json, size_t& index) {
        skipWhitespace(json, index);
        if (json[index] == '{') {
            return std::make_shared<JsonValue>(parseObject(json, index));
        } else if (json[index] == '[') {
            return std::make_shared<JsonValue>(parseArray(json, index));
        } else if (json[index] == '"') {
            return std::make_shared<JsonValue>(parseString(json, index));
        } else if (json.compare(index, 4, "true") == 0) {
            index += 4; // Skip "true"
            return std::make_shared<JsonValue>("true");
        } else if (json.compare(index, 5, "false") == 0) {
            index += 5; // Skip "false"
            return std::make_shared<JsonValue>("false");
        } else if (std::isdigit(json[index]) || json[index] == '-') {
            return std::make_shared<JsonValue>(parseNumber(json, index));
        } else {
            throw std::runtime_error("Invalid JSON format at index " + std::to_string(index));
        }
    }

    // Fonction pour analyser un objet JSON
    static JsonObject parseObject(const std::string& json, size_t& index) {
        JsonObject object;
        ++index; // Skip '{'

        while (true) {
            skipWhitespace(json, index);

            if (json[index] == '}') {
                ++index; // Skip '}'
                break;
            }

            std::string key = parseString(json, index);
            skipWhitespace(json, index);

            if (json[index] != ':') {
                throw std::runtime_error("Expected ':' after key in JSON object at index " + std::to_string(index));
            }
            ++index; // Skip ':'

            object[key] = parseJson(json, index);

            skipWhitespace(json, index);

            if (json[index] == ',') {
                ++index; // Skip ','
                skipWhitespace(json, index);
                if (json[index] == '}') {
                    throw std::runtime_error("Trailing comma in JSON object at index " + std::to_string(index));
                }
            } else if (json[index] == '}') {
                continue;
            } else {
                throw std::runtime_error("Expected ',' or '}' in JSON object at index " + std::to_string(index));
            }
        }

        return object;
    }

    // Fonction pour analyser un tableau JSON
    static JsonArray parseArray(const std::string& json, size_t& index) {
        JsonArray array;
        ++index; // Skip '['

        while (true) {
            skipWhitespace(json, index);

            if (json[index] == ']') {
                ++index; // Skip ']'
                break;
            }

            array.push_back(parseJson(json, index));

            skipWhitespace(json, index);

            if (json[index] == ',') {
                ++index; // Skip ','
                skipWhitespace(json, index);
                if (json[index] == ']') {
                    throw std::runtime_error("Trailing comma in JSON array at index " + std::to_string(index));
                }
            } else if (json[index] == ']') {
                continue;
            } else {
                throw std::runtime_error("Expected ',' or ']' in JSON array at index " + std::to_string(index));
            }
        }

        return array;
    }

    // Fonction pour analyser une chaîne JSON
    static std::string parseString(const std::string& json, size_t& index) {
        if (json[index] != '"') {
            throw std::runtime_error("Expected '\"' at index " + std::to_string(index));
        }
        ++index; // Skip '"'

        std::string result;
        while (json[index] != '"') {
            if (json[index] == '\\') {
                ++index; // Skip '\'
                if (json[index] == '"') {
                    result += '"';
                } else {
                    result += '\\';
                }
            } else {
                result += json[index];
            }
            ++index;
        }
        ++index; // Skip closing '"'

        return result;
    }

    static std::string parseNumber(const std::string& json, size_t& index) {
        size_t start = index;
        if (json[index] == '-') {
            ++index; // Skip '-'
        }
        while (index < json.size() && (std::isdigit(json[index]) || json[index] == '.')) {
            ++index;
        }
        return json.substr(start, index - start);
    }

    // Fonction pour ignorer les espaces blancs
    static void skipWhitespace(const std::string& json, size_t& index) {
        while (index < json.size() && (json[index] == ' ' || json[index] == '\t' || json[index] == '\n' || json[index] == '\r')) {
            ++index;
        }
    }
};

#endif // JSON_H