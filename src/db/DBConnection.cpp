#include "db/DBConnection.hpp"
#include "exceptions.hpp"
#include <libpq-fe.h>

namespace SyncLayer::DB {

DBConnection::DBConnection(const std::string& conninfo)
{
    conn_ = PQconnectdb(conninfo.c_str());
    if (PQstatus(conn_) != CONNECTION_OK) {
        std::string err = PQerrorMessage(conn_);
        PQfinish(conn_);
        conn_ = nullptr;
        throw SyncLayer::Exception::DatabaseError(err);
    }
}

DBConnection::~DBConnection()
{
    if (conn_) PQfinish(conn_);
}

DBConnection::DBConnection(DBConnection&& other) noexcept : conn_(other.conn_) {
    other.conn_ = nullptr;
}

DBConnection& DBConnection::operator=(DBConnection&& other) noexcept {
    if (this != &other) {
        if (conn_) PQfinish(conn_);
        conn_ = other.conn_;
        other.conn_ = nullptr;
    }
    return *this;
}

bool DBConnection::isOpen() const { return conn_ != nullptr; }
PGconn* DBConnection::raw() { return conn_; }

} // namespace SyncLayer::DB


