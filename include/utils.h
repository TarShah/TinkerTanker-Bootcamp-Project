#pragma once
#include <cpptrace/cpptrace.hpp>
#include <glaze/glaze.hpp>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>
#include <json/error.h>
#include <json/error_response.h>
#include <utility>
template <typename T, typename BufferT>
static T ReadJson(BufferT json_buffer)
{
  T result;
  auto json_error = glz::read_json(result, json_buffer);
  if (json_error)
  {
    auto buffer = fmt::format("JSON Parsing Error: {}\nBuffer:", json_buffer);
    // SPDLOG_ERROR(buffer);
    auto error_message = fmt::format("JSON Parsing Error: {}\nBuffer: {}", json_buffer,
                                     glz::format_error(json_error, json_buffer));
    // cpptrace::generate_trace().print();
    SPDLOG_ERROR(error_message);
    // throw std::runtime_error(error_message);
    // Need to do more error handling based on the type of error
    // if the type of result is JobId, then return 0 i.e. The job was not accepted
    if constexpr (std::is_same_v<T, JobId>)
    {
      SPDLOG_ERROR(error_message);
      std::string err_message = glz::format_error(json_error, json_buffer);
      if (error_message.find("kNotEnoughResources") != std::string::npos or error_message.find("kResourceIdNotFound") != std::string::npos)
      {
          return 2;
      }
      return 0;
    }
    else{
      // SPDLOG_ERROR(error_message);
    }
  }
  return result;
}

// template <typename T, typename ParseError>
// static std::pair<T, ParseError> ReadJsonWithError(std::string_view json_buffer)
// {
//   T result;
//   ParseError parse_error = glz::read_json(result, json_buffer);
//   if(parse_error != miningbots::json::Error::kSuccess){
//       SPDLOG_ERROR(error_message);
//   }
//   return std::make_pair(result, parse_error);
// }

static std::string ReadFile(std::string file_name)
{
  std::ifstream infile(file_name);
  if (infile.fail())
  {
    auto error_message = fmt::format("Error reading file: \"{}\" ", file_name);
    SPDLOG_ERROR(error_message);
    throw std::runtime_error(error_message);
  }
  std::ostringstream str_stream;
  str_stream << infile.rdbuf();
  return str_stream.str();
}

// ryml::Tree ReadYaml(std::string file_name);