#ifndef __STATUS_H__
#define __STATUS_H__

#include <iostream>
#include <string>

namespace cycling {

class Status {
 public:
  enum StatusCode {
    OK,
    FAILURE,
  };

  static Status OkStatus(const std::string& message = "");
  static Status FailureStatus(const std::string& message);

  Status() : Status(OK, "") {}
  Status(const Status& rhs) = default;
  Status(Status&& rhs) = default;
  Status& operator=(const Status& rhs) = default;
  Status& operator=(Status&& rhs) = default;
  ~Status() = default;

  bool ok() const { return code_ == OK; }
  StatusCode error_code() const { return code_; }
  const std::string& error_message() const { return message_; }

 private:
  Status(const StatusCode error_code, const std::string& error_message)
      : code_(error_code), message_(error_message) {}

  StatusCode code_;
  std::string message_;
};

std::ostream& operator<<(std::ostream& out, const Status& status);

#define RETURN_IF_ERROR(expr)                                               \
  {                                                                         \
    Status status = (expr);                                                 \
    if (!status.ok()) {                                                     \
      return Status::FailureStatus(StrCat(__FILE__, ":", __FUNCTION__, ":", \
                                          __LINE__, "\n",                   \
                                          status.error_message()));         \
    }                                                                       \
  }

}  // namespace cycling

#endif
