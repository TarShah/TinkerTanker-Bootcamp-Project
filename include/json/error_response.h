#pragma once

#include "error.h"
#include <string>

namespace miningbots::json
{

  struct ErrorResponse
  {
    Error error;
    std::string error_message;
  };

} // namespace miningbots::json