#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <mysql/mysql.h> // Assurez-vous d'avoir la bibliothèque MySQL installée

class SqlDb {
private:
    MYSQL* conn;

public:
    // Constructor to initialize the MySQL connection
    SqlDb(const std::string& host, const std::string& user, const std::string& password, const std::string& dbName) {
        // Initialize the MySQL connection object
        conn = mysql_init(nullptr);
        if (!conn) {
            // Throw an exception if initialization fails
            throw std::runtime_error("MySQL initialization failed");
        }

        // Attempt to establish a connection to the MySQL database
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), dbName.c_str(), 0, nullptr, 0)) {
            // Throw an exception if the connection fails, including the MySQL error message
            throw std::runtime_error("MySQL connection failed: " + std::string(mysql_error(conn)));
        }
    }

    ~SqlDb() {
        if (conn) {
            mysql_close(conn);
        }
    }

    void insertData(const std::vector<std::string>& fieldNames, const std::vector<std::string>& values, const std::string& tableName) {
        if (fieldNames.size() != values.size()) {
            throw std::invalid_argument("Field names and values size mismatch");
        }

        std::string query = "INSERT INTO " + tableName + " (";
        for (size_t i = 0; i < fieldNames.size(); ++i) {
            query += "`" + fieldNames[i] + "`";
            if (i < fieldNames.size() - 1) {
                query += ", ";
            }
        }
        query += ") VALUES (";
        for (size_t i = 0; i < values.size(); ++i) {
            query += "'" + escapeString(values[i]) + "'";
            if (i < values.size() - 1) {
                query += ", ";
            }
        }
        query += ");";

        if (mysql_query(conn, query.c_str())) {
            throw std::runtime_error("MySQL query failed: " + std::string(mysql_error(conn)));
        }
    }
    // Select data from the database
    std::vector<std::vector<std::string>> selectData(const std::string& query) {
        if (mysql_query(conn, query.c_str())) {
            throw std::runtime_error("MySQL selectet query failed: " + std::string(mysql_error(conn)));
        }

        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            throw std::runtime_error("MySQL store result failed: " + std::string(mysql_error(conn)));
        }

        std::vector<std::vector<std::string>> data;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            std::vector<std::string> rowData;
            for (unsigned int i = 0; i < mysql_num_fields(result); ++i) {
                rowData.push_back(row[i] ? row[i] : "NULL");
            }
            data.push_back(rowData);
        }

        mysql_free_result(result);
        return data;
    }

private:
    std::string escapeString(const std::string& value) {
        char* escaped = new char[value.length() * 2 + 1];
        mysql_real_escape_string(conn, escaped, value.c_str(), value.length());
        std::string result(escaped);
        delete[] escaped;
        return result;
    }
};