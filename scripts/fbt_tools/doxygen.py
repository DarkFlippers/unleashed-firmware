from SCons.Script import Action, Builder


def exists(env):
    return True


def DoxyBuild(env, target, source, doxy_env_variables=None):
    if doxy_env_variables:
        doxy_env = env.Clone()
        doxy_env.Append(ENV=doxy_env_variables)
    else:
        doxy_env = env

    return doxy_env._DoxyBuilder(target, source)


def generate(env):
    if not env["VERBOSE"]:
        env.SetDefault(
            DOXYGENCOMSTR="\tDOXY\t${TARGET}",
        )

    env.SetDefault(
        DOXYGEN="doxygen",
    )

    env.AddMethod(DoxyBuild)
    env.Append(
        BUILDERS={
            "_DoxyBuilder": Builder(
                action=[
                    Action(
                        [["$DOXYGEN", "$SOURCE"]],
                        "$DOXYGENCOMSTR",
                    ),
                ],
            )
        }
    )
