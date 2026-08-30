#pragma once

#include "model.hpp"

// What is: A stable key for one independently authored trial screen.
std::string authoring_context(const TrialModel& model);

// What is: One-time source migrations applied to older full-view documents.
void migrate_authored_view(gview::View& authored, const gview::View& generated);

// What is: Resolution-independent rebasing for authored pixel metrics.
void adapt_authored_view_resolution(gview::View& authored, const gview::View& generated);

// What is: A runtime view with authored structure and fresh host-owned content.
gview::View merge_authored_view(const gview::View& authored, const gview::View& generated);
