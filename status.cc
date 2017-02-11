#include "status.h"

namespace cycling {

Status Status::OkStatus(const std::string& message) {
  return Status(Status::OK, message);
}

Status Status::FailureStatus(const std::string& message) {
  return Status(Status::FAILURE, message);
}

std::ostream& operator<<(std::ostream& out, const Status& status) {
  switch (status.error_code()) {
    case Status::OK:
      out << "OK";
      break;
    case Status::FAILURE:
      out << "FAILURE";
      break;
  }
  if (status.error_message().empty()) return out;
  return out << " - '" << status.error_message() << "'";
}

}  // namespace cycling
