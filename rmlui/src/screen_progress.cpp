#include "app.h"
#include "ui_markup.h"

#include <sstream>

std::string GubsyApp::BuildProgress() const {
  // define game-provided campaigns
  struct Campaign {
    const char *id;
    const char *name;
    const char *quest;
    const char *time;
    const char *state;
  };
  constexpr Campaign campaigns[]{
      {"The Glass Caverns", "The Glass Caverns", "Moss · Temple Depths · 42%",
       "12h 48m", "READY"},
      {"A Quiet Beginning", "A Quiet Beginning", "Vega · Green Valley · 11%",
       "3h 06m", "READY"},
      {"Old Expedition", "Old Expedition", "Moss · Version 0.7 data", "8h 19m",
       "INCOMPATIBLE"}};
  // build campaign selector and checkpoint detail
  std::ostringstream out;
  out << R"(<div class="toolbar"><span class="toolbar-title"><small>GAME-PROVIDED PROGRESSION</small>3 campaigns · automatic checkpoints</span><button data-focus data-action="new-campaign" class="button primary">+ Choose a new quest</button></div><div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body"><small>CAMPAIGNS</small>)";
  for (const Campaign &campaign : campaigns)
    out << R"(<button data-focus data-action="campaign-)" << campaign.id
        << R"(" class=")"
        << markup::selected_class(state_.selected_campaign == campaign.name,
                          "campaign-row")
        << R"("><img class="quest-art" src="../splonks-title.png"/><span><strong>)"
        << campaign.name << R"(</strong><small>)" << campaign.quest
        << R"(</small></span><em>)" << campaign.time << R"(</em><b>)"
        << campaign.state << R"(</b></button>)";
  out << R"(</div>)";
  out << R"(</section><aside class="panel detail progress-detail"><small>CAMPAIGN · PLAY</small><h2>)"
      << state_.selected_campaign
      << R"(</h2><div class="owner-banner"><b>MO</b><span><strong>ASSOCIATED PROFILE</strong><small>Moss · ownership is recorded by this progression provider</small></span></div><div class="impact"><h3>RECORDED MOD SET · 7 PACKAGES</h3><div class="kv"><span>Base Content</span><b>v1.4.0</b></div><div class="kv"><span>Cartographer's Desk</span><b>v0.8.2</b></div><div class="kv"><span>Old Lanterns</span><b>Update available</b></div><div class="kv"><span>Manifest hash</span><b>8cc91a</b></div><button data-focus data-action="inspect-manifest" class="button">Inspect exact package set</button></div><div class="checkpoint-history"><h3>CHECKPOINT HISTORY</h3><button data-focus data-action="checkpoint-temple"><span><strong>Temple safe room</strong><small>Today, 06:52 · Stage 4</small></span><b>RESUME POINT</b></button><button data-focus data-action="checkpoint-flooded"><span><strong>Flooded archive</strong><small>Yesterday · Stage 3</small></span><b>BACKUP</b></button><button data-focus data-action="checkpoint-crossing"><span><strong>Mushroom crossing</strong><small>Aug 24 · Stage 2</small></span><b>BACKUP</b></button></div><div class="actions"><button data-focus data-action="resume-campaign" class="button primary">▶ Resume campaign</button><button data-focus data-action="export-campaign" class="button">Export</button><button data-focus data-action="delete-campaign" class="button danger">Delete</button></div></aside></div>)";
  return out.str();
}
