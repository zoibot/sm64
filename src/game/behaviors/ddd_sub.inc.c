// ddd_sub.c.inc

void bhv_bowsers_sub_loop(void) {
    if (gCurrActNum != 1) {
        mark_object_for_deletion(o);
    }
}
