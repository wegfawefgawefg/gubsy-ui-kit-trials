#pragma once

#include "model.hpp"

// What is: A stable key for one independently authored trial screen.
std::string authoring_context(const TrialModel& model);

// What is: A runtime view with authored structure and fresh host-owned content.
gview::View merge_authored_view(const gview::View& authored, const gview::View& generated);
