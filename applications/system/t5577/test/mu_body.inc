// Which frames carry a password, which is asked before and after the order for different
// reasons: every write lands on the chip as it was found, because block 0 goes back last, while
// the read back that follows lands on the chip the order has just made.
MU_TEST(test_lfrfid_t5577_restore_password) {
    const uint32_t config_protected = LFRFID_T5577_CONFIG_OPEN | LFRFID_T5577_PWD;
    bool with_pass = false;
    uint32_t password = 0;

    // Composed rather than read: protection is turned on with a word the chip never asked for,
    // so has_password is false and every write has to go out plain - a passworded frame would be
    // taken for a write by the open tag it is addressed to.
    T5577Dump composed = {0};
    t5577_dump_set(&composed, 0, 1, 0xFF80C000);
    t5577_dump_set(&composed, 0, 2, 0x060A4AF4);
    t5577_dump_set(&composed, 0, 6, 0xFFAD2D00);
    t5577_dump_set(&composed, 0, LFRFID_T5577_PASSWORD_BLOCK, 0xFFA94A50);
    t5577_dump_set(&composed, 0, 0, config_protected);

    mu_check(!composed.has_password);

    for(uint8_t i = 0; i < T5577_RESTORE_BLOCKS; i++) {
        with_pass = true; // poisoned, so a path that never writes it fails here
        t5577_restore_password(&composed, i, &with_pass);
        mu_check(!with_pass);
    }

    // The one frame sent after block 0, by which point the chip does ask - with the word block 7
    // was just given, not the one the dump was opened with.
    with_pass = false;
    password = t5577_restore_password(&composed, T5577_RESTORE_BLOCKS, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xFFA94A50, password);

    // Read off a protected chip and written back to it: asking throughout, and the word changes
    // under the order the moment block 7 goes back.
    T5577Dump protected_dump = {0};
    t5577_dump_set(&protected_dump, 0, 1, 0x11111111);
    t5577_dump_set(&protected_dump, 0, LFRFID_T5577_PASSWORD_BLOCK, 0xB0B0B0B0);
    t5577_dump_set(&protected_dump, 0, 0, config_protected);
    t5577_dump_set_password(&protected_dump, 0xA1B2C3D4);

    with_pass = false;
    password = t5577_restore_password(&protected_dump, 0, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xA1B2C3D4, password);

    // Block 7's place comes from the order rather than being written down here, for the reason
    // the function derives it that way: the two must not be able to drift apart.
    uint8_t pass_index = T5577_RESTORE_BLOCKS;
    for(uint8_t i = 0; i < T5577_RESTORE_BLOCKS; i++) {
        const T5577Target target = t5577_restore_target(i);
        if(target.page == 0 && target.block == LFRFID_T5577_PASSWORD_BLOCK) pass_index = i;
    }
    mu_check(pass_index < T5577_RESTORE_BLOCKS - 1);

    // The write of block 7 is still addressed with the word the chip holds, and every frame after
    // it with the word that block now has. Asserted on the writes and not only on the read back:
    // a switch applied to the last frame alone satisfies everything else in this test, while on a
    // chip it would leave every write after block 7 addressed wrongly and silently refused.
    with_pass = false;
    password = t5577_restore_password(&protected_dump, pass_index, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xA1B2C3D4, password);

    with_pass = false;
    password = t5577_restore_password(&protected_dump, pass_index + 1, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xB0B0B0B0, password);

    // Block 0's own write, the frame that writes the configuration.
    with_pass = false;
    password = t5577_restore_password(&protected_dump, T5577_RESTORE_BLOCKS - 1, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xB0B0B0B0, password);

    with_pass = false;
    password = t5577_restore_password(&protected_dump, T5577_RESTORE_BLOCKS, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xB0B0B0B0, password);

    // Clearing a password: an open configuration written onto a chip that wanted one. The writes
    // still carry it, and the read back must not - by then the chip has stopped asking, and that
    // frame would be a write of `password << 1` into block 0 with bit 31 on its lock bit.
    T5577Dump clearing = {0};
    t5577_dump_set(&clearing, 0, 1, 0x11111111);
    t5577_dump_set(&clearing, 0, 0, LFRFID_T5577_CONFIG_OPEN);
    t5577_dump_set_password(&clearing, 0xA1B2C3D4);

    with_pass = false;
    password = t5577_restore_password(&clearing, 0, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xA1B2C3D4, password);

    with_pass = true;
    t5577_restore_password(&clearing, T5577_RESTORE_BLOCKS, &with_pass);
    mu_check(!with_pass);

    // No block 0 in the dump, so the order never changes what the chip asks for and the read
    // back is addressed exactly as the writes were.
    T5577Dump headless = {0};
    t5577_dump_set(&headless, 0, 1, 0x11111111);
    t5577_dump_set_password(&headless, 0xA1B2C3D4);

    mu_check(!t5577_restore_wanted(&headless, T5577_RESTORE_BLOCKS - 1));

    with_pass = false;
    password = t5577_restore_password(&headless, T5577_RESTORE_BLOCKS, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xA1B2C3D4, password);

    // A protected configuration with no password anywhere: nothing to send, so the read back
    // goes out plain and simply fails rather than guessing with zero.
    T5577Dump unverifiable = {0};
    t5577_dump_set(&unverifiable, 0, 1, 0x11111111);
    t5577_dump_set(&unverifiable, 0, 0, config_protected);

    with_pass = true;
    t5577_restore_password(&unverifiable, T5577_RESTORE_BLOCKS, &with_pass);
    mu_check(!with_pass);

    // Block 7 went back, so a word is known from the order rather than from the dump - but the
    // configuration written is open, so the chip has stopped asking by the time of the read back.
    T5577Dump opened = {0};
    t5577_dump_set(&opened, 0, 1, 0x11111111);
    t5577_dump_set(&opened, 0, LFRFID_T5577_PASSWORD_BLOCK, 0xB0B0B0B0);
    t5577_dump_set(&opened, 0, 0, LFRFID_T5577_CONFIG_OPEN);

    with_pass = true;
    t5577_restore_password(&opened, T5577_RESTORE_BLOCKS, &with_pass);
    mu_check(!with_pass);

    // AOR protects reads without the password bit being set: such a chip says nothing at all
    // until a password frame wakes it, so the read back has to carry one here too.
    T5577Dump aor = {0};
    t5577_dump_set(&aor, 0, 1, 0x11111111);
    t5577_dump_set(&aor, 0, LFRFID_T5577_PASSWORD_BLOCK, 0xFFA94A50);
    t5577_dump_set(&aor, 0, 0, LFRFID_T5577_CONFIG_OPEN | LFRFID_T5577_AOR);

    with_pass = false;
    password = t5577_restore_password(&aor, T5577_RESTORE_BLOCKS, &with_pass);
    mu_check(with_pass);
    mu_assert_int_eq(0xFFA94A50, password);
}

