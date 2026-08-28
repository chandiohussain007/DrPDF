#pragma once

#include <string>
#include <utility>
#include <variant>

namespace drpdf::core {

struct Error {
    std::string message;
};

template <typename T>
class Result {
public:
    Result(T value) : data_(std::move(value)) {}
    Result(Error error) : data_(std::move(error)) {}

    bool ok() const { return std::holds_alternative<T>(data_); }
    explicit operator bool() const { return ok(); }

    const T& value() const { return std::get<T>(data_); }
    T& value() { return std::get<T>(data_); }
    T take() { return std::get<T>(std::move(data_)); }

    const std::string& error() const { return std::get<Error>(data_).message; }

private:
    std::variant<T, Error> data_;
};

template <>
class Result<void> {
public:
    Result() = default;
    Result(Error error) : error_(std::move(error.message)) {}

    bool ok() const { return error_.empty(); }
    explicit operator bool() const { return ok(); }
    const std::string& error() const { return error_; }

private:
    std::string error_;
};

} // namespace drpdf::core
