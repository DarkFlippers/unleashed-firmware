import json


class HardwareTargetLoader:
    def __init__(self, env, target_scons_dir, target_id):
        self.env = env
        self.target_scons_dir = target_scons_dir
        self.target_dir = self._getTargetDir(target_id)
        # self.target_id = target_id
        self.layered_target_dirs = []

        self.include_paths = []
        self.sdk_header_paths = []
        self.startup_script = None
        self.linker_script_flash = None
        self.linker_script_ram = None
        self.linker_script_app = None
        self.sdk_symbols = None
        self.linker_dependencies = []
        self.excluded_sources = []
        self.excluded_headers = []
        self.excluded_modules = []
        self._processTargetDefinitions(target_id)

    def _getTargetDir(self, target_id):
        return self.target_scons_dir.Dir(f"f{target_id}")

    def _loadDescription(self, target_id):
        target_json_file = self._getTargetDir(target_id).File("target.json")
        if not target_json_file.exists():
            raise Exception(f"Target file {target_json_file} does not exist")
        with open(target_json_file.get_abspath(), "r") as f:
            vals = json.load(f)
            return vals

    def _processTargetDefinitions(self, target_id):
        self.layered_target_dirs.append(f"targets/f{target_id}")

        config = self._loadDescription(target_id)

        for path_list in ("include_paths", "sdk_header_paths"):
            getattr(self, path_list).extend(
                f"#/firmware/targets/f{target_id}/{p}"
                for p in config.get(path_list, [])
            )

        self.excluded_sources.extend(config.get("excluded_sources", []))
        self.excluded_headers.extend(config.get("excluded_headers", []))
        self.excluded_modules.extend(config.get("excluded_modules", []))

        file_attrs = (
            # (name, use_src_node)
            ("startup_script", False),
            ("linker_script_flash", True),
            ("linker_script_ram", True),
            ("linker_script_app", True),
            ("sdk_symbols", True),
        )

        for attr_name, use_src_node in file_attrs:
            if (val := config.get(attr_name)) and not getattr(self, attr_name):
                node = self.env.File(f"firmware/targets/f{target_id}/{val}")
                if use_src_node:
                    node = node.srcnode()
                setattr(self, attr_name, node)

        for attr_name in ("linker_dependencies",):
            if (val := config.get(attr_name)) and not getattr(self, attr_name):
                setattr(self, attr_name, val)

        if inherited_target := config.get("inherit", None):
            self._processTargetDefinitions(inherited_target)

    def gatherSources(self):
        sources = [self.startup_script]
        seen_filenames = set(self.excluded_sources)
        # print("Layers: ", self.layered_target_dirs)
        for target_dir in self.layered_target_dirs:
            accepted_sources = list(
                filter(
                    lambda f: f.name not in seen_filenames,
                    self.env.GlobRecursive("*.c", target_dir),
                )
            )
            seen_filenames.update(f.name for f in accepted_sources)
            sources.extend(accepted_sources)
        # print(f"Found {len(sources)} sources: {list(f.name for f in sources)}")
        return sources

    def gatherSdkHeaders(self):
        sdk_headers = []
        seen_sdk_headers = set(self.excluded_headers)
        for sdk_path in self.sdk_header_paths:
            # dirty, but fast - exclude headers from overlayed targets by name
            # proper way would be to use relative paths, but names will do for now
            for header in self.env.GlobRecursive("*.h", sdk_path, "*_i.h"):
                if header.name not in seen_sdk_headers:
                    seen_sdk_headers.add(header.name)
                    sdk_headers.append(header)
        return sdk_headers


def ConfigureForTarget(env, target_id):
    target_loader = HardwareTargetLoader(env, env.Dir("#/firmware/targets"), target_id)
    env.Replace(
        TARGET_CFG=target_loader,
        SDK_DEFINITION=target_loader.sdk_symbols,
        SKIP_MODULES=target_loader.excluded_modules,
    )

    env.Append(
        CPPPATH=target_loader.include_paths,
        SDK_HEADERS=target_loader.gatherSdkHeaders(),
    )


def ApplyLibFlags(env):
    flags_to_apply = env["FW_LIB_OPTS"].get(
        env.get("FW_LIB_NAME"),
        env["FW_LIB_OPTS"]["Default"],
    )
    # print("Flags for ", env.get("FW_LIB_NAME", "Default"), flags_to_apply)
    env.MergeFlags(flags_to_apply)


def generate(env):
    env.AddMethod(ConfigureForTarget)
    env.AddMethod(ApplyLibFlags)


def exists(env):
    return True
