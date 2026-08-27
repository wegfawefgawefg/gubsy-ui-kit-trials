#include "trial_host.hpp"
#include "trial_options.hpp"

int main(int argc, char** argv) {
    TrialOptions options;
    if (!parse_trial_options(argc, argv, options))
        return 2;
    TrialHost host{options};
    return host.run();
}
