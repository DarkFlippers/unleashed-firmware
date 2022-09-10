# Flipper Application Manifests (.fam)

```
App(
    appid="example_app", => App id, used in fbt app lists only, like applications\meta
    name="My Plugin", => App name in menu
    apptype=FlipperAppType.PLUGIN, => App type APP / PLUGIN / GAME (or service)
    entry_point="my_example_app", => App entry point / main function
    cdefines=["APP_MYEXAMPLE"], => C style define that will be used in generated file
    requires=[
        "gui",
        "dialogs",
    ], => Requirements (other app id's that required for this app)
    stack_size=2 * 1024, => Memory stack size
    order=60, => App order in menu
)
```