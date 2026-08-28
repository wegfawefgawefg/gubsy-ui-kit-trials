#pragma once

#include "model.hpp"

class ViewBuilder {
  public:
    ViewBuilder(std::string id, int width, int height);

    float scale() const;
    bool compact() const;
    bool phone() const;
    glayout::ContainerKind split() const;
    gview::View finish();
    glayout::GraphNode& layout(std::string_view id);
    gview::NodeSpec& spec(std::string_view id);

    void container(std::string_view parent, std::string id, glayout::ContainerKind kind,
                   glayout::Length width, glayout::Length height, float gap = 0.0f,
                   glayout::Insets padding = {});
    void label(std::string_view parent, std::string id, std::string text, float height,
               float size = 16.0f, gview::TextAlign align = gview::TextAlign::Start);
    void button(std::string_view parent, std::string id, std::string text, std::string action,
                std::string group, float height = 48.0f,
                gview::ActivationPolicy activation = gview::ActivationPolicy::Manual);
    void toggle(std::string_view parent, std::string id, std::string text, std::string binding,
                std::string group, float height = 56.0f);
    void slider(std::string_view parent, std::string id, std::string text, std::string binding,
                std::string group, double minimum, double maximum, double step,
                float height = 62.0f);
    void select(std::string_view parent, std::string id, std::string text, std::string binding,
                std::vector<std::string> options, std::string group, float height = 56.0f);
    void text_input(std::string_view parent, std::string id, std::string placeholder,
                    std::string binding, std::string group, float height = 48.0f);
    void image(std::string_view parent, std::string id, std::string asset, float height);
    void focus_group(std::string id, std::string entry, std::string owner = {});
    void edge(std::string from, gview::NavAction action, std::string to);

  private:
    gview::NodeSpec base_spec(std::string id) const;
    void append(std::string_view parent, glayout::GraphNode node, gview::NodeSpec spec);

    gview::View view_;
    float scale_ = 1.0f;
    int width_ = 1280;
};

gview::View build_shell_view(const TrialModel& model, int width, int height);
void build_play(ViewBuilder& ui, const TrialModel& model, std::string_view content);
void build_players(ViewBuilder& ui, const TrialModel& model, std::string_view content);
void build_settings(ViewBuilder& ui, const TrialModel& model, std::string_view content);
void build_controls(ViewBuilder& ui, const TrialModel& model, std::string_view content);
void build_progress(ViewBuilder& ui, const TrialModel& model, std::string_view content);
void build_mods(ViewBuilder& ui, const TrialModel& model, std::string_view content);
