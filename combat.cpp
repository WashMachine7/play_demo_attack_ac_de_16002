#include "combat.hpp"

namespace hit_window_lab {

bool HitResolver::start(int instance, int open_tick, int close_tick) {
    if (instance <= instance_ || open_tick < 0 || close_tick <= open_tick) return false;
    instance_ = instance;
    open_tick_ = open_tick;
    close_tick_ = close_tick;
    interrupted_ = false;
    for (bool& landed : landed_) landed = false;
    return true;
}

bool HitResolver::interrupt(int instance) {
    if (instance_ == 0 || instance != instance_ || interrupted_) return false;
    interrupted_ = true;
    return true;
}

Result HitResolver::evaluate(int instance, int tick, int target, Defense defense) {
    if (target < 0 || target >= kTargets) return Result::None;
    if (instance_ == 0 || instance != instance_ || interrupted_) return Result::Interrupted;
    if (tick < open_tick_ || tick >= close_tick_) return Result::Outside;
    if (landed_[target]) return Result::Duplicate;
    if (hp_[target] == 0) return Result::None;

    // Defense outcomes leave this target eligible for later contact in the same window.
    switch (defense) {
        case Defense::Invulnerable: return Result::Invulnerable;
        case Defense::Guard: return Result::Blocked;
        case Defense::Parry: return Result::Parried;
        case Defense::Open: break;
        default: return Result::None;
    }

    landed_[target] = true;
    hp_[target] = hp_[target] <= kDamage ? 0 : hp_[target] - kDamage;
    ++commits_;
    return Result::Hit;
}

int HitResolver::hp(int target) const {
    return target >= 0 && target < kTargets ? hp_[target] : -1;
}

bool sweep_circle(double ax, double ay, double bx, double by,
                  double cx, double cy, double combined_radius) {
    if (combined_radius < 0.0) return false;
    const double dx = bx - ax;
    const double dy = by - ay;
    const double length_squared = dx * dx + dy * dy;
    double t = length_squared > 0.0 ? ((cx - ax) * dx + (cy - ay) * dy) / length_squared : 0.0;
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    const double ox = ax + t * dx - cx;
    const double oy = ay + t * dy - cy;
    return ox * ox + oy * oy <= combined_radius * combined_radius;
}

}

namespace {
hit_window_lab::HitResolver match;
}

extern "C" void reset_match() { match = {}; }

extern "C" int start_attack(int instance, int open_tick, int close_tick) {
    return match.start(instance, open_tick, close_tick);
}

extern "C" int interrupt_attack(int instance) { return match.interrupt(instance); }

extern "C" int resolve_candidate(int instance, int tick, int target, int defense) {
    return static_cast<int>(match.evaluate(instance, tick, target,
                                          static_cast<hit_window_lab::Defense>(defense)));
}

extern "C" int target_hp(int target) { return match.hp(target); }

extern "C" int damage_commits() { return match.commits(); }

extern "C" int sweep_contact(double ax, double ay, double bx, double by,
                              double cx, double cy, double combined_radius) {
    return hit_window_lab::sweep_circle(ax, ay, bx, by, cx, cy, combined_radius);
}
