#ifndef KF_COMBAT_H
#define KF_COMBAT_H

/* Component order shared by actor attacks/defenses and weapon records.
 * Actor attacks carry only the three physical components. */
enum {
    KF_COMBAT_COMPONENT_CUTTING = 0,
    KF_COMBAT_COMPONENT_STRIKING = 1,
    KF_COMBAT_COMPONENT_PIERCING = 2,
    KF_COMBAT_PHYSICAL_COMPONENT_COUNT = 3,
    KF_COMBAT_COMPONENT_HOLY = 3,
    KF_COMBAT_COMPONENT_FIRE = 4,
    KF_COMBAT_COMPONENT_COUNT = 5
};

/* Actor and player component formulas accumulate damage in tenths of HP. */
enum {
    KF_DAMAGE_SUBUNITS_PER_HP = 10
};

#endif
