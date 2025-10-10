#pragma once

#include <memory>
#include <string>

#include <libpq-fe.h>

namespace SyncLayer::DB {

class DBConnection {
public:
    explicit DBConnection(const std::string& conninfo);
    ~DBConnection();

    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;

    DBConnection(DBConnection&&) noexcept;
    DBConnection& operator=(DBConnection&&) noexcept;

    bool isOpen() const;
    PGconn* raw();

private:
    PGconn* conn_ {nullptr};
};

} // namespace SyncLayer::DB


