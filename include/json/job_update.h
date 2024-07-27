#pragma once

#include "../base_types.h"
#include "action.h"
#include "job_status.h"
#include "position.h"
#include "json/resource_chunk.h"

namespace miningbots::json
{

  struct JobUpdate
  {
    Id id{0}; // todo rename to job id
    Action action{Action::kNoAction};
    JobStatus status{JobStatus::kNotStarted};
    Position target_position{0, 0};
    Id target_id{0};
    ResourceChunk target_chunk{};

    TimeStamp_t job_start_time{0};
    TimeStamp_t time_when_checkpoint{0};
  };

} // namespace miningbots::json