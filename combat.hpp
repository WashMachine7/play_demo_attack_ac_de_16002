#pragma once

namespace hit_window_lab {

constexpr int kTargets = 2;
constexpr int kDamage = 25;

enum class Defense : int { Open, Invulnerable, Guard, Parry };
enum class Result : int { None, Outside, Interrupted, Invulnerable, Blocked, Parried, Duplicate, Hit };

class HitResolver {
public:
    bool start(int instance, int open_tick, int close_tick);
    bool interrupt(int instance);
    Result evaluate(int instance, int tick, int target, Defense defense);
    int hp(int target) const;
    int commits() const { return commits_; }

private:
    int instance_ = 0;
    int open_tick_ = 0;
    int close_tick_ = 0;
    bool interrupted_ = false;
    bool landed_[kTargets]{};
    int hp_[kTargets]{100, 100};
    int commits_ = 0;
};

bool sweep_circle(double ax, double ay, double bx, double by,
                  double cx, double cy, double combined_radius);

}

extern "C" {
void reset_match();
int start_attack(int instance, int open_tick, int close_tick);
int interrupt_attack(int instance);
int resolve_candidate(int instance, int tick, int target, int defense);
int target_hp(int target);
int damage_commits();
int sweep_contact(double ax, double ay, double bx, double by,
                  double cx, double cy, double combined_radius);
}
