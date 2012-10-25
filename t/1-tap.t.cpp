

Testfile tap ("1-tap", [](){
    plan(12);
    pass("pass");
    todo(1, "Auto fail");
    fail("fail");
    ok(true, "ok(true)");
    todo(1, "Auto fail");
    ok(false, "ok(false)");
    is(1, 1, "is(1, 1)");
    todo(1, "Auto fail");
    is(0, 1, "is(0, 1)");
    todo(1, "Auto fail");
    isnt(1, 1, "is(1, 1)");
    isnt(0, 1, "is(0, 1)");
    within(0.0, 1.0, 2.0, "within(0.0, 1.0, 2.0)");
    todo(1, "Auto fail");
    within(0.0, 1.0, 0.5, "within(0.0, 1.0, 0.5)");
    about(1.001, 1.0, "about(1.001, 1.0)");
    todo(1, "Auto fail");
    about(0.0, 1.0, "about(0.0, 1.0)");
});

